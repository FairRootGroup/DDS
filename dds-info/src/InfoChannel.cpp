// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "InfoChannel.h"

using namespace MiscCommon;
using namespace dds;
using namespace std;

bool CInfoChannel::on_cmdREPLY_HANDSHAKE_OK(const CProtocolMessage& _msg)
{
    m_isHandShakeOK = true;

    // ask the server what we wnated to ask :)
    if (m_options.m_bNeedCommanderPid || m_options.m_bNeedDDSStatus)
        pushMsg<cmdGED_PID>();
    else if (m_options.m_bNeedAgentsNumber || m_options.m_bNeedAgentsList)
        pushMsg<cmdGET_AGENTS_INFO>();

    return true;
}

bool CInfoChannel::on_cmdSIMPLE_MSG(const CProtocolMessage& _msg)
{
    SSimpleMsgCmd cmd;
    cmd.convertFromData(_msg.bodyToContainer());
    if (!cmd.m_sMsg.empty())
        LOG(log_stdout) << "Server reports: " << cmd.m_sMsg;
    return true;
}

bool CInfoChannel::on_cmdREPLY_PID(const CProtocolMessage& _msg)
{
    SSimpleMsgCmd cmd;
    cmd.convertFromData(_msg.bodyToContainer());
    LOG(debug) << "UI agent has recieved pid of the commander server: " << cmd.m_sMsg;
    if (m_options.m_bNeedCommanderPid)
        LOG(log_stdout_clean) << cmd.m_sMsg;

    // Checking for "status" option
    if (m_options.m_bNeedDDSStatus)
    {
        if (!cmd.m_sMsg.empty())
            LOG(log_stdout_clean) << "DDS commander server process (" << cmd.m_sMsg << ") is running...";
        else
            LOG(log_stdout_clean) << "DDS commander server is not running.";
    }

    // Close communication channel
    stop();
    return true;
}

bool CInfoChannel::on_cmdREPLY_AGENTS_INFO(const CProtocolMessage& _msg)
{
    SAgentsInfoCmd cmd;
    cmd.convertFromData(_msg.bodyToContainer());
    LOG(debug) << "UI agent has recieved Agents Info from the commander server.";
    if (m_options.m_bNeedAgentsNumber)
        LOG(log_stdout_clean) << cmd.m_nActiveAgents;
    if (m_options.m_bNeedAgentsList && !cmd.m_sListOfAgents.empty())
        LOG(log_stdout_clean) << cmd.m_sListOfAgents;

    // Close communication channel
    stop();
    return true;
}
