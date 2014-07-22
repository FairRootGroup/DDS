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
    BYTEVector_t data_to_send;
    ver_src.convertToData(&data_to_send);
    CProtocolMessage msg_src;
    msg_src.encode_message(cmdHANDSHAKE, data_to_send);

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

BOOST_AUTO_TEST_CASE(Test_ProtocolMessage_cmdSUBMIT)
{
    const string sTestPath = "/Users/dummy/Documents/workspace/dummy.xml";
    // Create a message
    SSubmitCmd cmd_src;
    cmd_src.m_sTopoFile = sTestPath;
    BYTEVector_t data_to_send;
    cmd_src.convertToData(&data_to_send);
    CProtocolMessage msg_src;
    msg_src.encode_message(cmdSUBMIT, data_to_send);

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

BOOST_AUTO_TEST_SUITE_END();
