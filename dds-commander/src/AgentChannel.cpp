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

CAgentChannel::CAgentChannel(boost::asio::io_service& _service)
    : CServerChannelImpl<CAgentChannel>(_service, { protocol_api::EChannelType::AGENT, protocol_api::EChannelType::UI })
    , m_id(0)
    , m_remoteHostInfo()
    , m_sCurrentTopoFile()
    , m_taskID(0)
    , m_startUpTime(0)
    , m_state(EAgentState::unknown)
{
    subscribeOnEvent(protocol_api::EChannelEvents::OnRemoteEndDissconnected,
                     [](CAgentChannel* _channel) { LOG(MiscCommon::info) << "The Agent has closed the connection."; });

    subscribeOnEvent(protocol_api::EChannelEvents::OnHandshakeOK, [this](CAgentChannel* _channel) {
        switch (getChannelType())
        {
            case protocol_api::EChannelType::AGENT:
            {
                m_state = EAgentState::idle;
                pushMsg<protocol_api::cmdGET_ID>();
                pushMsg<protocol_api::cmdGET_HOST_INFO>();
            }
                return;
            case protocol_api::EChannelType::UI:
            {
                LOG(MiscCommon::info) << "The UI agent [" << socket().remote_endpoint().address().to_string()
                                      << "] has successfully connected.";

                // All UI channels get unique IDs, so that user tasks and agents can send
                // back the
                // information to a particular UI channel.
                m_id = DDSChannelId::getChannelId();
            }
                return;
            default:
                // TODO: log unknown connection attempt
                return;
        }
    });
}

uint64_t CAgentChannel::getId() const
{
    return m_id;
}

void CAgentChannel::setId(uint64_t _id)
{
    m_id = _id;
}

uint64_t CAgentChannel::getTaskID() const
{
    return m_taskID;
}

void CAgentChannel::setTaskID(uint64_t _taskID)
{
    m_taskID = _taskID;
}

const SHostInfoCmd& CAgentChannel::getRemoteHostInfo() const
{
    return m_remoteHostInfo;
}

void CAgentChannel::setRemoteHostInfo(const SHostInfoCmd& _hostInfo)
{
    m_remoteHostInfo = _hostInfo;
}

std::chrono::milliseconds CAgentChannel::getStartupTime() const
{
    return m_startUpTime;
}

EAgentState CAgentChannel::getState() const
{
    return m_state;
}

void CAgentChannel::setState(EAgentState _state)
{
    m_state = _state;
}

string CAgentChannel::_remoteEndIDString()
{
    if (getChannelType() == EChannelType::AGENT)
        return to_string(m_id);
    else
        return "UI client";
}

bool CAgentChannel::on_cmdSUBMIT(SCommandAttachmentImpl<cmdSUBMIT>::ptr_t _attachment)
{
    try
    {
        LOG(info) << "Received a Submit command; RMS: " << _attachment->m_sRMSType << " from: " << remoteEndIDString();
    }
    catch (exception& e)
    {
        pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(e.what(), MiscCommon::fatal, cmdSUBMIT));
        return true;
    }

    // The agent can't submit to the cluster by itself. Let others to process this message.
    return false;
}

bool CAgentChannel::on_cmdREPLY_HOST_INFO(SCommandAttachmentImpl<cmdREPLY_HOST_INFO>::ptr_t _attachment)
{
    m_remoteHostInfo = *_attachment;
    LOG(debug) << "cmdREPLY_HOST_INFO attachment [" << m_remoteHostInfo << "] received from: " << remoteEndIDString();

    // Calculating startup time of the agent
    m_startUpTime = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch());
    m_startUpTime -= chrono::milliseconds(_attachment->m_submitTime);
    // everything is OK, we can work with this agent
    LOG(info) << "The Agent [" << socket().remote_endpoint().address().to_string()
              << "] has successfully connected. Startup time: " << m_startUpTime.count() << " ms.";

    return true;
}

