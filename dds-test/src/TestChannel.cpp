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

bool CTestChannel::on_cmdDOWNLOAD_TEST_RECIEVED(CProtocolMessage::protocolMessagePtr_t _msg)
{
    SSimpleMsgCmd recieved_cmd;
    recieved_cmd.convertFromData(_msg->bodyToContainer());
    LOG(log_stdout) << recieved_cmd.m_sMsg;

    return true;
}

bool CTestChannel::on_cmdALL_DOWNLOAD_TESTS_RECIEVED(CProtocolMessage::protocolMessagePtr_t _msg)
{
    SSimpleMsgCmd recieved_cmd;
    recieved_cmd.convertFromData(_msg->bodyToContainer());
    LOG(log_stdout) << recieved_cmd.m_sMsg;

    stop();
    exit(EXIT_SUCCESS);
}

bool CTestChannel::on_cmdDOWNLOAD_TEST_ERROR(CProtocolMessage::protocolMessagePtr_t _msg)
{
    SSimpleMsgCmd recieved_cmd;
    recieved_cmd.convertFromData(_msg->bodyToContainer());
    LOG(log_stdout) << recieved_cmd.m_sMsg;

    return true;
}

bool CTestChannel::on_cmdDOWNLOAD_TEST_FATAL(CProtocolMessage::protocolMessagePtr_t _msg)
{
    SSimpleMsgCmd recieved_cmd;
    recieved_cmd.convertFromData(_msg->bodyToContainer());
    LOG(log_stdout) << recieved_cmd.m_sMsg;

    stop();
    exit(EXIT_FAILURE);
}
