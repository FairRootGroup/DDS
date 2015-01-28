// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "GenericChannel.h"
#include "UpdateKeyCmd.h"
#include "ProgressDisplay.h"

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
    if (m_options.m_verbose)
        LOG(log_stdout) << _attachment->m_sMsg;
    else
        LOG(static_cast<ELogSeverityLevel>(_attachment->m_msgSeverity)) << _attachment->m_sMsg;
    return true;
}

bool CGenericChannel::on_cmdSHUTDOWN(SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment)
{
    stop();
    return true;
}

bool CGenericChannel::on_cmdPROGRESS(SCommandAttachmentImpl<cmdPROGRESS>::ptr_t _attachment)
{
    if (m_options.m_verbose)
        return true;

    int completed = _attachment->m_completed + _attachment->m_errors;
    if (completed < _attachment->m_total)
    {
        cout << getProgressDisplayString(completed, _attachment->m_total);
        cout.flush();
    }
    else
    {
        cout << getProgressDisplayString(completed, _attachment->m_total) << endl;
        cout << "Received: " << _attachment->m_completed << " errors: " << _attachment->m_errors
             << " total: " << _attachment->m_total << endl;
    }
    return true;
}
