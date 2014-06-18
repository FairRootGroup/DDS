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
#include "Logger.h"

using namespace std;
using namespace MiscCommon;
namespace bpo = boost::program_options;
using namespace dds::commander;

//=============================================================================
int main(int argc, char* argv[])
{
    using namespace boost::log::trivial;
    Logger::instance().init("dds_commander.log");

    // Command line parser
    SOptions_t options;
    try
    {
        if (!ParseCmdLine(argc, argv, &options))
            return EXIT_SUCCESS;
    }
    catch (exception& e)
    {
        // TODO: Log me!
        std::cout << e.what() << endl;
        return EXIT_FAILURE;
    }

    /*    // Normalizing paths of common options
        PoD::SCommonOptions_t& common = (Server == options.m_agentMode) ? Options.m_podOptions.m_server.m_common : Options.m_podOptions.m_worker.m_common;
        // resolving user's home dir from (~/ or $HOME, if present)
        smart_path(&common.m_workDir);
        // We need to be sure that there is "/" always at the end of the path
        smart_append<string>(&common.m_workDir, '/');
        smart_path(&common.m_logFileDir);
        smart_append<string>(&common.m_logFileDir, '/');

        // pidfile name
        string pidfile_name(common.m_workDir);
        pidfile_name += "pod-agent.pid";
    */

    string pidfile_name("pidfile.txt"); // ONLY TEMP

    // Checking for "status" option
    if (SOptions_t::cmd_status == options.m_Command)
    {
        pid_t pid = CPIDFile::GetPIDFromFile(pidfile_name);
        if (pid > 0 && IsProcessExist(pid))
        {
            LOG(info) << PROJECT_NAME << " process (" << pid << ") is running..." << endl;
        }
        else
        {
            LOG(info) << PROJECT_NAME << " is not running..." << endl;
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
            std::cout << PROJECT_NAME << ": self exiting (" << pid_to_kill << ")..." << endl;
            // TODO: Maybe we need more validations of the process before
            // sending a signal. We don't want to kill someone else.
            kill(pid_to_kill, SIGTERM);

            // Waiting for the process to finish
            size_t iter(0);
            const size_t max_iter = 30;
            while (iter <= max_iter)
            {
                if (!IsProcessExist(pid_to_kill))
                {
                    std::cout << endl;
                    break;
                }
                std::cout << ".";
                // cout.flush();
                sleep(1); // sleeping for 1 second
                ++iter;
            }
            if (IsProcessExist(pid_to_kill))
                std::cout << "FAILED to close the process." << endl;
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
            // After fork we have to reinit log engine
            Logger::instance().init("dds_commander_2.log");
            try
            {
                LOG(boost::log::trivial::info) << "Log created.";
            }
            catch (std::exception& e)
            {
                LOG(boost::log::trivial::info) << "Log exception: " << e.what();
            }
            server.start();
        }
        catch (exception& e)
        {
            //  agent.FaultLog(erError, e.what());
            return EXIT_FAILURE; // exitCode_GENERAL_ERROR;
        }
        catch (...)
        {
            //  string errMsg("Unexpected Exception occurred.");
            //  agent.FaultLog(erXMLInit, errMsg);
            return EXIT_FAILURE; // exitCode_GENERAL_ERROR;
        }
    }

    //   return agent.getExitCode();
    return EXIT_SUCCESS;
}
