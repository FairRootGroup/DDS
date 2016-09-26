// Copyright 2016 GSI, Inc. All rights reserved.
//
//
//
// DDS
#include "TestPing.h"

using namespace std;
using namespace dds;
using namespace dds_octopus;

CTestPing::CTestPing(const SOptions_t& _options)
    : COctopusTestImpl<CTestPing>(_options)
    , m_nConfirmedPings(0)
{
}

void CTestPing::_init()
{
    cout << "Sending ping..." << endl;
    SOctopusProtocol_GetPing getPing;
    boost::property_tree::ptree root;
    getPing.get(&root);
    stringstream ss;
    boost::property_tree::write_json(ss, root);
    // Send to all
    m_customCmd.send(ss.str(), "");
}

void CTestPing::onPingCmd(const SOctopusProtocol_Ping& _ping, uint64_t _senderId)
{
    cout << "Recieved ping from [" << _senderId << "]" << endl;
    ++m_nConfirmedPings;
    if (m_nConfirmedPings == m_options.m_taskCount)
    {
        m_status = TS_OK;
        // Test is done
        m_waitMutex.unlock();
        m_waitCondition.notify_one();
    }
}
