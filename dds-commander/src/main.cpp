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
// API
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
// DDS
#include "version.h"
#include "Options.h"
#include "Process.h"
#include "ErrorCode.h"
#include "CommanderServer.h"
#include "BOOSTHelper.h"
#include "Res.h"
#include "UserDefaults.h"

using namespace std;
using namespace MiscCommon;
namespace bpo = boost::program_options;
using namespace dds::commander;

//=============================================================================
void PrintVersion()
{
    cout << PROJECT_NAME << " v" << PROJECT_VERSION_STRING << "\n"
         << "DDS configuration"
         << " v" << USER_DEFAULTS_CFG_VERSION << "\n" << g_cszReportBugsAddr << endl;
}
//=============================================================================
// Command line parser
bool ParseCmdLine(int _argc, char* _argv[], SOptions* _options) throw(exception)
{
    if (nullptr == _options)
        throw runtime_error("Internal error: options' container is empty.");

    // Generic options
    bpo::options_description options("dds-commander options");
    options.add_options()("help,h", "Produce help message");
    options.add_options()("version,v", "Version information");
    options.add_options()("command",
                          bpo::value<string>(),
                          "The command is a name of dds-commander command."
                          " Can be one of the following: start, stop, status.\n"
                          "For user's convenience it is allowed to call dds-commander without \"--command\" option"
                          " by just specifying the command name directly, like:\ndds-commander start or dds-commander status.\n\n"
                          "Commands:\n"
                          "   start: \tStart dds-commander daemon\n"
                          "   stop: \tStop dds-commander daemon\n"
                          "   status: \tQuery current status of dds-command daemon\n");

    //...positional
    bpo::positional_options_description pd;
    pd.add("command", 1);

    // Parsing command-line
    bpo::variables_map vm;
    bpo::store(bpo::command_line_parser(_argc, _argv).options(options).positional(pd).run(), vm);
    bpo::notify(vm);

    if (vm.count("help") || vm.empty())
    {
        cout << options << endl;
        return false;
    }
    if (vm.count("version"))
    {
        PrintVersion();
        return false;
    }

    // Command
    if (vm.count("command"))
    {
        if (SOptions::cmd_unknown == SOptions::getCommandByName(vm["command"].as<string>()))
        {
            cout << PROJECT_NAME << " error: unknown command: " << vm["command"].as<string>() << "\n\n" << options << endl;
            return false;
        }
    }
    else
    {
        cout << PROJECT_NAME << ": Nothing to do\n\n" << options << endl;
        return false;
    }

    _options->m_Command = SOptions::getCommandByName(vm["command"].as<string>());

    return true;
}
//=============================================================================
int main(int argc, char* argv[])
{
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
        cerr << e.what() << endl;
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
            cout << PROJECT_NAME << " process (" << pid << ") is running..." << endl;
        }
        else
        {
            cout << PROJECT_NAME << " is not running..." << endl;
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
            cout << PROJECT_NAME << ": self exiting (" << pid_to_kill << ")..." << endl;
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
                    cout << endl;
                    break;
                }
                cout << ".";
                cout.flush();
                sleep(1); // sleeping for 1 second
                ++iter;
            }
            if (IsProcessExist(pid_to_kill))
                cerr << "FAILED to close the process." << endl;
        }

        return EXIT_SUCCESS;
    }

    // Checking for "start" option
    if (SOptions_t::cmd_start == options.m_Command)
    {
        // process ID and Session ID
        pid_t pid;
        pid_t sid;

        // Fork off the parent process
        pid = ::fork();
        if (pid < 0)
            return EXIT_FAILURE;

        // If we got a good PID, then we can exit the parent process.
        if (pid > 0)
            return EXIT_SUCCESS;

        // Change the file mode mask
        ::umask(0);

        // Create a new SID for the child process
        sid = ::setsid();
        if (sid < 0) // TODO:  Log the failure
            return EXIT_FAILURE;

        // Main object - agent itself
        //   CPROOFAgent agent;

        try
        {
            CPIDFile pidfile(pidfile_name, ::getpid());

            // Change the current working directory
            // chdir("/") to ensure that our process doesn't keep any directory
            // in use. Failure to do this could make it so that an administrator
            // couldn't unmount a file system, because it was our current directory.
            if (::chdir("/") < 0) // TODO: Log the failure
                return EXIT_FAILURE;

            // Close out the standard file descriptors
            close(STDIN_FILENO);
            close(STDOUT_FILENO);
            close(STDERR_FILENO);

            // Establish new open descriptors for stdin, stdout, and stderr. Even if
            // we don't plan to use them, it is still a good idea to have them open.
            int fd = open("/dev/null", O_RDWR); // stdin - file handle 0.
            // stdout - file handle 1.
            if (dup(fd) < 0)
                throw MiscCommon::system_error("Error occurred while duplicating stdout descriptor");
            // stderr - file handle 2.
            if (dup(fd) < 0)
                throw MiscCommon::system_error("Error occurred while duplicating stderr descriptor");

            CCommanderServer server;
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
