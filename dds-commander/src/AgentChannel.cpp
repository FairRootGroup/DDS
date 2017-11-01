// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "AgentChannel.h"
#include "CRC.h"
#include "ChannelId.h"
// BOOST
#include <boost/filesystem.hpp>

using namespace std;
using namespace MiscCommon;
using namespace dds;
using namespace dds::commander_cmd;
using namespace dds::user_defaults_api;
using namespace dds::protocol_api;

CAgentChannel::CAgentChannel(boost::asio::io_service& _service, uint64_t _protocolHeaderID)
    : CServerChannelImpl<CAgentChannel>(_service, { EChannelType::AGENT, EChannelType::UI })
{
    registerHandler<EChannelEvents::OnRemoteEndDissconnected>(
        [](const SSenderInfo& _sender) { LOG(MiscCommon::info) << "The Agent has closed the connection."; });

    registerHandler<EChannelEvents::OnHandshakeOK>([this](const SSenderInfo& _sender) {
        SAgentInfo info;
        // any TCP connection channel's end is considerd as a lobby leader
        info.m_lobbyLeader = true;
        switch (getChannelType())
        {
            case EChannelType::AGENT:
            {
                info.m_state = EAgentState::idle;
                pushMsg<cmdGET_ID>(_sender.m_ID);
                pushMsg<cmdGET_HOST_INFO>(_sender.m_ID);
            }
            break;
            case EChannelType::UI:
            {
                LOG(MiscCommon::info) << "The UI agent [" << socket().remote_endpoint().address().to_string()
                                      << "] has successfully connected.";

                // All UI channels get unique IDs, so that user tasks and agents can send
                // back the
                // information to a particular UI channel.
                info.m_id = DDSChannelId::getChannelId();
            }
            break;
            default:
                // TODO: log unknown connection attempt
                return;
        }

        updateAgentInfo(_sender, info);
    });

    // Subsribe on lobby member handshake
    registerHandler<EChannelEvents::OnLobbyMemberHandshakeOK>([this](const SSenderInfo& _sender) -> void {
        {
            SAgentInfo info;

            info.m_state = EAgentState::idle;
            pushMsg<cmdGET_ID>(_sender.m_ID);
            pushMsg<cmdGET_HOST_INFO>(_sender.m_ID);

            updateAgentInfo(_sender, info);
        }
    });
}

void CAgentChannel::updateAgentInfo(const SSenderInfo& _sender, const SAgentInfo& _info)
{
    lock_guard<mutex> lock(m_mtxInfo);
    m_info[_sender.m_ID] = _info;
}

SAgentInfo CAgentChannel::getAgentInfo(uint64_t _protocolHeaderID)
{
    lock_guard<mutex> lock(m_mtxInfo);
    auto it = m_info.find(_protocolHeaderID);
    if (it != m_info.end())
        return it->second;

    // return empty info the requested header ID is not in the list
    return SAgentInfo();
}

SAgentInfo CAgentChannel::getAgentInfo(const SSenderInfo& _sender)
{
    return getAgentInfo(_sender.m_ID);
}

uint64_t CAgentChannel::getId(const SSenderInfo& _sender)
{
    SAgentInfo info = getAgentInfo(_sender);
    return info.m_id;
}

LobbyProtocolHeaderIdContainer_t CAgentChannel::getLobbyPHID() const
{
    LobbyProtocolHeaderIdContainer_t ret;
    for (auto it : m_info)
        ret.push_back(it->first);

    return ret;
}

void CAgentChannel::setId(const SSenderInfo& _sender, uint64_t _id)
{
    SAgentInfo info = getAgentInfo(_sender);

    info.m_id = _id;

    updateAgentInfo(_sender, info);
}

uint64_t CAgentChannel::getTaskID(const SSenderInfo& _sender)
{
    SAgentInfo info = getAgentInfo(_sender);
    return info.m_taskID;
}

void CAgentChannel::setTaskID(const SSenderInfo& _sender, uint64_t _taskID)
{
    SAgentInfo info = getAgentInfo(_sender);

    info.m_taskID = _taskID;

    updateAgentInfo(_sender, info);
}

SHostInfoCmd CAgentChannel::getRemoteHostInfo(const SSenderInfo& _sender)
{
    SAgentInfo info = getAgentInfo(_sender);
    return info.m_remoteHostInfo;
}

