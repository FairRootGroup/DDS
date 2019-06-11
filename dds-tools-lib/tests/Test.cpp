// Copyright 2019 GSI, Inc. All rights reserved.
//
//
//

// BOOST: tests
// Defines test_main function to link with actual unit test code.
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/property_tree/json_parser.hpp>
#include <boost/test/output_test_stream.hpp>
#include <boost/test/unit_test.hpp>

// DDS
#include "Tools.h"

using namespace std;
using namespace dds;
using namespace dds::tools_api;
using namespace boost::property_tree;

BOOST_AUTO_TEST_SUITE(test_dds_tools)

BOOST_AUTO_TEST_CASE(test_dds_tools_protocol_Done)
{
    SDoneResponseData data;
    data.m_requestID = 1234;

    boost::property_tree::ptree pt;
    data.toPT(pt);

    SDoneResponseData dataTest;
    dataTest.fromPT(pt);

    BOOST_CHECK(data == dataTest);
}

BOOST_AUTO_TEST_CASE(test_dds_tools_protocol)
{
    ptree pt;

    read_json("/Users/anar/DDS/2.3.30.g1de2e54/tests/test_protocol_1.json", pt);

    const ptree& childPT = pt.get_child("dds.tools-api");

    for (const auto& child : childPT)
    {
        const string& tag = child.first;
        if (tag == "commanderInfo")
        {
            SCommanderInfoResponseData dataCommanderInfoTest;
            dataCommanderInfoTest.m_idleAgentsCount = 105;
            dataCommanderInfoTest.m_pid = 432;
            dataCommanderInfoTest.m_requestID = 435;

            SCommanderInfoResponseData dataCommanderInfo;
            dataCommanderInfo.fromPT(child.second);

            BOOST_CHECK(dataCommanderInfo == dataCommanderInfoTest);
        }
    }
}

BOOST_AUTO_TEST_CASE(test_dds_tools_protocol_toJSON)
{
    SCommanderInfoResponseData dataCommanderInfoTest;
    dataCommanderInfoTest.m_idleAgentsCount = 105;
    dataCommanderInfoTest.m_pid = 432;
    dataCommanderInfoTest.m_requestID = 435;

    stringstream strBuf(dataCommanderInfoTest.toJSON());

    ptree pt;
    read_json(strBuf, pt);
    const ptree& childPT = pt.get_child("dds.tools-api.commanderInfo");

    SCommanderInfoResponseData dataCommanderInfo;
    dataCommanderInfo.fromPT(childPT);

    BOOST_CHECK(dataCommanderInfo == dataCommanderInfoTest);
}

BOOST_AUTO_TEST_CASE(test_dds_tools_session)
{
    CSession session;
    boost::uuids::uuid sid = session.create();
    BOOST_CHECK(!sid.is_nil());
    BOOST_CHECK(session.IsRunning());

    CSession sessionAttach;
    sessionAttach.attach(session.getSessionID());
    BOOST_CHECK(!sessionAttach.getSessionID().is_nil());
    BOOST_CHECK(sessionAttach.IsRunning());

    session.shutdown();
    BOOST_CHECK(session.getSessionID().is_nil());
    BOOST_CHECK(!session.IsRunning());
    BOOST_CHECK(!sessionAttach.IsRunning());
}

BOOST_AUTO_TEST_SUITE_END()
