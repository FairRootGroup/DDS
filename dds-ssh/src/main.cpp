// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
// BOOST
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/options_description.hpp>
// STD
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <list>
// DDS
#include "BOOSTHelper.h"
#include "SysHelper.h"
#include "DDSSysFiles.h"
#include "version.h"
#include "config.h"
#include "worker.h"
#include "Process.h"
#include "logEngine.h"
#include "local_types.h"
#include "UserDefaults.h"
#include "Logger.h"
#include "Res.h"

using namespace std;
using namespace dds;
using namespace MiscCommon;
namespace bpo = boost::program_options;
namespace boost_hlp = MiscCommon::BOOSTHelper;
//=============================================================================
const LPCSTR g_pipeName = ".dds_ssh_pipe";
typedef list<CWorker> workersList_t;
typedef CThreadPool<CWorker, ETaskType> threadPool_t;
//=============================================================================
void printVersion()
{
    LOG(log_stdout) << " v" << PROJECT_VERSION_STRING << "DDS configuration"
                    << " v" << USER_DEFAULTS_CFG_VERSION << "\n" << MiscCommon::g_cszReportBugsAddr;
}
//=============================================================================
enum ECommands
{
    cmd_unknown,
    cmd_submit,
    cmd_status,
    cmd_clean,
    cmd_fast_clean
};
ECommands getCommandByName(const string& _name)
{
    if ("submit" == _name)
        return cmd_submit;
    if ("status" == _name)
        return cmd_status;
    if ("clean" == _name)
        return cmd_clean;
    if ("fast-clean" == _name)
        return cmd_fast_clean;

    return cmd_unknown;
}
//=============================================================================
// Command line parser
bool parseCmdLine(int _Argc, char* _Argv[], bpo::variables_map* _vm)
{
    // Generic options
    bpo::options_description visible("Options");
    visible.add_options()("help,h", "Produce help message")("version,v", "Version information")(
        "command",
        bpo::value<string>(),
        "The command is a name of dds-ssh command."
        " Can be one of the following: submit, status, clean, fast-clean.\n"
        "For user's convenience it is allowed to call dds-ssh without \"--command\" option"
        " by just specifying the command name directly, like:\ndds-ssh submit or dds-ssh clean.\n\n"
        "Commands:\n"
        "   submit: \tSubmit workers\n"
        "   status: \tRequest status of the workers\n"
        "   clean: \tClean all workers\n"
        "   fast-clean: \tThe fast version of the clean procedure."
        " It shuts worker nodes down. It doesn't actually clean workers' directories.\n\n")(
        "config,c", bpo::value<string>(), "DDS's ssh plug-in configuration file")(
        "exec,e", bpo::value<string>(), "Execute a local shell script on the remote worker nodes")(
        "logs", "Download all log files from the worker nodes. Can be used only together with the clean option")(
        "for-worker",
        bpo::value<vector<string>>()->multitoken(),
        "Perform an action on defined worker nodes. (arg is a space separated list of WN names)"
        " Can only be used in connection with \"submit\", \"clean\", \"fast-clean\", \"exec\".")(
        "debug,d", "Verbose mode. Causes dds-ssh to print debugging messages about its progress")(
        "threads,t",
        bpo::value<size_t>()->default_value(5),
        "It defines a number of threads in dds-ssh's thread pool."
        " Min value is 1, max value is (Core*2)."
        " Default: 5");

    //...positional
    bpo::positional_options_description pd;
    pd.add("command", 1);

    // Parsing command-line
    bpo::variables_map vm;
    bpo::store(bpo::command_line_parser(_Argc, _Argv).options(visible).positional(pd).run(), vm);
    bpo::notify(vm);

    if (vm.count("help") || vm.empty())
    {
        LOG(log_stdout) << visible;
        return false;
    }
    if (vm.count("version"))
    {
        printVersion();
        return false;
    }

    // Command
    if (vm.count("command"))
    {
        if (getCommandByName(vm["command"].as<string>()) == cmd_unknown)
        {
            LOG(log_stdout) << " error: unknown command: " << vm["command"].as<string>() << "\n\n" << visible;
            return false;
        }
    }
    else
    {
        LOG(log_stdout) << ": Nothing to do\n\n" << visible;
        return false;
    }

    ECommands command = getCommandByName(vm["command"].as<string>());

    if (vm.count("logs") && cmd_clean != command && cmd_fast_clean != command)
        throw runtime_error("The option \"--logs\" can be used only together with \"--clean\" or \"--fast-clean\"");

    _vm->swap(vm);
    return true;
}
//=============================================================================
void repackPkg(string* _cmdOutput, bool _needInlineBashScript = false)
{
    // re-create the worker package if needed
    string out;
    string err;
    try
    {
        // invoking a new bash process can in some case overwrite env. vars
        // To be shure that our env is there, we call DDS_env.sh
        string cmd_env("$DDS_LOCATION/DDS_env.sh");
        smart_path(&cmd_env);
        string cmd("$DDS_LOCATION/bin/dds-prep-worker");
        if (_needInlineBashScript)
            cmd += " -i";
        smart_path(&cmd);
        string arg("source ");
        arg += cmd_env;
        arg += " ; ";
        arg += cmd;

        StringVector_t params;
        params.push_back("-c");
        params.push_back(arg);
        // 10 sec time-out for this command
        do_execv("/bin/bash", params, 10, &out, &err);
    }
    catch (exception& e)
    {
        stringstream ss;
        ss << "Can't create DDS worker package:\nSTDOUT:" << out << ";\n STDERR: " << err;
        throw runtime_error(ss.str());
    }
    if (_cmdOutput)
        *_cmdOutput = out;
}
//=============================================================================
int main(int argc, char* argv[])
{
    Logger::instance().init(); // Initialize log
    CUserDefaults::instance(); // Initialize user defaults

    vector<std::string> arguments(argv + 1, argv + argc);
    ostringstream ss;
    copy(arguments.begin(), arguments.end(), ostream_iterator<string>(ss, " "));
    LOG(info) << "Starting with arguments: " << ss.str();

    bpo::variables_map vm;
    try
    {
        if (!parseCmdLine(argc, argv, &vm))
            return 0;
    }
    catch (exception& e)
    {
        LOG(log_stderr) << ": " << e.what();
        return 1;
    }

    CLogEngine slog(vm.count("debug"));
    try
    {
        // Collect workers list
        string pipeName(CUserDefaults::instance().getOptions().m_server.m_workDir);
        smart_append(&pipeName, '/');
        pipeName += g_pipeName;
        slog.start(pipeName);

        string configFile;
        if (!vm.count("config"))
        {
            //   DDS::SDDSSSHOptions opt_file;
            //   opt_file.load(env.pod_sshCfgFile());
            //   configFile = opt_file.m_config;
        }
        else
        {
            configFile = vm["config"].as<string>();
            smart_path(&configFile);
        }

        if (!file_exists(configFile))
            throw runtime_error("DDS SSH config file doesn't exist: " + configFile);

        ifstream f(configFile.c_str());
        if (!f.is_open())
        {
            string msg("can't open configuration file \"");
            msg += vm["config"].as<string>();
            msg += "\"";
            throw runtime_error(msg);
        }

        ECommands command = getCommandByName(vm["command"].as<string>());

        // Check that DDS server is running
        if (cmd_submit == command)
        {
            try
            {
                string cmd("$DDS_LOCATION/bin/dds-server");
                smart_path(&cmd);
                StringVector_t params;
                params.push_back("status_with_code");
                string output;
                string errout;
                do_execv(cmd, params, 2, &output, &errout);
            }
            catch (exception& e)
            {
                throw runtime_error("DDS server is NOT running. Please, start DDS server first.");
            }

            // re-pack PoD worker package if needed
            // We will call repack once again if there is an inline bash script
            string cmdOutput;
            repackPkg(&cmdOutput);
            stringstream ss(cmdOutput);
            // send the output line by line to the log
            StringVector_t vec;
            std::copy(custom_istream_iterator<std::string>(ss),
                      custom_istream_iterator<std::string>(),
                      std::back_inserter(vec));
            StringVector_t::const_iterator iter = vec.begin();
            StringVector_t::const_iterator iter_end = vec.end();
            for (; iter != iter_end; ++iter)
                LOG(debug) << (*iter + '\n');
        }

        size_t wrkCount(0);
        bool dynWrk(false);
        SWNOptions options;
        options.m_debug = vm.count("debug");
        options.m_logs = vm.count("logs");
        options.m_fastClean = vm.count("fast-clean");
        if (vm.count("exec"))
            options.m_scriptName = vm["exec"].as<string>();

        workersList_t workers;
        string inlineShellScripCmds;
        {
            CConfig config;
            config.readFrom(f);
            inlineShellScripCmds = config.getBashEnvCmds();

            configRecords_t recs(config.getRecords());
            configRecords_t::const_iterator iter = recs.begin();
            configRecords_t::const_iterator iter_end = recs.end();
            for (; iter != iter_end; ++iter)
            {
                configRecord_t rec(*iter);
                CWorker wrk(rec, options);
                workers.push_back(wrk);

                if (0 == rec->m_nWorkers)
                    dynWrk = true; // user wants us to dynamicly decide on how many job slots to create

                wrkCount += rec->m_nWorkers;
            }
        }

        // Need to repack worker package
        // in order to insert a user defined shell script
        if (cmd_submit == command && !inlineShellScripCmds.empty())
        {
            LOG(debug) << "dds-ssh config contains an inline shell script. It will be injected it into wrk. package";
            string scriptFileName(CUserDefaults::instance().getWrkPkgDir());
            scriptFileName += "user_worker_env.sh";
            smart_path(&scriptFileName);

            ofstream f_script(scriptFileName.c_str());
            if (!f_script.is_open())
                throw runtime_error("Can't open for writing: " + scriptFileName);

            f_script << inlineShellScripCmds;
            f_script.close();

            // re-pack PoD worker package if needed
            string cmdOutput;
            repackPkg(&cmdOutput, !inlineShellScripCmds.empty());
            stringstream ss(cmdOutput);
            // send the output line by line to the log
            StringVector_t vec;
            std::copy(custom_istream_iterator<std::string>(ss),
                      custom_istream_iterator<std::string>(),
                      std::back_inserter(vec));
            StringVector_t::const_iterator iter = vec.begin();
            StringVector_t::const_iterator iter_end = vec.end();
            for (; iter != iter_end; ++iter)
                LOG(debug) << (*iter + '\n');
        }
        else if (cmd_submit == command)
        {
            //            stringstream ssWarning;
            //            ssWarning << "\n********************************************\n"
            //                      << "Warning! There is no inline environment script found in " << configFile << "\n"
            //                      << "Be advised, with SSH plug-in it is very often the case,\n"
            //                      << "that PoD can't start workers, because xproofd/ROOT is not\n"
            //                      << "in the PATH on worker nodes.\n"
            //                      << "If your PoD job fails, just after submission it shows DONE status, \n"
            //                      << "then you may want use inline environment script in your pod-ssh config file.\n"
            //                      << "See http://pod.gsi.de/doc/pro/pod-ssh.html for more information\n"
            //                      << "\n"
            //                      << "Usage of user_worker_env.sh in pod-ssh is deprecated.\n"
            //                      << "********************************************\n";
            //            slog(ssWarning.str());
        }

        // a number of threads in the thread-pool
        // size_t nThreads( env.getUD().m_server.m_agentThreads );
        size_t nThreads(vm["threads"].as<size_t>());
        // Protection for a number of threads
        if (nThreads <= 0 || nThreads > (getNCores() * 2))
        {
            LOG(debug) << "Warning: bad number of threads. The default will be used.";
            nThreads = 5;
        }
        // some control information
        LOG(debug) << "There are " << nThreads << " threads in the tread-pool.";
        LOG(debug) << "Number of DDS workers: " << workers.size();
        //        if (dynWrk)
        //            ss << "Number of PROOF workers: on some workers is dynamic, according to a number of CPU cores\n";
        //        else
        //            ss << "Number of PROOF workers: " << wrkCount << "\n";

        LOG(debug) << "Workers list:\n";

        // start thread-pool and push tasks into it
        threadPool_t threadPool(nThreads);

        // Did user requested some specific worker nodes
        StringVector_t defined_wns;
        if (vm.count("for-worker"))
            defined_wns = vm["for-worker"].as<vector<string>>();

        workersList_t::iterator iter = workers.begin();
        workersList_t::iterator iter_end = workers.end();
        for (; iter != iter_end; ++iter)
        {
            if (!defined_wns.empty())
            {
                StringVector_t::const_iterator found_id = find(defined_wns.begin(), defined_wns.end(), iter->getID());
                if (found_id == defined_wns.end())
                    continue;
            }

            ostringstream ss;
            iter->printInfo(ss);
            LOG(debug) << ss.str();

            // pash pre-tasks
            if (vm.count("exec"))
                threadPool.pushTask(*iter, task_exec);

            // push main tasks
            switch (command)
            {
                case cmd_clean:
                case cmd_fast_clean:
                    threadPool.pushTask(*iter, task_clean);
                    break;
                case cmd_status:
                    threadPool.pushTask(*iter, task_status);
                    break;
                case cmd_submit:
                    threadPool.pushTask(*iter, task_submit);
                    break;
                default:
                    throw runtime_error("Unknown command was specified.");
            }
        }
        threadPool.stop(true);

        // Check the status of all tasks Failed
        size_t badFailedCount = threadPool.tasksCount() - threadPool.successfulTasks();
        LOG(debug) << "\n*******************\n"
                   << "Successfully processed tasks: " << threadPool.successfulTasks() << '\n'
                   << "Failed tasks: " << badFailedCount << '\n' << "*******************";

        if (badFailedCount > 0 && !vm.count("debug"))
            LOG(error) << ("WARNING: some tasks have failed.");

        if (!vm.count("debug") && cmd_submit == command)
            LOG(info) << "DDS jobs have been submitted.";

        //        DDS::SDDDSSSHOptions opt_file;
        //        opt_file.m_config = configFile;
        //        opt_file.save(env.pod_sshCfgFile());
    }
    catch (exception& e)
    {
        LOG(log_stderr) << e.what();
        return 1;
    }

    return 0;
}
