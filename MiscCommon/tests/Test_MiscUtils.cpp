// Copyright 2014 GSI, Inc. All rights reserved.
//
// Unit tests
//
// BOOST: tests
// Defines test_main function to link with actual unit test code.
#define BOOST_TEST_DYN_LINK
#define BOOST_AUTO_TEST_MAIN // Boost 1.33
#define BOOST_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

using boost::unit_test::test_suite;

// STD
#include <string>

// Our
#include "MiscUtils.h"

using namespace MiscCommon;
using namespace std;

BOOST_AUTO_TEST_SUITE(MiscCommon_MiscCommon1);
//=============================================================================
BOOST_AUTO_TEST_CASE(Test_MiscCommon_smart_append)
{
    const string sTempl("/Test1/Test/");
    const string sTempl2("Test1/Test/");

    {
        string sVal = "/Test1/Test/";
        smart_append(&sVal, '/');
        BOOST_CHECK(sTempl == sVal);
    }

    {
        string sVal = "/Test1/Test";
        smart_append(&sVal, '/');
        BOOST_CHECK(sTempl == sVal);
    }

    {
        string sVal = "Test1/Test";
        smart_append(&sVal, '/');
        BOOST_CHECK(sTempl2 == sVal);
    }
}
//=============================================================================
BOOST_AUTO_TEST_CASE(Test_MiscCommonreplace)
{
    const string sTempl("Test_HELLO/Tset");

    {
        string sVal = "%1_HELLO/Tset";
        replace<string>(&sVal, "%1", "Test");
        BOOST_CHECK(sTempl == sVal);
    }

    {
        string sVal = "Test_HELLO/%1";
        replace<string>(&sVal, "%1", "Tset");
        BOOST_CHECK(sTempl == sVal);
    }

    {
        string sVal = "Test_%1/Tset";
        replace<string>(&sVal, "%1", "HELLO");
        BOOST_CHECK(sTempl == sVal);
    }
}
//=============================================================================
BOOST_AUTO_TEST_CASE(Test_MiscCommon_to_lower)
{
    const string sTempl("test_4hello");

    {
        string sVal = "TesT_4HEllO";
        to_lower(sVal);
        BOOST_CHECK(sTempl == sVal);
    }
}
//=============================================================================
BOOST_AUTO_TEST_CASE(Test_MiscCommon_to_upper)
{
    const string sTempl("TEST2_HELLO");

    {
        string sVal = "TesT2_HEllo";
        to_upper(sVal);
        BOOST_CHECK(sTempl == sVal);
    }
}
//=============================================================================
BOOST_AUTO_TEST_CASE(Test_MiscCommon_trim)
{
    const string sTempl("TEST2_HELLO");

    {
        string sVal = " TEST2_HELLO ";
        trim<string>(&sVal, ' ');
        BOOST_CHECK(sTempl == sVal);
    }

    {
        string sVal = "/TEST2_HELLO";
        trim<string>(&sVal, '/');
        BOOST_CHECK(sTempl == sVal);
    }

    {
        string sVal = "TEST2_HELLO/";
        trim<string>(&sVal, '/');
        BOOST_CHECK(sTempl == sVal);
    }
}
//=============================================================================
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
    testParseExe("/my_location/test_exe --arg1 arg1 --arg2 arg2 --arg3",
                 "${DDS_LOCATION}/",
                 "/my_location/test_exe",
                 "test_exe",
                 "${DDS_LOCATION}/test_exe --arg1 arg1 --arg2 arg2 --arg3");

    const string bashPath = boost::process::search_path("bash").string();
    testParseExe(
        "bash --arg1 arg1 --arg2 arg2 --arg3", "", bashPath, "bash", bashPath + " --arg1 arg1 --arg2 arg2 --arg3");

    testParseExe("bash --arg1 arg1 --arg2 arg2 --arg3",
                 "${DDS_LOCATION}/",
                 bashPath,
                 "bash",
                 "${DDS_LOCATION}/bash --arg1 arg1 --arg2 arg2 --arg3");

    BOOST_CHECK_THROW(testParseExe("my_location/test_exe --arg1 arg1", "", "", "", ""), std::runtime_error);
}
//=============================================================================

BOOST_AUTO_TEST_SUITE_END();
