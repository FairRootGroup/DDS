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
#include <sstream>

using namespace std;
using namespace boost::property_tree;
using namespace dds;

BOOST_AUTO_TEST_SUITE(test_protocol_parser)

BOOST_AUTO_TEST_CASE(test_protocol_parser_1)
{
    stringstream json;
    json << "{" << endl
         << "\"submit\":" << endl
         << "{" << endl
         << "\"nInstances\": 11," << endl
         << "\"cfgFilePath\": \"/path/to/cfg/dds_plugin.cfg\"" << endl
         << "}," << endl
         << "\"message\":" << endl
         << "{" << endl
         << "\"message\": \"Message to DDS plugin\"," << endl
         << "\"severity\": \"info\"" << endl
         << "}," << endl
         << "\"requirement\":" << endl
         << "{" << endl
         << "\"hostName\" : \"host.gsi.de\"" << endl
         << "}" << endl
         << "}" << endl;

    CRMSPluginProtocol parser;

    parser.subscribeSubmit([](const SSubmit& _submit) {
        BOOST_CHECK(_submit.m_nInstances == 11);
        BOOST_CHECK(_submit.m_cfgFilePath == "/path/to/cfg/dds_plugin.cfg");
    });

    parser.subscribeMessage([](const SMessage& _message) {
        BOOST_CHECK(_message.m_msg == "Message to DDS plugin");
        BOOST_CHECK(_message.m_msgSeverity == EMsgSeverity::info);
    });

    parser.subscribeRequirement(
        [](const SRequirement& _requirement) { BOOST_CHECK(_requirement.m_hostName == "host.gsi.de"); });

    parser.parse(json);
}

BOOST_AUTO_TEST_SUITE_END()
