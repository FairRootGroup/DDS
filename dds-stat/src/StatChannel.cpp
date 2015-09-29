// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "StatChannel.h"

using namespace MiscCommon;
using namespace dds;
using namespace dds::protocol_api;
using namespace dds::stat_cmd;
using namespace std;

bool CStatChannel::on_cmdSIMPLE_MSG(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment)
{
    if (_attachment->m_srcCommand == cmdENABLE_STAT || _attachment->m_srcCommand == cmdDISABLE_STAT ||
        _attachment->m_srcCommand == cmdGET_STAT)
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
