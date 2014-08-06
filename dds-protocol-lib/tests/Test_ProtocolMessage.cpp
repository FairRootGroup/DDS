// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
// Unit tests
//
// BOOST: tests
// Defines test_main function to link with actual unit test code.
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
// DDS
#include "ProtocolMessage.h"
#include "ProtocolCommands.h"

using boost::unit_test::test_suite;
using namespace MiscCommon;
using namespace std;
using namespace dds;

BOOST_AUTO_TEST_SUITE(Test_ProtocolMessage);

//----------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(Test_ProtocolMessage_cmdHANDSHAKE)
{
    const unsigned int cmdSize = 2;

    // Create a message
    SVersionCmd ver_src;
    ver_src.m_version = 444;
    CProtocolMessage msg_src;
    msg_src.encodeWithAttachment<cmdHANDSHAKE>(ver_src);

    BOOST_CHECK(msg_src.header().m_cmd == cmdHANDSHAKE);

    BOOST_CHECK(ver_src.size() == cmdSize);

    // "Send" message
    CProtocolMessage msg_dest;
    msg_dest.resize(msg_src.length()); // resize internal buffer to appropriate size.
    memcpy(msg_dest.data(), msg_src.data(), msg_src.length());

    // Decode the message
    BOOST_CHECK(msg_dest.decode_header());

    // Check that we got the proper command ID
    BOOST_CHECK(msg_src.header().m_cmd == msg_dest.header().m_cmd);

    // Read the message
    SVersionCmd ver_dest;
    ver_dest.convertFromData(msg_dest.bodyToContainer());

    BOOST_CHECK(ver_src == ver_dest);
}

BOOST_AUTO_TEST_CASE(Test_ProtocolMessage_cmdHANDSHAKE_AGENT)
{
    const unsigned int cmdSize = 2;

    // Create a message
    SVersionCmd ver_src;
    ver_src.m_version = 777;
    CProtocolMessage msg_src;
    msg_src.encodeWithAttachment<cmdHANDSHAKE_AGENT>(ver_src);

    BOOST_CHECK(msg_src.header().m_cmd == cmdHANDSHAKE_AGENT);

    BOOST_CHECK(ver_src.size() == cmdSize);

    // "Send" message
    CProtocolMessage msg_dest;
    msg_dest.resize(msg_src.length()); // resize internal buffer to appropriate size.
    memcpy(msg_dest.data(), msg_src.data(), msg_src.length());

    // Decode the message
    BOOST_CHECK(msg_dest.decode_header());

    // Check that we got the proper command ID
    BOOST_CHECK(msg_src.header().m_cmd == msg_dest.header().m_cmd);

    // Read the message
    SVersionCmd ver_dest;
    ver_dest.convertFromData(msg_dest.bodyToContainer());

    BOOST_CHECK(ver_src == ver_dest);
}

BOOST_AUTO_TEST_CASE(Test_ProtocolMessage_cmdSUBMIT)
{
    const string sTestPath = "/Users/dummy/Documents/workspace/dummy.xml";
    const unsigned int cmdSize = 43;

    // Create a message
    SSubmitCmd cmd_src;
    cmd_src.m_sTopoFile = sTestPath;
    CProtocolMessage msg_src;
    msg_src.encodeWithAttachment<cmdSUBMIT>(cmd_src);

    BOOST_CHECK(msg_src.header().m_cmd == cmdSUBMIT);

    BOOST_CHECK(cmd_src.size() == cmdSize);

    // "Send" message
    CProtocolMessage msg_dest;
    msg_dest.resize(msg_src.length()); // resize internal buffer to appropriate size.
    memcpy(msg_dest.data(), msg_src.data(), msg_src.length());

    // Decode the message
    BOOST_CHECK(msg_dest.decode_header());

    // Check that we got the proper command ID
    BOOST_CHECK(msg_src.header().m_cmd == msg_dest.header().m_cmd);

    // Read the message
    SSubmitCmd cmd_dest;
    cmd_dest.convertFromData(msg_dest.bodyToContainer());

    BOOST_CHECK(cmd_src == cmd_dest);
    BOOST_CHECK(sTestPath == cmd_dest.m_sTopoFile);
}

