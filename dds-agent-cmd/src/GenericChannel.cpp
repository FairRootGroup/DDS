// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "GenericChannel.h"
#include "ProgressDisplay.h"
#include "UpdateKeyCmd.h"

using namespace MiscCommon;
using namespace dds;
using namespace dds::protocol_api;
using namespace dds::agent_cmd_cmd;
using namespace std;

bool CGenericChannel::on_cmdSIMPLE_MSG(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment)
{
    bool isErrorMsg = _attachment->m_msgSeverity == fatal || _attachment->m_msgSeverity == error;
    if (m_options.m_verbose || isErrorMsg)
    {
        if (!_attachment->m_sMsg.empty())
            LOG((isErrorMsg) ? log_stderr : log_stdout) << _attachment->m_sMsg;
    }
    else
    {
        LOG(static_cast<ELogSeverityLevel>(_attachment->m_msgSeverity)) << _attachment->m_sMsg;
    }
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
