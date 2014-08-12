// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "AgentChannel.h"
// BOOST
#include <boost/uuid/uuid_generators.hpp>

using namespace MiscCommon;
using namespace dds;
using namespace std;

void CAgentChannel::onHeaderRead()
{
}

int CAgentChannel::on_cmdHANDSHAKE(const CProtocolMessage& _msg)
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
        m_type = ETalkToAgentType::UI;
        // everything is OK, we can work with this agent
        LOG(info) << "The Agent [" << socket().remote_endpoint().address().to_string()
                  << "] has succesfully connected.";

        pushMsg<cmdREPLY_HANDSHAKE_OK>();
    }
    return 0;
}

int CAgentChannel::on_cmdHANDSHAKE_AGENT(const CProtocolMessage& _msg)
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
        m_type = ETalkToAgentType::AGENT;
        // everything is OK, we can work with this agent
        LOG(info) << "The Agent [" << socket().remote_endpoint().address().to_string()
                  << "] has succesfully connected.";

        pushMsg<cmdREPLY_HANDSHAKE_OK>();
        pushMsg<cmdGET_UUID>();
        pushMsg<cmdGET_HOST_INFO>();
    }
    return 0;
}

int CAgentChannel::on_cmdSUBMIT(const CProtocolMessage& _msg)
{
    SSubmitCmd cmd;
    cmd.convertFromData(_msg.bodyToContainer());
    LOG(info) << "Recieved a Submit command of the topo [" << cmd.m_sTopoFile
              << "]; RMS: " << cmd.RMSTypeCodeToString[cmd.m_nRMSTypeCode]
              << " from: " << socket().remote_endpoint().address().to_string();

    if (cmd.m_nRMSTypeCode == SSubmitCmd::SSH)
    {
        LOG(info) << "SSH RMS is defined by: [" << cmd.m_sSSHCfgFile << "]";
    }

    // TODO: Implement me. So far we always send OK
    SSimpleMsgCmd msg_cmd;
    msg_cmd.m_sMsg = "Dummy job info, JOBIds";
    CProtocolMessage msg;
    msg.encodeWithAttachment<cmdREPLY_SUBMIT_OK>(msg_cmd);
    pushMsg(msg);

    return 0;
}

int CAgentChannel::on_cmdREPLY_HOST_INFO(const CProtocolMessage& _msg)
{
    SHostInfoCmd cmd;
    cmd.convertFromData(_msg.bodyToContainer());

    LOG(info) << "Recieved a cmdREPLY_HOST_INFO [" << cmd
              << "] command from: " << socket().remote_endpoint().address().to_string();

    // pushMsg<cmdDISCONNECT>();

    return 0;
}

int CAgentChannel::on_cmdGED_PID(const CProtocolMessage& _msg)
{
    pid_t pid = getpid();
    SSimpleMsgCmd cmd_attachment;
    stringstream ss;
    ss << pid;
    cmd_attachment.m_sMsg = ss.str();
    CProtocolMessage msg;
    msg.encodeWithAttachment<cmdREPLY_PID>(cmd_attachment);
    pushMsg(msg);

    return 0;
}

int CAgentChannel::on_cmdBINARY_DOWNLOAD_STAT(const CProtocolMessage& _msg)
{
    SBinaryDownloadStatCmd cmd;
    cmd.convertFromData(_msg.bodyToContainer());

    LOG(info) << "Recieved a cmdBINARY_DOWNLOAD_STAT [" << cmd
              << "] command from: " << socket().remote_endpoint().address().to_string();

    return 0;
}

int CAgentChannel::on_cmdREPLY_GET_UUID(const CProtocolMessage& _msg)
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

    return 0;
}

int CAgentChannel::on_cmdGET_LOG(const CProtocolMessage& _msg)
{
    LOG(info) << "Recieved a cmdGET_LOG command from: " << socket().remote_endpoint().address().to_string();

    return 0;
}
