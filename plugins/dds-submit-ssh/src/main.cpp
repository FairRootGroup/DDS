// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
// BOOST
#include <boost/filesystem/operations.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
// STD
#include <chrono>
#include <fstream>
#include <list>
#include <stdexcept>
#include <thread>
// DDS
#include "BoostHelper.h"
#include "Intercom.h"
#include "Logger.h"
#include "MiscSetup.h"
#include "PipeLogEngine.h"
#include "SSHConfigFile.h"
#include "SysHelper.h"
#include "ToolsProtocol.h"
#include "UserDefaults.h"
#include "local_types.h"
#include "worker.h"

using namespace std;
using namespace dds;
using namespace dds::intercom_api;
using namespace dds::ssh_cmd;
using namespace dds::user_defaults_api;
using namespace dds::pipe_log_engine;
using namespace dds::misc;
using namespace dds::tools_api;
namespace bpo = boost::program_options;
namespace fs = boost::filesystem;

//=============================================================================
const LPCSTR g_pipeName = ".dds_ssh_pipe";
//=============================================================================
// Command line parser
bool parseCmdLine(int _argc, char* _argv[], bpo::variables_map* _vm)
{
    // Generic options
    bpo::options_description options("Options");
    options.add_options()("session", bpo::value<std::string>(), "DDS Session ID");
    options.add_options()("id", bpo::value<std::string>(), "DDS submission ID");
    options.add_options()("path", bpo::value<std::string>(), "Path to DDS plugins directory");

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
string createLocalhostCfg(size_t& _nInstances, const string& _sessionId)
{
    CRMSPluginProtocol proto(_sessionId);

    stringstream ss;
    ss << "Will use the local host to deploy ";
    if (_nInstances == 0)
        _nInstances = thread::hardware_concurrency();

    ss << _nInstances << " agents";
    proto.sendMessage(dds::intercom_api::EMsgSeverity::info, ss.str());
    ss.str("");

    // Create temporary ssh configuration
    fs::path tempDirPath = fs::temp_directory_path();
    fs::path wrkDirPath(tempDirPath);
    string tmpDir = get_temp_dir("dds");
    wrkDirPath /= tmpDir;

    if (!fs::exists(wrkDirPath) && !fs::create_directories(wrkDirPath))
    {
        stringstream ssErr;
        ssErr << "Can't create working directory: " << wrkDirPath.string();
        throw runtime_error(ssErr.str());
    }

    ss << "Using \'" << wrkDirPath.string() << "\' to spawn agents";
    proto.sendMessage(EMsgSeverity::info, ss.str());
    ss.str("");

    boost::filesystem::path tmpfileName(wrkDirPath);
    tmpfileName /= "dds_ssh.cfg";
    ofstream f(tmpfileName.string());

    string userName;
    get_cuser_name(&userName);

    stringstream ssCfg;
    ssCfg << "wn, " << userName << "@localhost, ," << wrkDirPath.string() << ", " << _nInstances;
    f << ssCfg.str();
    f.close();

    return tmpfileName.string();
}
//=============================================================================
int main(int argc, char* argv[])
{
    // FIX: A fix for cases when the parent process sets SIG_IGN (if it was created by
    // bosot::process::spawn). Restore default handler. If we don't do so, we might fail to waitpid our
    // children. After we started using boost::process we noticed that ::waitpid fails. boost:process either
    // sets its own handler or there is a call for signal(SIGCHLD, SIG_IGN);
    std::signal(SIGCHLD, SIG_DFL);
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    sigprocmask(SIG_UNBLOCK, &mask, NULL);

    bpo::variables_map vm;
    if (dds::misc::defaultExecSetup<bpo::variables_map>(argc, argv, &vm, &parseCmdLine) == EXIT_FAILURE)
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

    if (dds::misc::defaultExecReinit(sid) == EXIT_FAILURE)
        return EXIT_FAILURE;

    // Init communication with DDS commander server
    shared_ptr<CRMSPluginProtocol> proto = make_shared<CRMSPluginProtocol>(vm["id"].as<string>());

    try
    {
        // Create a thread pull
        // may return 0 when not able to detect
        unsigned int concurrentThreads = thread::hardware_concurrency();
        // we need at least 4 threads
        if (concurrentThreads < 4)
            concurrentThreads = 4;

        stringstream ssMsg;
        ssMsg << "Starting thread-pool using " << concurrentThreads << " threads.";
        proto->sendMessage(dds::intercom_api::EMsgSeverity::info, ssMsg.str());
        boost::asio::thread_pool pool(concurrentThreads);

        // Subscribe on onSubmit command
        proto->onSubmit(
            [&submissionID, &pool, proto, &vm](const SSubmit& _submit)
            {
                size_t wrkCount(0);
                size_t taskCount(0);
                atomic<size_t> successfulTasks(0);

                // Check if lightweight mode is enabled
                bool isLightweightMode = SSubmitRequestData::isFlagEnabled(
                    _submit.m_flags, SSubmitRequestData::ESubmitRequestFlags::enable_lightweight);

                try
                {
                    // Create the pipe log engine to cartch logs from external commands
                    CLogEngine slog(true);

                    fs::path pathWorkDirLocalFiles(
                        smart_path(CUserDefaults::instance().getValueForKey("server.work_dir")));
                    pathWorkDirLocalFiles /= submissionID;

                    // init pipe log engine to get log messages from the child scripts
                    fs::path pathPipeName(pathWorkDirLocalFiles);
                    pathPipeName /= g_pipeName;

                    slog.start(pathPipeName.string(),
                               [proto](const string& _msg) { proto->sendMessage(EMsgSeverity::info, _msg); });

                    // Log lightweight mode detection
                    if (isLightweightMode)
                    {
                        proto->sendMessage(EMsgSeverity::info,
                                           "Lightweight deployment mode detected. Workers will validate DDS "
                                           "installation on remote nodes.");
                    }

                    bool needLocalHost = _submit.m_cfgFilePath.empty();
                    string configFile(_submit.m_cfgFilePath);
                    size_t nInstances(_submit.m_nInstances);
                    if (needLocalHost)
                    {
                        configFile = createLocalhostCfg(nInstances, vm["id"].as<string>());
                    }

                    if (!file_exists(configFile))
                        throw runtime_error("DDS SSH config file doesn't exist: " + configFile);

                    CSSHConfigFile config(configFile);
                    string inlineShellScripCmds{ config.getBash() };

                    SWNOptions options;
                    options.m_logs = false;
                    options.m_fastClean = false;

                    configRecords_t recs(config.getRecords());
                    taskCount = recs.size();
                    for (auto& rec : recs)
                    {
                        rec->m_submissionID = _submit.m_id;

                        shared_ptr<CWorker> task =
                            shared_ptr<CWorker>(new CWorker(rec, options, vm["path"].as<string>()));

                        stringstream ssWorkerInfoMsg;
                        task->printInfo(ssWorkerInfoMsg);
                        if (isLightweightMode)
                        {
                            ssWorkerInfoMsg << " (lightweight mode)";
                        }
                        proto->sendMessage(dds::intercom_api::EMsgSeverity::info, ssWorkerInfoMsg.str());

                        wrkCount += rec->m_nSlots;

                        boost::asio::post(pool,
                                          [task, &successfulTasks, proto]
                                          {
                                              try
                                              {
                                                  if (task && task->run(task_submit))
                                                      ++successfulTasks;
                                              }
                                              catch (const exception& _e)
                                              {
                                                  proto->sendMessage(dds::intercom_api::EMsgSeverity::info, _e.what());
                                              }
                                          });
                    }

                    stringstream ss;
                    ss << "Deploying " << wrkCount << " agents";
                    if (isLightweightMode)
                    {
                        ss << " (lightweight mode - using existing DDS installation)";
                    }
                    ss << "...";
                    proto->sendMessage(dds::intercom_api::EMsgSeverity::info, ss.str());

                    pool.join();
                }
                catch (exception& _e)
                {
                    proto->sendMessage(dds::intercom_api::EMsgSeverity::error, _e.what());
                    LOG(error) << "Exception: " << _e.what();
                }

                // Check the status of all tasks Failed
                stringstream ss;
                size_t badFailedCount = taskCount - successfulTasks;
                ss << "Successfully processed tasks: " << successfulTasks;
                proto->sendMessage(dds::intercom_api::EMsgSeverity::info, ss.str());
                ss.str("");
                ss << "Failed tasks: " << badFailedCount;
                proto->sendMessage(dds::intercom_api::EMsgSeverity::info, ss.str());

                if (badFailedCount > 0)
                    proto->sendMessage(dds::intercom_api::EMsgSeverity::error, "WARNING: some tasks have failed.");

                if (successfulTasks > 0)
                {
                    string successMsg = "DDS agents have been submitted";
                    if (isLightweightMode)
                    {
                        successMsg += " (lightweight mode)";
                    }
                    successMsg += ".";
                    proto->sendMessage(dds::intercom_api::EMsgSeverity::info, successMsg);
                }

                proto->stop();
            });

        // Let DDS know that we are online and start listening waiting for notifications
        LOG(info) << "Start plug-in protocol";
        proto->start();
        LOG(info) << "Stop plug-in protocol";
    }
    catch (exception& e)
    {
        proto->sendMessage(dds::intercom_api::EMsgSeverity::error, e.what());
        LOG(error) << "Exception: " << e.what();
        return 1;
    }

    return 0;
}
