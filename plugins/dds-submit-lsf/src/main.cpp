// Copyright 2016 GSI, Inc. All rights reserved.
//
//
//
// BOOST
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
// STD
#include <chrono>
#include <list>
#include <stdexcept>
#include <thread>
// DDS
#include "Intercom.h"
#include "Logger.h"
#include "MiscSetup.h"
#include "PipeLogEngine.h"
#include "Process.h"
#include "SysHelper.h"
#include "ToolsProtocol.h"
#include "UserDefaults.h"

using namespace std;
using namespace dds;
using namespace dds::intercom_api;
using namespace dds::user_defaults_api;
using namespace dds::pipe_log_engine;
using namespace dds::tools_api;
using namespace dds::misc;
;
namespace bpo = boost::program_options;
namespace fs = boost::filesystem;

//=============================================================================
// file is located in the DDS server working dir
const LPCSTR g_pipeName = ".dds_lsf_pipe";
// file is located in the RMS sandbox directory
const LPCSTR g_jobIDFile = ".dds_lsf_jobid";
//=============================================================================

std::string getLightweightValidationScript()
{
    return R"(
# DDS Lightweight Package Prerequisite Validation
echo "Validating DDS lightweight package prerequisites..."

# Function to check if a path exists and is accessible
check_path() {
    local path="$1"
    local description="$2"
    
    if [ -z "$path" ]; then
        echo "ERROR: $description is not set"
        return 1
    fi
    
    if [ ! -e "$path" ]; then
        echo "ERROR: $description does not exist: $path"
        return 1
    fi
    
    if [ ! -r "$path" ]; then
        echo "ERROR: $description is not readable: $path"
        return 1
    fi
    
    echo "OK: $description is valid: $path"
    return 0
}

# Check DDS_COMMANDER_BIN_LOCATION
if ! check_path "$DDS_COMMANDER_BIN_LOCATION" "DDS_COMMANDER_BIN_LOCATION"; then
    echo "FATAL: DDS lightweight package validation failed - missing commander binary location"
    echo "Please ensure DDS_COMMANDER_BIN_LOCATION is properly set in your environment"
    exit 1
fi

# Check DDS_COMMANDER_LIBS_LOCATION  
if ! check_path "$DDS_COMMANDER_LIBS_LOCATION" "DDS_COMMANDER_LIBS_LOCATION"; then
    echo "FATAL: DDS lightweight package validation failed - missing commander libraries location"
    echo "Please ensure DDS_COMMANDER_LIBS_LOCATION is properly set in your environment"
    exit 1
fi

echo "All DDS lightweight package prerequisites validated successfully"
)";
}

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
    bpo::variables_map vm;
    if (dds::misc::defaultExecSetup<bpo::variables_map>(argc, argv, &vm, &parseCmdLine) == EXIT_FAILURE)
        return EXIT_FAILURE;

    // Session ID
    boost::uuids::uuid sid(boost::uuids::nil_uuid());
    if (vm.count("session"))
        sid = boost::uuids::string_generator()(vm["session"].as<std::string>());

    if (dds::misc::defaultExecReinit(sid) == EXIT_FAILURE)
        return EXIT_FAILURE;

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
        proto.onSubmit(
            [&proto, &vm](const SSubmit& _submit)
            {
                try
                {
                    // location of the plug-in files
                    fs::path pathPluginDir(vm["path"].as<string>());
                    // Generate lsf job script
                    fs::path pathJobScriptSourceFilepath(pathPluginDir);
                    pathJobScriptSourceFilepath /= "job.lsf.in";
                    // Checxk that the source of the script exists
                    if (!fs::exists(pathJobScriptSourceFilepath))
                        throw runtime_error(
                            "Can't find source of the job script. Plug-in's installation is inconsistent.");

                    fs::ifstream f_src(pathJobScriptSourceFilepath);
                    stringstream ssSrcScript;
                    ssSrcScript << f_src.rdbuf();
                    string sSrcScript(ssSrcScript.str());

                    proto.sendMessage(dds::intercom_api::EMsgSeverity::info, "Generating lsf Job script...");

                    // Check if lightweight mode is enabled and log it
                    bool isLightweightMode = dds::tools_api::SSubmitRequestData::isFlagEnabled(
                        _submit.m_flags, dds::tools_api::SSubmitRequestData::ESubmitRequestFlags::enable_lightweight);
                    if (isLightweightMode)
                    {
                        proto.sendMessage(dds::intercom_api::EMsgSeverity::info,
                                          "Lightweight deployment mode: Workers will use existing DDS installation on "
                                          "compute nodes.");
                    }
                    // Replace #DDS_NEED_ARRAY
                    if (_submit.m_nInstances > 0)
                        boost::replace_all(sSrcScript, "#DDS_NEED_ARRAY", to_string(_submit.m_nInstances));

                    // Replace %DDS_JOB_ROOT_WRK_DIR%
                    std::time_t now = chrono::system_clock::to_time_t(chrono::system_clock::now());
                    struct std::tm* ptm = std::localtime(&now);
                    char buffer[20];
                    std::strftime(buffer, 20, "%Y-%m-%d-%H-%M-%S", ptm);
                    string sSandboxDir(smart_path(CUserDefaults::instance().getValueForKey("server.sandbox_dir")));
                    fs::path pathJobWrkDir(sSandboxDir);
                    pathJobWrkDir /= "lsf_jobs";
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

                    // Replace #DDS_LIGHTWEIGHT_VALIDATION
                    string sLightweightValidation = getLightweightValidationScript();
                    boost::replace_all(sSrcScript, "#DDS_LIGHTWEIGHT_VALIDATION", sLightweightValidation);

                    // Replace %DDS_SCOUT%
                    string sScoutScriptPath(CUserDefaults::instance().getWrkScriptPath(_submit.m_id));
                    boost::replace_all(sSrcScript, "%DDS_SCOUT%", sScoutScriptPath);

                    // Generate new job script
                    // It is going to be saved in the sandbox dir
                    fs::path pathJobScriptFilepath(sSandboxDir);
                    pathJobScriptFilepath /= "job.lsf";
                    fs::ofstream f_dest(pathJobScriptFilepath);
                    f_dest << sSrcScript;
                    f_dest.flush();
                    f_dest.close();
                    fs::permissions(pathJobScriptFilepath,
                                    fs::add_perms | fs::owner_all | fs::group_read | fs::group_exe | fs::others_read |
                                        fs::others_exe);

                    // Execute the submitter script
                    fs::path pathlsfScript(pathPluginDir);
                    pathlsfScript /= "dds-submit-lsf-worker";
                    stringstream cmd;
                    cmd << bp::search_path("dds-daemonize").string() << " " << quoted(sSandboxDir) << " "
                        << bp::search_path("bash").string() << " -c " << quoted(pathlsfScript.string());

                    proto.sendMessage(dds::intercom_api::EMsgSeverity::info, "Preparing job submission...");
                    string output;
                    pid_t exitCode = execute(cmd.str(), chrono::seconds(30), &output);

                    // In case of error there can be a bash.out.log created, let's check it's content
                    fs::path pathBashlog(sSandboxDir);
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
                                string submitMsg = "DDS agents have been submitted";
                                if (isLightweightMode)
                                {
                                    submitMsg += " (lightweight mode)";
                                }
                                proto.sendMessage(EMsgSeverity::info, submitMsg);
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
