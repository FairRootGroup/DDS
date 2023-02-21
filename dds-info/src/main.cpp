// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
// DDS
#include "BoostHelper.h"
#include "DDSHelper.h"
#include "ErrorCode.h"
#include "MiscSetup.h"
#include "Options.h"
#include "Res.h"
#include "SysHelper.h"
#include "Tools.h"
#include "UserDefaults.h"
// STD
#include <thread>

using namespace std;
using namespace dds;
using namespace dds::info_cmd;
using namespace dds::user_defaults_api;
using namespace dds::tools_api;
using namespace dds::misc;

//=============================================================================
void requestCommanderInfo(CSession& _session, const SOptions_t& _options)
{
    SCommanderInfoRequest::request_t requestInfo;
    SCommanderInfoRequest::ptr_t requestPtr = SCommanderInfoRequest::makeRequest(requestInfo);

    requestPtr->setMessageCallback(
        [](const SMessageResponseData& _message)
        {
            LOG((_message.m_severity == dds::intercom_api::EMsgSeverity::error) ? log_stderr : log_stdout)
                << "Server reports: " << _message.m_msg;
        });

    requestPtr->setDoneCallback([&_session]() { _session.unblockCurrentThread(); });

    requestPtr->setResponseCallback(
        [&_session, &_options](const SCommanderInfoResponseData& _info)
        {
            if (_options.m_bNeedCommanderPid)
            {
                LOG(log_stdout_clean) << _info.m_pid;
                _session.unblockCurrentThread();
            }
            // Checking for "status" option
            if (_options.m_bNeedStatus)
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

    requestPtr->setMessageCallback(
        [](const SMessageResponseData& message)
        {
            LOG((message.m_severity == dds::intercom_api::EMsgSeverity::error) ? log_stderr : log_stdout)
                << "Server reports: " << message.m_msg;
        });

    requestPtr->setDoneCallback([&_session]() { _session.unblockCurrentThread(); });

    requestPtr->setResponseCallback(
        [](const SAgentInfoResponseData& _info)
        {
            LOG(log_stdout_clean) << "Agent " << _info.m_index << ": id (" << _info.m_agentID << "), pid ("
                                  << _info.m_agentPid << "),"
                                  << " group name (" << _info.m_groupName << "),"
                                  << " startup time (" << chrono::duration<double>(_info.m_startUpTime).count()
                                  << "s), slots total/executing/idle (" << _info.m_nSlots << "/"
                                  << _info.m_nExecutingSlots << "/" << _info.m_nIdleSlots << "), host ("
                                  << _info.m_username << "@" << _info.m_host << "), wrkDir (" << quoted(_info.m_DDSPath)
                                  << ")";
        });

    _session.sendRequest<SAgentInfoRequest>(requestPtr);
}

void requestSlotInfo(CSession& _session, const SOptions_t& /*_options*/)
{
    SSlotInfoRequest::request_t requestInfo;
    SSlotInfoRequest::ptr_t requestPtr{ SSlotInfoRequest::makeRequest(requestInfo) };

    requestPtr->setMessageCallback(
        [](const SMessageResponseData& message)
        {
            LOG((message.m_severity == dds::intercom_api::EMsgSeverity::error) ? log_stderr : log_stdout)
                << "Server reports: " << message.m_msg;
        });

    requestPtr->setDoneCallback([&_session]() { _session.unblockCurrentThread(); });

    requestPtr->setResponseCallback(
        [](const SSlotInfoResponseData& _info)
        {
            LOG(log_stdout_clean) << "Slot " << _info.m_index << ": agentID (" << _info.m_agentID << "), slotID ("
                                  << _info.m_slotID << "), taskID (" << _info.m_taskID << "), state (" << _info.m_state
                                  << "), host (" << _info.m_host << "), wrkDir (" << quoted(_info.m_wrkDir) << ")";
        });

    _session.sendRequest<SSlotInfoRequest>(requestPtr);
}

void requestAgentCount(CSession& _session, const SOptions_t& _options)
{
    SAgentCountRequest::request_t requestInfo;
    SAgentCountRequest::ptr_t requestPtr = SAgentCountRequest::makeRequest(requestInfo);

    requestPtr->setMessageCallback(
        [](const SMessageResponseData& message)
        {
            LOG((message.m_severity == dds::intercom_api::EMsgSeverity::error) ? log_stderr : log_stdout)
                << "Server reports: " << message.m_msg;
        });

    requestPtr->setDoneCallback(
        [&_session, &_options]()
        {
            // Stop only if we don't need to wait for the required number of agents
            if (_options.m_nWaitCount == 0)
                _session.unblockCurrentThread();
        });

    requestPtr->setResponseCallback(
        [&_session, &_options](const SAgentCountResponseData& _info)
        {
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
    SOptions_t options;
    if (dds::misc::defaultExecSetup<SOptions_t>(argc, argv, &options, &ParseCmdLine) == EXIT_FAILURE)
        return EXIT_FAILURE;
    if (dds::misc::defaultExecReinit(options.m_sid) == EXIT_FAILURE)
        return EXIT_FAILURE;

    if (options.m_bHelp || options.m_bVersion) // Nothing to do, exiting.
        return EXIT_SUCCESS;

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

        if (options.m_bNeedCommanderPid || options.m_bNeedStatus || options.m_bNeedActiveTopology)
        {
            requestCommanderInfo(session, options);
        }
        else if (options.m_bNeedAgentList)
        {
            requestAgentInfo(session, options);
        }
        else if (options.m_bNeedSlotList)
        {
            requestSlotInfo(session, options);
        }
        else if (options.m_bNeedActiveCount || options.m_bNeedIdleCount || options.m_bNeedExecutingCount)
        {
            requestAgentCount(session, options);
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
