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

        pushMsg<cmdREPLY_HANDSHAKE_OK>();
        pushMsg<cmdGET_UUID>();
        pushMsg<cmdGET_HOST_INFO>();
    }
    return true;
}

bool CAgentChannel::on_cmdSUBMIT(const CProtocolMessage& _msg)
{
    SSubmitCmd cmd;
    cmd.convertFromData(_msg.bodyToContainer());
    LOG(info) << "Recieved a Submit command of the topo [" << cmd.m_sTopoFile
              << "]; RMS: " << cmd.RMSTypeCodeToString[cmd.m_nRMSTypeCode]
              << " from: " << socket().remote_endpoint().address().to_string();

    if (cmd.m_nRMSTypeCode == SSubmitCmd::SSH)
    {
        LOG(info) << "SSH RMS is defined by: [" << cmd.m_sSSHCfgFile << "]";

        // TODO: Job submission should be moved from here
        // -------
        // TODO: Resolve topology

        // Submitting the job
        string outPut;
        string sCommand("$DDS_LOCATION/bin/dds-ssh");
        smart_path(&sCommand);
        StringVector_t params;
        const size_t nCmdTimeout = 35; // in sec.
        params.push_back("-c" + cmd.m_sSSHCfgFile);
        params.push_back("submit");
        try
        {
            do_execv(sCommand, params, nCmdTimeout, &outPut);

            SSimpleMsgCmd msg_cmd;
            msg_cmd.m_sMsg = "Dummy job info, JOBIds";
            CProtocolMessage msg;
            msg.encodeWithAttachment<cmdREPLY_SUBMIT_OK>(msg_cmd);
            pushMsg(msg);
        }
        catch (exception& e)
        {
            ostringstream ss;
            ss << "Failed to process the task: " << e.what();
            LOG(error) << ss.str();
            SSimpleMsgCmd msg_cmd;
            msg_cmd.m_sMsg = ss.str();
            CProtocolMessage msg;
            msg.encodeWithAttachment<cmdREPLY_ERR_SUBMIT>(msg_cmd);
            pushMsg(msg);
        }
        if (!outPut.empty())
        {
            ostringstream ss;
            ss << "Cmnd Output: " << outPut;
            LOG(info) << ss.str();
            SSimpleMsgCmd msg_cmd;
            msg_cmd.m_sMsg = ss.str();
            CProtocolMessage msg;
            msg.encodeWithAttachment<cmdSIMPLE_MSG>(msg_cmd);
            pushMsg(msg);
        }

        // -------
    }

    return true;
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
        const string sAgentLogDir("log/agents/");
        string sLogDir(CUserDefaults::instance().getValueForKey("server.work_dir"));
        smart_path(&sLogDir);
        smart_append(&sLogDir, '/');
        sLogDir += sAgentLogDir;
        boost::filesystem::path dir(sLogDir);
        if (!boost::filesystem::exists(dir) && !boost::filesystem::create_directories(dir))
        {
            string msg("Could not create directory " + sLogDir + " to save log files.");
            throw runtime_error(msg);
        }

        const string logFileName(sLogDir + cmd.m_fileName);
        LOG(MiscCommon::info) << "Saving an agent LOG file: " << logFileName;
        ofstream f(logFileName.c_str());
        if (!f.is_open() || !f.good())
        {
            string msg("Could not open an agent LOG file: " + logFileName);
            throw runtime_error(msg);
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

bool CAgentChannel::on_cmdGET_AGENTS_INFO(const CProtocolMessage& _msg)
{
    LOG(info) << "Recieved a cmdGET_AGENTS_INFO command from: " << socket().remote_endpoint().address().to_string();

    // Return false.
    // Give possibility to further process this message.
    // For example, send information to UI.
    return false;
}

bool CAgentChannel::on_cmdGET_LOG_ERROR(const CProtocolMessage& _msg)
{
    SBinaryAttachmentCmd cmd;
    cmd.convertFromData(_msg.bodyToContainer());

    LOG(info) << "Recieved a cmdGET_LOG_ERROR [" << cmd
              << " ] command from: " << socket().remote_endpoint().address().to_string();

    // Return false. This message will be processed by ConnectionManager.
    return false;
}
