// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "AgentChannel.h"
// BOOST
#include <boost/uuid/uuid_generators.hpp>
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

bool CAgentChannel::on_cmdHANDSHAKE(CProtocolMessage::protocolMessagePtr_t _msg)
{
    SVersionCmd ver;
    ver.convertFromData(_msg->bodyToContainer());
    // send shutdown if versions are incompatible
    if (ver != SVersionCmd())
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

bool CAgentChannel::on_cmdHANDSHAKE_AGENT(CProtocolMessage::protocolMessagePtr_t _msg)
{
    SVersionCmd ver;
    ver.convertFromData(_msg->bodyToContainer());
    // send shutdown if versions are incompatible
    if (ver != SVersionCmd())
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

bool CAgentChannel::on_cmdSUBMIT(CProtocolMessage::protocolMessagePtr_t _msg)
{
    try
    {
        SSubmitCmd cmd;
        cmd.convertFromData(_msg->bodyToContainer());
        LOG(info) << "Recieved a Submit command of the topo [" << cmd.m_sTopoFile
                  << "]; RMS: " << cmd.RMSTypeCodeToString[cmd.m_nRMSTypeCode] << " from: " << remoteEndIDString();

        // check, that topo file exists
        if (!boost::filesystem::exists(cmd.m_sTopoFile))
        {
            string sMsg("Can't find the topo file: ");
            sMsg += cmd.m_sTopoFile;
            throw runtime_error(sMsg);
        }
    }
    catch (exception& e)
    {
        SSimpleMsgCmd msg_cmd;
        msg_cmd.m_sMsg = e.what();
        CProtocolMessage::protocolMessagePtr_t msg = make_shared<CProtocolMessage>();
        msg->encodeWithAttachment<cmdREPLY_ERR_SUBMIT>(msg_cmd);
        pushMsg(msg);

        return true;
    }

    // The agent can't submit to the cluster by itself. Let others to process this message.
    return false;
}

bool CAgentChannel::on_cmdSUBMIT_START(CProtocolMessage::protocolMessagePtr_t _msg)
{
    // The agent channel can't activate all agents. Let others to process this message.
    return false;
}

bool CAgentChannel::on_cmdREPLY_HOST_INFO(CProtocolMessage::protocolMessagePtr_t _msg)
{
    m_remoteHostInfo.convertFromData(_msg->bodyToContainer());
    LOG(debug) << "cmdREPLY_HOST_INFO attachment [" << m_remoteHostInfo << "] received from: " << remoteEndIDString();
    return true;
}

bool CAgentChannel::on_cmdGED_PID(CProtocolMessage::protocolMessagePtr_t _msg)
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

bool CAgentChannel::on_cmdBINARY_DOWNLOAD_STAT(CProtocolMessage::protocolMessagePtr_t _msg)
{
    SBinaryDownloadStatCmd cmd;
    cmd.convertFromData(_msg->bodyToContainer());

    LOG(debug) << "cmdBINARY_DOWNLOAD_STAT attachment [" << cmd << "] received from: " << remoteEndIDString();

    return true;
}

bool CAgentChannel::on_cmdREPLY_UUID(CProtocolMessage::protocolMessagePtr_t _msg)
{
    SUUIDCmd cmd;
    cmd.convertFromData(_msg->bodyToContainer());

    LOG(debug) << "cmdREPLY_GET_UUID attachment [" << cmd << "] received from: " << remoteEndIDString();

    if (cmd.m_id.is_nil())
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
        m_id = cmd.m_id;
    }

    return true;
}

bool CAgentChannel::on_cmdGET_LOG(CProtocolMessage::protocolMessagePtr_t _msg)
{
    // Return false. This message will be processed by ConnectionManager.
    return false;
}

bool CAgentChannel::on_cmdBINARY_ATTACHMENT_LOG(CProtocolMessage::protocolMessagePtr_t _msg)
{
    SBinaryAttachmentCmd cmd;
    cmd.convertFromData(_msg->bodyToContainer());

    // Calculate CRC32 of the recieved file data
    boost::crc_32_type crc32;
    crc32.process_bytes(&cmd.m_fileData[0], cmd.m_fileData.size());

    if (crc32.checksum() == cmd.m_crc32)
    {
        const string sLogStorageDir(CUserDefaults::instance().getAgentLogStorageDir());
        const string logFileName(sLogStorageDir + cmd.m_fileName);
        ofstream f(logFileName.c_str());
        if (!f.is_open() || !f.good())
        {
            string msg("Could not open log archive: " + logFileName);
            LOG(error) << msg;
            return false;
        }

        for (const auto& v : cmd.m_fileData)
        {
            f << v;
        }
    }
    else
    {
        LOG(error) << "Recieved LOG file with wrong CRC32 checksum: " << crc32.checksum() << " instead of "
                   << cmd.m_crc32;
    }

    // Return false.
    // Give possibility to further process this message.
    // For example, send information to UI.
    return false;
}

bool CAgentChannel::on_cmdGET_LOG_ERROR(CProtocolMessage::protocolMessagePtr_t _msg)
{
    SSimpleMsgCmd cmd;
    cmd.convertFromData(_msg->bodyToContainer());
    LOG(debug) << "cmdGET_LOG_ERROR attachment [" << cmd << "] received from: " << remoteEndIDString();

    // Return false. This message will be processed by ConnectionManager.
    return false;
}

bool CAgentChannel::on_cmdGET_AGENTS_INFO(CProtocolMessage::protocolMessagePtr_t _msg)
{
    // Return false.
    // Give possibility to further process this message.
    // For example, send information to UI.
    return false;
}

bool CAgentChannel::on_cmdSTART_DOWNLOAD_TEST(CProtocolMessage::protocolMessagePtr_t _msg)
{
    // Return false.
    // Give possibility to further process this message.
    // For example, send information to UI.
    return false;
}

bool CAgentChannel::on_cmdDOWNLOAD_TEST_STAT(CProtocolMessage::protocolMessagePtr_t _msg)
{
    SBinaryDownloadStatCmd cmd;
    cmd.convertFromData(_msg->bodyToContainer());

    LOG(info) << "cmdDOWNLOAD_TEST_STAT attachment [" << cmd << "] command from " << remoteEndIDString();

    return false;
}

bool CAgentChannel::on_cmdDOWNLOAD_TEST_ERROR(CProtocolMessage::protocolMessagePtr_t _msg)
{
    SSimpleMsgCmd cmd;
    cmd.convertFromData(_msg->bodyToContainer());

    LOG(info) << "cmdDOWNLOAD_TEST_ERROR attachment [" << cmd << " ] command from " << remoteEndIDString();

    // Return false. This message will be processed by ConnectionManager.
    return false;
}

bool CAgentChannel::on_cmdSIMPLE_MSG(CProtocolMessage::protocolMessagePtr_t _msg)
{
    SSimpleMsgCmd cmd;
    cmd.convertFromData(_msg->bodyToContainer());

    LOG(info) << "on_cmdSIMPLE_MSG attachment [" << cmd << "] command from " << remoteEndIDString();

    switch (cmd.m_srcCommand)
    {
        case cmdACTIVATE_AGENT:
            return false; // let others to process this message

        default:
            LOG(static_cast<ELogSeverityLevel>(cmd.m_msgSeverity)) << "remote: " << cmd.m_sMsg;
            return true;
    }
}
