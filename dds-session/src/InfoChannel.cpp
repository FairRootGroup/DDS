// Copyright 2018 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "InfoChannel.h"

using namespace MiscCommon;
using namespace dds;
using namespace dds::session_cmd;
using namespace dds::protocol_api;
using namespace std;

bool CInfoChannel::on_cmdSIMPLE_MSG(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment,
                                    const protocol_api::SSenderInfo& _sender)
{
    if (_attachment->m_srcCommand == cmdGET_PROP_LIST || _attachment->m_srcCommand == cmdGET_PROP_VALUES)
    {
        if (!_attachment->m_sMsg.empty())
            LOG(log_stdout) << "\n" << _attachment->m_sMsg;
        stop();
        return true;
    }

    if (!_attachment->m_sMsg.empty())
        LOG(log_stdout) << "Server reports: " << _attachment->m_sMsg;

    return true;
}

bool CInfoChannel::on_cmdREPLY_PID(SCommandAttachmentImpl<cmdREPLY_PID>::ptr_t _attachment,
                                   const protocol_api::SSenderInfo& _sender)
{
    LOG(debug) << "UI agent has recieved pid of the commander server: " << _attachment->m_sMsg;
    LOG(log_stdout_clean) << "------------------------";
    LOG(log_stdout_clean) << "DDS commander server: " << _attachment->m_sMsg;
    LOG(log_stdout_clean) << "------------------------";

    // Close communication channel
    stop();
    return true;
}
