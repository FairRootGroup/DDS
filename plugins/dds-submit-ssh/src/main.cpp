// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
// BOOST
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-local-typedef"
#include <boost/program_options/parsers.hpp>
#pragma clang diagnostic pop

#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/options_description.hpp>
// STD
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <list>
#include <thread>
#include <chrono>
// DDS
#include "BOOSTHelper.h"
#include "SysHelper.h"
#include "DDSSysFiles.h"
#include "version.h"
#include "ncf.h"
#include "worker.h"
#include "Process.h"
#include "logEngine.h"
#include "local_types.h"
#include "UserDefaults.h"
#include "Res.h"
#include "dds_intercom.h"
#include "ncf.h"

using namespace std;
using namespace dds;
using namespace dds::ncf;
using namespace dds::ssh_cmd;
using namespace dds::user_defaults_api;
using namespace dds::pipe_log_engine;
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
    // LOG(log_stdout) << " v" << PROJECT_VERSION_STRING << "DDS configuration"
    //                 << " v" << USER_DEFAULTS_CFG_VERSION << "\n" << MiscCommon::g_cszReportBugsAddr;
}
//=============================================================================
// Command line parser
bool parseCmdLine(int _argc, char* _argv[], bpo::variables_map* _vm)
{
    // Generic options
    bpo::options_description options("Options");
    options.add_options()("id", "DDS submission ID");

    // Parsing command-line
    bpo::variables_map vm;
    bpo::store(bpo::command_line_parser(_argc, _argv).options(options).run(), vm);
    bpo::notify(vm);

    /* if (!vm.count("id") || vm["id"].as<string>().empty())
     {
         printVersion();
         return false;
     }*/

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
            return 0;
    }
    catch (exception& e)
    {
        // LOG(log_stderr) << ": " << e.what();
        return 1;
    }

    CLogEngine slog(true);
    try
    {
        // Collect workers list
        string pipeName(CUserDefaults::instance().getOptions().m_server.m_workDir);
        smart_append(&pipeName, '/');
        pipeName += g_pipeName;
        slog.start(pipeName);

        CCustomCmd ddsCustomCmd;

        // initiate communication with DDS commander
        string sSubmitID = vm["id"].as<string>();
        ddsCustomCmd.send("submiter_id=" + sSubmitID, g_sRmsAgentSign);

        // Subscribe on custom commands
        ddsCustomCmd.subscribe([&ddsCustomCmd](const string& _command, const string& _condition, uint64_t _senderId)
                               {
                                   cout << "Received custom command: " << _command << " condition: " << _condition
                                        << " senderId: " << _senderId << endl;
                                   if (_command == "please-reply")
                                   {
                                       string senderIdStr = to_string(_senderId);
                                       ddsCustomCmd.send("reply-to-" + senderIdStr, senderIdStr);
                                   }
                                   else if (_command == "please-reply-ui")
                                   {
                                       string senderIdStr = to_string(_senderId);
                                       ddsCustomCmd.send("reply-to-ui-" + senderIdStr, senderIdStr);
                                   }
                               });

        // Subscribe on reply from DDS commander server
        ddsCustomCmd.subscribeReply([](const string& _msg)
                                    {
                                        string configFile = _msg;
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

                                        // a thread pool for the DDS transport engine
                                        // may return 0 when not able to detect
                                        unsigned int concurrentThreads = thread::hardware_concurrency();
                                        // we need at least 4 threads
                                        if (concurrentThreads < 4)
                                            concurrentThreads = 4;

                                        concurrentThreads *= 2;
                                        //     LOG(info) << "Starting dds-ssh thread pool using " << concurrentThreads
                                        //     << " concurrent threads.";

                                        //     LOG(info) << "Number of DDS slots: " << workers.size();

                                        //    LOG(info) << "Workers list:\n";

                                        // start thread-pool and push tasks into it
                                        threadPool_t threadPool(concurrentThreads);

                                        workersList_t::iterator iter = workers.begin();
                                        workersList_t::iterator iter_end = workers.end();
                                        for (; iter != iter_end; ++iter)
                                        {
                                            ostringstream ss;
                                            iter->printInfo(ss);
                                            //           LOG(debug) << ss.str();

                                            // pash pre-tasks
                                            threadPool.pushTask(*iter, task_exec);

                                            // push main tasks
                                            threadPool.pushTask(*iter, task_submit);
                                        }
                                        threadPool.stop(true);

                                        // Check the status of all tasks Failed
                                        size_t badFailedCount = threadPool.tasksCount() - threadPool.successfulTasks();
                                        //             LOG(log_stdout) << "\n*******************\n"
                                        //                             << "Successfully processed tasks: " <<
                                        //                             threadPool.successfulTasks() << '\n'
                                        //                             << "Failed tasks: " << badFailedCount << '\n' <<
                                        //                             "*******************";

                                        if (badFailedCount > 0)
                                        {
                                            //               LOG(log_stdout) << ("WARNING: some tasks have failed. Check
                                            //               dds.log for more information.");
                                            // return 1;
                                        }

                                        //         if (cmd_submit == command)
                                        //             LOG(log_stdout) << "DDS jobs have been submitted.";

                                        //        DDS::SDDDSSSHOptions opt_file;
                                        //        opt_file.m_config = configFile;
                                        //        opt_file.save(env.pod_sshCfgFile());

                                    });
    }
    catch (exception& e)
    {
        // LOG(log_stderr) << e.what();
        return 1;
    }

    return 0;
}
