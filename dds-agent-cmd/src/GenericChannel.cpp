// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "GenericChannel.h"
#include "UpdateKeyCmd.h"

using namespace MiscCommon;
using namespace dds;
using namespace std;

void CGenericChannel::onHandshakeOK()
{
    switch (m_options.m_agentCmd)
    {
        case EAgentCmdType::GETLOG:
            LOG(log_stdout) << "Requesting log files from agents...";
            pushMsg<cmdGET_LOG>();
            break;
        case EAgentCmdType::UPDATE_KEY:
        {
            LOG(log_stdout) << "Sending key update command...";
            SUpdateKeyCmd cmd;
            cmd.m_sKey = m_options.m_sUpdKey_key;
            cmd.m_sValue = m_options.m_sUpdKey_value;
            pushMsg<cmdUPDATE_KEY>(cmd);
        }
        break;
        default:
            LOG(log_stderr) << "Uknown command.";
            stop();
    }
}

void CGenericChannel::onHandshakeERR()
{
}

bool CGenericChannel::on_cmdSIMPLE_MSG(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment)
{
    LOG(log_stdout) << _attachment->m_sMsg;
    return true;
}

bool CGenericChannel::on_cmdSHUTDOWN(SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment)
{
    stop();
    return true;
}
