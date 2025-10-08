// Copyright 2019 GSI, Inc. All rights reserved.
//
//
//

// BOOST: tests
// Defines test_main function to link with actual unit test code.
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/property_tree/json_parser.hpp>
#include <boost/test/tools/output_test_stream.hpp>
#include <boost/test/unit_test.hpp>

// DDS
#include "Tools.h"

using namespace std;
using namespace dds;
using namespace dds::tools_api;
using namespace boost::property_tree;

BOOST_AUTO_TEST_SUITE(test_dds_tools_protocol)

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
            testData.m_severity = dds::intercom_api::EMsgSeverity::info;
            testData.m_requestID = 123;

            SMessageResponseData data(child.second);

            // Test availability of the stream insertion
            stringstream ss;
            ss << data;

            BOOST_CHECK(testData == data);
        }
        else if (tag == "done")
        {
            SDoneResponseData testData;
            testData.m_requestID = 435;

            SDoneResponseData data(child.second);

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

            SProgressResponseData data(child.second);

            // Test availability of the stream insertion
            stringstream ss;
            ss << data;

            BOOST_CHECK(data == testData);
        }
        else if (tag == "submit")
        {
            SSubmitRequestData testData;
            testData.m_rms = "string";
            testData.m_instances = 123;
            testData.m_slots = 15;
            testData.m_config = "string";
            testData.m_pluginPath = "string";
            testData.m_requestID = 123;
            testData.setFlag(SSubmitRequestData::ESubmitRequestFlags::enable_overbooking, true);

            SSubmitRequestData data(child.second);

            // Test availability of the stream insertion
            stringstream ss;
            ss << data;

            BOOST_CHECK(data == testData);
            BOOST_CHECK(data.isFlagEnabled(SSubmitRequestData::ESubmitRequestFlags::enable_overbooking));
        }
        else if (tag == "topology")
        {
            STopologyRequestData testData;
            testData.m_updateType = dds::tools_api::STopologyRequestData::EUpdateType::UPDATE;
            testData.m_topologyFile = "string";
            testData.m_disableValidation = false;
            testData.m_requestID = 123;

            STopologyRequestData data(child.second);

            // Test availability of the stream insertion
            stringstream ss;
            ss << data;

            BOOST_CHECK(data == testData);
        }
        else if (tag == "getlog")
        {
            SGetLogRequestData testData;
            testData.m_requestID = 123;

            SGetLogRequestData data(child.second);

            BOOST_CHECK(data == testData);
        }
        else if (tag == "agentInfo")
        {
            SAgentInfoResponseData testData;
            testData.m_index = 123;
            testData.m_agentID = 3456;
            testData.m_startUpTime = std::chrono::milliseconds(12345);
            testData.m_username = "user1";
            testData.m_host = "host1";
            testData.m_DDSPath = "/path/to/dds";
            testData.m_groupName = "test_Group";
            testData.m_agentPid = 34;
            testData.m_requestID = 123;
            testData.m_nSlots = 10;
            testData.m_nIdleSlots = 3;
            testData.m_nExecutingSlots = 7;

            SAgentInfoResponseData data(child.second);

            // Test availability of the stream insertion
            stringstream ss;
            ss << data;

            BOOST_CHECK(data == testData);
        }
        else if (tag == "slotInfo")
        {
            SSlotInfoResponseData testData;
            testData.m_index = 123;
            testData.m_agentID = 3456;
            testData.m_slotID = 1234;
            testData.m_taskID = 5678;
            testData.m_state = 1;
            testData.m_host = "host1";
            testData.m_wrkDir = "/path/to/dds";
            testData.m_requestID = 123;

            SSlotInfoResponseData data(child.second);

            // Test availability of the stream insertion
            stringstream ss;
            ss << data;

            BOOST_CHECK(data == testData);
        }
        else if (tag == "agentCount")
        {
            SAgentCountResponseData testData;
            testData.m_activeSlotsCount = 123;
            testData.m_idleSlotsCount = 234;
            testData.m_executingSlotsCount = 345;
            testData.m_requestID = 123;

            SAgentCountResponseData data(child.second);

            // Test availability of the stream insertion
            stringstream ss;
            ss << data;

            BOOST_CHECK(data == testData);
        }
        else if (tag == "commanderInfo")
        {
            SCommanderInfoResponseData testData;
            testData.m_pid = 432;
            testData.m_requestID = 435;
            testData.m_activeTopologyName = "TopoName";
            testData.m_activeTopologyPath = "/topology/path";

            SCommanderInfoResponseData data(child.second);

            // Test availability of the stream insertion
            stringstream ss;
            ss << data;

            BOOST_CHECK(data == testData);
        }
        else if (tag == "onTaskDone")
        {
            SOnTaskDoneResponseData testData;
            testData.m_requestID = 543;
            testData.m_taskID = 678;
            testData.m_exitCode = 23;
            testData.m_signal = 56;
            testData.m_host = "dds.gsi.de";
            testData.m_wrkDir = "/tmp/wn_dds";
            testData.m_taskPath = "/main/task";

            SOnTaskDoneResponseData data(child.second);

            // Test availability of the stream insertion
            stringstream ss;
            ss << data;

            BOOST_CHECK(data == testData);
        }
        else if (tag == "agentCommand")
        {
            SAgentCommandRequestData testData;
            testData.m_commandType = SAgentCommandRequestData::EAgentCommandType::shutDownByID;
            testData.m_arg1 = 235;
            testData.m_arg2 = "testString";

            SAgentCommandRequestData data(child.second);

            // Test availability of the stream insertion
            stringstream ss;
            ss << data;

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

    SCommanderInfoResponseData dataCommanderInfo(childPT);

    BOOST_CHECK(dataCommanderInfo == dataCommanderInfoTest);
}

