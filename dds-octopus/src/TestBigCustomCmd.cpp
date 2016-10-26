// Copyright 2016 GSI, Inc. All rights reserved.
//
//
//
// DDS
#include "TestBigCustomCmd.h"
// BOOST
#include <boost/crc.hpp>

using namespace std;
using namespace dds;
using namespace dds_octopus;

CTestBigCustomCmd::CTestBigCustomCmd(const SOptions_t& _options)
    : COctopusTestImpl<CTestBigCustomCmd>(_options)
    , m_cmdCrc32(0)
    , m_nConfirmedCRC(0)
{
}

void CTestBigCustomCmd::_init()
{
    SOctopusProtocol_BigCmd bigCmd;
    bigCmd.m_sVal.reserve(40000);
    // fill the buffer with dummy pattern
    for (size_t i = 0; i < 4000; ++i)
    {
        bigCmd.m_sVal += "0123456789";
    }

    boost::crc_32_type bufCrc32;
    bufCrc32.process_bytes(bigCmd.m_sVal.data(), bigCmd.m_sVal.length());
    m_cmdCrc32 = bufCrc32.checksum();

    cout << "Expected CRC = " << to_string(m_cmdCrc32) << endl;

    boost::property_tree::ptree root;
    bigCmd.get(&root);
    stringstream ss;
    boost::property_tree::write_json(ss, root);

    cout << "Command value size = " << bigCmd.m_sVal.length() << " bytes" << endl;
    m_customCmd.send(ss.str(), "");
}

void CTestBigCustomCmd::onReturnCmd(const SOctopusProtocol_Return& _return, uint64_t _senderId)
{
    ++m_nConfirmedCRC;

    cout << "Recieved crc from [" << _senderId << "]: " << _return.m_sVal << ":";

    if (_return.m_sVal == to_string(m_cmdCrc32))
        cout << " CRC matched" << endl;
    else
    {
        cout << " CRC match failed" << endl;
        m_status = TS_FAILED;
        // Test is done
        m_waitMutex.unlock();
        m_waitCondition.notify_one();
    }

    if (m_nConfirmedCRC == m_options.m_taskCount)
    {
        m_status = TS_OK;
        // Test is done
        m_waitMutex.unlock();
        m_waitCondition.notify_one();
    }
}
