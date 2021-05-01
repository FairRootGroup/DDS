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
// STD
#include <thread>

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

    requestPtr->setDoneCallback([&_session]() { _session.unblockCurrentThread(); });

    requestPtr->setResponseCallback([&_session, &_options](const SCommanderInfoResponseData& _info) {
        if (_options.m_bNeedCommanderPid)
        {
            LOG(log_stdout_clean) << _info.m_pid;
            _session.unblockCurrentThread();
        }
        // Checking for "status" option
        if (_options.m_bNeedDDSStatus)
        {
            if (_info.m_pid > 0)
                LOG(log_stdout_clean) << "DDS commander server process (" << _info.m_pid << ") is running...";
            else
                LOG(log_stdout_clean) << "DDS commander server is not running.";

            _session.unblockCurrentThread();
        }

        if (_options.m_bNeedActiveTopology)
        {
            if (_info.m_activeTopologyName.empty())
                LOG(log_stdout_clean) << "no active topology";
            else
                LOG(log_stdout_clean) << "active topology: " << _info.m_activeTopologyName
                                      << "; path: " << _info.m_activeTopologyPath;

            _session.unblockCurrentThread();
        }
    });

    _session.sendRequest<SCommanderInfoRequest>(requestPtr);
}

void requestAgentInfo(CSession& _session, const SOptions_t& /*_options*/)
{
    SAgentInfoRequest::request_t requestInfo;
    SAgentInfoRequest::ptr_t requestPtr = SAgentInfoRequest::makeRequest(requestInfo);

    requestPtr->setMessageCallback([](const SMessageResponseData& message) {
        LOG((message.m_severity == dds::intercom_api::EMsgSeverity::error) ? log_stderr : log_stdout)
            << "Server reports: " << message.m_msg;
    });

    requestPtr->setDoneCallback([&_session]() { _session.unblockCurrentThread(); });

    requestPtr->setResponseCallback([](const SAgentInfoResponseData& _info) {
        LOG(log_stdout_clean) << " -------------->>> " << _info.m_agentID << "\nHost Info: " << _info.m_username << "@"
                              << _info.m_host << ":" << _info.m_DDSPath << "\nAgent pid: " << _info.m_agentPid
                              << "\nAgent startup time: " << chrono::duration<double>(_info.m_startUpTime).count()
                              << " s"
                              << "\nSlots: " << _info.m_nSlots << "\n";
    });

    _session.sendRequest<SAgentInfoRequest>(requestPtr);
}

void requestAgentCount(CSession& _session, const SOptions_t& _options)
{
    SAgentCountRequest::request_t requestInfo;
    SAgentCountRequest::ptr_t requestPtr = SAgentCountRequest::makeRequest(requestInfo);

    requestPtr->setMessageCallback([](const SMessageResponseData& message) {
        LOG((message.m_severity == dds::intercom_api::EMsgSeverity::error) ? log_stderr : log_stdout)
            << "Server reports: " << message.m_msg;
    });

    requestPtr->setDoneCallback([&_session, &_options]() {
        // Stop only if we don't need to wait for the required number of agents
        if (_options.m_nWaitCount == 0)
            _session.unblockCurrentThread();
    });

    requestPtr->setResponseCallback([&_session, &_options](const SAgentCountResponseData& _info) {
        bool needToWait = _options.m_nWaitCount > 0;
        if (needToWait)
        {
            // Check if we have the required number of agents
            if ((_options.m_bNeedActiveCount && (_info.m_activeSlotsCount < _options.m_nWaitCount)) ||
                (_options.m_bNeedIdleCount && (_info.m_idleSlotsCount < _options.m_nWaitCount)) ||
                (_options.m_bNeedExecutingCount && (_info.m_executingSlotsCount < _options.m_nWaitCount)))
            {
                this_thread::sleep_for(chrono::milliseconds(500));
                requestAgentCount(_session, _options);
                return;
            }

            LOG(log_stdout_clean) << "Active agents online: " + to_string(_info.m_activeSlotsCount) << endl
                                  << "Idle agents online: " + to_string(_info.m_idleSlotsCount) << endl
                                  << "Executing agents online: " + to_string(_info.m_executingSlotsCount);
            _session.unblockCurrentThread();
        }
        else
        {
            if (_options.m_bNeedActiveCount)
                LOG(log_stdout_clean) << _info.m_activeSlotsCount;
            else if (_options.m_bNeedIdleCount)
                LOG(log_stdout_clean) << _info.m_idleSlotsCount;
            else if (_options.m_bNeedExecutingCount)
                LOG(log_stdout_clean) << _info.m_executingSlotsCount;

            _session.unblockCurrentThread();
        }
    });

    _session.sendRequest<SAgentCountRequest>(requestPtr);
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

        if (options.m_bNeedCommanderPid || options.m_bNeedDDSStatus || options.m_bNeedActiveTopology)
        {
            requestCommanderInfo(session, options);
        }
        else if (options.m_bNeedAgentsList)
        {
            requestAgentInfo(session, options);
        }
        else if (options.m_bNeedActiveCount || options.m_bNeedIdleCount || options.m_bNeedExecutingCount)
        {
            requestAgentCount(session, options);
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
