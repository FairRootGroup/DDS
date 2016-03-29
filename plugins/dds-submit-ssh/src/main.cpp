// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
// BOOST
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-local-typedef"
#include <boost/program_options/parsers.hpp>
#pragma clang diagnostic pop

#include <boost/filesystem/operations.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
// STD
#include <chrono>
#include <fstream>
#include <list>
#include <stdexcept>
#include <thread>
// DDS
#include "BOOST_FILESYSTEM.h"
#include "BOOSTHelper.h"
#include "DDSSysFiles.h"
#include "Process.h"
#include "SysHelper.h"
#include "UserDefaults.h"
#include "dds_intercom.h"
#include "local_types.h"
#include "logEngine.h"
#include "ncf.h"
#include "version.h"
#include "worker.h"

using namespace std;
using namespace dds;
using namespace dds::intercom_api;
using namespace dds::ncf;
using namespace dds::ssh_cmd;
using namespace dds::user_defaults_api;
using namespace dds::pipe_log_engine;
using namespace MiscCommon;
namespace bpo = boost::program_options;
namespace bfs = boost::filesystem;
namespace boost_hlp = MiscCommon::BOOSTHelper;
//=============================================================================
const LPCSTR g_pipeName = ".dds_ssh_pipe";
typedef list<CWorker> workersList_t;
typedef CThreadPool<CWorker, ETaskType> threadPool_t;
//=============================================================================
// Command line parser
bool parseCmdLine(int _argc, char* _argv[], bpo::variables_map* _vm)
{
    // Generic options
    bpo::options_description options("Options");
    options.add_options()("id", bpo::value<std::string>(), "DDS submission ID");

    // Parsing command-line
    bpo::variables_map vm;
    bpo::store(bpo::command_line_parser(_argc, _argv).options(options).run(), vm);
    bpo::notify(vm);

    if (!vm.count("id") || vm["id"].as<string>().empty())
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
    ss << "Will use the local host to deply ";
    if (_nInstances == 0)
        _nInstances = thread::hardware_concurrency();

    ss << _nInstances << " agents";
    proto.sendMessage(dds::intercom_api::EMsgSeverity::info, ss.str());
    ss.str("");

    // Create temporary ssh configuration
    bfs::path tempDirPath = bfs::temp_directory_path();
    bfs::path wrkDirPath(tempDirPath);
    string tmpDir = BOOSTHelper::get_temp_dir("dds");
    wrkDirPath /= tmpDir;
    
    if (!bfs::exists(wrkDirPath) && !bfs::create_directories(wrkDirPath))
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
    MiscCommon::get_cuser_name(&userName);

    stringstream ssCfg;
    ssCfg << "wn, " << userName << "@localhost, ," << wrkDirPath.string() << ", " << _nInstances;
    f << ssCfg.str();
    f.close();

    return tmpfileName.string();
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

    CLogEngine slog(true);

    // Init communication with DDS commander server
    CRMSPluginProtocol proto(vm["id"].as<string>());

    try
    {
        // Collect workers list
        string pipeName(CUserDefaults::instance().getOptions().m_server.m_workDir);
        smart_append(&pipeName, '/');
        pipeName += g_pipeName;
        slog.start(pipeName);

        // Subscribe on onSubmit command
        proto.onSubmit([&proto, &vm](const SSubmit& _submit) {
            bool needLocalHost = _submit.m_cfgFilePath.empty();
            string configFile(_submit.m_cfgFilePath);
            size_t nInstances(_submit.m_nInstances);
            if (needLocalHost)
            {
                configFile = createLocalhostCfg(nInstances, vm["id"].as<string>());
            }

            if (!file_exists(configFile))
                throw runtime_error("DDS SSH config file doesn't exist: " + configFile);

            ifstream f(configFile.c_str());
            if (!f.is_open())
            {
                string msg("can't open configuration file \"");
                msg += configFile;
                msg += "\"";
                throw runtime_error(msg);
            }

            size_t wrkCount(0);

            workersList_t workers;
            string inlineShellScripCmds;
            {
                CNcf config;
                config.readFrom(f);
                inlineShellScripCmds = config.getBashEnvCmds();

                SWNOptions options;
                options.m_debug = true;
                options.m_logs = false;
                options.m_fastClean = false;

                configRecords_t recs(config.getRecords());
                configRecords_t::const_iterator iter = recs.begin();
                configRecords_t::const_iterator iter_end = recs.end();
                for (; iter != iter_end; ++iter)
                {
                    configRecord_t rec(*iter);
                    CWorker wrk(rec, options);
                    workers.push_back(wrk);

                    // if (0 == rec->m_nWorkers)
                    //     dynWrk = true; // user wants us to dynamicly decide on how many
                    //     job slots to create

                    wrkCount += rec->m_nWorkers;
                }
            }
            stringstream ssMsg;
            ssMsg << "Deploying " << wrkCount << " agents...";
            proto.sendMessage(dds::intercom_api::EMsgSeverity::info, ssMsg.str());
            ssMsg.str("");

            // a thread pool for the DDS transport engine
            // may return 0 when not able to detect
            unsigned int concurrentThreads = thread::hardware_concurrency();
            // we need at least 4 threads
            if (concurrentThreads < 4)
                concurrentThreads = 4;
            // we don't need many threads if there are not so many agents to deploy
            if (concurrentThreads > wrkCount)
                concurrentThreads = wrkCount;

            ssMsg << "Starting thread pool using " << concurrentThreads << " threads.";
            proto.sendMessage(dds::intercom_api::EMsgSeverity::info, ssMsg.str());
            ssMsg.str("");

            // start thread-pool and push tasks into it
            threadPool_t threadPool(concurrentThreads);

            workersList_t::iterator iter = workers.begin();
            workersList_t::iterator iter_end = workers.end();
            for (; iter != iter_end; ++iter)
            {
                // push main tasks
                threadPool.pushTask(*iter, task_submit);
            }
            threadPool.stop(true);

            // Check the status of all tasks Failed
            size_t badFailedCount = threadPool.tasksCount() - threadPool.successfulTasks();
            ssMsg << "Successfully processed tasks: " << threadPool.successfulTasks();
            proto.sendMessage(dds::intercom_api::EMsgSeverity::info, ssMsg.str());
            ssMsg.str("");
            ssMsg << "Failed tasks: " << badFailedCount;
            proto.sendMessage(dds::intercom_api::EMsgSeverity::info, ssMsg.str());

            if (badFailedCount > 0)
                proto.sendMessage(dds::intercom_api::EMsgSeverity::info, "WARNING: some tasks have failed.");

            if (threadPool.successfulTasks() > 0)
                proto.sendMessage(dds::intercom_api::EMsgSeverity::info, "DDS agents have been submitted.");

            proto.stop();
        });
        
        // let DDS know that we are online
        proto.sendInit();

        proto.wait();
    }
    catch (exception& e)
    {
        proto.sendMessage(dds::intercom_api::EMsgSeverity::error, e.what());
        return 1;
    }

    return 0;
}
