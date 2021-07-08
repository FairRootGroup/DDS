// Copyright 2014 GSI, Inc. All rights reserved.
//
// Unit tests
//
// BOOST: tests
// Defines test_main function to link with actual unit test code.
#define BOOST_TEST_DYN_LINK
#define BOOST_AUTO_TEST_MAIN // Boost 1.33
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

using boost::unit_test::test_suite;

// STD
#include <string>
// Our
#include "MiscCli.h"

using namespace dds::misc;
using namespace std;

BOOST_AUTO_TEST_SUITE(MiscCommon_Cli);

void testParseExe(const string& _exeStr,
                  const string& _exePrefix,
                  const string& _resultFilePath,
                  const string& _resultFilename,
                  const string& _resultCmdStr)
{
    string filePath, filename, cmdStr;

    parseExe(_exeStr, _exePrefix, filePath, filename, cmdStr);

    BOOST_CHECK(filePath == _resultFilePath);
    BOOST_CHECK(filename == _resultFilename);
    BOOST_CHECK(cmdStr == _resultCmdStr);
}

BOOST_AUTO_TEST_CASE(Test_MiscCommon_parseExe)
{
    testParseExe("/path/to/test_exe --arg1 arg1 --arg2 arg2 --arg3",
                 "${DDS_LOCATION}/",
                 "/path/to/test_exe",
                 "test_exe",
                 "${DDS_LOCATION}/test_exe --arg1 arg1 --arg2 arg2 --arg3");

    const string bashPath{ boost::process::search_path("bash").string() };
    testParseExe(
        "bash --arg1 arg1 --arg2 arg2 --arg3", "", bashPath, "bash", bashPath + " --arg1 arg1 --arg2 arg2 --arg3");

    testParseExe("bash --arg1 arg1 --arg2 arg2 --arg3",
                 "${DDS_LOCATION}/",
                 bashPath,
                 "bash",
                 "${DDS_LOCATION}/bash --arg1 arg1 --arg2 arg2 --arg3");

    testParseExe("bash --arg1 arg1 --arg2 arg2 --arg3",
                 "${DDS_LOCATION}/",
                 bashPath,
                 "bash",
                 "${DDS_LOCATION}/bash --arg1 arg1 --arg2 arg2 --arg3");

    testParseExe("\"/path/to/test_exe\" --arg1 \"arg1 arg1\" --arg2 \"arg2 arg2\" --arg3",
                 "${DDS_LOCATION}/",
                 "/path/to/test_exe",
                 "test_exe",
                 "\"${DDS_LOCATION}/test_exe\" --arg1 \"arg1 arg1\" --arg2 \"arg2 arg2\" --arg3");

    BOOST_CHECK_THROW(testParseExe("relative/path/to/task --arg1 --arg2", "", "", "", ""), std::runtime_error);
    BOOST_CHECK_THROW(testParseExe("non_existing_task --arg1 --arg2", "", "", "", ""), std::runtime_error);
    BOOST_CHECK_THROW(testParseExe("", "", "", "", ""), std::runtime_error);
}
//=============================================================================

BOOST_AUTO_TEST_SUITE_END();