BOOST_AUTO_TEST_CASE(test_dds_tools_protocol_submit_response)
{
    SSubmitResponseData testData;
    testData.m_jobIDs = { "123.job", "124.job", "125.job" }; // Test multiple job IDs
    testData.m_allocNodes = 42;
    testData.m_state = 1; // RUNNING
    testData.m_jobInfoAvailable = true;
    testData.m_requestID = 123;

    stringstream strBuf(testData.toJSON());

    ptree pt;
    read_json(strBuf, pt);
    const ptree& childPT = pt.get_child("dds.tools-api.submit");

    SSubmitResponseData data(childPT);

    // Test availability of the stream insertion
    stringstream ss;
    ss << data;

    BOOST_CHECK(data == testData);
    BOOST_CHECK_EQUAL(data.m_jobIDs.size(), 3);
    BOOST_CHECK_EQUAL(data.m_jobIDs[0], "123.job");
    BOOST_CHECK_EQUAL(data.m_jobIDs[1], "124.job");
    BOOST_CHECK_EQUAL(data.m_jobIDs[2], "125.job");
}

BOOST_AUTO_TEST_CASE(test_dds_tools_protocol_lightweight_env_helper)
{
    // Test helper function with different environment variable values

    // Test with value "1"
    setenv("DDS_LIGHTWEIGHT_PACKAGE", "1", 1);
    BOOST_CHECK(isLightweightModeEnabledByEnv() == true);

    // Test with value "true"
    setenv("DDS_LIGHTWEIGHT_PACKAGE", "true", 1);
    BOOST_CHECK(isLightweightModeEnabledByEnv() == true);

    // Test with value "TRUE" (case insensitive)
    setenv("DDS_LIGHTWEIGHT_PACKAGE", "TRUE", 1);
    BOOST_CHECK(isLightweightModeEnabledByEnv() == true);

    // Test with value "yes"
    setenv("DDS_LIGHTWEIGHT_PACKAGE", "yes", 1);
    BOOST_CHECK(isLightweightModeEnabledByEnv() == true);

    // Test with value "YES" (case insensitive)
    setenv("DDS_LIGHTWEIGHT_PACKAGE", "YES", 1);
    BOOST_CHECK(isLightweightModeEnabledByEnv() == true);

    // Test with value "on"
    setenv("DDS_LIGHTWEIGHT_PACKAGE", "on", 1);
    BOOST_CHECK(isLightweightModeEnabledByEnv() == true);

    // Test with value "ON" (case insensitive)
    setenv("DDS_LIGHTWEIGHT_PACKAGE", "ON", 1);
    BOOST_CHECK(isLightweightModeEnabledByEnv() == true);

    // Test with value "0" (should be false)
    setenv("DDS_LIGHTWEIGHT_PACKAGE", "0", 1);
    BOOST_CHECK(isLightweightModeEnabledByEnv() == false);

    // Test with value "false"
    setenv("DDS_LIGHTWEIGHT_PACKAGE", "false", 1);
    BOOST_CHECK(isLightweightModeEnabledByEnv() == false);

    // Test with value "no"
    setenv("DDS_LIGHTWEIGHT_PACKAGE", "no", 1);
    BOOST_CHECK(isLightweightModeEnabledByEnv() == false);

    // Test with invalid value
    setenv("DDS_LIGHTWEIGHT_PACKAGE", "invalid", 1);
    BOOST_CHECK(isLightweightModeEnabledByEnv() == false);

    // Test with empty value
    setenv("DDS_LIGHTWEIGHT_PACKAGE", "", 1);
    BOOST_CHECK(isLightweightModeEnabledByEnv() == false);

    // Test with unset variable
    unsetenv("DDS_LIGHTWEIGHT_PACKAGE");
    BOOST_CHECK(isLightweightModeEnabledByEnv() == false);
}

