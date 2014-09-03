// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "TestChannel.h"

using namespace MiscCommon;
using namespace dds;
using namespace std;

bool CTestChannel::on_cmdREPLY_HANDSHAKE_OK(CProtocolMessage::protocolMessagePtr_t _msg)
{
    m_isHandShakeOK = true;

    pushMsg<cmdSTART_DOWNLOAD_TEST>();

    return true;
}

bool CTestChannel::on_cmdSIMPLE_MSG(CProtocolMessage::protocolMessagePtr_t _msg)
{
    SSimpleMsgCmd cmd;
    cmd.convertFromData(_msg->bodyToContainer());
    LOG(log_stdout) << cmd.m_sMsg;
    return true;
}

bool CTestChannel::on_cmdSHUTDOWN(CProtocolMessage::protocolMessagePtr_t _msg)
{
    stop();
    return true;
}
