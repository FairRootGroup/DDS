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
    // Create a message
    SVersionCmd ver_src;
    ver_src.m_version = 444;
    CProtocolMessage msg_src;
    msg_src.encodeWithAttachment<cmdHANDSHAKE>(ver_src);

    BOOST_CHECK(msg_src.header().m_cmd == cmdHANDSHAKE);

    // "Send" message
    CProtocolMessage msg_dest;
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
    // Create a message
    SVersionCmd ver_src;
    ver_src.m_version = 777;
    BYTEVector_t data_to_send;
    ver_src.convertToData(&data_to_send);
    CProtocolMessage msg_src;
    msg_src.encode_message(cmdHANDSHAKE_AGENT, data_to_send);

    BOOST_CHECK(msg_src.header().m_cmd == cmdHANDSHAKE_AGENT);

    // "Send" message
    CProtocolMessage msg_dest;
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
    // Create a message
    SSubmitCmd cmd_src;
    cmd_src.m_sTopoFile = sTestPath;
    CProtocolMessage msg_src;
    msg_src.encodeWithAttachment<cmdSUBMIT>(cmd_src);

    BOOST_CHECK(msg_src.header().m_cmd == cmdSUBMIT);

    // "Send" message
    CProtocolMessage msg_dest;
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

    // Create a message
    SHostInfoCmd cmd_src;
    cmd_src.m_username = sUsername;
    cmd_src.m_host = sHost;
    cmd_src.m_version = sVersion;
    cmd_src.m_DDSPath = sDDSPath;
    cmd_src.m_agentPort = nAgentPort;
    cmd_src.m_agentPid = nAgentPid;
    cmd_src.m_timeStamp = nTimeStamp;

    BYTEVector_t data_to_send;
    cmd_src.convertToData(&data_to_send);
    CProtocolMessage msg_src;
    msg_src.encode_message(cmdREPLY_HOST_INFO, data_to_send);

    BOOST_CHECK(msg_src.header().m_cmd == cmdREPLY_HOST_INFO);

    // "Send" message
    CProtocolMessage msg_dest;
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

BOOST_AUTO_TEST_SUITE_END();
