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

        // Send test binary attachment command
        const MiscCommon::BYTEVector_t testData1{ 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
                                                  'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z' };

        // Calculate CRC32 of the test file data
        boost::crc_32_type crc;
        crc.process_bytes(&testData1[0], testData1.size());

        SBinaryAttachmentCmd bin_cmd;
        bin_cmd.m_crc32 = crc.checksum();
        bin_cmd.m_fileData = testData1;
        bin_cmd.m_fileName = "test_data_1.bin";
        bin_cmd.m_fileSize = testData1.size();
        bin_cmd.m_timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

        CProtocolMessage msg;
        msg.encodeWithAttachment<cmdBINARY_ATTACHMENT>(bin_cmd);
        pushMsg(msg);
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