void CAgentChannel::setRemoteHostInfo(const SSenderInfo& _sender, const SHostInfoCmd& _hostInfo)
{
    SAgentInfo info = getAgentInfo(_sender);

    info.m_remoteHostInfo = _hostInfo;

    updateAgentInfo(_sender, info);
}

chrono::milliseconds CAgentChannel::getStartupTime(const SSenderInfo& _sender)
{
    SAgentInfo info = getAgentInfo(_sender);
    return info.m_startUpTime;
}

EAgentState CAgentChannel::getState(const SSenderInfo& _sender)
{
    SAgentInfo info = getAgentInfo(_sender.m_ID);
    return info.m_state;
}

void CAgentChannel::setState(const SSenderInfo& _sender, EAgentState _state)
{
    SAgentInfo info = getAgentInfo(_sender.m_ID);
    info.m_state = _state;
}

string CAgentChannel::_remoteEndIDString()
{
    // The remote end is shown only for the lobby leader
    if (getChannelType() == EChannelType::AGENT)
    {
        SAgentInfo info = getAgentInfo(m_ProtocolHeaderID);
        if (info.m_lobbyLeader)
            return to_string(info.m_id);
    }

    return "UI client";
}

bool CAgentChannel::on_cmdSUBMIT(SCommandAttachmentImpl<cmdSUBMIT>::ptr_t _attachment, const SSenderInfo& _sender)
{
    try
    {
        LOG(info) << "Received a Submit command; RMS: " << _attachment->m_sRMSType << " from: " << remoteEndIDString();
    }
    catch (exception& e)
    {
        pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(e.what(), MiscCommon::fatal, cmdSUBMIT), _sender.m_ID);
        return true;
    }

    // The agent can't submit to the cluster by itself. Let others to process this message.
    return false;
}

bool CAgentChannel::on_cmdREPLY_HOST_INFO(SCommandAttachmentImpl<cmdREPLY_HOST_INFO>::ptr_t _attachment,
                                          const SSenderInfo& _sender)
{
    SAgentInfo info = getAgentInfo(_sender);

    info.m_remoteHostInfo = *_attachment;
    LOG(debug) << "cmdREPLY_HOST_INFO attachment [" << info.m_remoteHostInfo
               << "] received from: " << remoteEndIDString();

    // Calculating startup time of the agent
    info.m_startUpTime = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch());
    info.m_startUpTime -= chrono::milliseconds(_attachment->m_submitTime);
    // everything is OK, we can work with this agent
    LOG(info) << "The Agent [" << socket().remote_endpoint().address().to_string()
              << "] has successfully connected. Startup time: " << info.m_startUpTime.count() << " ms.";

    updateAgentInfo(_sender.m_ID, info);

    return true;
}

bool CAgentChannel::on_cmdGED_PID(SCommandAttachmentImpl<cmdGED_PID>::ptr_t _attachment, const SSenderInfo& _sender)
{
    pid_t pid = getpid();
    SSimpleMsgCmd cmd_attachment;
    stringstream ss;
    ss << pid;
    cmd_attachment.m_sMsg = ss.str();
    pushMsg<cmdREPLY_PID>(cmd_attachment, _sender.m_ID);

    return true;
}

bool CAgentChannel::on_cmdREPLY_ID(SCommandAttachmentImpl<cmdREPLY_ID>::ptr_t _attachment, const SSenderInfo& _sender)
{
    // Return false. This message will be processed by ConnectionManager.
    return false;
}

bool CAgentChannel::on_cmdGET_LOG(SCommandAttachmentImpl<cmdGET_LOG>::ptr_t _attachment, const SSenderInfo& _sender)
{
    // Return false. This message will be processed by ConnectionManager.
    return false;
}

bool CAgentChannel::on_cmdBINARY_ATTACHMENT_RECEIVED(
    SCommandAttachmentImpl<cmdBINARY_ATTACHMENT_RECEIVED>::ptr_t _attachment, const SSenderInfo& _sender)
{
    switch (_attachment->m_srcCommand)
    {
        case cmdGET_LOG:
        {
            const string sLogStorageDir(CUserDefaults::instance().getAgentLogStorageDir());
            const string logFileName(sLogStorageDir + _attachment->m_requestedFileName);
            const boost::filesystem::path logFilePath(logFileName);
            const boost::filesystem::path receivedFilePath(_attachment->m_receivedFilePath);

            boost::filesystem::rename(receivedFilePath, logFilePath);

            return false;
        }

        case cmdTRANSPORT_TEST:
        {
            LOG(info) << "cmdBINARY_ATTACHMENT_RECEIVED attachment [" << *_attachment << "] command from "
                      << remoteEndIDString();

            return false;
        }

        default:
            LOG(debug) << "Received BINARY_ATTACHMENT_RECEIVED has no listener.";
            return true;
    }
    return true;
}

