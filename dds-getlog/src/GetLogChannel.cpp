// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "GetLogChannel.h"

using namespace MiscCommon;
using namespace dds;
using namespace std;

bool CGetLogChannel::on_cmdREPLY_HANDSHAKE_OK(CProtocolMessage::protocolMessagePtr_t _msg)
{
    m_isHandShakeOK = true;

    pushMsg<cmdGET_LOG>();

    return true;
}

bool CGetLogChannel::on_cmdSIMPLE_MSG(CProtocolMessage::protocolMessagePtr_t _msg)
{
    SSimpleMsgCmd cmd;
    cmd.convertFromData(_msg->bodyToContainer());
    LOG(log_stdout) << cmd.m_sMsg;
    return true;
}

bool CGetLogChannel::on_cmdSHUTDOWN(CProtocolMessage::protocolMessagePtr_t _msg)
{
    stop();
    return true;
}
