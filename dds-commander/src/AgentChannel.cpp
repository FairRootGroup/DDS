// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "AgentChannel.h"
#include "Process.h"
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

bool CAgentChannel::on_cmdHANDSHAKE(const CProtocolMessage& _msg)
{
    SVersionCmd ver;
    ver.convertFromData(_msg.bodyToContainer());
    // send shutdown if versions are incompatible
    if (ver != SVersionCmd())
    {
        m_isHandShakeOK = false;
        // Send reply that the version of the protocol is incompatible
        LOG(warning) << "Client's protocol version is incompatable. Client: "
                     << socket().remote_endpoint().address().to_string();

        pushMsg<cmdREPLY_ERR_BAD_PROTOCOL_VERSION>();
    }
    else
    {
        m_isHandShakeOK = true;
        m_type = EAgentChannelType::UI;
        // everything is OK, we can work with this agent
        LOG(info) << "The Agent [" << socket().remote_endpoint().address().to_string()
                  << "] has succesfully connected.";

        pushMsg<cmdREPLY_HANDSHAKE_OK>();
    }
    return true;
}

bool CAgentChannel::on_cmdHANDSHAKE_AGENT(const CProtocolMessage& _msg)
{
    SVersionCmd ver;
    ver.convertFromData(_msg.bodyToContainer());
    // send shutdown if versions are incompatible
    if (ver != SVersionCmd())
    {
        m_isHandShakeOK = false;
        // Send reply that the version of the protocol is incompatible
        LOG(warning) << "Client's protocol version is incompatable. Client: "
                     << socket().remote_endpoint().address().to_string();
        CProtocolMessage msg;
        msg.encode<cmdREPLY_ERR_BAD_PROTOCOL_VERSION>();
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

bool CAgentChannel::on_cmdSUBMIT(const CProtocolMessage& _msg)
{
    try
    {
        SSubmitCmd cmd;
        cmd.convertFromData(_msg.bodyToContainer());
        LOG(info) << "Recieved a Submit command of the topo [" << cmd.m_sTopoFile
                  << "]; RMS: " << cmd.RMSTypeCodeToString[cmd.m_nRMSTypeCode]
                  << " from: " << socket().remote_endpoint().address().to_string();

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
        CProtocolMessage msg;
        msg.encodeWithAttachment<cmdREPLY_ERR_SUBMIT>(msg_cmd);
        pushMsg(msg);

        return true;
    }

    // The agent can't submit to the cluster by itself. Let others to process this message.
    return false;
}

bool CAgentChannel::on_cmdSUBMIT_START(const CProtocolMessage& _msg)
{
    LOG(info) << "Recieved request to start distribuiting user tasks from: "
              << socket().remote_endpoint().address().to_string();
    return false;
}

bool CAgentChannel::on_cmdREPLY_HOST_INFO(const CProtocolMessage& _msg)
{
    m_remoteHostInfo.convertFromData(_msg.bodyToContainer());

    LOG(info) << "Recieved a cmdREPLY_HOST_INFO [" << m_remoteHostInfo
              << "] command from: " << socket().remote_endpoint().address().to_string();

    return true;
}

bool CAgentChannel::on_cmdGED_PID(const CProtocolMessage& _msg)
{
    pid_t pid = getpid();
    SSimpleMsgCmd cmd_attachment;
    stringstream ss;
    ss << pid;
    cmd_attachment.m_sMsg = ss.str();
    CProtocolMessage msg;
    msg.encodeWithAttachment<cmdREPLY_PID>(cmd_attachment);
    pushMsg(msg);

    return true;
}

bool CAgentChannel::on_cmdBINARY_DOWNLOAD_STAT(const CProtocolMessage& _msg)
{
    SBinaryDownloadStatCmd cmd;
    cmd.convertFromData(_msg.bodyToContainer());

    LOG(info) << "Recieved a cmdBINARY_DOWNLOAD_STAT [" << cmd
              << "] command from: " << socket().remote_endpoint().address().to_string();

    return true;
}

bool CAgentChannel::on_cmdREPLY_UUID(const CProtocolMessage& _msg)
{
    SUUIDCmd cmd;
    cmd.convertFromData(_msg.bodyToContainer());

    LOG(info) << "Recieved a cmdREPLY_GET_UUID [" << cmd
              << "] command from: " << socket().remote_endpoint().address().to_string();

    if (cmd.m_id.is_nil())
    {
        // If UUID was not assigned to agent than generate new UUID and send it to agent
        m_id = boost::uuids::random_generator()();
        SUUIDCmd msg_cmd;
        msg_cmd.m_id = m_id;
        CProtocolMessage msg;
        msg.encodeWithAttachment<cmdSET_UUID>(msg_cmd);
        pushMsg(msg);
    }
    else
    {
        m_id = cmd.m_id;
    }

    return true;
}

bool CAgentChannel::on_cmdGET_LOG(const CProtocolMessage& _msg)
{
    LOG(info) << "Recieved a cmdGET_LOG command from: " << socket().remote_endpoint().address().to_string();

    // Return false. This message will be processed by ConnectionManager.
    return false;
}

bool CAgentChannel::on_cmdBINARY_ATTACHMENT_LOG(const CProtocolMessage& _msg)
{
    SBinaryAttachmentCmd cmd;
    cmd.convertFromData(_msg.bodyToContainer());

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

bool CAgentChannel::on_cmdGET_LOG_ERROR(const CProtocolMessage& _msg)
{
    SSimpleMsgCmd cmd;
    cmd.convertFromData(_msg.bodyToContainer());

    LOG(info) << "Recieved a cmdGET_LOG_ERROR [" << cmd
              << " ] command from: " << socket().remote_endpoint().address().to_string();

    // Return false. This message will be processed by ConnectionManager.
    return false;
}

bool CAgentChannel::on_cmdGET_AGENTS_INFO(const CProtocolMessage& _msg)
{
    LOG(info) << "Recieved a cmdGET_AGENTS_INFO command from: " << socket().remote_endpoint().address().to_string();

    // Return false.
    // Give possibility to further process this message.
    // For example, send information to UI.
    return false;
}
