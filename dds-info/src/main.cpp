// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
// DDS
#include "BOOSTHelper.h"
#include "DDSHelper.h"
#include "ErrorCode.h"
#include "Options.h"
#include "Process.h"
#include "SysHelper.h"
#include "Tools.h"
#include "UserDefaults.h"

using namespace std;
using namespace MiscCommon;
using namespace dds;
using namespace dds::info_cmd;
using namespace dds::user_defaults_api;
using namespace dds::tools_api;

//=============================================================================
int main(int argc, char* argv[])
{

    // Command line parser
    SOptions_t options;

    try
    {
        CUserDefaults::instance(); // Initialize user defaults
        Logger::instance().init(); // Initialize log

        vector<std::string> arguments(argv + 1, argv + argc);
        ostringstream ss;
        copy(arguments.begin(), arguments.end(), ostream_iterator<string>(ss, " "));
        LOG(info) << "Starting with arguments: " << ss.str();

        if (!ParseCmdLine(argc, argv, &options))
            return EXIT_SUCCESS;

        // Reinit UserDefaults and Log with new session ID
        CUserDefaults::instance().reinit(options.m_sid, CUserDefaults::instance().currentUDFile());
        Logger::instance().reinit();
    }
    catch (exception& e)
    {
        LOG(log_stderr) << e.what();
        return EXIT_FAILURE;
    }

    string sid;
    try
    {
        sid = CUserDefaults::instance().getLockedSID();
    }
    catch (...)
    {
        LOG(log_stderr) << g_cszDDSServerIsNotFound_StartIt << endl;
        return EXIT_FAILURE;
    }

    try
    {
        CSession session;
        session.attach(sid);

        session.onResponse<SMessage>([&session](const SMessage& _message) {
            LOG((_message.m_severity == dds::intercom_api::EMsgSeverity::error) ? log_stderr : log_stdout)
                << "Server reports: " << _message.m_msg;

            // stop communication on errors
            if (_message.m_severity == dds::intercom_api::EMsgSeverity::error)
                session.stop();
            return true;
        });

        session.onResponse<SDone>([&session](const SDone& _message) {
            session.stop();
            return true;
        });

        session.onResponse<SCommanderInfo>([&session, &options](const SCommanderInfo& _info) {
            if (options.m_nIdleAgentsCount > 0)
            {
                if (_info.m_idleAgentsCount < options.m_nIdleAgentsCount)
                {
                    this_thread::sleep_for(chrono::milliseconds(500));
                    SCommanderInfo commanderInfo;
                    session.sendRequest(commanderInfo);
                    return true;
                }

                LOG(log_stdout_clean) << "idle agents online: " << _info.m_idleAgentsCount;
                session.stop();
            }

            if (options.m_bNeedCommanderPid)
                LOG(log_stdout_clean) << _info.m_pid;

            // Checking for "status" option
            if (options.m_bNeedDDSStatus)
            {
                if (_info.m_pid > 0)
                    LOG(log_stdout_clean) << "DDS commander server process (" << _info.m_pid << ") is running...";
                else
                    LOG(log_stdout_clean) << "DDS commander server is not running.";
            }

            session.stop();
            return true;
        });

        session.onResponse<SAgentInfo>([&session, &options](const SAgentInfo& _info) {
            if (options.m_bNeedAgentsNumber)
            {
                LOG(log_stdout_clean) << _info.m_activeAgentsCount;
                // Close communication channel
                session.stop();
                return true;
            }

            if (options.m_bNeedAgentsList && !_info.m_agentInfo.empty())
            {
                LOG(log_stdout_clean) << _info.m_agentInfo;
            }
            else
                session.stop();

            return true;
        });

        if (options.m_bNeedCommanderPid || options.m_bNeedDDSStatus)
        {
            SCommanderInfo commanderInfo;
            session.sendRequest(commanderInfo);
        }
        else if (options.m_bNeedAgentsNumber || options.m_bNeedAgentsList)
        {
            SAgentInfo agentInfo;
            session.sendRequest(agentInfo);
        }
        else if (options.m_bNeedPropList)
        {
            // TODO: NOT Implemented
            LOG(log_stderr) << "The feature is disiabled for this version";
            // pushMsg<protocol_api::cmdGET_PROP_LIST>();
        }
        else if (options.m_bNeedPropValues)
        {
            // TODO: NOT Implemented
            LOG(log_stderr) << "The feature is disiabled for this version";
            // protocol_api::SGetPropValuesCmd cmd;
            // cmd.m_sPropertyName = m_options.m_propertyName;
            // pushMsg<protocol_api::cmdGET_PROP_VALUES>(cmd);
        }
        else if (options.m_nIdleAgentsCount > 0)
        {
            SCommanderInfo commanderInfo;
            session.sendRequest(commanderInfo);
        }

        session.blockCurrentThread();
    }
    catch (exception& e)
    {
        LOG(log_stderr) << e.what();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
