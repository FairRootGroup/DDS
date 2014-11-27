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
    : CConnectionImpl<CAgentChannel>(_service)
{
    m_mtxChannelReady.lock();
}

bool CAgentChannel::on_cmdREPLY_HANDSHAKE_OK(SCommandAttachmentImpl<cmdREPLY_HANDSHAKE_OK>::ptr_t _attachment)
{
    m_mtxChannelReady.unlock();
    m_cvChannelReady.notify_all();
    //    m_isHandShakeOK = true;
    return true;
}

bool CAgentChannel::on_cmdSIMPLE_MSG(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment)
{
    switch (_attachment->m_srcCommand)
    {
        case cmdUPDATE_KEY:
            LOG(static_cast<ELogSeverityLevel>(_attachment->m_msgSeverity)) << _attachment->m_sMsg;
            if (m_syncHelper == nullptr)
                throw invalid_argument("syncHelper is NULL");
            m_syncHelper->m_cvUpdateKey.notify_all();
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

void CAgentChannel::onRemoteEndDissconnected()
{
    sendYourself<cmdSHUTDOWN>();
}

bool CAgentChannel::on_cmdUPDATE_KEY(SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment)
{
    if (m_syncHelper == nullptr)
        throw invalid_argument("syncHelper is NULL");
    m_syncHelper->m_cvWaitKey.notify_all();
    return true;
}
