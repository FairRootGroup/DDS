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

    read_json("test_protocol_1.json", pt);

    const ptree& childPT = pt.get_child("dds.tools-api");

    for (const auto& child : childPT)
    {
        const string& tag = child.first;
        if (tag == "message")
        {
            SMessageResponseData testData;
            testData.m_msg = "string";
            testData.m_severity = dds::intercom_api::EMsgSeverity(123);
            testData.m_requestID = 123;

            SMessageResponseData data;
            data.fromPT(child.second);

            BOOST_CHECK(testData == data);
        }
        else if (tag == "done")
        {
            SDoneResponseData testData;
            testData.m_requestID = 435;

            SDoneResponseData data;
            data.fromPT(child.second);

            BOOST_CHECK(data == testData);
        }
        else if (tag == "progress")
        {
            SProgressResponseData testData;
            testData.m_completed = 123;
            testData.m_total = 123;
            testData.m_errors = 123;
            testData.m_time = 123;
            testData.m_srcCommand = 123;

            SProgressResponseData data;
            data.fromPT(child.second);

            BOOST_CHECK(data == testData);
        }
        else if (tag == "submit")
        {
            SSubmitRequestData testData;
            testData.m_rms = "string";
            testData.m_instances = 123;
            testData.m_config = "string";
            testData.m_pluginPath = "string";
            testData.m_requestID = 123;

            SSubmitRequestData data;
            data.fromPT(child.second);

            BOOST_CHECK(data == testData);
        }
        else if (tag == "topology")
        {
            STopologyRequestData testData;
            testData.m_updateType = dds::tools_api::STopologyRequestData::EUpdateType(123);
            testData.m_topologyFile = "string";
            testData.m_disableValidation = false;
            testData.m_requestID = 123;

            STopologyRequestData data;
            data.fromPT(child.second);

            BOOST_CHECK(data == testData);
        }
        else if (tag == "getlog")
        {
            SGetLogRequestData testData;
            testData.m_requestID = 123;

            SGetLogRequestData data;
            data.fromPT(child.second);

            BOOST_CHECK(data == testData);
        }
        else if (tag == "agentInfo")
        {
            SAgentInfoResponseData testData;
            testData.m_activeAgentsCount = 123;
            testData.m_idleAgentsCount = 105;
            testData.m_executingAgentsCount = 35;
            testData.m_index = 123;
            testData.m_agentInfo = "string";
            testData.m_requestID = 123;

            SAgentInfoResponseData data;
            data.fromPT(child.second);

            BOOST_CHECK(data == testData);
        }
        else if (tag == "commanderInfo")
        {
            SCommanderInfoResponseData testData;
            testData.m_pid = 432;
            testData.m_requestID = 435;
            testData.m_activeTopologyName = "TopoName";

            SCommanderInfoResponseData data;
            data.fromPT(child.second);

            BOOST_CHECK(data == testData);
        }
    }
}

BOOST_AUTO_TEST_CASE(test_dds_tools_protocol_toJSON)
{
    SCommanderInfoResponseData dataCommanderInfoTest;
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
