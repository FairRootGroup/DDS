// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "TestChannel.h"

using namespace MiscCommon;
using namespace dds;
using namespace std;

bool CTestChannel::on_cmdREPLY_HANDSHAKE_OK(const CProtocolMessage& _msg)
{
    m_isHandShakeOK = true;

    pushMsg<cmdSTART_DOWNLOAD_TEST>();

    return true;
}

bool CTestChannel::on_cmdDOWNLOAD_TEST_RECIEVED(const CProtocolMessage& _msg)
{
    SSimpleMsgCmd recieved_cmd;
    recieved_cmd.convertFromData(_msg.bodyToContainer());
    LOG(log_stdout) << recieved_cmd.m_sMsg;

    return true;
}

bool CTestChannel::on_cmdALL_DOWNLOAD_TESTS_RECIEVED(const CProtocolMessage& _msg)
{
    SSimpleMsgCmd recieved_cmd;
    recieved_cmd.convertFromData(_msg.bodyToContainer());
    LOG(log_stdout) << recieved_cmd.m_sMsg;

    stop();
    exit(EXIT_SUCCESS);
}

bool CTestChannel::on_cmdDOWNLOAD_TEST_ERROR(const CProtocolMessage& _msg)
{
    SSimpleMsgCmd recieved_cmd;
    recieved_cmd.convertFromData(_msg.bodyToContainer());
    LOG(log_stdout) << recieved_cmd.m_sMsg;

    return true;
}

bool CTestChannel::on_cmdDOWNLOAD_TEST_FATAL(const CProtocolMessage& _msg)
{
    SSimpleMsgCmd recieved_cmd;
    recieved_cmd.convertFromData(_msg.bodyToContainer());
    LOG(log_stdout) << recieved_cmd.m_sMsg;

    stop();
    exit(EXIT_FAILURE);
}
