// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "TestChannel.h"
// BOOST
#include <boost/crc.hpp>

using namespace MiscCommon;
using namespace dds;
using namespace std;

int CTestChannel::on_cmdHANDSHAKE_AGENT(const CProtocolMessage& _msg)
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
        // everything is OK, we can work with this agent
        LOG(info) << "The Agent [" << socket().remote_endpoint().address().to_string()
                  << "] has succesfully connected.";

        pushMsg<cmdREPLY_HANDSHAKE_OK>();

        sendTestBinaryAttachment(1000);
        sendTestBinaryAttachment(10000);
        sendTestBinaryAttachment(100000);
        sendTestBinaryAttachment(1000000);
        sendTestBinaryAttachment(10000000);
    }
    return 0;
}

int CTestChannel::on_cmdBINARY_DOWNLOAD_STAT(const CProtocolMessage& _msg)
{
    SBinaryDownloadStatCmd cmd;
    cmd.convertFromData(_msg.bodyToContainer());

    LOG(info) << "Recieved a DownloadStat [" << cmd
              << "] command from: " << socket().remote_endpoint().address().to_string();

    return 0;
}

void CTestChannel::sendTestBinaryAttachment(size_t _binarySize)
{
    SBinaryAttachmentCmd cmd;

    for (size_t i = 0; i < _binarySize; ++i)
    {
        char c = rand() % 256;
        cmd.m_fileData.push_back(c);
    }

    // Calculate CRC32 of the test file data
    boost::crc_32_type crc;
    crc.process_bytes(&cmd.m_fileData[0], cmd.m_fileData.size());

    cmd.m_crc32 = crc.checksum();
    cmd.m_fileName = "test_data_" + std::to_string(_binarySize) + ".bin";
    cmd.m_fileSize = cmd.m_fileData.size();
    cmd.m_timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    CProtocolMessage msg;
    msg.encodeWithAttachment<cmdBINARY_ATTACHMENT>(cmd);
    pushMsg(msg);
}
