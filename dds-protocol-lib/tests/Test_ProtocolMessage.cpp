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
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

// DDS
#include "CommandAttachmentImpl.h"
#include "ProtocolCommands.h"
#include "ProtocolMessage.h"
#include "TestCmd.h"
#include "def.h"

using boost::unit_test::test_suite;
using namespace MiscCommon;
using namespace std;
using namespace dds;
using namespace dds::protocol_api;

BOOST_AUTO_TEST_SUITE(Test_ProtocolMessage);

template <typename T>
void TestCommand(const T& _srcCmd, uint16_t _srcCmdID, size_t _expectedCmdSize)
{
    MiscCommon::BYTEVector_t data;
    _srcCmd.convertToData(&data);
    CProtocolMessage srcMsg;
    srcMsg.encode(_srcCmdID, data, 0);

    BOOST_CHECK(srcMsg.header().m_cmd == _srcCmdID);

    BOOST_CHECK(_srcCmd.size() == _expectedCmdSize);

    // "Send" message
    CProtocolMessage msg_dest;
    msg_dest.resize(srcMsg.length()); // resize internal buffer to appropriate size.
    memcpy(msg_dest.data(), srcMsg.data(), srcMsg.length());

    // Decode the message
    BOOST_CHECK(msg_dest.decode_header());

    // Check that we got the proper command ID
    BOOST_CHECK(srcMsg.header().m_cmd == msg_dest.header().m_cmd);

    // Read the message
    T destCmd;
    destCmd.convertFromData(msg_dest.bodyToContainer());

    BOOST_CHECK(_srcCmd == destCmd);
}

//----------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(Test_ProtocolMessage_cmdHANDSHAKE)
{
    const unsigned int cmdSize = 14;

    SVersionCmd cmd;
    cmd.m_version = 444;
    cmd.m_channelType = 2;
    cmd.m_sSID = "TEST SID";

    TestCommand(cmd, cmdHANDSHAKE, cmdSize);
}

BOOST_AUTO_TEST_CASE(Test_ProtocolMessage_cmdSUBMIT)
{
    const unsigned int cmdSize = 39;

    SSubmitCmd cmd;
    cmd.m_sRMSType = "ssh";
    cmd.m_sCfgFile = "/Users/dummy/dummy.cfg";
    cmd.m_sPath = "path";
    cmd.m_nNumberOfAgents = 100;

    TestCommand(cmd, cmdSUBMIT, cmdSize);
}

BOOST_AUTO_TEST_CASE(Test_ProtocolMessage_cmdREPLY_HOST_INFO)
{
    const unsigned int cmdSize = 67;

    SHostInfoCmd cmd;
    cmd.m_username = "username";
    cmd.m_host = "host.com";
    cmd.m_version = "1.0.0";
    cmd.m_DDSPath = "/Users/andrey/DDS";
    cmd.m_agentPid = 1111;
    cmd.m_slots = 10;
    cmd.m_submitTime = 23465677;
    cmd.m_workerId = "wn5";

    TestCommand(cmd, cmdREPLY_HOST_INFO, cmdSize);
}

BOOST_AUTO_TEST_CASE(Test_ProtocolMessage_cmdBINARY_ATTACHMENT)
{
    const MiscCommon::BYTEVector_t data{ 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
                                         'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z' };
    boost::crc_32_type crc32;
    crc32.process_bytes(&data[0], data.size());
    const uint32_t offset = 12345;
    const boost::uuids::uuid fileId = boost::uuids::random_generator()();

    const unsigned int cmdSize = 58;

    SBinaryAttachmentCmd cmd;
    cmd.m_data = data;
    cmd.m_fileId = fileId;
    cmd.m_size = data.size();
    cmd.m_offset = offset;
    cmd.m_crc32 = crc32.checksum();

    TestCommand(cmd, cmdBINARY_ATTACHMENT, cmdSize);
}

BOOST_AUTO_TEST_CASE(Test_ProtocolMessage_cmdBINARY_ATTACHMENT_RECEIVED)
{
    const unsigned int cmdSize = 51;

    SBinaryAttachmentReceivedCmd cmd;
    cmd.m_receivedFilePath = "received_file_name";
    cmd.m_requestedFileName = "requested_file_name";
    cmd.m_srcCommand = cmdBINARY_ATTACHMENT_RECEIVED;
    cmd.m_downloadTime = 12345;
    cmd.m_receivedFileSize = 123456;

    TestCommand(cmd, cmdBINARY_ATTACHMENT_RECEIVED, cmdSize);
}

