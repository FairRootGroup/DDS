// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "InfoChannel.h"

using namespace MiscCommon;
using namespace dds;
using namespace dds::info_cmd;
using namespace dds::protocol_api;
using namespace std;

bool CInfoChannel::on_cmdSIMPLE_MSG(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment)
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

bool CInfoChannel::on_cmdREPLY_PID(SCommandAttachmentImpl<cmdREPLY_PID>::ptr_t _attachment)
{
    LOG(debug) << "UI agent has recieved pid of the commander server: " << _attachment->m_sMsg;
    if (m_options.m_bNeedCommanderPid)
        LOG(log_stdout_clean) << _attachment->m_sMsg;

    // Checking for "status" option
    if (m_options.m_bNeedDDSStatus)
    {
        if (!_attachment->m_sMsg.empty())
            LOG(log_stdout_clean) << "DDS commander server process (" << _attachment->m_sMsg << ") is running...";
        else
            LOG(log_stdout_clean) << "DDS commander server is not running.";
    }

    // Close communication channel
    stop();
    return true;
}

bool CInfoChannel::on_cmdREPLY_AGENTS_INFO(SCommandAttachmentImpl<cmdREPLY_AGENTS_INFO>::ptr_t _attachment)
{
    LOG(debug) << "UI agent has recieved Agents Info from the commander server.";
    if (m_options.m_bNeedAgentsNumber)
    {
        LOG(log_stdout_clean) << _attachment->m_nActiveAgents;
        // Close communication channel
        stop();
    }

    if (m_options.m_bNeedAgentsList && !_attachment->m_sAgentInfo.empty())
    {
        std::lock_guard<std::mutex> lock(m_mutexCounter);
        ++m_nCounter;
        LOG(log_stdout_clean) << _attachment->m_sAgentInfo;

        if (m_nCounter == _attachment->m_nActiveAgents)
            stop();
    }
    else
        stop();

    return true;
}
