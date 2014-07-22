// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
// DDS
#include "Options.h"
#include "Process.h"
#include "ErrorCode.h"
#include "ConnectionManager.h"
#include "BOOSTHelper.h"
#include "UserDefaults.h"
#include "SysHelper.h"
#include "SendCommandToItself.h"
#include "INet.h"
// BOOST
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/asio.hpp>

using namespace std;
using namespace MiscCommon;
namespace bpo = boost::program_options;
using namespace dds;
using boost::asio::ip::tcp;

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
        LOG(log_stderr) << e.what();
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
            LOG(log_stdout) << " process (" << pid << ") is running...";
        }
        else
        {
            LOG(log_stdout) << " is not running...";
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
            // get a free port from a given range
            int nSrvPort = MiscCommon::INet::get_free_port(options.m_userDefaults.getOptions().m_general.m_ddsCommanderPortRangeMin,
                                                           options.m_userDefaults.getOptions().m_general.m_ddsCommanderPortRangeMax);

            tcp::endpoint endpoint(tcp::v4(), nSrvPort);

            CConnectionManager server(options, io_service, endpoint);
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
        try
        {
            // Read server info file
            const string sSrvCfg(CUserDefaults::getServerInfoFile());
            LOG(info) << "Reading server info from: " << sSrvCfg;
            if (sSrvCfg.empty())
                throw runtime_error("Can't find server info file.");

            boost::property_tree::ptree pt;
            boost::property_tree::ini_parser::read_ini(sSrvCfg, pt);
            const string sHost(pt.get<string>("server.host"));
            const string sPort(pt.get<string>("server.port"));

            LOG(log_stdout) << "Contacting DDS commander on " << sHost << ":" << sPort;

            boost::asio::io_service io_service;

            tcp::resolver resolver(io_service);
            tcp::resolver::query query(sHost, sPort);
            tcp::resolver::iterator iterator = resolver.resolve(query);

            //            boost::thread t(boost::bind(&boost::asio::io_service::run, &io_service));

            CSendCommandToItself cmd(io_service, iterator);
            io_service.run();
        }
        catch (exception& e)
        {
            LOG(log_stderr) << e.what();
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
