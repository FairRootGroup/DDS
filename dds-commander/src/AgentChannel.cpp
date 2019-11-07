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

CAgentChannel::CAgentChannel(boost::asio::io_context& _context, uint64_t _protocolHeaderID)
    : CServerChannelImpl<CAgentChannel>(_context, { EChannelType::AGENT, EChannelType::UI })
{
    registerHandler<EChannelEvents::OnRemoteEndDissconnected>(
        [](const SSenderInfo& _sender) { LOG(MiscCommon::info) << "The Agent has closed the connection."; });

    registerHandler<EChannelEvents::OnHandshakeOK>([this](const SSenderInfo& _sender) {
        switch (getChannelType())
        {
            case EChannelType::AGENT:
            {
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
                m_info.m_id = DDSChannelId::getChannelId();
            }
            break;
            default:
                // TODO: log unknown connection attempt
                return;
        }
    });

    // Subsribe on lobby member handshake
    //    registerHandler<EChannelEvents::OnLobbyMemberHandshakeOK>([this](const SSenderInfo& _sender) -> void {
    //        {
    //            if (_sender.m_ID == this->m_protocolHeaderID)
    //                return;
    //
    //            SAgentInfo inf = this->getAgentInfo(_sender.m_ID);
    //
    //            inf.m_state = EAgentState::idle;
    //            pushMsg<cmdGET_ID>(_sender.m_ID);
    //            pushMsg<cmdGET_HOST_INFO>(_sender.m_ID);
    //
    //            updateAgentInfo(_sender, inf);
    //        }
    //    });
}

// void CAgentChannel::updateAgentInfo(const SSenderInfo& _sender, const SAgentInfo& _info)
//{
//    updateAgentInfo(_sender.m_ID, _info);
//}
//
// void CAgentChannel::updateAgentInfo(uint64_t _protocolHeaderID, const SAgentInfo& _info)
//{
//    lock_guard<mutex> lock(m_mtxInfo);
//    m_info[_protocolHeaderID] = _info;
//}

// SAgentInfo CAgentChannel::getAgentInfo(uint64_t _protocolHeaderID)
//{
//    lock_guard<mutex> lock(m_mtxInfo);
//    auto it = m_info.find(_protocolHeaderID);
//    if (it != m_info.end())
//        return it->second;
//
//    LOG(warning) << "Unknown agent info for PHID " << _protocolHeaderID;
//
//    // return empty info the requested header ID is not in the list
//    return SAgentInfo();
//}

SAgentInfo& CAgentChannel::getAgentInfo()
{
    return m_info;
}

uint64_t CAgentChannel::getId() const
{
    return m_info.m_id;
}

// LobbyProtocolHeaderIdContainer_t CAgentChannel::getLobbyPHID() const
//{
//    //    LobbyProtocolHeaderIdContainer_t ret;
//    //    for (const auto& v : m_info)
//    //        ret.push_back(v.first);
//    //
//    //    return ret;
//    return LobbyProtocolHeaderIdContainer_t();
//}

void CAgentChannel::setId(uint64_t _id)
{
    m_info.m_id = _id;
}

uint64_t CAgentChannel::getTaskID(const SSenderInfo& _sender)
{
    // SAgentInfo info = getAgentInfo(_sender);
    // return info.m_taskID;
    return 0;
}

void CAgentChannel::setTaskID(const SSenderInfo& _sender, uint64_t _taskID)
{
    //    SAgentInfo info = getAgentInfo(_sender);
    //
    //    info.m_taskID = _taskID;
    //
    //    updateAgentInfo(_sender, info);
}

SHostInfoCmd CAgentChannel::getRemoteHostInfo(const SSenderInfo& _sender)
{
    return m_info.m_remoteHostInfo;
}

void CAgentChannel::setRemoteHostInfo(const SSenderInfo& _sender, const SHostInfoCmd& _hostInfo)
{
    //    SAgentInfo info = getAgentInfo(_sender);
    //
    //    info.m_remoteHostInfo = _hostInfo;
    //
    //    updateAgentInfo(_sender, info);
}

