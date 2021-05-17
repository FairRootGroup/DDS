// Copyright 2018 GSI, Inc. All rights reserved.
//
//
//
// DDS
#include "Stop.h"
#include "DDSHelper.h"
#include "Process.h"
#include "UserDefaults.h"
// STD
#include <chrono>
// BOOST
#include <boost/filesystem.hpp>

using namespace std;
using namespace dds::session_cmd;
using namespace dds::user_defaults_api;
using namespace MiscCommon;
namespace fs = boost::filesystem;

void CStop::stop(const std::string& _sessionID)
{
    LOG(log_stdout_clean) << "Stopping DDS commander: " << _sessionID;
    // Reinit UserDefaults and Log with new session ID
    CUserDefaults::instance().reinit(boost::uuids::string_generator()(_sessionID),
                                     CUserDefaults::instance().currentUDFile());
    Logger::instance().reinit();

    ifstream fPid(CUserDefaults::instance().getCommanderPidFile());
    if (!fPid.is_open())
    {
        LOG(log_stdout_clean) << "Can't find commander with session: " << _sessionID;
        return;
    }

    pid_t pidCommander(0);
    fPid >> pidCommander;
    fPid.close();

    if (!MiscCommon::IsProcessRunning(pidCommander))
    {
        LOG(log_stdout_clean) << "Can't find commander with pid: " << pidCommander;
        return;
    }

    LOG(log_stdout_clean) << "Sending a graceful stop signal to Commander (pid/sessionID): " << pidCommander << "/"
                          << _sessionID;
    string sOut;
    string sErr;
    int nExitCode(0);

    stringstream ssCmd;
    ssCmd << boost::process::search_path("dds-commander").string() << " --session " << _sessionID << " stop";
    try
    {
        execute(ssCmd.str(), std::chrono::seconds(50), &sOut, &sErr, &nExitCode);
        LOG(log_stdout_clean) << sOut;
    }
    catch (exception& _e)
    {
        LOG(log_stderr) << _e.what();
    }

    if (MiscCommon::IsProcessRunning(pidCommander))
    {
        LOG(log_stdout_clean) << "Commander is still running. I have no choice but to kill it...";
        // Manually killing the process
        if (MiscCommon::IsProcessRunning(pidCommander))
        {
            if (::kill(pidCommander, SIGKILL))
            {
                string err;
                errno2str(&err);
                LOG(MiscCommon::error) << "Failed to kill commander: " << err;
            }
        }
    }
}
