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
    , m_isHandShakeOK(false)
{
}

bool CAgentChannel::on_cmdREPLY_HANDSHAKE_OK(SCommandAttachmentImpl<cmdREPLY_HANDSHAKE_OK>::ptr_t _attachment)
{
    m_isHandShakeOK = true;

    switch (m_cmdContainer.m_cmdType)
    {
        case cmdWAIT_FOR_KEY_UPDATE:
            LOG(info) << "wait for key update has been requested";
            syncPushMsg<cmdWAIT_FOR_KEY_UPDATE>();
            break;
        case cmdUPDATE_KEY:
            LOG(info) << "key update has been requested (key:value) " << m_cmdContainer.m_cmd.m_sKey << ":"
                      << m_cmdContainer.m_cmd.m_sValue;
            syncPushMsg<cmdUPDATE_KEY>(m_cmdContainer.m_cmd);
            sendYourself<cmdSHUTDOWN>(); // TODO: don't shutdown. Wait for response on cmdSIMPLE_MSG.
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

    // return false to let connection manager to catch this message as weel
    return false;
}

void CAgentChannel::onRemoteEndDissconnected()
{
    sendYourself<cmdSHUTDOWN>();
}
