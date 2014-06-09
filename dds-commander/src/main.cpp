// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// STD
#include <iostream>
// BOOST
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/options_description.hpp>
// API
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
// DDS
#include "version.h"
#include "Options.h"
#include "Process.h"
#include "ErrorCode.h"
#include "CommanderServer.h"
#include "BOOSTHelper.h"
#include "Res.h"
#include "UserDefaults.h"

using namespace std;
using namespace MiscCommon;
namespace bpo = boost::program_options;
using namespace dds::commander;

void PrintVersion()
{
    cout << PROJECT_NAME << " v" << PROJECT_VERSION_STRING << "\n"
         << "DDS configuration"
         << " v" << USER_DEFAULTS_CFG_VERSION << "\n" << g_cszReportBugsAddr << endl;
}

// Command line parser
bool ParseCmdLine(int _argc, char* _argv[], SOptions* _options) throw(exception)
{
    if (nullptr == _options)
        throw runtime_error("Internal error: options' container is empty.");

    // Generic options
    bpo::options_description options("dds-commander options");
    options.add_options()("help,h", "Produce help message");
    options.add_options()("version,v", "Version information");
    options.add_options()("start", "Start dds-commander daemon");
    options.add_options()("stop", "Stop dds-commander daemon");
    options.add_options()("status", "Query current status of dds-command daemon");

    //    options.add_options()("help,h", "Produce help message")("version,v", "Version information")("config,c", bpo::value<string>(), "A PoD configuration
    // file")(
    //        "mode,m", bpo::value<string>()->default_value("server"), "Agent's mode (use: server or worker)")("daemonize,d", "Run agent as a daemon")(
    //        "start", "Start agent daemon (default action)")("stop", "Stop agent daemon")("status", "Query current status of agent daemon")(
    //        "serverinfo", bpo::value<string>()->default_value("~/.PoD/etc/server_info.cfg"), "A server info file name")
    //        // This parameter is mostly used by SSH plug-in to allow
    //        // spawning several PROOF workers in one PoD session on a single worker node.
    //        // The parameter can only be used by pod-agent workers and only when in
    //        // the native PROOF connection.
    //        ("workers",
    //         bpo::value<unsigned int>(),
    //         "A number of PROOF workers to spawn. Used only by agents in a worker mode and only in the native PROOF connection.");
    //

    // Parsing command-line
    bpo::variables_map vm;
    bpo::store(bpo::parse_command_line(_argc, _argv, options), vm);
    bpo::notify(vm);

    if (vm.count("help") || vm.empty())
    {
        cout << options << endl;
        return false;
    }
    if (vm.count("version"))
    {
        PrintVersion();
        return false;
    }
    //
    //    if (!vm.count("config"))
    //    {
    //        cout << options << endl;
    //        throw runtime_error("You need to specify a PoD configuration file at least.");
    //    }
    //
    //    {
    //        PoD::CPoDUserDefaults user_defaults;
    //        user_defaults.init(vm["config"].as<string>());
    //        _Options->m_podOptions = user_defaults.getOptions();
    //    }
    //

    MiscCommon::BOOSTHelper::conflicting_options(vm, "start", "stop");
    MiscCommon::BOOSTHelper::conflicting_options(vm, "start", "status");
    MiscCommon::BOOSTHelper::conflicting_options(vm, "stop", "status");
    //    boost_hlp::option_dependency(vm, "start", "daemonize");

    if (vm.count("start"))
        _options->m_Command = SOptions_t::Start;
    if (vm.count("stop"))
        _options->m_Command = SOptions_t::Stop;
    if (vm.count("status"))
        _options->m_Command = SOptions_t::Status;

    return true;
}