BOOST_AUTO_TEST_CASE(Test_ProtocolMessage_cmdREPLY_HOST_INFO)
{
    const string sUsername = "username";
    const string sHost = "host.com";
    const string sVersion = "1.0.0";
    const string sDDSPath = "/Users/andrey/DDS";
    const uint16_t nAgentPort = 20000;
    const uint32_t nAgentPid = 1111;
    const uint32_t nTimeStamp = 111111111;
    const unsigned int cmdSize = 52;

    // Create a message
    SHostInfoCmd cmd_src;
    cmd_src.m_username = sUsername;
    cmd_src.m_host = sHost;
    cmd_src.m_version = sVersion;
    cmd_src.m_DDSPath = sDDSPath;
    cmd_src.m_agentPort = nAgentPort;
    cmd_src.m_agentPid = nAgentPid;
    cmd_src.m_timeStamp = nTimeStamp;
    CProtocolMessage msg_src;
    msg_src.encodeWithAttachment<cmdREPLY_HOST_INFO>(cmd_src);

    BOOST_CHECK(msg_src.header().m_cmd == cmdREPLY_HOST_INFO);

    BOOST_CHECK(cmd_src.size() == cmdSize);

    // "Send" message
    CProtocolMessage msg_dest;
    msg_dest.resize(msg_src.length()); // resize internal buffer to appropriate size.
    memcpy(msg_dest.data(), msg_src.data(), msg_src.length());

    // Decode the message
    BOOST_CHECK(msg_dest.decode_header());

    // Check that we got the proper command ID
    BOOST_CHECK(msg_src.header().m_cmd == msg_dest.header().m_cmd);

    // Read the message
    SHostInfoCmd cmd_dest;
    cmd_dest.convertFromData(msg_dest.bodyToContainer());

    BOOST_CHECK(cmd_src == cmd_dest);
    BOOST_CHECK(sUsername == cmd_dest.m_username);
    BOOST_CHECK(sHost == cmd_dest.m_host);
    BOOST_CHECK(sVersion == cmd_dest.m_version);
    BOOST_CHECK(sDDSPath == cmd_dest.m_DDSPath);
    BOOST_CHECK(nAgentPort == cmd_dest.m_agentPort);
    BOOST_CHECK(nAgentPid == cmd_dest.m_agentPid);
    BOOST_CHECK(nTimeStamp == cmd_dest.m_timeStamp);
}

BOOST_AUTO_TEST_CASE(Test_ProtocolMessage_cmdBINARY_ATTACHMENT)
{
    const uint32_t crc32 = 1000;
    const string fileName = "filename.exe";
    const uint32_t fileSize = 26;
    const MiscCommon::BYTEVector_t fileData{ 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
                                             'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z' };
    const unsigned int cmdSize = 51;

    // Create a message
    SBinaryAttachmentCmd cmd_src;
    cmd_src.m_crc32 = crc32;
    cmd_src.m_fileName = fileName;
    cmd_src.m_fileSize = fileSize;
    cmd_src.m_fileData = fileData;
    CProtocolMessage msg_src;
    msg_src.encodeWithAttachment<cmdBINARY_ATTACHMENT>(cmd_src);

    BOOST_CHECK(msg_src.header().m_cmd == cmdBINARY_ATTACHMENT);

    BOOST_CHECK(cmd_src.size() == cmdSize);

    // "Send" message
    CProtocolMessage msg_dest;
    msg_dest.resize(msg_src.length()); // resize internal buffer to appropriate size.
    memcpy(msg_dest.data(), msg_src.data(), msg_src.length());

    // Decode the message
    BOOST_CHECK(msg_dest.decode_header());

    // Check that we got the proper command ID
    BOOST_CHECK(msg_src.header().m_cmd == msg_dest.header().m_cmd);

    // Read the message
    SBinaryAttachmentCmd cmd_dest;
    cmd_dest.convertFromData(msg_dest.bodyToContainer());

    BOOST_CHECK(cmd_src == cmd_dest);
    BOOST_CHECK(crc32 == cmd_dest.m_crc32);
    BOOST_CHECK(fileName == cmd_dest.m_fileName);
    BOOST_CHECK(fileSize == cmd_dest.m_fileSize);
    unsigned int i = 0;
    for (auto c : fileData)
    {
        BOOST_CHECK(c == cmd_dest.m_fileData[i]);
        i++;
    }
}

BOOST_AUTO_TEST_CASE(Test_ProtocolMessage_cmdBINARY_DOWNLOAD_STAT)
{
    const uint32_t recievedCrc32 = 1000;
    const uint32_t recievedFileSize = 26;
    const uint32_t downloadTime = 221;
    const unsigned int cmdSize = 12;

    // Create a message
    SBinaryDownloadStatCmd cmd_src;
    cmd_src.m_recievedFileSize = recievedFileSize;
    cmd_src.m_recievedCrc32 = recievedCrc32;
    cmd_src.m_downloadTime = downloadTime;
    CProtocolMessage msg_src;
    msg_src.encodeWithAttachment<cmdBINARY_DOWNLOAD_STAT>(cmd_src);

    BOOST_CHECK(msg_src.header().m_cmd == cmdBINARY_DOWNLOAD_STAT);

    BOOST_CHECK(cmd_src.size() == cmdSize);

    // "Send" message
    CProtocolMessage msg_dest;
    msg_dest.resize(msg_src.length()); // resize internal buffer to appropriate size.
    memcpy(msg_dest.data(), msg_src.data(), msg_src.length());

    // Decode the message
    BOOST_CHECK(msg_dest.decode_header());

    // Check that we got the proper command ID
    BOOST_CHECK(msg_src.header().m_cmd == msg_dest.header().m_cmd);

    // Read the message
    SBinaryDownloadStatCmd cmd_dest;
    cmd_dest.convertFromData(msg_dest.bodyToContainer());

    BOOST_CHECK(cmd_src == cmd_dest);
    BOOST_CHECK(recievedCrc32 == cmd_dest.m_recievedCrc32);
    BOOST_CHECK(recievedFileSize == cmd_dest.m_recievedFileSize);
    BOOST_CHECK(downloadTime == cmd_src.m_downloadTime);
}

BOOST_AUTO_TEST_SUITE_END();