BOOST_AUTO_TEST_CASE(Test_ProtocolMessage_cmdBINARY_ATTACHMENT_START)
{
    const unsigned int cmdSize = 37;

    SBinaryAttachmentStartCmd cmd;
    cmd.m_fileId = boost::uuids::random_generator()();
    cmd.m_srcCommand = cmdBINARY_ATTACHMENT_START;
    cmd.m_fileName = "file_name";
    cmd.m_fileSize = 123456;
    cmd.m_fileCrc32 = 123456;

    TestCommand(cmd, cmdBINARY_ATTACHMENT_START, cmdSize);
}

BOOST_AUTO_TEST_CASE(Test_ProtocolMessage_cmdSET_ID)
{
    const unsigned int cmdSize = 8;

    SIDCmd cmd;
    cmd.m_id = 123;

    TestCommand(cmd, cmdSET_ID, cmdSize);
}

BOOST_AUTO_TEST_CASE(Test_ProtocolMessage_cmdREPLY_GET_ID)
{
    const unsigned int cmdSize = 8;

    SIDCmd cmd;
    cmd.m_id = 123;

    TestCommand(cmd, cmdREPLY_ID, cmdSize);
}

BOOST_AUTO_TEST_CASE(Test_ProtocolMessage_cmdSIMPLE_MSG)
{
    const unsigned int cmdSize = 18;

    SSimpleMsgCmd cmd;
    cmd.m_srcCommand = cmdSIMPLE_MSG;
    cmd.m_msgSeverity = MiscCommon::error;
    cmd.m_sMsg = "Test Message";

    TestCommand(cmd, cmdSIMPLE_MSG, cmdSize);
}

BOOST_AUTO_TEST_CASE(Test_ProtocolMessage_cmdREPLY_HANDSHAKE_ERR)
{
    const unsigned int cmdSize = 21;

    SSimpleMsgCmd cmd;
    cmd.m_srcCommand = cmdREPLY_HANDSHAKE_ERR;
    cmd.m_msgSeverity = MiscCommon::error;
    cmd.m_sMsg = "Handshake error";

    TestCommand(cmd, cmdSIMPLE_MSG, cmdSize);
}

BOOST_AUTO_TEST_CASE(Test_ProtocolMessage_cmdREPLY_PID)
{
    const unsigned int cmdSize = 11;

    SSimpleMsgCmd cmd;
    cmd.m_srcCommand = cmdREPLY_PID;
    cmd.m_msgSeverity = MiscCommon::info;
    cmd.m_sMsg = "12345";

    TestCommand(cmd, cmdSIMPLE_MSG, cmdSize);
}

BOOST_AUTO_TEST_CASE(Test_ProtocolMessage_cmdASSIGN_USER_TASK)
{
    SAssignUserTaskCmd src;
    src.m_taskID = 121;
    src.m_sExeFile = "test.exe -l -n --test";
    src.m_taskIndex = 99;
    src.m_collectionIndex = 77;
    src.m_taskPath = "/main/group1/collection1_1/task_2";
    src.m_groupName = "group1";
    src.m_collectionName = "collection1";
    src.m_taskName = "task1";
    src.m_topoHash = 321;
    src.m_sEnvFile = "env.sh";
    // expected attachment size
    const unsigned int cmdSize = src.m_sExeFile.size() + sizeof(uint16_t) + sizeof(uint64_t) + sizeof(uint32_t) +
                                 sizeof(uint32_t) + src.m_taskPath.size() + sizeof(uint16_t) + src.m_groupName.size() +
                                 sizeof(uint16_t) + src.m_collectionName.size() + sizeof(uint16_t) +
                                 src.m_taskName.size() + sizeof(uint16_t) + sizeof(uint32_t) + src.m_sEnvFile.size() +
                                 sizeof(uint16_t);

    TestCommand(src, cmdASSIGN_USER_TASK, cmdSize);
}

BOOST_AUTO_TEST_CASE(Test_ProtocolMessage_cmdUPDATE_KEY)
{
    const string propertyName = "test_Property";
    const string value = "test_Value";
    const uint64_t senderTaskID = 1111111111;
    const uint64_t receiverTaskID = 2222222222;
    const unsigned int cmdSize =
        propertyName.size() + sizeof(uint16_t) + value.size() + sizeof(uint16_t) + sizeof(uint64_t) + sizeof(uint64_t);

    SUpdateKeyCmd cmd_src;
    cmd_src.m_propertyName = propertyName;
    cmd_src.m_value = value;
    cmd_src.m_senderTaskID = senderTaskID;
    cmd_src.m_receiverTaskID = receiverTaskID;

    TestCommand(cmd_src, cmdUPDATE_KEY, cmdSize);
}

