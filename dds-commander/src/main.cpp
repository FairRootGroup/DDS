// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
// DDS
#include "ConnectionManager.h"
#include "ErrorCode.h"
#include "INet.h"
#include "SessionIDFile.h"
#include "SysHelper.h"
// BOOST
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>

using namespace std;
using namespace MiscCommon;
using namespace dds;
using namespace dds::commander_cmd;
using namespace dds::user_defaults_api;
using boost::asio::ip::tcp;
namespace fs = boost::filesystem;

//=============================================================================
int createDirectories(const boost::uuids::uuid& _sid)
{
    // create sessions directories
    CUserDefaults::instance().reinit(_sid, CUserDefaults::instance().currentUDFile());
    vector<string> dirs;
    string sSandboxDir = CUserDefaults::instance().getValueForKey("server.sandbox_dir");
    if (!sSandboxDir.empty())
    {
        smart_append<string>(&sSandboxDir, '/');
        dirs.push_back(sSandboxDir + "wrk");
    }
    string sWrdDir = CUserDefaults::instance().getValueForKey("server.work_dir");
    smart_append<string>(&sWrdDir, '/');
    if (sSandboxDir.empty())
    {
        dirs.push_back(sWrdDir + "wrk");
    }
    dirs.push_back(sWrdDir + "etc");
    dirs.push_back(sWrdDir + "tmp");

    string sLogDir = CUserDefaults::instance().getValueForKey("server.log_dir");
    smart_append<string>(&sLogDir, '/');
    dirs.push_back(sLogDir);

    for (auto& dir : dirs)
    {
        smart_path(&dir);
        LOG(debug) << "Creating directory: " << dir;
        if (!fs::create_directories(dir))
        {
            LOG(fatal) << "Failed to create " << dir;
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

//=============================================================================
int main(int argc, char* argv[])
{
    CUserDefaults::instance(); // Initialize user defaults
    Logger::instance().init(); // Initialize log

    vector<string> arguments(argv + 1, argv + argc);
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

    // Checking for "prep-session" option
    if (SOptions_t::cmd_prep_session == options.m_Command)
    {
        // generate new session id
        CSessionIDFile sid;
        boost::uuids::uuid sessionid = sid.generate();
        LOG(info) << "GENERATED SESSION ID: " << sid.string();
        int ret(EXIT_SUCCESS);
        if ((ret = createDirectories(sessionid)) == EXIT_SUCCESS)
            cout << sid.string();

        return ret;
    }

    if (options.m_sid.is_nil())
    {
        LOG(log_stderr) << "Need a session ID to continue. Terminating...";
        return EXIT_FAILURE;
    }

    // Reinit UserDefaults and Log with new session ID
    CUserDefaults::instance().reinit(options.m_sid, CUserDefaults::instance().currentUDFile());
    Logger::instance().reinit();

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
        if (pid_to_kill > 0 && IsProcessRunning(pid_to_kill))
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
                if (!IsProcessRunning(pid_to_kill))
                {
                    cout << "\n";
                    break;
                }
                cout << ".";
                sleep(1); // sleeping for 1 second
                ++iter;
            }
            if (IsProcessRunning(pid_to_kill))
                LOG(log_stderr) << "FAILED to close the process.";
        }

        return EXIT_SUCCESS;
    }

    // Checking for "start" option
    if (SOptions_t::cmd_start == options.m_Command)
    {
        // a visual log marker for a new DDS session
        LOG(info) << "\n"
                  << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"
                  << "+++++++++ N E W  D D S  C O M M A N D E R  S E R V E R  S E S S I O N +++++++++\n"
                  << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++";
        // a parsable log marker for a new DDS session
        LOG(info) << "---> DDS commander session <---";

        try
        {
            CPIDFile pidfile(pidfile_name, ::getpid());
            CSessionIDFile sid(dds::user_defaults_api::CUserDefaults::instance().getMainSIDFileName());
            sid.lock(options.m_sid);
            if (sid.getLockedSID().empty())
            {
                LOG(fatal) << "Failed to create session ID. Stopping the session...";
                return EXIT_FAILURE;
            }
            LOG(info) << "NEW SESSION ID: " << sid.getLockedSID();
            shared_ptr<CConnectionManager> server = make_shared<CConnectionManager>(options);
            server->start();
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

    LOG(info) << "DDS commander server is Done. Bye, Bye!";

    return EXIT_SUCCESS;
}
