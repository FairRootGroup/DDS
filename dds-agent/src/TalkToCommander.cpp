// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "TalkToCommander.h"
#include "UserDefaults.h"
#include "Process.h"
// BOOST
#include "boost/crc.hpp"

using namespace MiscCommon;
using namespace dds;
using namespace std;

CTalkToCommander::CTalkToCommander(boost::asio::io_service& _service)
    : CConnectionImpl<CTalkToCommander>(_service)
    , m_isHandShakeOK(false)
{
}

int CTalkToCommander::on_cmdREPLY_HANDSHAKE_OK(const CProtocolMessage& _msg)
{
    m_isHandShakeOK = true;

    return 0;
}

int CTalkToCommander::on_cmdSIMPLE_MSG(const CProtocolMessage& _msg)
{
    return 0;
}

int CTalkToCommander::on_cmdGET_HOST_INFO(const CProtocolMessage& _msg)
{
    // Create the command's attachment
    string pidFileName(CUserDefaults::getDDSPath());
    pidFileName += "dds-agent.pid";
    pid_t pid = CPIDFile::GetPIDFromFile(pidFileName);

    SHostInfoCmd cmd;
    get_cuser_name(&cmd.m_username);
    get_hostname(&cmd.m_host);
    cmd.m_version = PROJECT_VERSION_STRING;
    cmd.m_DDSPath = CUserDefaults::getDDSPath();
    cmd.m_agentPort = 0;
    cmd.m_agentPid = pid;
    cmd.m_timeStamp = 0;

    CProtocolMessage msg;
    msg.encodeWithAttachment<cmdREPLY_HOST_INFO>(cmd);
    pushMsg(msg);
    return 0;
}

int CTalkToCommander::on_cmdDISCONNECT(const CProtocolMessage& _msg)
{
    stop();
    LOG(info) << "Agent disconnected...Bye";
    return 0;
}

int CTalkToCommander::on_cmdBINARY_ATTACHMENT(const CProtocolMessage& _msg)
{
    SBinaryAttachmentCmd cmd;
    cmd.convertFromData(_msg.bodyToContainer());

    std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    uint32_t downloadTime = now - cmd.m_timestamp;

    // Calculate CRC32 of the recieved file data
    boost::crc_32_type crc;
    crc.process_bytes(&cmd.m_fileData[0], cmd.m_fileData.size());

    // Form reply command
    SBinaryDownloadStatCmd reply_cmd;
    reply_cmd.m_recievedCrc32 = crc.checksum();
    reply_cmd.m_recievedFileSize = cmd.m_fileData.size();
    reply_cmd.m_downloadTime = downloadTime;

    CProtocolMessage msg;
    msg.encodeWithAttachment<cmdBINARY_DOWNLOAD_STAT>(reply_cmd);
    pushMsg(msg);
    return 0;
}
