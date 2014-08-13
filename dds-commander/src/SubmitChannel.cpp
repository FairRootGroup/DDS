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

bool CSubmitChannel::on_cmdREPLY_HANDSHAKE_OK(const CProtocolMessage& _msg)
{
    m_isHandShakeOK = true;

    // Create the command's attachment
    SSubmitCmd cmd;
    cmd.m_sTopoFile = m_sTopoFile;
    cmd.m_nRMSTypeCode = m_RMS;
    cmd.m_sSSHCfgFile = m_sSSHCfgFile;

    CProtocolMessage msg;
    msg.encodeWithAttachment<cmdSUBMIT>(cmd);
    pushMsg(msg);

    return true;
}

bool CSubmitChannel::on_cmdSIMPLE_MSG(const CProtocolMessage& _msg)
{
    SSimpleMsgCmd cmd;
    cmd.convertFromData(_msg.bodyToContainer());
    if (!cmd.m_sMsg.empty())
        LOG(log_stdout) << "Server reports: " << cmd.m_sMsg;
    return true;
}

bool CSubmitChannel::on_cmdREPLY_SUBMIT_OK(const CProtocolMessage& _msg)
{
    LOG(log_stdout) << "Successfully done.";

    on_cmdSIMPLE_MSG(_msg);

    // Close communication channel
    stop();
    return true;
}

bool CSubmitChannel::on_cmdREPLY_ERR_SUBMIT(const CProtocolMessage& _msg)
{
    LOG(log_stderr) << "Submission has failed.";

    on_cmdSIMPLE_MSG(_msg);

    // Close communication channel
    stop();
    return true;
}
