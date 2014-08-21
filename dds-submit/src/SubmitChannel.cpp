// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "SubmitChannel.h"
// BOOST
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>

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

    if (!m_sTopoFile.empty() && SSubmitCmd::UNKNOWN != m_RMS)
    {
        // make absolute path
        boost::filesystem::path pathTopoFile(m_sTopoFile);
        boost::filesystem::path pathSSHCfgFile(m_sSSHCfgFile);

        // Create the command's attachment
        SSubmitCmd cmd;
        cmd.m_sTopoFile = boost::filesystem::absolute(pathTopoFile).string();
        cmd.m_nRMSTypeCode = m_RMS;
        cmd.m_sSSHCfgFile = boost::filesystem::absolute(pathSSHCfgFile).string();

        CProtocolMessage msg;
        msg.encodeWithAttachment<cmdSUBMIT>(cmd);
        pushMsg(msg);
    }
    // Check wheather we need to start distribuiting tasks
    if (m_bSendStart)
    {
        pushMsg<cmdSUBMIT_START>();
    }

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
