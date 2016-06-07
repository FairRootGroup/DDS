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

CAgentChannel::CAgentChannel(boost::asio::io_service& _service)
    : CClientChannelImpl<CAgentChannel>(_service, EChannelType::UNKNOWN)
    , m_syncHelper(nullptr)
    , m_connectionAttempts(1)
{
    subscribeOnEvent(EChannelEvents::OnRemoteEndDissconnected, [this](CAgentChannel* _channel) {
        LOG(info) << "DDS commander server has suddenly dropped the connection. Sending yourself a shutdown signal...";
        this->sendYourself<cmdSHUTDOWN>();
    });

    subscribeOnEvent(protocol_api::EChannelEvents::OnConnected,
                     [this](CAgentChannel* _channel) { LOG(info) << "Connected to the commander server"; });

    subscribeOnEvent(protocol_api::EChannelEvents::OnFailedToConnect,
                     [this](CAgentChannel* _channel) { LOG(log_stderr) << "Failed to connect to commander server."; });
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

bool CAgentChannel::on_cmdSIMPLE_MSG(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment)
{
    switch (_attachment->m_srcCommand)
    {
        case cmdCUSTOM_CMD:
            LOG(static_cast<ELogSeverityLevel>(_attachment->m_msgSeverity)) << _attachment->m_sMsg;
            if (m_syncHelper == nullptr)
                throw invalid_argument("syncHelper is NULL");

            // Call user callback function
            m_syncHelper->m_customCmdReplySignal(_attachment->m_sMsg);
            break;

        case cmdUPDATE_KEY:
            LOG(static_cast<ELogSeverityLevel>(_attachment->m_msgSeverity)) << _attachment->m_sMsg;
            if (m_syncHelper == nullptr)
                throw invalid_argument("syncHelper is NULL");
            // m_syncHelper->m_cvUpdateKey.notify_all();
            if (_attachment->m_msgSeverity == MiscCommon::error)
            {
                if (m_syncHelper == nullptr)
                    throw invalid_argument("syncHelper is NULL");
                m_syncHelper->m_errorSignal(intercom_api::EErrorCode::UpdateKeyValueFailed, _attachment->m_sMsg);
            }
            break;

        default:
            LOG(debug) << "Received command cmdSIMPLE_MSG does not have a listener";
            return true;
    }

    return true;
}

bool CAgentChannel::on_cmdSHUTDOWN(SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment)
{
    LOG(info) << "api-guard connection channel exited.";
    stop();
    return false;
}

bool CAgentChannel::on_cmdCUSTOM_CMD(SCommandAttachmentImpl<cmdCUSTOM_CMD>::ptr_t _attachment)
{
    if (m_syncHelper == nullptr)
        throw invalid_argument("syncHelper is NULL");

    LOG(info) << "CAgentChannel::on_cmdCUSTOM_CMD: received custom command: " << *_attachment;

    // Call user callback function
    m_syncHelper->m_customCmdSignal(_attachment->m_sCmd, _attachment->m_sCondition, _attachment->m_senderId);
    return true;
}

bool CAgentChannel::on_cmdUPDATE_KEY(SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment)
{
    if (m_syncHelper == nullptr)
        throw invalid_argument("syncHelper is NULL");
    m_syncHelper->m_keyValueUpdateSig(_attachment->m_sKey, _attachment->m_sValue);
    return true;
}