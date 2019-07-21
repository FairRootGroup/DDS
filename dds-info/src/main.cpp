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
void requestCommanderInfo(CSession& _session, const SOptions_t& _options)
{
    SCommanderInfoRequest::request_t requestInfo;
    SCommanderInfoRequest::ptr_t requestPtr = SCommanderInfoRequest::makeRequest(requestInfo);

    requestPtr->setMessageCallback([](const SMessageResponseData& _message) {
        LOG((_message.m_severity == dds::intercom_api::EMsgSeverity::error) ? log_stderr : log_stdout)
            << "Server reports: " << _message.m_msg;
    });

    requestPtr->setDoneCallback([&_session, &_options]() {
        // Stop only if we don't wait for idle agents
        if (_options.m_nIdleAgentsCount == 0)
            _session.stop();
    });

    requestPtr->setResponseCallback([&_session, &_options](const SCommanderInfoResponseData& _info) {
        if (_options.m_nIdleAgentsCount > 0)
        {
            if (_info.m_idleAgentsCount < _options.m_nIdleAgentsCount)
            {
                this_thread::sleep_for(chrono::milliseconds(500));
                requestCommanderInfo(_session, _options);
                return;
            }

            LOG(log_stdout_clean) << "idle agents online: " << _info.m_idleAgentsCount;
            _session.stop();
        }

        if (_options.m_bNeedCommanderPid)
        {
            LOG(log_stdout_clean) << _info.m_pid;
            _session.stop();
        }
        // Checking for "status" option
        if (_options.m_bNeedDDSStatus)
        {
            if (_info.m_pid > 0)
                LOG(log_stdout_clean) << "DDS commander server process (" << _info.m_pid << ") is running...";
            else
                LOG(log_stdout_clean) << "DDS commander server is not running.";

            _session.stop();
        }

        if (_options.m_bNeedActiveTopology)
        {
            if (_info.m_activeTopologyName.empty())
                LOG(log_stdout_clean) << "no active topology";
            else
                LOG(log_stdout_clean) << "active topology: " << _info.m_activeTopologyName;

            _session.stop();
        }
    });

    _session.sendRequest<SCommanderInfoRequest>(requestPtr);
}
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

        if (options.m_bNeedCommanderPid || options.m_bNeedDDSStatus || options.m_nIdleAgentsCount > 0 || options.m_bNeedActiveTopology)
        {
            requestCommanderInfo(session, options);
        }
        else if (options.m_bNeedAgentsNumber || options.m_bNeedAgentsList)
        {
            SAgentInfoRequest::request_t requestInfo;
            SAgentInfoRequest::ptr_t requestPtr = SAgentInfoRequest::makeRequest(requestInfo);

            requestPtr->setMessageCallback([](const SMessageResponseData& message) {
                LOG((message.m_severity == dds::intercom_api::EMsgSeverity::error) ? log_stderr : log_stdout)
                    << "Server reports: " << message.m_msg;
            });

            requestPtr->setDoneCallback([&session]() { session.stop(); });

            requestPtr->setResponseCallback([&session, &options](const SAgentInfoResponseData& _info) {
                if (options.m_bNeedAgentsNumber)
                {
                    LOG(log_stdout_clean) << _info.m_activeAgentsCount;
                    // Close communication channel
                    session.stop();
                    return;
                }

                if (options.m_bNeedAgentsList && !_info.m_agentInfo.empty())
                {
                    LOG(log_stdout_clean) << _info.m_agentInfo;
                }
                else
                    session.stop();
            });

            session.sendRequest<SAgentInfoRequest>(requestPtr);
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

        session.blockCurrentThread();
    }
    catch (exception& e)
    {
        LOG(log_stderr) << e.what();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
