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
#include "AgentClient.h"
#include "BOOSTHelper.h"
#include "Logger.h"

using namespace std;
using namespace MiscCommon;
namespace bpo = boost::program_options;
using namespace dds::commander;

void PrintVersion()
{
    // TODO: make VERSION to be taken from the build
    cout << PROJECT_NAME << " v" << PROJECT_VERSION_STRING << "\n";
    //         << "protocol: v" << g_protocolCommandsVersion << "\n"
    //         << "Report bugs/comments to A.Manafov@gsi.de" << endl;
}

// Command line parser
bool ParseCmdLine(int _argc, char* _argv[], SOptions* _options) throw(exception)
{
    if (nullptr == _options)
        throw runtime_error("Internal error: options' container is empty.");

    // Generic options
    bpo::options_description options("dds-agent options");
    options.add_options()("help,h", "Produce help message")("version,v", "Version information")("start", "Start dds-agent")("stop", "Stop dds-agent")(
        "status", "Query current status of dds-agent daemon");

    // Parsing command-line
    bpo::variables_map vm;
    bpo::store(bpo::parse_command_line(_argc, _argv, options), vm);
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

    MiscCommon::BOOSTHelper::conflicting_options(vm, "start", "stop");
    MiscCommon::BOOSTHelper::conflicting_options(vm, "start", "status");
    MiscCommon::BOOSTHelper::conflicting_options(vm, "stop", "status");

    if (vm.count("start"))
        _options->m_Command = SOptions_t::Start;
    if (vm.count("stop"))
        _options->m_Command = SOptions_t::Stop;
    if (vm.count("status"))
        _options->m_Command = SOptions_t::Status;

    return true;
}

int main(int argc, char* argv[])
{
    Logger::instance().init("dds_adent.log");

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
    if (options.m_Command == SOptions_t::Status)
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
    if (SOptions_t::Stop == options.m_Command)
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
    if (SOptions_t::Start == options.m_Command)
    {
        try
        {
            CPIDFile pidfile(pidfile_name, ::getpid());

            CAgentClient agent;
            agent.start();

            // Main loop
            /*   while (1)
               {
                   sleep(30); // wait 30 seconds
                   cout << "running..." << endl;
               }*/
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
