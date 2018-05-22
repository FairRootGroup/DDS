// Copyright 2016 GSI, Inc. All rights reserved.
//
//
//
// BOOST
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-local-typedef"
#include <boost/program_options/parsers.hpp>
#pragma clang diagnostic pop
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
// STD
#include <chrono>
#include <list>
#include <stdexcept>
#include <thread>
// DDS
#include "Process.h"
#include "SysHelper.h"
#include "UserDefaults.h"
#include "dds_intercom.h"
#include "logEngine.h"
#include "version.h"

using namespace std;
using namespace dds;
using namespace dds::intercom_api;
using namespace dds::user_defaults_api;
using namespace dds::pipe_log_engine;
using namespace MiscCommon;
namespace bpo = boost::program_options;
namespace fs = boost::filesystem;

//=============================================================================
// file is located in the DDS server working dir
const LPCSTR g_pipeName = ".dds_slurm_pipe";
// file is located in the RMS sandbox directory
const LPCSTR g_jobIDFile = ".dds_slurm_jobid";
//=============================================================================

// Command line parser
bool parseCmdLine(int _argc, char* _argv[], bpo::variables_map* _vm)
{
    // Generic options
    bpo::options_description options("Options");
    options.add_options()("session", bpo::value<std::string>(), "DDS Session ID");
    options.add_options()("id", bpo::value<std::string>(), "DDS submission ID");
    options.add_options()("path", bpo::value<std::string>(), "Plug-in's directory path");

    // Parsing command-line
    bpo::variables_map vm;
    bpo::store(bpo::command_line_parser(_argc, _argv).options(options).run(), vm);
    bpo::notify(vm);

    if (!vm.count("id") || vm["id"].as<string>().empty())
    {
        cout << options;
        return false;
    }
    if (!vm.count("path") || vm["path"].as<string>().empty())
    {
        cout << options;
        return false;
    }

    if (_vm)
        _vm->swap(vm);

    return true;
}
//=============================================================================
int main(int argc, char* argv[])
{
    CUserDefaults::instance(); // Initialize user defaults
    bpo::variables_map vm;
    try
    {
        if (!parseCmdLine(argc, argv, &vm))
            return 1;
    }
    catch (exception& e)
    {
        return 1;
    }

    // Session ID
    boost::uuids::uuid sid(boost::uuids::nil_uuid());
    if (vm.count("session"))
        sid = boost::uuids::string_generator()(vm["session"].as<std::string>());

    // Reinit UserDefaults and Log with new session ID
    CUserDefaults::instance().reinit(sid, CUserDefaults::instance().currentUDFile());

    // Init communication with DDS commander server
    CRMSPluginProtocol proto(vm["id"].as<string>());

    // IMPORTANT! Should be create only after DDS log is inited
    CLogEngine slog(true);

    try
    {
        // init pipe log engine to get log messages from the child scripts
        string pipeName(CUserDefaults::instance().getOptions().m_server.m_workDir);
        smart_append(&pipeName, '/');
        pipeName += g_pipeName;

        // instruct pipe log engine to reflect output on user's output as well
        slog.start(pipeName, [&proto](const string& _msg) { proto.sendMessage(EMsgSeverity::info, _msg); });

        // Subscribe on onSubmit command
        proto.onSubmit([&proto, &vm](const SSubmit& _submit) {
            try
            {
                // location of the plug-in files
                fs::path pathPluginDir(vm["path"].as<string>());
                // Generate SLURM job script
                fs::path pathJobScriptSourceFilepath(pathPluginDir);
                pathJobScriptSourceFilepath /= "job.slurm.in";
                // Checxk that the source of the script exists
                if (!fs::exists(pathJobScriptSourceFilepath))
                    throw runtime_error("Can't find source of the job script. Plug-in's installation is inconsistent.");

                fs::ifstream f_src(pathJobScriptSourceFilepath);
                stringstream ssSrcScript;
                ssSrcScript << f_src.rdbuf();
                string sSrcScript(ssSrcScript.str());

                proto.sendMessage(dds::intercom_api::EMsgSeverity::info, "Generating SLURM Job script...");
                // Replace #DDS_NEED_ARRAY
                if (_submit.m_nInstances > 0)
                    boost::replace_all(
                        sSrcScript, "#DDS_NEED_ARRAY", "#SBATCH --array=1-" + to_string(_submit.m_nInstances));

                // Replace %DDS_JOB_ROOT_WRK_DIR%
                std::time_t now = chrono::system_clock::to_time_t(chrono::system_clock::now());
                struct std::tm* ptm = std::localtime(&now);
                char buffer[20];
                std::strftime(buffer, 20, "%Y-%m-%d-%H-%M-%S", ptm);
                string sSandboxDir(smart_path(CUserDefaults::instance().getValueForKey("server.sandbox_dir")));
                fs::path pathJobWrkDir(sSandboxDir);
                pathJobWrkDir /= buffer;
                boost::replace_all(sSrcScript, "%DDS_JOB_ROOT_WRK_DIR%", pathJobWrkDir.string());
                // create ROOT wrk dir for jobs
                fs::create_directories(pathJobWrkDir);

                // Replace #DDS_USER_OPTIONS
                fs::path pathUserOptions(_submit.m_cfgFilePath);
                if (fs::exists(pathUserOptions))
                {
                    fs::ifstream f_userOptions(pathUserOptions);
                    string sUserOptions((istreambuf_iterator<char>(f_userOptions)), istreambuf_iterator<char>());
                    boost::replace_all(sSrcScript, "#DDS_USER_OPTIONS", sUserOptions);
                }

                // Replace %DDS_SCOUT%
                string sScoutScriptPath(CUserDefaults::instance().getWrkScriptPath());
                boost::replace_all(sSrcScript, "%DDS_SCOUT%", sScoutScriptPath);

                // Generate new job script
                // It is going to be saved in the sandbox dir
                fs::path pathJobScriptFilepath(sSandboxDir);
                pathJobScriptFilepath /= "job.slurm";
                fs::ofstream f_dest(pathJobScriptFilepath);
                f_dest << sSrcScript;
                f_dest.flush();
                f_dest.close();

                // Execute the submitter script
                fs::path pathSLURMScript(pathPluginDir);
                pathSLURMScript /= "dds-submit-slurm-worker";
                stringstream cmd;
                cmd << "$DDS_LOCATION/bin/dds-daemonize " << pathPluginDir.string()
                    << " /bin/bash -c \"unset DDS_LOG_LOCATION; " << pathSLURMScript.string() << "\"";

                proto.sendMessage(dds::intercom_api::EMsgSeverity::info, "Preparing job submission...");
                string output;
                pid_t exitCode = execute(cmd.str(), chrono::seconds(30), &output);

                // In case of error there can be a bash.out.log created, let's check it's content
                fs::path pathBashlog(pathPluginDir);
                pathBashlog /= "bash.out.log";

                if (exitCode == 0)
                {
                    bool started(false);
                    fs::path pathJobIDFile(smart_path(CUserDefaults::instance().getValueForKey("server.work_dir")));
                    pathJobIDFile /= g_jobIDFile;

                    // Give the job 2 minutes to submit
                    for (int t = 0; t < 240; ++t)
                    {
                        if (fs::exists(pathJobIDFile))
                        {
                            started = true;
                            proto.sendMessage(EMsgSeverity::info, "DDS agents have been submitted");
                            // remove jobid file
                            fs::remove(pathJobIDFile);
                            break;
                        }
                        // give bash 20 sec to write the log file if it fails to start our script
                        else if (t > 40 && fs::exists(pathBashlog))
                        {
                            // if bash log exists, then we have a problem starting the submit script
                            started = false;
                            break;
                        }
                        else
                        {
                            this_thread::sleep_for(chrono::milliseconds(500));
                        }
                    }
                    if (!started)
                    {
                        if (fs::exists(pathBashlog))
                        {
                            // read the file
                            fs::ifstream f_bashlog(pathBashlog);
                            stringstream ssErrLog;
                            ssErrLog << f_bashlog.rdbuf();
                            string sErrLog(ssErrLog.str());

                            // we don't need it anymore, remove the file
                            fs::remove(pathBashlog);

                            proto.sendMessage(EMsgSeverity::info, sErrLog);
                        }

                        proto.sendMessage(EMsgSeverity::error, "Failed to deploy agents");
                    }
                }
                else
                {
                    proto.sendMessage(EMsgSeverity::error, "Failed to deploy agents: can't start worker script");
                }

                proto.stop();
            }
            catch (exception& e)
            {
                proto.sendMessage(dds::intercom_api::EMsgSeverity::error, e.what());
            }
        });

        // Let DDS commander know that we are online and start listening for notifications.
        proto.start();
    }
    catch (exception& e)
    {
        proto.sendMessage(dds::intercom_api::EMsgSeverity::error, e.what());
        return 1;
    }

    return 0;
}
