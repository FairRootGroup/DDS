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

BOOST_AUTO_TEST_CASE(Test_ProtocolMessage_cmdVERSION)
{
    // Create a message
    SVersionCmd ver_src;
    ver_src.m_version = 444;
    BYTEVector_t ver;
    ver_src.convertToData(&ver);
    CProtocolMessage msg_src;
    msg_src.encode_message(cmdVERSION, ver);

    // "Send" message
    CProtocolMessage msg_dest;
    memcpy(msg_dest.data(), msg_src.data(), msg_src.length());

    // Decode the message
    BOOST_CHECK(msg_dest.decode_header());

    // Read the message
    SVersionCmd ver_dest;
    BYTEVector_t buf(msg_dest.body(), msg_dest.body() + msg_dest.body_length());
    ver_dest.convertFromData(buf);

    BOOST_CHECK(ver_src == ver_dest);
}

BOOST_AUTO_TEST_SUITE_END();
