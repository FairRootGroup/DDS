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

// BOOST
#include <boost/test/unit_test.hpp>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-register"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#pragma clang diagnostic pop

// DDS
#include "ProtocolMessage.h"
#include "ProtocolCommands.h"
#include "CommandAttachmentImpl.h"
#include "def.h"

using boost::unit_test::test_suite;
using namespace MiscCommon;
using namespace std;
using namespace dds;

BOOST_AUTO_TEST_SUITE(Test_ProtocolMessage);

//----------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(Test_ProtocolMessage_cmdHANDSHAKE)
{
    const unsigned int cmdSize = 4;

    // Create a message
    SVersionCmd ver_src;
    ver_src.m_version = 444;
    ver_src.m_channelType = 2;
    MiscCommon::BYTEVector_t data;
    ver_src.convertToData(&data);
    CProtocolMessage msg_src;
    msg_src.encode(cmdHANDSHAKE, data);

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

BOOST_AUTO_TEST_CASE(Test_ProtocolMessage_cmdSUBMIT)
{
    const string sTestPath = "/Users/dummy/Documents/workspace/dummy.xml";
    const uint16_t nTestRMSTypeCode = 1;
    const string sTestSSHCfgFile = "/Users/dummy/dummy.cfg";
    const unsigned int cmdSize = 70;

    // Create a message
    SSubmitCmd cmd_src;
    cmd_src.m_sTopoFile = sTestPath;
    cmd_src.m_nRMSTypeCode = nTestRMSTypeCode;
    cmd_src.m_sSSHCfgFile = sTestSSHCfgFile;
    MiscCommon::BYTEVector_t data;
    cmd_src.convertToData(&data);
    CProtocolMessage msg_src;
    msg_src.encode(cmdSUBMIT, data);

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
    const uint64_t lSubmitTime = 23465677;
    const unsigned int cmdSize = 56;

    // Create a message
    SHostInfoCmd cmd_src;
    cmd_src.m_username = sUsername;
    cmd_src.m_host = sHost;
    cmd_src.m_version = sVersion;
    cmd_src.m_DDSPath = sDDSPath;
    cmd_src.m_agentPort = nAgentPort;
    cmd_src.m_agentPid = nAgentPid;
    cmd_src.m_submitTime = lSubmitTime;
    MiscCommon::BYTEVector_t data;
    cmd_src.convertToData(&data);
    CProtocolMessage msg_src;
    msg_src.encode(cmdREPLY_HOST_INFO, data);

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
    BOOST_CHECK(lSubmitTime == cmd_dest.m_submitTime);
}

BOOST_AUTO_TEST_CASE(Test_ProtocolMessage_cmdBINARY_ATTACHMENT)
{
    // const uint32_t crc32 = 1000;
    const string fileName = "filename.exe";
    // const uint32_t fileSize = 26;
    const MiscCommon::BYTEVector_t fileData{ 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
                                             'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z' };
    //  const unsigned int cmdSize = 47;

    // Create a message
    SBinaryAttachmentCmd cmd_src;
    // cmd_src.m_fileCrc32 = crc32;
    // cmd_src.m_fileName = fileName;
    // cmd_src.m_fileSize = fileSize;
    cmd_src.m_data = fileData;
    MiscCommon::BYTEVector_t data;
    cmd_src.convertToData(&data);
    CProtocolMessage msg_src;
    msg_src.encode(cmdBINARY_ATTACHMENT, data);

    BOOST_CHECK(msg_src.header().m_cmd == cmdBINARY_ATTACHMENT);

    //  BOOST_CHECK(cmd_src.size() == cmdSize);

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
    //  BOOST_CHECK(crc32 == cmd_dest.m_fileCrc32);
    //  BOOST_CHECK(fileName == cmd_dest.m_fileName);
    //  BOOST_CHECK(fileSize == cmd_dest.m_fileSize);
    unsigned int i = 0;
    for (auto c : fileData)
    {
        BOOST_CHECK(c == cmd_dest.m_data[i]);
        i++;
    }
}

BOOST_AUTO_TEST_CASE(Test_ProtocolMessage_cmdSET_UUID)
{
    const boost::uuids::uuid id = boost::uuids::random_generator()();
    const unsigned int cmdSize = 16;

    // Create a message
    SUUIDCmd cmd_src;
    cmd_src.m_id = id;
    MiscCommon::BYTEVector_t data;
    cmd_src.convertToData(&data);
    CProtocolMessage msg_src;
    msg_src.encode(cmdSET_UUID, data);

    BOOST_CHECK(msg_src.header().m_cmd == cmdSET_UUID);

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
    SUUIDCmd cmd_dest;
    cmd_dest.convertFromData(msg_dest.bodyToContainer());

    BOOST_CHECK(cmd_src == cmd_dest);
    BOOST_CHECK(id == cmd_dest.m_id);
}

BOOST_AUTO_TEST_CASE(Test_ProtocolMessage_cmdREPLY_GET_UUID)
{
    const boost::uuids::uuid id = boost::uuids::random_generator()();
    const unsigned int cmdSize = 16;

    // Create a message
    SUUIDCmd cmd_src;
    cmd_src.m_id = id;
    MiscCommon::BYTEVector_t data;
    cmd_src.convertToData(&data);
    CProtocolMessage msg_src;
    msg_src.encode(cmdREPLY_UUID, data);

    BOOST_CHECK(msg_src.header().m_cmd == cmdREPLY_UUID);

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
    SUUIDCmd cmd_dest;
    cmd_dest.convertFromData(msg_dest.bodyToContainer());

    BOOST_CHECK(cmd_src == cmd_dest);
    BOOST_CHECK(id == cmd_dest.m_id);
}

BOOST_AUTO_TEST_CASE(Test_ProtocolMessage_cmdSIMPLE_MSG)
{
    const unsigned int cmdSize = 17;

    // Create a message
    SSimpleMsgCmd cmd_src;
    cmd_src.m_srcCommand = cmdSIMPLE_MSG;
    cmd_src.m_msgSeverity = MiscCommon::error;
    cmd_src.m_sMsg = "Test Message";
    MiscCommon::BYTEVector_t data;
    cmd_src.convertToData(&data);
    CProtocolMessage msg_src;
    msg_src.encode(cmdSIMPLE_MSG, data);

    BOOST_CHECK(msg_src.header().m_cmd == cmdSIMPLE_MSG);

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
    SSimpleMsgCmd cmd_dest;
    cmd_dest.convertFromData(msg_dest.bodyToContainer());

    BOOST_CHECK(cmd_src == cmd_dest);
}

BOOST_AUTO_TEST_CASE(Test_ProtocolMessage_cmdASSIGN_USER_TASK)
{
    // Create a message
    SAssignUserTaskCmd src;
    src.m_sID = "121";
    src.m_sExeFile = "test.exe -l -n --test";
    // expected attachment size
    const unsigned int cmdSize = src.m_sExeFile.size() + 1 + src.m_sID.size() + 1;
    MiscCommon::BYTEVector_t data;
    src.convertToData(&data);
    CProtocolMessage msg_src;
    msg_src.encode(cmdASSIGN_USER_TASK, data);

    BOOST_CHECK(msg_src.header().m_cmd == cmdASSIGN_USER_TASK);

    BOOST_CHECK(src.size() == cmdSize);

    // "Send" message
    CProtocolMessage msg_dest;
    msg_dest.resize(msg_src.length()); // resize internal buffer to appropriate size.
    memcpy(msg_dest.data(), msg_src.data(), msg_src.length());

    // Decode the message
    BOOST_CHECK(msg_dest.decode_header());

    // Check that we got the proper command ID
    BOOST_CHECK(msg_src.header().m_cmd == msg_dest.header().m_cmd);

    // Read the message
    SAssignUserTaskCmd dest;
    dest.convertFromData(msg_dest.bodyToContainer());

    BOOST_CHECK(src == dest);
}

BOOST_AUTO_TEST_CASE(Test_ProtocolMessage_cmdUPDATE_KEY)
{
    const string sKey = "test_Key";
    const string sValue = "test_Value";
    const unsigned int cmdSize = sKey.size() + 1 + sValue.size() + 1;

    // Create a message
    SUpdateKeyCmd cmd_src;
    cmd_src.m_sKey = sKey;
    cmd_src.m_sValue = sValue;
    MiscCommon::BYTEVector_t data;
    cmd_src.convertToData(&data);
    CProtocolMessage msg_src;
    msg_src.encode(cmdUPDATE_KEY, data);

    BOOST_CHECK(msg_src.header().m_cmd == cmdUPDATE_KEY);

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
    SUpdateKeyCmd cmd_dest;
    cmd_dest.convertFromData(msg_dest.bodyToContainer());

    BOOST_CHECK(cmd_src == cmd_dest);
}

BOOST_AUTO_TEST_SUITE_END();
