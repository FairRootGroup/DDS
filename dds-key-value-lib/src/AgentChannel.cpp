// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "version.h"
#include "AgentChannel.h"
#include "BOOST_FILESYSTEM.h"
#include "KeyValueGuard.h"
// MiscCommon
#include "FindCfgFile.h"

using namespace MiscCommon;
using namespace dds;
using namespace std;

CAgentChannel::CAgentChannel(boost::asio::io_service& _service)
    : CClientChannelImpl<CAgentChannel>(_service, EChannelType::KEY_VALUE_GUARD)
{
    subscribeOnEvent(
        EChannelEvents::OnRemoteEndDissconnected,
        [this](CAgentChannel* _channel)
        {
            LOG(info)
                << "DDS commander server has suddenly dropped the connection. Sending yourself a shutdown signal...";
            this->sendYourself<cmdSHUTDOWN>();
        });
}

bool CAgentChannel::on_cmdSIMPLE_MSG(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment)
{
    switch (_attachment->m_srcCommand)
    {
        case cmdUPDATE_KEY:
            LOG(static_cast<ELogSeverityLevel>(_attachment->m_msgSeverity)) << _attachment->m_sMsg;
            if (m_syncHelper == nullptr)
                throw invalid_argument("syncHelper is NULL");
            // m_syncHelper->m_cvUpdateKey.notify_all();
            break;
        default:
            LOG(debug) << "Received command cmdSIMPLE_MSG does not have a listener";
            return true;
    }

    return true;
}

bool CAgentChannel::on_cmdDISCONNECT(SCommandAttachmentImpl<cmdDISCONNECT>::ptr_t _attachment)
{
    LOG(info) << "key-value-gurad connection channel disconnected... Bye";
    sendYourself<cmdSHUTDOWN>();

    return true;
}

bool CAgentChannel::on_cmdSHUTDOWN(SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment)
{
    LOG(info) << "key-value-gurad connection channel exited.";
    stop();
    // return false to let connection manager to catch this message as weel
    return false;
}

bool CAgentChannel::on_cmdUPDATE_KEY(SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment)
{
    if (m_syncHelper == nullptr)
        throw invalid_argument("syncHelper is NULL");
    m_syncHelper->m_updateSig(_attachment->m_sKey, _attachment->m_sValue);
    return true;
}
