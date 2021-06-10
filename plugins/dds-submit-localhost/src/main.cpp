// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
// BOOST
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
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
#include "Intercom.h"
#include "Logger.h"
#include "MiscSetup.h"
#include "Process.h"
#include "SysHelper.h"
#include "UserDefaults.h"
#include "ncf.h"

using namespace std;
using namespace dds;
using namespace dds::intercom_api;
using namespace dds::ncf;
using namespace dds::user_defaults_api;
using namespace MiscCommon;
namespace bfs = boost::filesystem;
namespace bpo = boost::program_options;
namespace boost_hlp = MiscCommon::BOOSTHelper;

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

    if (_vm)
        _vm->swap(vm);

    return true;
}

bool checkAgentStatus(const bfs::path _wrkDirPath,
                      const string& _fileName,
                      int _numInstances,
                      CRMSPluginProtocol& _proto,
                      const string& _successMsg)
{
    int failedInstance(-1);
    // Give agents 30 seconds to start
    for (int time = 0; time < 60; ++time)
    {
        failedInstance = -1;
        // Check whether all agent have started successfully
        for (int i = 0; i < _numInstances; ++i)
        {
            bfs::path filePath((i == 0) ? _wrkDirPath.string() : _wrkDirPath.string() + "_" + to_string(i));
            filePath /= _fileName;

            if (!bfs::exists(filePath))
            {
                // One of the files does not exist
                failedInstance = i;
                break;
            }
        }

        if (failedInstance == -1)
        {
            _proto.sendMessage(EMsgSeverity::info, _successMsg);
            break;
        }
        else
        {
            this_thread::sleep_for(chrono::milliseconds(200));
        }
    }
    if (failedInstance != -1)
    {
        stringstream ss;
        ss << "Failed to deploy agents: some agents failed to start\n";
        ss << "Attaching scout's log of the first instance:\n";
        bfs::path logPath((failedInstance == 0) ? _wrkDirPath.string()
                                                : _wrkDirPath.string() + "_" + to_string(failedInstance));

        for (auto& p : bfs::recursive_directory_iterator(logPath))
        {
            if (p.path().extension() == ".log")
            {
                ss << "------------------------------------------\n";
                ss << "Log file: " << p.path().string() << "\n";
                ss << "----------\n";
                std::ifstream file(p.path().string());
                if (file)
                {
                    ss << file.rdbuf();
                    file.close();
                }
                else
                {
                    ss << "Failed to open log file: " << p.path().string();
                }
                ss << "\n------------------------------------------\n";
            }
        }

        _proto.sendMessage(EMsgSeverity::error, ss.str());
        return false;
    }
    return true;
}

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

    try
    {
        // Subscribe on onSubmit command
        proto.onSubmit([&proto](const SSubmit& _submit) {
            // Stop submission, if the number of instances is more than 1
            // localhost needs only 1 agent to work
            if (_submit.m_nInstances > 1)
            {
                proto.sendMessage(
                    EMsgSeverity::error,
                    "Submitting more than one agent on localhost is an overkill. Please omit -n [ --number ] option, "
                    "or call dds-submit multiple times if you really need multiple agents.");
                proto.stop();
                return;
            }

            if (_submit.m_slots < 1)
            {
                proto.sendMessage(EMsgSeverity::error,
                                  "Please, specify a number of task slots by using the \"--slots arg\" option");
                proto.stop();
                return;
            }

            unsigned int nInstances = _submit.m_nInstances;

            stringstream ss;
            ss << "Will use the local host to deploy " << nInstances << " agents";
            proto.sendMessage(EMsgSeverity::info, ss.str());
            ss.str("");

            // Create temp directory for DDS agents
            bfs::path tempDirPath = bfs::temp_directory_path();
            bfs::path wrkDirPath(tempDirPath);
            string tmpDir = BOOSTHelper::get_temp_dir("dds");
            wrkDirPath /= CUserDefaults::instance().getLockedSID();
            wrkDirPath /= tmpDir;
            wrkDirPath /= "wn";

            if (!bfs::exists(wrkDirPath) && !bfs::create_directories(wrkDirPath))
            {
                ss << "Can't create working directory: " << wrkDirPath.string();
                proto.sendMessage(EMsgSeverity::error, ss.str());
                ss.str("");
                proto.stop();
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

            bfs::path bashPath = bp::search_path("bash");
            bfs::path logPath(wrkDirPath);
            logPath /= "scout.log";
            stringstream cmd;

            cmd << bashPath.string() << " -c \"" << dstWrkScriptPath.string() << " " << nInstances << " &> " << logPath
                << "\"";

            try
            {
                execute(cmd.str());

                proto.sendMessage(EMsgSeverity::info, "DDS agents have been submitted");

                proto.sendMessage(EMsgSeverity::info, "Checking status of agents...");

                bool statusLock = checkAgentStatus(
                    wrkDirPath, "DDSWorker.lock", nInstances, proto, "All agents have been started successfully");

                if (statusLock)
                {
                    proto.sendMessage(EMsgSeverity::info, "Validating...");

                    checkAgentStatus(wrkDirPath,
                                     CUserDefaults::getAgentIDFileName(),
                                     nInstances,
                                     proto,
                                     "All agents have been validated successfully");
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

        // Let DDS know that we are online and start listening waiting for notifications
        proto.start();
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
