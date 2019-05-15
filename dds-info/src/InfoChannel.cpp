// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "InfoChannel.h"
// STD
#include <chrono>

using namespace MiscCommon;
using namespace dds;
using namespace dds::info_cmd;
using namespace dds::protocol_api;
using namespace std;

CInfoChannel::CInfoChannel(boost::asio::io_context& _service, uint64_t _protocolHeaderID)
    : CClientChannelImpl<CInfoChannel>(_service, protocol_api::EChannelType::UI, _protocolHeaderID)
    , m_nCounter(0)
{
    registerHandler<protocol_api::EChannelEvents::OnHandshakeOK>([this](const protocol_api::SSenderInfo& _sender) {
        // ask the server what we wnated to ask :)
        if (m_options.m_bNeedCommanderPid || m_options.m_bNeedDDSStatus)
            pushMsg<protocol_api::cmdGED_PID>();
        else if (m_options.m_bNeedAgentsNumber || m_options.m_bNeedAgentsList)
            pushMsg<protocol_api::cmdGET_AGENTS_INFO>();
        else if (m_options.m_bNeedPropList)
            pushMsg<protocol_api::cmdGET_PROP_LIST>();
        else if (m_options.m_bNeedPropValues)
        {
            protocol_api::SGetPropValuesCmd cmd;
            cmd.m_sPropertyName = m_options.m_propertyName;
            pushMsg<protocol_api::cmdGET_PROP_VALUES>(cmd);
        }
        else if (m_options.m_nIdleAgentsCount > 0)
            pushMsg<protocol_api::cmdGET_IDLE_AGENTS_COUNT>();
    });

    registerHandler<protocol_api::EChannelEvents::OnConnected>(
        [](const protocol_api::SSenderInfo& _sender) { LOG(MiscCommon::info) << "Connected to the commander server"; });

    registerHandler<protocol_api::EChannelEvents::OnFailedToConnect>([](const protocol_api::SSenderInfo& _sender) {
        LOG(MiscCommon::log_stderr) << "Failed to connect to commander server.";
    });
}

bool CInfoChannel::on_cmdSIMPLE_MSG(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment,
                                    const protocol_api::SSenderInfo& _sender)
{
    if (_attachment->m_srcCommand == cmdGET_PROP_LIST || _attachment->m_srcCommand == cmdGET_PROP_VALUES)
    {
        if (!_attachment->m_sMsg.empty())
            LOG(log_stdout) << "\n" << _attachment->m_sMsg;
        stop();
        return true;
    }

    if (!_attachment->m_sMsg.empty())
        LOG(log_stdout) << "Server reports: " << _attachment->m_sMsg;

    return true;
}

bool CInfoChannel::on_cmdREPLY_PID(SCommandAttachmentImpl<cmdREPLY_PID>::ptr_t _attachment,
                                   const protocol_api::SSenderInfo& _sender)
{
    LOG(debug) << "UI agent has recieved pid of the commander server: " << _attachment->m_sMsg;
    if (m_options.m_bNeedCommanderPid)
        LOG(log_stdout_clean) << _attachment->m_sMsg;

    // Checking for "status" option
    if (m_options.m_bNeedDDSStatus)
    {
        if (!_attachment->m_sMsg.empty())
            LOG(log_stdout_clean) << "DDS commander server process (" << _attachment->m_sMsg << ") is running...";
        else
            LOG(log_stdout_clean) << "DDS commander server is not running.";
    }

    // Close communication channel
    stop();
    return true;
}

bool CInfoChannel::on_cmdREPLY_AGENTS_INFO(SCommandAttachmentImpl<cmdREPLY_AGENTS_INFO>::ptr_t _attachment,
                                           const protocol_api::SSenderInfo& _sender)
{
    LOG(debug) << "UI agent has recieved Agents Info from the commander server.";
    if (m_options.m_bNeedAgentsNumber)
    {
        LOG(log_stdout_clean) << _attachment->m_nActiveAgents;
        // Close communication channel
        stop();
    }

    if (m_options.m_bNeedAgentsList && !_attachment->m_sAgentInfo.empty())
    {
        std::lock_guard<std::mutex> lock(m_mutexCounter);
        ++m_nCounter;
        LOG(log_stdout_clean) << _attachment->m_sAgentInfo;

        if (m_nCounter == _attachment->m_nActiveAgents)
            stop();
    }
    else
        stop();

    return true;
}

bool CInfoChannel::on_cmdREPLY_IDLE_AGENT_COUNT(
    protocol_api::SCommandAttachmentImpl<protocol_api::cmdREPLY_IDLE_AGENTS_COUNT>::ptr_t _attachment,
    const protocol_api::SSenderInfo& _sender)
{
    LOG(debug) << "UI agent has recieved Idle Agents Count from the commander server.";

    uint nIdleAgentsoCunt(0);
    try
    {
        nIdleAgentsoCunt = stoi(_attachment->m_sMsg);
    }
    catch (...)
    {
    }

    if (nIdleAgentsoCunt < m_options.m_nIdleAgentsCount)
    {
        this_thread::sleep_for(chrono::milliseconds(500));
        pushMsg<protocol_api::cmdGET_IDLE_AGENTS_COUNT>();
        return true;
    }

    LOG(log_stdout_clean) << "idle agents online: " << nIdleAgentsoCunt;
    stop();

    return true;
}
