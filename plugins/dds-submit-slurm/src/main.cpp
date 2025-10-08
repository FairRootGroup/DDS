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
#include <boost/regex.hpp>
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
// protobuf
#include "submit_info.pb.h"
#include "submit_info_slurm.pb.h"
#include <google/protobuf/util/time_util.h>
using google::protobuf::util::TimeUtil;

using namespace std;
using namespace dds;
using namespace dds::intercom_api;
using namespace dds::user_defaults_api;
using namespace dds::pipe_log_engine;
using namespace dds::misc;
namespace bpo = boost::program_options;
namespace fs = boost::filesystem;
// Note: namespace bp is already defined in Process.h

//=============================================================================
// file is located in the DDS server working dir
const LPCSTR g_pipeName = ".dds_slurm_pipe";
const LPCSTR g_jobIDFile = ".dds_slurm_jobid";
// TODO: Move this to DDS commander once ToolsAPI supports protobuf.
// Ideally the commander should create and read this file. Plug-ins will only receive info data blocks via DDS
// transport.
const LPCSTR g_submitInfoFile = "submit.inf";
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
    // Verify that the version of the library that we linked against is
    // compatible with the version of the headers we compiled against.
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    bpo::variables_map vm;
    if (defaultExecSetup<bpo::variables_map>(argc, argv, &vm, &parseCmdLine) == EXIT_FAILURE)
        return EXIT_FAILURE;

    // Session ID
    boost::uuids::uuid sid(boost::uuids::nil_uuid());
    if (vm.count("session"))
        sid = boost::uuids::string_generator()(vm["session"].as<std::string>());

    // Submission ID
    string submissionID;
    if (vm.count("id"))
        submissionID = vm["id"].as<std::string>();

    if (submissionID.empty())
    {
        cerr << "Submission ID is empty" << endl;
        return EXIT_FAILURE;
    }

    if (defaultExecReinit(sid) == EXIT_FAILURE)
        return EXIT_FAILURE;

    // Init communication with DDS commander server
    CRMSPluginProtocol proto(vm["id"].as<string>());

    // IMPORTANT! Should be create only after DDS log is inited
    CLogEngine slog(true);

    try
    {
        // The working directory for local files, like pipes, pids, jobid, etc...
        fs::path pathWorkDirLocalFiles(smart_path(CUserDefaults::instance().getValueForKey("server.work_dir")));
        pathWorkDirLocalFiles /= submissionID;

        // init pipe log engine to get log messages from the child scripts
        fs::path pathPipeName(pathWorkDirLocalFiles);
        pathPipeName /= g_pipeName;

        // instruct pipe log engine to reflect output on user's output as well
        slog.start(pathPipeName.string(),
                   [&proto](const string& _msg) { proto.sendMessage(EMsgSeverity::info, _msg); });

        // Subscribe on protocol messages
        proto.onSubmit(
            [&proto, &vm, &pathWorkDirLocalFiles](const SSubmit& _submit)
            {
                const string submissionId{ _submit.m_id };
                try
                {
                    // location of the plug-in files
                    fs::path pathPluginDir(vm["path"].as<string>());
                    // Generate SLURM job script
                    fs::path pathJobScriptSourceFilepath(pathPluginDir);
                    pathJobScriptSourceFilepath /= "job.slurm.in";
                    // Checxk that the source of the script exists
                    if (!fs::exists(pathJobScriptSourceFilepath))
                        throw runtime_error(
                            "Can't find source of the job script. Plug-in's installation is inconsistent.");

                    fs::ifstream f_src(pathJobScriptSourceFilepath);
                    stringstream ssSrcScript;
                    ssSrcScript << f_src.rdbuf();
                    string sSrcScript(ssSrcScript.str());

                    proto.sendMessage(dds::intercom_api::EMsgSeverity::info, "Generating SLURM Job script...");

                    // Check if lightweight mode is enabled and log it
                    bool isLightweightMode = dds::tools_api::SSubmitRequestData::isFlagEnabled(
                        _submit.m_flags, dds::tools_api::SSubmitRequestData::ESubmitRequestFlags::enable_lightweight);
                    if (isLightweightMode)
                    {
                        proto.sendMessage(dds::intercom_api::EMsgSeverity::info,
                                          "Lightweight deployment mode: Workers will use existing DDS installation on "
                                          "compute nodes.");
                    }

                    // Replace #DDS_USER_OPTIONS
                    fs::path pathUserOptions(_submit.m_cfgFilePath);
                    if (fs::exists(pathUserOptions))
                    {
                        fs::ifstream f_userOptions(pathUserOptions);
                        string sUserOptions((istreambuf_iterator<char>(f_userOptions)), istreambuf_iterator<char>());
                        boost::replace_all(sSrcScript, "#DDS_USER_OPTIONS", sUserOptions);
                    }

                    // Replace %DDS_NINSTANCES%
                    boost::replace_all(sSrcScript, "%DDS_NINSTANCES%", to_string(_submit.m_nInstances));

                    // Replace %DDS_NMININSTANCES%
                    if (_submit.m_nMinInstances > 0)
                        boost::replace_all(sSrcScript, "%DDS_NMININSTANCES%", to_string(_submit.m_nMinInstances) + "-");
                    else
                        boost::replace_all(sSrcScript, "%DDS_NMININSTANCES%", "");

                    // Replace %DDS_NSLOTS%
                    boost::replace_all(sSrcScript, "%DDS_NSLOTS%", to_string(_submit.m_slots));

                    // #DDS_AGENT_CPU_REQUIREMENT
                    if (!dds::tools_api::SSubmitRequestData::isFlagEnabled(
                            _submit.m_flags,
                            dds::tools_api::SSubmitRequestData::ESubmitRequestFlags::enable_overbooking))
                    {
                        stringstream ss;
                        ss << "#SBATCH --cpus-per-task=" << _submit.m_slots;
                        boost::replace_all(sSrcScript, "#DDS_AGENT_CPU_REQUIREMENT", ss.str());
                    }
                    else
                        boost::replace_all(sSrcScript, "#DDS_AGENT_CPU_REQUIREMENT", "");

                    // #DDS_INLINE_CONFIG
                    boost::replace_all(sSrcScript, "#DDS_INLINE_CONFIG", _submit.m_inlineConfig);

                    // Replace %DDS_SUBMISSION_TAG%
                    boost::replace_all(sSrcScript, "%DDS_SUBMISSION_TAG%", _submit.m_submissionTag);

                    // Replace %DDS_JOB_ROOT_WRK_DIR%
                    string sSandboxDir(smart_path(CUserDefaults::instance().getWrkPkgDir(submissionId)));
                    fs::path pathJobWrkDir(sSandboxDir);
                    boost::replace_all(sSrcScript, "%DDS_JOB_ROOT_WRK_DIR%", pathJobWrkDir.string());
                    // create ROOT wrk dir for jobs
                    fs::create_directories(pathJobWrkDir);

                    const string sAgentWrkDirSpecial(CUserDefaults::instance().getValueForKey("agent.work_dir"));
                    fs::path pathAgentWrkDirSpecial(sAgentWrkDirSpecial);
                    pathAgentWrkDirSpecial /= CUserDefaults::instance().getCurrentSID();
                    pathAgentWrkDirSpecial /= submissionId;
                    // Uses AgentWrkDir as the agent's wrk dir, if not empty
                    fs::path pathAgentWrkDir(sAgentWrkDirSpecial.empty() ? sSandboxDir : pathAgentWrkDirSpecial);
                    // Non need to create the pathAgentWrkDir directory as the job script will do that
                    boost::replace_all(sSrcScript, "%DDS_AGENT_ROOT_WRK_DIR%", pathAgentWrkDir.string());

                    // Replace %DDS_SCOUT%
                    string sScoutScriptPath(CUserDefaults::instance().getWrkScriptPath(submissionId));
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
                    cmd << bp::search_path("dds-daemonize").string() << " " << quoted(sSandboxDir) << " "
                        << bp::search_path("bash").string() << " -c "
                        << quoted(pathSLURMScript.string() + " -s " + submissionId + " -a " +
                                  CUserDefaults::instance().getCurrentSID());

                    proto.sendMessage(dds::intercom_api::EMsgSeverity::info,
                                      "Preparing job submission. SubmissionID: " + submissionId);
                    proto.sendMessage(dds::intercom_api::EMsgSeverity::info, "Sandbox dir: " + sSandboxDir);
                    string output;
                    pid_t exitCode{ execute(cmd.str(), chrono::seconds(30), &output) };

                    // In case of error there can be a bash.out.log created, let's check it's content
                    fs::path pathBashlog(sSandboxDir);
                    pathBashlog /= "bash.out.log";

                    if (exitCode == 0)
                    {
                        bool started(false);
                        fs::path pathJobIDFile(pathWorkDirLocalFiles);
                        pathJobIDFile /= g_jobIDFile;

                        // Give the job 2 minutes to submit (240s = 2400 * 100ms)
                        for (int t = 0; t < 2400; ++t)
                        {
                            if (fs::exists(pathJobIDFile))
                            {
                                string rmsJobID;
                                ifstream f;
                                f.open(pathJobIDFile.native());
                                if (f.is_open())
                                {
                                    f >> rmsJobID;
                                }
                                // Create submit info block
                                dds::protocol::SlurmSubmitInfo protoSlurmSubmitInfo;
                                protoSlurmSubmitInfo.add_slurm_job_id(rmsJobID);

                                // Default values
                                protoSlurmSubmitInfo.set_alloc_nodes(0);
                                protoSlurmSubmitInfo.set_state(0);
                                protoSlurmSubmitInfo.set_job_info_available(false);

                                // Try to read job info if available
                                fs::path pathJobInfoFile(pathWorkDirLocalFiles);
                                pathJobInfoFile /= "job.info";
                                if (fs::exists(pathJobInfoFile))
                                {
                                    uint32_t allocNodes = 0;
                                    uint32_t state = 0;
                                    ifstream f(pathJobInfoFile.native());
                                    if (f.is_open())
                                    {
                                        if (f >> allocNodes >> state)
                                        {
                                            protoSlurmSubmitInfo.set_alloc_nodes(allocNodes);
                                            protoSlurmSubmitInfo.set_state(state);
                                            protoSlurmSubmitInfo.set_job_info_available(true);

                                            // Add additional log message for debugging
                                            proto.sendMessage(EMsgSeverity::info,
                                                              "Job info read successfully: Nodes=" +
                                                                  to_string(allocNodes) + " State=" + to_string(state));
                                        }
                                        else
                                        {
                                            proto.sendMessage(EMsgSeverity::error,
                                                              "Failed to parse job info file format");
                                        }
                                    }
                                    else
                                    {
                                        proto.sendMessage(EMsgSeverity::error, "Failed to open job info file");
                                    }
                                }

                                dds::protocol::SubmitInfo protoSubmitInfo;
                                protoSubmitInfo.set_session_id(CUserDefaults::instance().getCurrentSID());
                                protoSubmitInfo.set_submission_id(submissionId);
                                protoSubmitInfo.set_dds_sandbox_dir(sSandboxDir);
                                *protoSubmitInfo.mutable_submission_timestamp() =
                                    TimeUtil::SecondsToTimestamp(time(NULL));
                                protoSubmitInfo.set_rms_plugin("dds-submit-slurm");
                                protoSubmitInfo.mutable_rms_plugin_data()->PackFrom(protoSlurmSubmitInfo);

                                // Write submit info
                                fs::path pathSubmitInfoFile(pathWorkDirLocalFiles);
                                pathSubmitInfoFile /= g_submitInfoFile;
                                fstream output(pathSubmitInfoFile.native(), ios::out | ios::trunc | ios::binary);
                                if (!output.is_open())
                                {
                                    proto.sendMessage(EMsgSeverity::error, "Failed to open submission metadata file");
                                }
                                else if (!protoSubmitInfo.SerializeToOstream(&output))
                                {
                                    proto.sendMessage(EMsgSeverity::error, "Failed to save submission metadata");
                                }
                                output.close();

                                started = true;
                                string submitMsg = "DDS agents have been submitted";
                                if (isLightweightMode)
                                {
                                    submitMsg += " (lightweight mode)";
                                }
                                proto.sendMessage(EMsgSeverity::info, submitMsg);
                                break;
                            }
                            // give bash 40 sec to write the log file if it fails to start our script (40s = 400 *
                            // 100ms)
                            else if (t > 400 && fs::exists(pathBashlog))
                            {
                                // if bash log exists, then we have a problem starting the submit script
                                started = false;
                                break;
                            }
                            else
                            {
                                this_thread::sleep_for(chrono::milliseconds(100));
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
