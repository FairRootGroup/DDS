// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
// DDS
#include "Process.h"
#include "ErrorCode.h"
#include "ConnectionManager.h"
#include "TestConnectionManager.h"
#include "AgentChannel.h"
#include "TestChannel.h"
#include "BOOSTHelper.h"
#include "UserDefaults.h"
#include "SysHelper.h"
#include "INet.h"
// BOOST
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

using namespace std;
using namespace MiscCommon;
using namespace dds;
using boost::asio::ip::tcp;

//=============================================================================
int main(int argc, char* argv[])
{
    Logger::instance().init(); // Initialize log
    CUserDefaults::instance(); // Initialize user defaults

    vector<std::string> arguments(argv + 1, argv + argc);
    ostringstream ss;
    copy(arguments.begin(), arguments.end(), ostream_iterator<string>(ss, " "));
    LOG(info) << "Starting with arguments: " << ss.str();

    // Command line parser
    SOptions_t options;
    try
    {
        if (!ParseCmdLine(argc, argv, &options))
            return EXIT_SUCCESS;
    }
    catch (exception& e)
    {
        LOG(log_stderr) << e.what();
        return EXIT_FAILURE;
    }

    // resolving user's home dir from (~/ or $HOME, if present)
    string sWorkDir(CUserDefaults::instance().getOptions().m_server.m_workDir);
    smart_path(&sWorkDir);
    // We need to be sure that there is "/" always at the end of the path
    smart_append<string>(&sWorkDir, '/');
    // pidfile name
    string pidfile_name(sWorkDir);
    pidfile_name += "dds-commander.pid";

    // Checking for "stop" option
    if (SOptions_t::cmd_stop == options.m_Command)
    {
        // TODO: make wait for the process here to check for errors
        const pid_t pid_to_kill = CPIDFile::GetPIDFromFile(pidfile_name);
        if (pid_to_kill > 0 && IsProcessExist(pid_to_kill))
        {
            LOG(log_stdout) << "self exiting (" << pid_to_kill << ")...";
            // TODO: Maybe we need more validations of the process before
            // sending a signal. We don't want to kill someone else.
            kill(pid_to_kill, SIGTERM);

            // Waiting for the process to finish
            size_t iter(0);
            const size_t max_iter = 30;
            while (iter <= max_iter)
            {
                // show "progress dots". Don't use Log, as it will create a new line after each dot.
                if (!IsProcessExist(pid_to_kill))
                {
                    cout << "\n";
                    break;
                }
                cout << ".";
                sleep(1); // sleeping for 1 second
                ++iter;
            }
            if (IsProcessExist(pid_to_kill))
                LOG(log_stderr) << "FAILED to close the process.";
        }

        return EXIT_SUCCESS;
    }

    // Checking for "start" option
    if (SOptions_t::cmd_start == options.m_Command)
    {
        try
        {
            CPIDFile pidfile(pidfile_name, ::getpid());

            boost::asio::io_service io_service;
            const CUserDefaults& userDefaults = CUserDefaults::instance();
            // get a free port from a given range
            int nSrvPort =
                MiscCommon::INet::get_free_port(userDefaults.getOptions().m_server.m_ddsCommanderPortRangeMin,
                                                userDefaults.getOptions().m_server.m_ddsCommanderPortRangeMax);

            tcp::endpoint endpoint(tcp::v4(), nSrvPort);

            CConnectionManager server(options, io_service, endpoint);
            server.start();
        }
        catch (exception& e)
        {
            LOG(fatal) << e.what();
            return EXIT_FAILURE;
        }
        catch (...)
        {
            LOG(fatal) << "Unexpected Exception occurred.";
            return EXIT_FAILURE;
        }
    }

    // Checking for "test" option
    if (SOptions_t::cmd_test == options.m_Command)
    {
        try
        {
            CPIDFile pidfile(pidfile_name, ::getpid());

            boost::asio::io_service io_service;
            const CUserDefaults& userDefaults = CUserDefaults::instance();
            // get a free port from a given range
            int nSrvPort =
                MiscCommon::INet::get_free_port(userDefaults.getOptions().m_server.m_ddsCommanderPortRangeMin,
                                                userDefaults.getOptions().m_server.m_ddsCommanderPortRangeMax);

            tcp::endpoint endpoint(tcp::v4(), nSrvPort);

            CTestConnectionManager server(options, io_service, endpoint);
            server.start();
        }
        catch (exception& e)
        {
            LOG(fatal) << e.what();
            return EXIT_FAILURE;
        }
        catch (...)
        {
            LOG(fatal) << "Unexpected Exception occurred.";
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
