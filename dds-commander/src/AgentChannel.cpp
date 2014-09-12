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
#include <boost/crc.hpp>
#include <boost/filesystem.hpp>

using namespace MiscCommon;
using namespace dds;
using namespace std;

void CAgentChannel::onHeaderRead()
{
}

EAgentChannelType CAgentChannel::getType() const
{
    return m_type;
}

const boost::uuids::uuid& CAgentChannel::getId() const
{
    return m_id;
}

bool CAgentChannel::on_cmdHANDSHAKE(SCommandAttachmentImpl<cmdHANDSHAKE>::ptr_t _attachment)
{
    // send shutdown if versions are incompatible
    if (*_attachment != SVersionCmd())
    {
        m_isHandShakeOK = false;
        // Send reply that the version of the protocol is incompatible
        LOG(warning) << "Incompatible protocol version of the client: " << remoteEndIDString();
        pushMsg<cmdREPLY_ERR_BAD_PROTOCOL_VERSION>();
    }
    else
    {
        m_isHandShakeOK = true;
        m_type = EAgentChannelType::UI;
        // everything is OK, we can work with this agent
        LOG(info) << "The Agent [" << socket().remote_endpoint().address().to_string()
                  << "] has successfully connected.";

        pushMsg<cmdREPLY_HANDSHAKE_OK>();
    }
    return true;
}

bool CAgentChannel::on_cmdHANDSHAKE_AGENT(SCommandAttachmentImpl<cmdHANDSHAKE_AGENT>::ptr_t _attachment)
{
    // send shutdown if versions are incompatible
    if (*_attachment != SVersionCmd())
    {
        m_isHandShakeOK = false;
        // Send reply that the version of the protocol is incompatible
        LOG(warning) << "Incompatible protocol version of the client: " << remoteEndIDString();
        CProtocolMessage::protocolMessagePtr_t msg = make_shared<CProtocolMessage>();
        msg->encode<cmdREPLY_ERR_BAD_PROTOCOL_VERSION>();
        pushMsg(msg);
    }
    else
    {
        m_isHandShakeOK = true;
        m_type = EAgentChannelType::AGENT;
        // everything is OK, we can work with this agent
        LOG(info) << "The Agent [" << socket().remote_endpoint().address().to_string()
                  << "] has succesfully connected.";

        // replay on handshake in sync push, to preserver order of messages. Otherwise the replay could be send after
        // other requests are sent and other will be ignored by the agent as there were no handshake ok received yet.
        syncPushMsg<cmdREPLY_HANDSHAKE_OK>();
        pushMsg<cmdGET_UUID>();
        pushMsg<cmdGET_HOST_INFO>();
    }
    return true;
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
        SSimpleMsgCmd msg_cmd;
        msg_cmd.m_sMsg = e.what();
        msg_cmd.m_srcCommand = cmdSUBMIT;
        msg_cmd.m_msgSeverity = fatal;
        CProtocolMessage::protocolMessagePtr_t msg = make_shared<CProtocolMessage>();
        msg->encodeWithAttachment<cmdSIMPLE_MSG>(msg_cmd);
        pushMsg(msg);

        return true;
    }

    // The agent can't submit to the cluster by itself. Let others to process this message.
    return false;
}

bool CAgentChannel::on_cmdACTIVATE_AGENT(SCommandAttachmentImpl<cmdACTIVATE_AGENT>::ptr_t _attachment)
{
    // The agent channel can't activate all agents. Let others to process this message.
    return false;
}

bool CAgentChannel::on_cmdREPLY_HOST_INFO(SCommandAttachmentImpl<cmdREPLY_HOST_INFO>::ptr_t _attachment)
{
    m_remoteHostInfo = *_attachment;
    LOG(debug) << "cmdREPLY_HOST_INFO attachment [" << m_remoteHostInfo << "] received from: " << remoteEndIDString();
    return true;
}

bool CAgentChannel::on_cmdGED_PID(SCommandAttachmentImpl<cmdGED_PID>::ptr_t _attachment)
{
    pid_t pid = getpid();
    SSimpleMsgCmd cmd_attachment;
    stringstream ss;
    ss << pid;
    cmd_attachment.m_sMsg = ss.str();
    CProtocolMessage::protocolMessagePtr_t msg = make_shared<CProtocolMessage>();
    msg->encodeWithAttachment<cmdREPLY_PID>(cmd_attachment);
    pushMsg(msg);

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
        CProtocolMessage::protocolMessagePtr_t msg = make_shared<CProtocolMessage>();
        msg->encodeWithAttachment<cmdSET_UUID>(msg_cmd);
        pushMsg(msg);
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

bool CAgentChannel::on_cmdBINARY_ATTACHMENT(SCommandAttachmentImpl<cmdBINARY_ATTACHMENT>::ptr_t _attachment)
{
    switch (_attachment->m_srcCommand)
    {
        case cmdGET_LOG:
        {
            // Calculate CRC32 of the recieved file data
            boost::crc_32_type crc32;
            crc32.process_bytes(&_attachment->m_data[0], _attachment->m_data.size());

            if (crc32.checksum() == _attachment->m_fileCrc32)
            {
                const string sLogStorageDir(CUserDefaults::instance().getAgentLogStorageDir());
                const string logFileName(sLogStorageDir + _attachment->m_fileName);
                ofstream f(logFileName.c_str());
                if (!f.is_open() || !f.good())
                {
                    string msg("Could not open log archive: " + logFileName);
                    LOG(error) << msg;
                    return false;
                }

                for (const auto& v : _attachment->m_data)
                {
                    f << v;
                }
            }
            else
            {
                LOG(error) << "Recieved LOG file with wrong CRC32 checksum: " << crc32.checksum() << " instead of "
                           << _attachment->m_fileCrc32;
            }

            // Return false.
            // Give possibility to further process this message.
            return false;
        }

        default:
            LOG(debug) << "Received BINARY_ATTACHMENT has no listener.";
            return true;
    }
    return true;
}

bool CAgentChannel::on_cmdBINARY_DOWNLOAD_STAT(SCommandAttachmentImpl<cmdBINARY_DOWNLOAD_STAT>::ptr_t _attachment)
{
    switch (_attachment->m_srcCommand)
    {
        case cmdTRANSPORT_TEST:
        {
            LOG(info) << "cmdDOWNLOAD_TEST_STAT attachment [" << *_attachment << "] command from "
                      << remoteEndIDString();

            return false;
        }

        default:
            LOG(debug) << "Received command cmdBINARY_DOWNLOAD_STAT does not have a listener";
            return true;
    }

    return true;
}

bool CAgentChannel::on_cmdGET_AGENTS_INFO(SCommandAttachmentImpl<cmdGET_AGENTS_INFO>::ptr_t _attachment)
{
    // Return false.
    // Give possibility to further process this message.
    // For example, send information to UI.
    return false;
}

bool CAgentChannel::on_cmdTRANSPORT_TEST(SCommandAttachmentImpl<cmdTRANSPORT_TEST>::ptr_t _attachment)
{
    // Return false.
    // Give possibility to further process this message.
    // For example, send information to UI.
    return false;
}

bool CAgentChannel::on_cmdSIMPLE_MSG(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment)
{
    LOG(info) << "on_cmdSIMPLE_MSG attachment [" << *_attachment << "] command from " << remoteEndIDString();

    switch (_attachment->m_srcCommand)
    {
        case cmdACTIVATE_AGENT:
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
