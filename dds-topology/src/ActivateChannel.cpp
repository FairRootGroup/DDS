// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "ActivateChannel.h"

using namespace MiscCommon;
using namespace dds;
using namespace std;

bool CActivateChannel::on_cmdREPLY_HANDSHAKE_OK(SCommandAttachmentImpl<cmdREPLY_HANDSHAKE_OK>::ptr_t /*_attachment*/)
{
    m_isHandShakeOK = true;

    pushMsg<cmdACTIVATE_AGENT>();

    return true;
}

bool CActivateChannel::on_cmdSIMPLE_MSG(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment)
{
    if (!_attachment->m_sMsg.empty())
        LOG((_attachment->m_msgSeverity == fatal || _attachment->m_msgSeverity == error) ? log_stderr : log_stdout)
            << "Server reports: " << _attachment->m_sMsg;

    // stop communication if a fatal error is recieved
    if (_attachment->m_msgSeverity == fatal)
        stop();
    return true;
}

bool CActivateChannel::on_cmdSHUTDOWN(SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t /*_attachment*/)
{
    // Close communication channel
    stop();
    return true;
}
