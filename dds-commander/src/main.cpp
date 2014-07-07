// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
// DDS
#include "Options.h"
#include "Process.h"
#include "ErrorCode.h"
#include "CommanderServer.h"
#include "BOOSTHelper.h"
#include "UserDefaults.h"
#include "SysHelper.h"

using namespace std;
using namespace MiscCommon;
namespace bpo = boost::program_options;
using namespace dds;

//=============================================================================
int main(int argc, char* argv[])
{
    Logger::instance().init();
    LOG(info) << "Starting dds-commander";

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

    // resolving user's home dir from (~/ or $HOME, if present)
    string sWorkDir(options.m_userDefaults.getOptions().m_general.m_workDir);
    smart_path(&sWorkDir);
    // We need to be sure that there is "/" always at the end of the path
    smart_append<string>(&sWorkDir, '/');
    // pidfile name
    string pidfile_name(sWorkDir);
    pidfile_name += "dds-commander.pid";

    // Checking for "status" option
    if (SOptions_t::cmd_status == options.m_Command)
    {
        pid_t pid = CPIDFile::GetPIDFromFile(pidfile_name);
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
        const pid_t pid_to_kill = CPIDFile::GetPIDFromFile(pidfile_name);
        if (pid_to_kill > 0 && IsProcessExist(pid_to_kill))
        {
            LOG(log_stdout) << PROJECT_NAME << ": self exiting (" << pid_to_kill << ")...";
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

            CCommanderServer server(options);
            try
            {
                LOG(info) << "Log created.";
            }
            catch (std::exception& e)
            {
                LOG(error) << "Log exception: " << e.what();
            }
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

    // Checking for "submit" option
    if (SOptions_t::cmd_submit == options.m_Command)
    {
    }

    return EXIT_SUCCESS;
}