BOOST_AUTO_TEST_CASE(test_dds_tools_protocol_submit_request_env_lightweight)
{
    // Test that SSubmitRequestData constructor respects DDS_LIGHTWEIGHT_PACKAGE

    // Test with environment variable set to "1"
    setenv("DDS_LIGHTWEIGHT_PACKAGE", "1", 1);
    {
        SSubmitRequestData data;
        BOOST_CHECK(data.isFlagEnabled(SSubmitRequestData::ESubmitRequestFlags::enable_lightweight) == true);
    }

    // Test with environment variable set to "true"
    setenv("DDS_LIGHTWEIGHT_PACKAGE", "true", 1);
    {
        SSubmitRequestData data;
        BOOST_CHECK(data.isFlagEnabled(SSubmitRequestData::ESubmitRequestFlags::enable_lightweight) == true);
    }

    // Test with environment variable unset
    unsetenv("DDS_LIGHTWEIGHT_PACKAGE");
    {
        SSubmitRequestData data;
        BOOST_CHECK(data.isFlagEnabled(SSubmitRequestData::ESubmitRequestFlags::enable_lightweight) == false);
    }

    // Test that explicit flag setting overrides environment variable
    setenv("DDS_LIGHTWEIGHT_PACKAGE", "1", 1);
    {
        SSubmitRequestData data;
        // First check it's enabled by env
        BOOST_CHECK(data.isFlagEnabled(SSubmitRequestData::ESubmitRequestFlags::enable_lightweight) == true);

        // Now explicitly disable it
        data.setFlag(SSubmitRequestData::ESubmitRequestFlags::enable_lightweight, false);
        BOOST_CHECK(data.isFlagEnabled(SSubmitRequestData::ESubmitRequestFlags::enable_lightweight) == false);

        // And enable it again
        data.setFlag(SSubmitRequestData::ESubmitRequestFlags::enable_lightweight, true);
        BOOST_CHECK(data.isFlagEnabled(SSubmitRequestData::ESubmitRequestFlags::enable_lightweight) == true);
    }

    // Clean up
    unsetenv("DDS_LIGHTWEIGHT_PACKAGE");
}

BOOST_AUTO_TEST_CASE(test_dds_tools_protocol_submit_request_serialization_with_lightweight)
{
    // Test that lightweight flag is properly serialized/deserialized

    setenv("DDS_LIGHTWEIGHT_PACKAGE", "1", 1);
    {
        SSubmitRequestData dataTest;
        dataTest.m_rms = "slurm";
        dataTest.m_instances = 10;
        dataTest.m_slots = 32;
        dataTest.m_config = "/path/to/config";
        dataTest.m_pluginPath = "/path/to/plugin";
        dataTest.m_requestID = 456;
        // Flag should already be set by constructor due to environment variable

        BOOST_CHECK(dataTest.isFlagEnabled(SSubmitRequestData::ESubmitRequestFlags::enable_lightweight) == true);

        // Serialize to JSON and back
        stringstream strBuf(dataTest.toJSON());

        ptree pt;
        read_json(strBuf, pt);
        const ptree& childPT = pt.get_child("dds.tools-api.submit");

        SSubmitRequestData data(childPT);

        BOOST_CHECK(data == dataTest);
        BOOST_CHECK(data.isFlagEnabled(SSubmitRequestData::ESubmitRequestFlags::enable_lightweight) == true);
    }

    // Clean up
    unsetenv("DDS_LIGHTWEIGHT_PACKAGE");
}

BOOST_AUTO_TEST_SUITE_END()
