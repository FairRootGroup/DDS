// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "ActivateChannel.h"
#include "ProgressDisplay.h"

using namespace MiscCommon;
using namespace dds;
using namespace std;
using namespace dds::topology_cmd;
using namespace dds::protocol_api;

CActivateChannel::CActivateChannel(boost::asio::io_service& _service, uint64_t _protocolHeaderID)
    : CClientChannelImpl<CActivateChannel>(_service, EChannelType::UI, _protocolHeaderID)
{
    registerHandler<EChannelEvents::OnRemoteEndDissconnected>(
        [](const SSenderInfo& _sender) { LOG(MiscCommon::log_stderr) << "Server has closed the connection."; });

    registerHandler<EChannelEvents::OnConnected>(
        [](const SSenderInfo& _sender) { LOG(MiscCommon::log_stdout) << "Connection established."; });

    registerHandler<EChannelEvents::OnFailedToConnect>(
        [](const SSenderInfo& _sender) { LOG(MiscCommon::log_stdout) << "Failed to connect."; });

    registerHandler<EChannelEvents::OnHandshakeOK>([this](const SSenderInfo& _sender) {
        switch (m_options.m_topologyCmd)
        {
            case ETopologyCmdType::ACTIVATE:
            case ETopologyCmdType::STOP:
            case ETopologyCmdType::UPDATE:
            {
                LOG(MiscCommon::log_stdout) << "Requesting server to activate/update/stop a topology...";
                SUpdateTopologyCmd cmd;
                cmd.m_sTopologyFile = m_options.m_sTopoFile;
                cmd.m_nDisableValidation = m_options.m_bDisableValidation;
                // Set the proper update type
                if (m_options.m_topologyCmd == ETopologyCmdType::ACTIVATE)
                    cmd.m_updateType = (uint8_t)SUpdateTopologyCmd::EUpdateType::ACTIVATE;
                else if (m_options.m_topologyCmd == ETopologyCmdType::UPDATE)
                    cmd.m_updateType = (uint8_t)SUpdateTopologyCmd::EUpdateType::UPDATE;
                else if (m_options.m_topologyCmd == ETopologyCmdType::STOP)
                    cmd.m_updateType = (uint8_t)SUpdateTopologyCmd::EUpdateType::STOP;
                pushMsg<cmdUPDATE_TOPOLOGY>(cmd);
            }
            break;
            default:
                return;
        }
    });
}

bool CActivateChannel::on_cmdSIMPLE_MSG(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment,
                                        const protocol_api::SSenderInfo& _sender)
{
    bool isErrorMsg = _attachment->m_msgSeverity == fatal || _attachment->m_msgSeverity == error;
    if (m_options.m_verbose || isErrorMsg)
    {
        if (!_attachment->m_sMsg.empty())
            LOG((isErrorMsg) ? log_stderr : log_stdout) << "Server reports: " << _attachment->m_sMsg;
    }
    else
    {
        LOG(static_cast<ELogSeverityLevel>(_attachment->m_msgSeverity)) << _attachment->m_sMsg;
    }

    // stop communication if a fatal error is received
    if (_attachment->m_msgSeverity == fatal)
        stop();
    return true;
}

bool CActivateChannel::on_cmdSHUTDOWN(SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t /*_attachment*/,
                                      const protocol_api::SSenderInfo& _sender)
{
    // Close communication channel
    stop();
    return true;
}

bool CActivateChannel::on_cmdPROGRESS(SCommandAttachmentImpl<cmdPROGRESS>::ptr_t _attachment,
                                      const protocol_api::SSenderInfo& _sender)
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

        std::chrono::milliseconds timeToActivate(_attachment->m_time);

        switch (_attachment->m_srcCommand)
        {
            case cmdACTIVATE_USER_TASK:
                cout << "Activated tasks: " << _attachment->m_completed << "\nErrors: " << _attachment->m_errors
                     << "\nTotal: " << _attachment->m_total
                     << "\nTime to Activate: " << std::chrono::duration<double>(timeToActivate).count() << " s" << endl;
                break;
            case cmdSTOP_USER_TASK:
                cout << "Stopped tasks: " << _attachment->m_completed << "\nErrors: " << _attachment->m_errors
                     << "\nTotal: " << _attachment->m_total
                     << "\nTime to Stop: " << std::chrono::duration<double>(timeToActivate).count() << " s" << endl;
                break;
            case cmdUPDATE_TOPOLOGY:
                cout << "Updated agent topologies: " << _attachment->m_completed
                     << "\nErrors: " << _attachment->m_errors << "\nTotal: " << _attachment->m_total
                     << "\nTime to update agent topologies: " << std::chrono::duration<double>(timeToActivate).count()
                     << " s" << endl;
                break;
            default:;
        }
    }
    return true;
}
