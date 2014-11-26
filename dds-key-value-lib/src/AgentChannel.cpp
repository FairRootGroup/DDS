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
    , m_cmdContainer(nullptr)
    , m_isHandShakeOK(false)
{
}

bool CAgentChannel::on_cmdREPLY_HANDSHAKE_OK(SCommandAttachmentImpl<cmdREPLY_HANDSHAKE_OK>::ptr_t _attachment)
{
    m_isHandShakeOK = true;

    if (nullptr == m_cmdContainer)
        throw invalid_argument("Command container is NULL");

    switch (m_cmdContainer->m_cmdType)
    {
        case cmdWAIT_FOR_KEY_UPDATE:
            LOG(info) << "wait for key update has been requested";
            syncPushMsg<cmdWAIT_FOR_KEY_UPDATE>();
            break;
        case cmdUPDATE_KEY:
            LOG(info) << "key update has been requested (key:value) " << m_cmdContainer->m_cmd.m_sKey << ":"
                      << m_cmdContainer->m_cmd.m_sValue;
            syncPushMsg<cmdUPDATE_KEY>(m_cmdContainer->m_cmd);
            break;
        default:
            LOG(error) << "No command to execute";
            break;
    }

    return true;
}

bool CAgentChannel::on_cmdSIMPLE_MSG(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment)
{
    switch (_attachment->m_srcCommand)
    {
        case cmdUPDATE_KEY:
            LOG(static_cast<ELogSeverityLevel>(_attachment->m_msgSeverity)) << _attachment->m_sMsg;
            sendYourself<cmdSHUTDOWN>();
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
    if (nullptr == m_cmdContainer)
        throw invalid_argument("Command container is NULL");

    // Notify watchers if the key their were waiting is updated
    auto found = m_cmdContainer->m_sKeysToWait.find(_attachment->m_sKey);
    if (found != m_cmdContainer->m_sKeysToWait.end())
    {
        LOG(info) << "Unlocking user process as it is waiting for update notifications of key = "
                  << _attachment->m_sKey;
        m_cmdContainer->m_sUpdatedKey = _attachment->m_sKey;
        m_cmdContainer->m_cvKeyWait.notify_all();
    }
    return true;
}