bool CAgentChannel::on_cmdGED_PID(SCommandAttachmentImpl<cmdGED_PID>::ptr_t _attachment)
{
    pid_t pid = getpid();
    SSimpleMsgCmd cmd_attachment;
    stringstream ss;
    ss << pid;
    cmd_attachment.m_sMsg = ss.str();
    pushMsg<cmdREPLY_PID>(cmd_attachment);

    return true;
}

bool CAgentChannel::on_cmdREPLY_ID(SCommandAttachmentImpl<cmdREPLY_ID>::ptr_t _attachment)
{
    // Return false. This message will be processed by ConnectionManager.
    return false;
}

bool CAgentChannel::on_cmdGET_LOG(SCommandAttachmentImpl<cmdGET_LOG>::ptr_t _attachment)
{
    // Return false. This message will be processed by ConnectionManager.
    return false;
}

bool CAgentChannel::on_cmdBINARY_ATTACHMENT_RECEIVED(
    SCommandAttachmentImpl<cmdBINARY_ATTACHMENT_RECEIVED>::ptr_t _attachment)
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

bool CAgentChannel::on_cmdGET_AGENTS_INFO(SCommandAttachmentImpl<cmdGET_AGENTS_INFO>::ptr_t _attachment)
{
    // Return false.
    // Give the possibility to further process this message.
    // For example, send information to UI.
    return false;
}

bool CAgentChannel::on_cmdTRANSPORT_TEST(SCommandAttachmentImpl<cmdTRANSPORT_TEST>::ptr_t _attachment)
{
    // Return false.
    // Give the possibility to further process this message.
    // For example, send information to UI.
    return false;
}

bool CAgentChannel::on_cmdSIMPLE_MSG(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment)
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

bool CAgentChannel::on_cmdUPDATE_KEY(SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment)
{
    // Return false.
    // The command can only be processed by the higher level object
    return false;
}

bool CAgentChannel::on_cmdUSER_TASK_DONE(SCommandAttachmentImpl<cmdUSER_TASK_DONE>::ptr_t _attachment)
{
    // Return false.
    // The command can only be processed by the higher level object
    return false;
}

bool CAgentChannel::on_cmdWATCHDOG_HEARTBEAT(SCommandAttachmentImpl<cmdWATCHDOG_HEARTBEAT>::ptr_t _attachment)
{
    // The main reason for this message is to tell commander that agents are note idle (see. GH-54)
    LOG(debug) << "Received Watchdog heartbeat from agent " << m_id << " running task = " << m_taskID;

    // TODO: So far we do nothing with this info.
    // In the future we might want to send more information about tasks being executed (pid, CPU info, memory)
    return true;
}

bool CAgentChannel::on_cmdGET_PROP_LIST(SCommandAttachmentImpl<cmdGET_PROP_LIST>::ptr_t _attachment)
{
    return false;
}

bool CAgentChannel::on_cmdGET_PROP_VALUES(SCommandAttachmentImpl<cmdGET_PROP_VALUES>::ptr_t _attachment)
{
    return false;
}

bool CAgentChannel::on_cmdUPDATE_TOPOLOGY(SCommandAttachmentImpl<cmdUPDATE_TOPOLOGY>::ptr_t _attachment)
{
    return false;
}

bool CAgentChannel::on_cmdENABLE_STAT(
    protocol_api::SCommandAttachmentImpl<protocol_api::cmdENABLE_STAT>::ptr_t _attachment)
{
    return false;
}

bool CAgentChannel::on_cmdDISABLE_STAT(
    protocol_api::SCommandAttachmentImpl<protocol_api::cmdDISABLE_STAT>::ptr_t _attachment)
{
    return false;
}

bool CAgentChannel::on_cmdGET_STAT(protocol_api::SCommandAttachmentImpl<protocol_api::cmdGET_STAT>::ptr_t _attachment)
{
    return false;
}

bool CAgentChannel::on_cmdCUSTOM_CMD(
    protocol_api::SCommandAttachmentImpl<protocol_api::cmdCUSTOM_CMD>::ptr_t _attachment)
{
    return false;
}
