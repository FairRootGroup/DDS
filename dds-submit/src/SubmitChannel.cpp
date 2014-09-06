// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "SubmitChannel.h"

using namespace MiscCommon;
using namespace dds;
using namespace std;

void CSubmitChannel::setTopoFile(const string& _val)
{
    m_sTopoFile = _val;
}

void CSubmitChannel::setSSHCfgFile(const string& _val)
{
    m_sSSHCfgFile = _val;
}

void CSubmitChannel::setRMSTypeCode(const SSubmitCmd::ERmsType& _val)
{
    m_RMS = _val;
}

bool CSubmitChannel::on_cmdREPLY_HANDSHAKE_OK(CProtocolMessage::protocolMessagePtr_t _msg)
{
    m_isHandShakeOK = true;

    if (!m_sTopoFile.empty() && SSubmitCmd::UNKNOWN != m_RMS)
    {
        // Create the command's attachment
        SSubmitCmd cmd;
        cmd.m_sTopoFile = m_sTopoFile;
        cmd.m_nRMSTypeCode = m_RMS;
        cmd.m_sSSHCfgFile = m_sSSHCfgFile;

        CProtocolMessage::protocolMessagePtr_t msg = make_shared<CProtocolMessage>();
        msg->encodeWithAttachment<cmdSUBMIT>(cmd);
        pushMsg(msg);
    }
    // Check wheather we need to start distribuiting tasks
    if (m_bSendStart)
    {
        pushMsg<cmdSUBMIT_START>();
    }

    return true;
}

bool CSubmitChannel::on_cmdSIMPLE_MSG(CProtocolMessage::protocolMessagePtr_t _msg)
{
    SSimpleMsgCmd cmd;
    cmd.convertFromData(_msg->bodyToContainer());
    if (!cmd.m_sMsg.empty())
        LOG((cmd.m_msgSeverity == fatal || cmd.m_msgSeverity == error) ? log_stderr : log_stdout)
            << "Server reports: " << cmd.m_sMsg;

    // stop communication if a fatal error is recieved
    if (cmd.m_msgSeverity == fatal)
        stop();
    return true;
}

bool CSubmitChannel::on_cmdREPLY_SUBMIT_OK(CProtocolMessage::protocolMessagePtr_t _msg)
{
    on_cmdSIMPLE_MSG(_msg);
    // Close communication channel
    stop();
    return true;
}

bool CSubmitChannel::on_cmdREPLY_ERR_SUBMIT(CProtocolMessage::protocolMessagePtr_t _msg)
{
    on_cmdSIMPLE_MSG(_msg);
    // Close communication channel
    stop();
    return true;
}

bool CSubmitChannel::on_cmdSHUTDOWN(CProtocolMessage::protocolMessagePtr_t _msg)
{
    // Close communication channel
    stop();
    return true;
}
