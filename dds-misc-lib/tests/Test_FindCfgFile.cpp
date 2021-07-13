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
// MiscCommon
#include "FindCfgFile.h"

using boost::unit_test::test_suite;
using namespace std;
using namespace dds::misc;

BOOST_AUTO_TEST_SUITE(MiscCommon_FindCfgFile);
//=============================================================================
BOOST_AUTO_TEST_CASE(Test_FindCfgFile_order)
{
    const string sT1("Test1");
    const string sT2("Test2");
    const string sT3("Test3");
    const string sTempl(sT1 + sT2 + sT3);
    CFindCfgFile<string> cfg;
    cfg.SetOrder(sT1)(sT2)(sT3);
    ostringstream ss;
    cfg.DumpOrder(&ss, "");
    BOOST_CHECK(ss.str() == sTempl);
}
//=============================================================================
BOOST_AUTO_TEST_CASE(Test_FindCfgFile_match1)
{
    CFindCfgFile<string> cfg;
    const string sTempl("/etc/bashrc");
    cfg.SetOrder("/etc/rtt")(sTempl)("testtesttest")("$HOME/.bashrc");
    string val;
    cfg.GetCfg(&val);
    // TODO: rewrite test, so that there will be a file, which always there on all systems.
    // /etc/bashrc is not always present
    //    BOOST_CHECK( val == sTempl );
}
//=============================================================================
BOOST_AUTO_TEST_CASE(Test_FindCfgFile_match2)
{
    CFindCfgFile<string> cfg;
    const string sTempl("$HOME/.bashrc");
    cfg.SetOrder(sTempl)("/etc/rtt")("/etc/bashrc")("testtesttest");
    string val;
    cfg.GetCfg(&val);
    BOOST_CHECK(val == sTempl);
}
BOOST_AUTO_TEST_SUITE_END();
