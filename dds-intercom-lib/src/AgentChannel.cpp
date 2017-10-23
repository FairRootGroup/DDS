// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "AgentChannel.h"
#include "DDSIntercomGuard.h"
#include "version.h"

using namespace MiscCommon;
using namespace dds;
using namespace dds::internal_api;
using namespace dds::protocol_api;
using namespace std;

const uint16_t g_MaxConnectionAttempts = 12;

CAgentChannel::CAgentChannel(boost::asio::io_service& _service, uint64_t _protocolHeaderID)
    : CClientChannelImpl<CAgentChannel>(_service, EChannelType::UNKNOWN, _protocolHeaderID)
    , m_syncHelper(nullptr)
    , m_connectionAttempts(1)
{
    registerHandler<EChannelEvents::OnRemoteEndDissconnected>([this](const SSenderInfo& _sender) {
        LOG(info) << "DDS commander server has suddenly dropped the connection. Sending yourself a shutdown signal...";
        this->sendYourself<cmdSHUTDOWN>();
    });

    registerHandler<protocol_api::EChannelEvents::OnConnected>(
        [](const SSenderInfo& _sender) { LOG(MiscCommon::info) << "Connected to the commander server"; });

    registerHandler<protocol_api::EChannelEvents::OnFailedToConnect>(
        [](const SSenderInfo& _sender) { LOG(MiscCommon::log_stderr) << "Failed to connect to commander server."; });
}

void CAgentChannel::reconnectAgentWithErrorHandler(const function<void(const string&)>& callback)
{
    if (m_connectionAttempts <= g_MaxConnectionAttempts)
    {
        LOG(log_stderr) << "Failed to connect to commander server. Trying to reconnect. Attempt "
                        << m_connectionAttempts << " out of " << g_MaxConnectionAttempts;
        this_thread::sleep_for(chrono::seconds(10));
        reconnect();
        ++m_connectionAttempts;
    }
    else
    {
        string errorMsg("Failed to connect to commander server. Sending yourself a shutdown command.");
        LOG(log_stderr) << errorMsg;
        this->sendYourself<cmdSHUTDOWN>();

        callback(errorMsg);
    }
}

bool CAgentChannel::on_cmdSIMPLE_MSG(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment,
                                     const protocol_api::SSenderInfo& _sender)
{
    switch (_attachment->m_srcCommand)
    {
        case cmdCUSTOM_CMD:
            LOG(static_cast<ELogSeverityLevel>(_attachment->m_msgSeverity)) << _attachment->m_sMsg;

            // Call user callback function
            CDDSIntercomGuard::instance().m_customCmdReplySignal(_attachment->m_sMsg);
            break;

        default:
            LOG(debug) << "Received command cmdSIMPLE_MSG does not have a listener";
            return true;
    }

    return true;
}

bool CAgentChannel::on_cmdSHUTDOWN(SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment,
                                   const protocol_api::SSenderInfo& _sender)
{
    LOG(info) << "api-guard connection channel exited.";
    stop();
    return false;
}

bool CAgentChannel::on_cmdCUSTOM_CMD(SCommandAttachmentImpl<cmdCUSTOM_CMD>::ptr_t _attachment,
                                     const protocol_api::SSenderInfo& _sender)
{
    LOG(info) << "CAgentChannel::on_cmdCUSTOM_CMD: received custom command: " << *_attachment;

    // Call user callback function
    CDDSIntercomGuard::instance().m_customCmdSignal(
        _attachment->m_sCmd, _attachment->m_sCondition, _attachment->m_senderId);
    return true;
}