bool CAgentChannel::on_cmdGET_AGENTS_INFO(SCommandAttachmentImpl<cmdGET_AGENTS_INFO>::ptr_t _attachment,
                                          const SSenderInfo& _sender)
{
    // Return false.
    // Give the possibility to further process this message.
    // For example, send information to UI.
    return false;
}

bool CAgentChannel::on_cmdTRANSPORT_TEST(SCommandAttachmentImpl<cmdTRANSPORT_TEST>::ptr_t _attachment,
                                         const SSenderInfo& _sender)
{
    // Return false.
    // Give the possibility to further process this message.
    // For example, send information to UI.
    return false;
}

bool CAgentChannel::on_cmdSIMPLE_MSG(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment,
                                     const SSenderInfo& _sender)
{
    LOG(debug) << "on_cmdSIMPLE_MSG attachment [" << *_attachment << "] command from " << remoteEndIDString();

    switch (_attachment->m_srcCommand)
    {
        case cmdACTIVATE_AGENT:
            return false; // let others to process this message

        case cmdSTOP_USER_TASK:
            return false; // let others to process this message

        case cmdGET_LOG:
            return false; // let others to process this message

        case cmdTRANSPORT_TEST:
            return false;

        default:
            LOG(static_cast<ELogSeverityLevel>(_attachment->m_msgSeverity)) << "remote: " << _attachment->m_sMsg;
            return true;
    }
}

bool CAgentChannel::on_cmdUPDATE_KEY(SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment,
                                     const SSenderInfo& _sender)
{
    // Return false.
    // The command can only be processed by the higher level object
    return false;
}

bool CAgentChannel::on_cmdUSER_TASK_DONE(SCommandAttachmentImpl<cmdUSER_TASK_DONE>::ptr_t _attachment,
                                         const SSenderInfo& _sender)
{
    // Return false.
    // The command can only be processed by the higher level object
    return false;
}

bool CAgentChannel::on_cmdWATCHDOG_HEARTBEAT(SCommandAttachmentImpl<cmdWATCHDOG_HEARTBEAT>::ptr_t _attachment,
                                             const SSenderInfo& _sender)
{
    SAgentInfo info = getAgentInfo(_sender.m_ID);
    // The main reason for this message is to tell commander that agents are note idle (see. GH-54)
    LOG(debug) << "Received Watchdog heartbeat from agent " << info.m_id << " running task = " << info.m_taskID;

    // TODO: So far we do nothing with this info.
    // In the future we might want to send more information about tasks being executed (pid, CPU info, memory)
    return true;
}

bool CAgentChannel::on_cmdGET_PROP_LIST(SCommandAttachmentImpl<cmdGET_PROP_LIST>::ptr_t _attachment,
                                        const SSenderInfo& _sender)
{
    return false;
}

bool CAgentChannel::on_cmdGET_PROP_VALUES(SCommandAttachmentImpl<cmdGET_PROP_VALUES>::ptr_t _attachment,
                                          const SSenderInfo& _sender)
{
    return false;
}

bool CAgentChannel::on_cmdUPDATE_TOPOLOGY(SCommandAttachmentImpl<cmdUPDATE_TOPOLOGY>::ptr_t _attachment,
                                          const SSenderInfo& _sender)
{
    return false;
}

bool CAgentChannel::on_cmdENABLE_STAT(SCommandAttachmentImpl<cmdENABLE_STAT>::ptr_t _attachment,
                                      const SSenderInfo& _sender)
{
    return false;
}

bool CAgentChannel::on_cmdDISABLE_STAT(SCommandAttachmentImpl<cmdDISABLE_STAT>::ptr_t _attachment,
                                       const SSenderInfo& _sender)
{
    return false;
}

bool CAgentChannel::on_cmdGET_STAT(SCommandAttachmentImpl<cmdGET_STAT>::ptr_t _attachment, const SSenderInfo& _sender)
{
    return false;
}

bool CAgentChannel::on_cmdCUSTOM_CMD(SCommandAttachmentImpl<cmdCUSTOM_CMD>::ptr_t _attachment,
                                     const SSenderInfo& _sender)
{
    return false;
}
