// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "InfoChannel.h"

using namespace MiscCommon;
using namespace dds;
using namespace std;

int CInfoChannel::on_cmdREPLY_HANDSHAKE_OK(const CProtocolMessage& _msg)
{
    m_isHandShakeOK = true;

    // ask the server what we wnated to ask :)
    if (m_bNeedCommanderPid)
        pushMsg<cmdGED_PID>();

    return 0;
}

int CInfoChannel::on_cmdSIMPLE_MSG(const CProtocolMessage& _msg)
{
    SSimpleMsgCmd cmd;
    cmd.convertFromData(_msg.bodyToContainer());
    if (!cmd.m_sMsg.empty())
        LOG(log_stdout) << "Server reports: " << cmd.m_sMsg;
    return 0;
}

int CInfoChannel::on_cmdREPLY_PID(const CProtocolMessage& _msg)
{
    SSimpleMsgCmd cmd;
    cmd.convertFromData(_msg.bodyToContainer());
    LOG(debug) << "UI agent has recieved pid of the commander server: " << cmd.m_sMsg;
    LOG(log_stdout_clean) << cmd.m_sMsg;
    // Close communication channel
    stop();
    return 0;
}
