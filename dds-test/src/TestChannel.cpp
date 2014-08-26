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

bool CTestChannel::on_cmdREPLY_HANDSHAKE_OK(const CProtocolMessage& _msg)
{
    m_isHandShakeOK = true;

    pushMsg<cmdSTART_DOWNLOAD_TEST>();

    return true;
}

bool CTestChannel::on_cmdDOWNLOAD_TEST_RECIEVED(const CProtocolMessage& _msg)
{
    SSimpleMsgCmd recieved_cmd;
    recieved_cmd.convertFromData(_msg.bodyToContainer());
    LOG(log_stdout) << recieved_cmd.m_sMsg;

    return true;
}

bool CTestChannel::on_cmdALL_DOWNLOAD_TESTS_RECIEVED(const CProtocolMessage& _msg)
{
    SSimpleMsgCmd recieved_cmd;
    recieved_cmd.convertFromData(_msg.bodyToContainer());
    LOG(log_stdout) << recieved_cmd.m_sMsg;

    stop();
    exit(EXIT_SUCCESS);
}

bool CTestChannel::on_cmdDOWNLOAD_TEST_ERROR(const CProtocolMessage& _msg)
{
    SSimpleMsgCmd recieved_cmd;
    recieved_cmd.convertFromData(_msg.bodyToContainer());
    LOG(log_stdout) << recieved_cmd.m_sMsg;

    return true;
}

bool CTestChannel::on_cmdDOWNLOAD_TEST_FATAL(const CProtocolMessage& _msg)
{
    SSimpleMsgCmd recieved_cmd;
    recieved_cmd.convertFromData(_msg.bodyToContainer());
    LOG(log_stdout) << recieved_cmd.m_sMsg;

    stop();
    exit(EXIT_FAILURE);
}

//
//        sendTestBinaryAttachment(1000);
//        sendTestBinaryAttachment(10000);
//        sendTestBinaryAttachment(100000);
//        sendTestBinaryAttachment(1000000);
//        sendTestBinaryAttachment(10000000);
//    }
//    return true;
//}

/*
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

    CProtocolMessage msg;
    msg.encodeWithAttachment<cmdBINARY_ATTACHMENT>(cmd);
    pushMsg(msg);
}
 */