chrono::milliseconds CAgentChannel::getStartupTime(const SSenderInfo& _sender)
{
    return m_info.m_startUpTime;
}

EAgentState CAgentChannel::getState(const SSenderInfo& _sender)
{
    //    SAgentInfo info = getAgentInfo(_sender.m_ID);
    //    return info.m_state;
    return EAgentState::unknown;
}

void CAgentChannel::setState(const SSenderInfo& _sender, EAgentState _state)
{
    //    SAgentInfo info = getAgentInfo(_sender.m_ID);
    //    info.m_state = _state;
}

string CAgentChannel::_remoteEndIDString()
{
    // The remote end is shown only for the lobby leader
    if (getChannelType() == EChannelType::AGENT)
        return to_string(m_info.m_id);

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
    m_info.m_remoteHostInfo = *_attachment;
    LOG(debug) << "cmdREPLY_HOST_INFO attachment [" << m_info.m_remoteHostInfo
               << "] received from: " << remoteEndIDString();

    // Calculating startup time of the agent
    m_info.m_startUpTime = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch());
    m_info.m_startUpTime -= chrono::milliseconds(_attachment->m_submitTime);
    // everything is OK, we can work with this agent
    LOG(info) << "The Agent [" << socket().remote_endpoint().address().to_string()
              << "] has successfully connected. Startup time: " << m_info.m_startUpTime.count() << " ms.";

    // Request agent to add Task Slots
    // We get the number of slots from the agent. On submit each agent is assiugned to a fixed number of slots. Then
    // when agent is up, we requast tyhe agent to actul,ly active each slot.
    LOG(info) << "Requesting " << _attachment->m_slots << " task slots from " << m_info.m_id;
    for (size_t i = 0; i < _attachment->m_slots; ++i)
    {
        SIDCmd msg_cmd;
        msg_cmd.m_id = DDSChannelId::getChannelId();

        pushMsg<cmdADD_SLOT>(msg_cmd);
    }

    return true;
}

bool CAgentChannel::on_cmdREPLY_ADD_SLOT(SCommandAttachmentImpl<protocol_api::cmdREPLY_ADD_SLOT>::ptr_t _attachment,
                                         const SSenderInfo& _sender)
{
    // Create a new tasks slot
    SSlotInfo slot;
    slot.m_id = _attachment->m_id;
    slot.m_state = EAgentState::idle;
    m_info.addSlot(slot);

    // Let other consumers (like a connection manager) to process this event
    SSenderInfo info;
    info.m_ID = _attachment->m_id;
    dispatchHandlers(EChannelEvents::OnReplyAddSlot, info);
    return true;
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

bool CAgentChannel::on_cmdREPLY(SCommandAttachmentImpl<cmdREPLY>::ptr_t _attachment, const SSenderInfo& _sender)
{
    LOG(debug) << "on_cmdREPLY attachment [" << *_attachment << "] command from " << remoteEndIDString();

    switch (_attachment->m_srcCommand)
    {
        case cmdASSIGN_USER_TASK:
            return false; // let others to process this message

        case cmdACTIVATE_USER_TASK:
            return false; // let others to process this message

        case cmdSTOP_USER_TASK:
            return false; // let others to process this message

        case cmdUPDATE_TOPOLOGY:
            return false; // let others to process this message

        case cmdGET_LOG:
            return false; // let others to process this message

        case cmdTRANSPORT_TEST:
            return false;

        default:
            LOG(warning) << "Received cmdREPLY doesn't have a handler: " << _attachment->m_sMsg;
            return true;
    }
}

bool CAgentChannel::on_cmdWATCHDOG_HEARTBEAT(SCommandAttachmentImpl<cmdWATCHDOG_HEARTBEAT>::ptr_t _attachment,
                                             const SSenderInfo& _sender)
{
    // The main reason for this message is to tell commander that agents are note idle (see. GH-54)
    LOG(debug) << "Received Watchdog heartbeat from agent " << m_info.m_id; //<< " running task = " << m_info.m_taskID;

    // TODO: So far we do nothing with this info.
    // In the future we might want to send more information about tasks being executed (pid, CPU info, memory)
    return true;
}
