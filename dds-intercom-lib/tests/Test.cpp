// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// BOOST: tests
// Defines test_main function to link with actual unit test code.
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/output_test_stream.hpp>
#include <boost/test/unit_test.hpp>

// DDS
#include "dds_intercom.h"

// BOOST
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

// STD
#include <iostream>
#include <sstream>

using namespace std;
using namespace boost::property_tree;
using namespace dds;

BOOST_AUTO_TEST_SUITE(test_protocol_parser)

BOOST_AUTO_TEST_CASE(test_protocol_parser_1)
{
    stringstream json;
    json << "{"
         << "\"dds\" :"
         << "{"
         << "\"plug-in\" :"
         << "{"
         << "\"id\": \"plug-in-id\","
         << "\"submit\":"
         << "{"
         << "\"nInstances\": 11,"
         << "\"cfgFilePath\": \"/path/to/cfg/dds_plugin.cfg\""
         << "},"
         << "\"message\":"
         << "{"
         << "\"msg\": \"Message to DDS plugin\","
         << "\"msgSeverity\": \"info\""
         << "},"
         << "\"requirement\":"
         << "{"
         << "\"hostName\" : \"host.gsi.de\""
         << "}"
         << "}"
         << "}"
         << "}";

    CRMSPluginProtocol parser("plug-in-id");

    parser.onSubmit([](const SSubmit& _submit) {
        BOOST_CHECK(_submit.m_nInstances == 11);
        BOOST_CHECK(_submit.m_cfgFilePath == "/path/to/cfg/dds_plugin.cfg");
        BOOST_CHECK(_submit.m_id == "plug-in-id");
    });

    parser.onMessage([](const SMessage& _message) {
        BOOST_CHECK(_message.m_msg == "Message to DDS plugin");
        BOOST_CHECK(_message.m_msgSeverity == EMsgSeverity::info);
        BOOST_CHECK(_message.m_id == "plug-in-id");
    });

    parser.onRequirement([](const SRequirement& _requirement) {
        BOOST_CHECK(_requirement.m_hostName == "host.gsi.de");
        BOOST_CHECK(_requirement.m_id == "plug-in-id");
    });

    parser.notify(json);
}

BOOST_AUTO_TEST_CASE(test_message)
{
    SMessage src;
    src.m_id = "plug-in-id";
    src.m_msg = "Sample message";
    src.m_msgSeverity = EMsgSeverity::info;

    string json = src.toJSON();

    SMessage out;
    out.fromJSON(json);

    BOOST_CHECK(src == out);
}

BOOST_AUTO_TEST_CASE(test_submit)
{
    SSubmit src;
    src.m_id = "plug-in-id";
    src.m_nInstances = 111;
    src.m_cfgFilePath = "/path/to/cfg/dds_plugin.cfg";

    string json = src.toJSON();

    SSubmit out;
    out.fromJSON(json);

    BOOST_CHECK(src == out);
}

BOOST_AUTO_TEST_CASE(test_requirement)
{
    SRequirement src;
    src.m_id = "plug-in-id";
    src.m_hostName = "host.gsi.de";

    string json = src.toJSON();

    SRequirement out;
    out.fromJSON(json);

    BOOST_CHECK(src == out);
}

BOOST_AUTO_TEST_CASE(test_init)
{
    SInit src;
    src.m_id = "plug-in-id";

    string json = src.toJSON();

    SInit out;
    out.fromJSON(json);

    BOOST_CHECK(src == out);
}

BOOST_AUTO_TEST_SUITE_END()
