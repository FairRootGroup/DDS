// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "AgentChannel.h"
// BOOST
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-register"
#include <boost/uuid/uuid_generators.hpp>
#pragma clang diagnostic pop
#include <boost/filesystem.hpp>

using namespace MiscCommon;
using namespace dds;
using namespace std;

void CAgentChannel::onHeaderRead()
{
}

const boost::uuids::uuid& CAgentChannel::getId() const
{
    return m_id;
}

uint64_t CAgentChannel::getTaskID() const
{
    return m_taskID;
}

void CAgentChannel::setTaskID(uint64_t _taskID)
{
    m_taskID = _taskID;
}

void CAgentChannel::onHandshakeOK()
{
    switch (getChannelType())
    {
        case EChannelType::AGENT:
        {
            pushMsg<cmdGET_UUID>();
            pushMsg<cmdGET_HOST_INFO>();
        }
            return;
        case EChannelType::UI:
            LOG(info) << "The UI agent [" << socket().remote_endpoint().address().to_string()
                      << "] has successfully connected.";
            return;
        default:
            // TODO: log unknown connection attempt
            return;
    }
}

void CAgentChannel::onHandshakeERR()
{
}

bool CAgentChannel::on_cmdSUBMIT(SCommandAttachmentImpl<cmdSUBMIT>::ptr_t _attachment)
{
    try
    {
        LOG(info) << "Recieved a Submit command of the topo [" << _attachment->m_sTopoFile
                  << "]; RMS: " << _attachment->RMSTypeCodeToString[_attachment->m_nRMSTypeCode]
                  << " from: " << remoteEndIDString();

        // check, that topo file exists
        if (!boost::filesystem::exists(_attachment->m_sTopoFile))
        {
            string sMsg("Can't find the topo file: ");
            sMsg += _attachment->m_sTopoFile;
            throw runtime_error(sMsg);
        }
    }
    catch (exception& e)
    {
        pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(e.what(), MiscCommon::fatal, cmdSUBMIT));
        return true;
    }

    // The agent can't submit to the cluster by itself. Let others to process this message.
    return false;
}

bool CAgentChannel::on_cmdACTIVATE_AGENT(SCommandAttachmentImpl<cmdACTIVATE_AGENT>::ptr_t _attachment)
{
    // The agent channel can't access all agents, therefore other should process this message.
    return false;
}

bool CAgentChannel::on_cmdSTOP_USER_TASK(SCommandAttachmentImpl<cmdSTOP_USER_TASK>::ptr_t _attachment)
{
    // The agent channel can't access all agents, therefore other should process this message.
    return false;
}

bool CAgentChannel::on_cmdREPLY_HOST_INFO(SCommandAttachmentImpl<cmdREPLY_HOST_INFO>::ptr_t _attachment)
{
    m_remoteHostInfo = *_attachment;
    LOG(debug) << "cmdREPLY_HOST_INFO attachment [" << m_remoteHostInfo << "] received from: " << remoteEndIDString();

    // Calculating startup time of the agent
    m_startUpTime = chrono::duration_cast<chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    m_startUpTime -= std::chrono::milliseconds(_attachment->m_submitTime);
    // everything is OK, we can work with this agent
    LOG(info) << "The Agent [" << socket().remote_endpoint().address().to_string()
              << "] has succesfully connected. Startup time: " << m_startUpTime.count() << " ms.";

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

bool CAgentChannel::on_cmdREPLY_UUID(SCommandAttachmentImpl<cmdREPLY_UUID>::ptr_t _attachment)
{
    LOG(debug) << "cmdREPLY_GET_UUID attachment [" << *_attachment << "] received from: " << remoteEndIDString();

    if (_attachment->m_id.is_nil())
    {
        // If UUID was not assigned to agent than generate new UUID and send it to agent
        m_id = boost::uuids::random_generator()();
        SUUIDCmd msg_cmd;
        msg_cmd.m_id = m_id;
        pushMsg<cmdSET_UUID>(msg_cmd);
    }
    else
    {
        m_id = _attachment->m_id;
    }

    return true;
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
