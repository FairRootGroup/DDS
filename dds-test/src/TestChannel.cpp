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

    if (cmd.m_srcCommand != cmdSTART_DOWNLOAD_TEST)
        return true;

    if (cmd.m_msgSeverity == MiscCommon::fatal)
    {
        stop();
        // TODO: Move exit from here to main
        exit(EXIT_SUCCESS);
    }

    return true;
}
