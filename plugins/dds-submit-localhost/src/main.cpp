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
#include <boost/filesystem/path.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
// STD
#include <chrono>
#include <fstream>
#include <iostream>
#include <list>
#include <stdexcept>
#include <thread>
// DDS
#include "BOOSTHelper.h"
#include "BOOST_FILESYSTEM.h"
#include "DDSSysFiles.h"
#include "Process.h"
#include "Res.h"
#include "SysHelper.h"
#include "UserDefaults.h"
#include "dds_intercom.h"
#include "logEngine.h"
#include "ncf.h"
#include "ncf.h"
#include "version.h"

using namespace std;
using namespace dds;
using namespace dds::intercom_api;
using namespace dds::ncf;
using namespace dds::user_defaults_api;
using namespace dds::pipe_log_engine;
using namespace MiscCommon;
namespace bfs = boost::filesystem;
namespace bpo = boost::program_options;
namespace boost_hlp = MiscCommon::BOOSTHelper;

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

    // Init communication with DDS commander server
    CRMSPluginProtocol proto(vm["id"].as<string>());

    try
    {
        // Subscribe on onSubmit command
        proto.onSubmit([&proto](const SSubmit& _submit) {

            stringstream ss;
            ss << "Will use the local host to deploy " << _submit.m_nInstances << " agents";
            proto.sendMessage(EMsgSeverity::info, ss.str());
            ss.str("");

            // Create temp directory for DDS agents
            bfs::path tempDirPath = bfs::temp_directory_path();
            bfs::path wrkDirPath(tempDirPath);
            string tmpDir = BOOSTHelper::get_temp_dir("dds");
            wrkDirPath /= tmpDir;
            wrkDirPath /= "wn";

            if (!bfs::exists(wrkDirPath) && !bfs::create_directories(wrkDirPath))
            {
                ss << "Can't create working directory: " << wrkDirPath.string();
                proto.sendMessage(EMsgSeverity::error, ss.str());
                ss.str("");
                return;
            }

            ss << "Using \'" << wrkDirPath.parent_path().string() << "\' to spawn agents";
            proto.sendMessage(EMsgSeverity::info, ss.str());
            ss.str("");

            ss << "Starting DDSScout in \'" << wrkDirPath.string() << "\'";
            proto.sendMessage(EMsgSeverity::info, ss.str());
            ss.str("");

            // Copy worker script to temp directory
            bfs::path wrkScriptPath(CUserDefaults::instance().getWrkScriptPath());
            bfs::path dstWrkScriptPath(wrkDirPath);
            dstWrkScriptPath /= wrkScriptPath.filename();
            bfs::copy_file(wrkScriptPath, dstWrkScriptPath, bfs::copy_option::overwrite_if_exists);

            stringstream cmd;
            cmd << "$DDS_LOCATION/bin/dds-daemonize " << wrkDirPath.string() << " "
                << "/bin/bash -c \"" << dstWrkScriptPath.string() << " " << _submit.m_nInstances << "\"";

            try
            {
                string output;
                pid_t exitCode = do_execv(cmd.str(), 30, &output);

                if (exitCode == 0)
                {
                    proto.sendMessage(EMsgSeverity::info, "DDS agents have been submitted");

                    proto.sendMessage(EMsgSeverity::info, "Checking status of agents...");

                    bool started(true);
                    // Give agents 30 seconds to start
                    for (int time = 0; time < 60; time++)
                    {
                        started = true;
                        // Check whether all agent have started successfully
                        for (int i = 0; i < _submit.m_nInstances; i++)
                        {
                            bfs::path lockFilePath((i == 0) ? wrkDirPath.string()
                                                            : wrkDirPath.string() + "_" + to_string(i));
                            lockFilePath /= "DDSWorker.lock";
                            if (!bfs::exists(lockFilePath))
                            {
                                // One of the lock files does not exist
                                started = false;
                                break;
                            }
                        }

                        if (started)
                        {
                            proto.sendMessage(EMsgSeverity::info, "All agents have been started successfully");
                            break;
                        }
                        else
                        {
                            this_thread::sleep_for(chrono::milliseconds(500));
                        }
                    }
                    if (!started)
                    {
                        proto.sendMessage(EMsgSeverity::error, "Failed to deploy agents: some agents failed to start");
                    }
                }
                else
                {
                    proto.sendMessage(EMsgSeverity::error, "Failed to deploy agents: can't start worker script");
                }
            }
            catch (exception& e)
            {
                ss << "Failed to submit agents: " << e.what();
                proto.sendMessage(EMsgSeverity::error, ss.str());
                ss.str("");
            }

            proto.stop();

        });

        proto.sendInit();

        proto.wait();
    }
    catch (exception& e)
    {
        stringstream ss;
        ss << "Exception: " << e.what();
        proto.sendMessage(EMsgSeverity::error, ss.str());
        return 1;
    }

    return 0;
}
