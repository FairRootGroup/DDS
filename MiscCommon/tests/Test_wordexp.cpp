// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// BOOST: tests
// Defines test_main function to link with actual unit test code.
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include "wordexp.h"
#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>

using namespace std;

BOOST_AUTO_TEST_SUITE(test_dds_topology)

void getSrcFilePathAndFileName(const string& _inputCmd,
                               string& _exeFilePath,
                               string& _exeFileName,
                               string& _exeFileNameWithArgs)
{
    wordexp_t result;
    wordexp(_inputCmd.c_str(), &result, 0);

    _exeFilePath = result.we_wordv[0];

    boost::filesystem::path exeFilePath(_exeFilePath);
    _exeFileName = exeFilePath.filename().generic_string();

    _exeFileNameWithArgs = _exeFileName;
    for (size_t i = 1; i < result.we_wordc; i++)
    {
        _exeFileNameWithArgs += " ";
        _exeFileNameWithArgs += result.we_wordv[i];
    }

    wordfree(&result);
}

BOOST_AUTO_TEST_CASE(test_dds_topology_1)
{
    std::string topoExe1 = "/Users/andrey/Development/gsi/fairroot_dds/build/bin/bsampler --id 101 --event-size 10000 "
                           "--output-socket-type push --output-buff-size 10000 --output-method bind --output-address "
                           "tcp://*:5565";
    string exeFilePath1;
    string exeFileName1;
    string exeFileNameWithArgs1;

    getSrcFilePathAndFileName(topoExe1, exeFilePath1, exeFileName1, exeFileNameWithArgs1);

    BOOST_CHECK(exeFilePath1 == "/Users/andrey/Development/gsi/fairroot_dds/build/bin/bsampler");
    BOOST_CHECK(exeFileName1 == "bsampler");
    BOOST_CHECK(exeFileNameWithArgs1 == "bsampler --id 101 --event-size 10000 --output-socket-type push "
                                        "--output-buff-size 10000 --output-method bind --output-address tcp://*:5565");

    std::string topoExe2 = "sink --id 101 --event-size 10000 --output-socket-type push --output-buff-size 10000 "
                           "--output-method bind --output-address tcp://*:5565";
    string exeFilePath2;
    string exeFileName2;
    string exeFileNameWithArgs2;

    getSrcFilePathAndFileName(topoExe2, exeFilePath2, exeFileName2, exeFileNameWithArgs2);

    BOOST_CHECK(exeFilePath2 == "sink");
    BOOST_CHECK(exeFileName2 == "sink");
    BOOST_CHECK(exeFileNameWithArgs2 == "sink --id 101 --event-size 10000 --output-socket-type push "
                                        "--output-buff-size 10000 --output-method bind --output-address tcp://*:5565");
}

BOOST_AUTO_TEST_SUITE_END()