BOOST_AUTO_TEST_CASE(Test_ProtocolMessage_cmdREPLY_AGENTS_INFO)
{
    const unsigned int cmdSize = 32;

    SAgentsInfoCmd cmd;
    cmd.m_nActiveAgents = 3;
    cmd.m_nIndex = 1;
    cmd.m_sAgentInfo = "Agent1, Agent2, Agent3";

    TestCommand(cmd, cmdREPLY_AGENTS_INFO, cmdSize);
}

BOOST_AUTO_TEST_CASE(Test_ProtocolMessage_cmdUSER_TASK_DONE)
{
    const unsigned int cmdSize = 12;

    SUserTaskDoneCmd cmd;
    cmd.m_exitCode = 12345;
    cmd.m_taskID = 54321;

    TestCommand(cmd, cmdUSER_TASK_DONE, cmdSize);
}

BOOST_AUTO_TEST_CASE(Test_ProtocolMessage_cmdPROGRESS)
{
    const unsigned int cmdSize = 18;

    SProgressCmd cmd;
    cmd.m_completed = 10;
    cmd.m_errors = 3;
    cmd.m_total = 20;
    cmd.m_time = 2345;
    cmd.m_srcCommand = 0;

    TestCommand(cmd, cmdPROGRESS, cmdSize);
}

BOOST_AUTO_TEST_CASE(Test_ProtocolMessage_cmdCUSTOM_CMD)
{
    const unsigned int cmdSize = 32;

    SCustomCmdCmd cmd;
    cmd.m_sCmd = "cmd";
    cmd.m_sCondition = "condition";
    cmd.m_senderId = 123456;

    TestCommand(cmd, cmdCUSTOM_CMD, cmdSize);
}

BOOST_AUTO_TEST_CASE(Test_ProtocolMessage_cmdTEST_CMD)
{
    const unsigned int cmdSize = 244;
    const uint16_t cmdTEST_CMD = 1000;

    STestCmd cmd;
    cmd.m_uint16 = 16;
    cmd.m_uint32 = 32;
    cmd.m_uint64 = 64;
    cmd.m_string1 = "string1";
    cmd.m_string2 = "string2";
    cmd.m_string3 = "string3";
    cmd.m_string4 = "string4";
    cmd.m_vuint16 = { 16, 17, 18, 19, 20, 21 };
    cmd.m_vuint32 = { 32, 33, 34, 35, 36, 37, 38 };
    cmd.m_vuint64 = { 64, 65, 66, 67, 68, 69, 70 };
    cmd.m_vstring1 = { "string1_1", "string1_2", "string1_3" };
    cmd.m_vstring2 = { "string2_1", "string2_2", "string2_3", "string2_4", "string2_5" };

    TestCommand(cmd, cmdTEST_CMD, cmdSize);
}

BOOST_AUTO_TEST_CASE(Test_ProtocolMessage_cmdLOBBY_MEMBER_INFO)
{
    const unsigned int cmdSize = 11;

    SSimpleMsgCmd cmd;
    cmd.m_sMsg = "lobby";
    cmd.m_srcCommand = cmdLOBBY_MEMBER_INFO;
    cmd.m_msgSeverity = info;

    TestCommand(cmd, cmdLOBBY_MEMBER_INFO, cmdSize);
}

BOOST_AUTO_TEST_CASE(Test_ProtocolMessage_cmdLOBBY_MEMBER_HANDSHAKE)
{
    const unsigned int cmdSize = 9;

    SVersionCmd cmd;
    cmd.m_sSID = "SID";
    cmd.m_version = 1;
    cmd.m_channelType = 2;

    TestCommand(cmd, cmdLOBBY_MEMBER_HANDSHAKE, cmdSize);
}

BOOST_AUTO_TEST_CASE(Test_ProtocolMessage_cmdREPLY)
{
    const unsigned int cmdSize = 17;

    SReplyCmd cmd;
    cmd.m_sMsg = "status OK";
    cmd.m_srcCommand = cmdHANDSHAKE;
    cmd.m_returnCode = 12;
    cmd.m_statusCode = (uint16_t)SReplyCmd::EStatusCode::OK;

    TestCommand(cmd, cmdREPLY, cmdSize);
}

BOOST_AUTO_TEST_SUITE_END();