int main(int argc, char* argv[])
{
    // Command line parser
    SOptions_t options;
    try
    {
        if (!ParseCmdLine(argc, argv, &options))
            return EXIT_SUCCESS;
    }
    catch (exception& e)
    {
        // TODO: Log me!
        cerr << e.what() << endl;
        return EXIT_FAILURE;
    }

    /*    // Normalizing paths of common options
        PoD::SCommonOptions_t& common = (Server == options.m_agentMode) ? Options.m_podOptions.m_server.m_common : Options.m_podOptions.m_worker.m_common;
        // resolving user's home dir from (~/ or $HOME, if present)
        smart_path(&common.m_workDir);
        // We need to be sure that there is "/" always at the end of the path
        smart_append<string>(&common.m_workDir, '/');
        smart_path(&common.m_logFileDir);
        smart_append<string>(&common.m_logFileDir, '/');

        // pidfile name
        string pidfile_name(common.m_workDir);
        pidfile_name += "pod-agent.pid";
    */

    string pidfile_name("pidfile.txt"); // ONLY TEMP

    // Checking for "status" option
    if (options.m_Command == SOptions_t::Status)
    {
        pid_t pid = CPIDFile::GetPIDFromFile(pidfile_name);
        if (pid > 0 && IsProcessExist(pid))
        {
            cout << PROJECT_NAME << " process (" << pid << ") is running..." << endl;
        }
        else
        {
            cout << PROJECT_NAME << " is not running..." << endl;
        }

        return EXIT_SUCCESS;
    }

    // Checking for "stop" option
    if (SOptions_t::Stop == options.m_Command)
    {
        // TODO: make wait for the process here to check for errors
        const pid_t pid_to_kill = CPIDFile::GetPIDFromFile(pidfile_name);
        if (pid_to_kill > 0 && IsProcessExist(pid_to_kill))
        {
            cout << PROJECT_NAME << ": self exiting (" << pid_to_kill << ")..." << endl;
            // TODO: Maybe we need more validations of the process before
            // sending a signal. We don't want to kill someone else.
            kill(pid_to_kill, SIGTERM);

            // Waiting for the process to finish
            size_t iter(0);
            const size_t max_iter = 30;
            while (iter <= max_iter)
            {
                if (!IsProcessExist(pid_to_kill))
                {
                    cout << endl;
                    break;
                }
                cout << ".";
                cout.flush();
                sleep(1); // sleeping for 1 second
                ++iter;
            }
            if (IsProcessExist(pid_to_kill))
                cerr << "FAILED to close the process." << endl;
        }

        return EXIT_SUCCESS;
    }

    // Checking for "start" option
    if (SOptions_t::Start == options.m_Command)
    {
        // process ID and Session ID
        pid_t pid;
        pid_t sid;

        // Fork off the parent process
        pid = ::fork();
        if (pid < 0)
            return EXIT_FAILURE;

        // If we got a good PID, then we can exit the parent process.
        if (pid > 0)
            return EXIT_SUCCESS;

        // Change the file mode mask
        ::umask(0);

        // Create a new SID for the child process
        sid = ::setsid();
        if (sid < 0) // TODO:  Log the failure
            return EXIT_FAILURE;

        // Main object - agent itself
        //   CPROOFAgent agent;

        try
        {
            CPIDFile pidfile(pidfile_name, ::getpid());

            // Daemon-specific initialization goes here
            // agent.setConfiguration(Options);

            //       if (options.m_bDaemonize)
            //       {
            // Change the current working directory
            // chdir("/") to ensure that our process doesn't keep any directory
            // in use. Failure to do this could make it so that an administrator
            // couldn't unmount a file system, because it was our current directory.
            if (::chdir("/") < 0) // TODO: Log the failure
                return EXIT_FAILURE;

            // Close out the standard file descriptors
            close(STDIN_FILENO);
            close(STDOUT_FILENO);
            close(STDERR_FILENO);

            // Establish new open descriptors for stdin, stdout, and stderr. Even if
            // we don't plan to use them, it is still a good idea to have them open.
            int fd = open("/dev/null", O_RDWR); // stdin - file handle 0.
            // stdout - file handle 1.
            if (dup(fd) < 0)
                throw MiscCommon::system_error("Error occurred while duplicating stdout descriptor");
            // stderr - file handle 2.
            if (dup(fd) < 0)
                throw MiscCommon::system_error("Error occurred while duplicating stderr descriptor");
            //     }

            // Starting Agent
            // agent.Start();

            CCommanderServer server;
            server.start();

            // Main loop
            //            while (1)
            //            {
            //                sleep(30); // wait 30 seconds
            //                cout << "running..." << endl;
            //            }
        }
        catch (exception& e)
        {
            //  agent.FaultLog(erError, e.what());
            return EXIT_FAILURE; // exitCode_GENERAL_ERROR;
        }
        catch (...)
        {
            //  string errMsg("Unexpected Exception occurred.");
            //  agent.FaultLog(erXMLInit, errMsg);
            return EXIT_FAILURE; // exitCode_GENERAL_ERROR;
        }
    }

    //   return agent.getExitCode();
    return EXIT_SUCCESS;
}
