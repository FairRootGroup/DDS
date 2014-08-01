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
// DDS
#include "version.h"
#include "Options.h"
#include "Process.h"
#include "ErrorCode.h"
#include "AgentConnectionManager.h"
#include "TalkToCommander.h"
#include "BOOSTHelper.h"
#include "Logger.h"
#include "UserDefaults.h"

using namespace std;
using namespace MiscCommon;
namespace bpo = boost::program_options;
using namespace dds;

int main(int argc, char* argv[])
{
    Logger::instance().init(); // Initialize log
    CUserDefaults::instance(); // Initialize user defaults

    // Command line parser
    SOptions_t options;
    try
    {
        if (!ParseCmdLine(argc, argv, &options))
            return EXIT_SUCCESS;
    }
    catch (exception& e)
    {
        LOG(fatal) << e.what();
        return EXIT_FAILURE;
    }

    string pidFileName(CUserDefaults::getDDSPath());
    pidFileName += "dds-agent.pid";

    // Checking for "status" option
    if (SOptions_t::cmd_status == options.m_Command)
    {
        pid_t pid = CPIDFile::GetPIDFromFile(pidFileName);
        if (pid > 0 && IsProcessExist(pid))
        {
            LOG(log_stdout) << PROJECT_NAME << " process (" << pid << ") is running...";
        }
        else
        {
            LOG(log_stdout) << PROJECT_NAME << " is not running...";
        }

        return EXIT_SUCCESS;
    }

    // Checking for "stop" option
    if (SOptions_t::cmd_stop == options.m_Command)
    {
        // TODO: make wait for the process here to check for errors
        const pid_t pidToKill = CPIDFile::GetPIDFromFile(pidFileName);
        if (pidToKill > 0 && IsProcessExist(pidToKill))
        {
            LOG(log_stdout) << PROJECT_NAME << ": self exiting (" << pidToKill << ")...";
            // TODO: Maybe we need more validations of the process before
            // sending a signal. We don't want to kill someone else.
            kill(pidToKill, SIGTERM);

            // Waiting for the process to finish
            size_t iter(0);
            const size_t max_iter = 30;
            while (iter <= max_iter)
            {
                if (!IsProcessExist(pidToKill))
                {
                    cout << endl;
                    break;
                }
                LOG(log_stdout) << ".";
                sleep(1); // sleeping for 1 second
                ++iter;
            }
            if (IsProcessExist(pidToKill))
                LOG(error) << "FAILED to close the process.";
        }

        return EXIT_SUCCESS;
    }

    // Checking for "start" option
    if (SOptions_t::cmd_start == options.m_Command)
    {
        try
        {
            CPIDFile pidfile(pidFileName, ::getpid());
            boost::asio::io_service service;
            CAgentConnectionManager agent(options, service);
            agent.start();
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
