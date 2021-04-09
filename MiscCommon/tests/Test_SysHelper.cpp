// Copyright 2014 GSI, Inc. All rights reserved.
//
// Unit tests
//
// BOOST: tests
// Defines test_main function to link with actual unit test code.
#define BOOST_TEST_DYN_LINK
#define BOOST_AUTO_TEST_MAIN // Boost 1.33
#define BOOST_TEST_MAIN
#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>
// STD
#include <fstream>
#include <string>
// Our
#include "MiscUtils.h"
#include "SysHelper.h"

using boost::unit_test::test_suite;
using namespace MiscCommon;
using namespace std;

//=============================================================================
BOOST_AUTO_TEST_SUITE(test_SysHelper);
//=============================================================================
BOOST_AUTO_TEST_CASE(test_MiscCommon_smart_path0)
{
    string path("$TEST1/opt/test.xml");
    const string path_res("/test1/demo/opt/test.xml");

    auto_setenv env("TEST1", "/test1/demo");
    smart_path(&path);
    BOOST_CHECK(path == path_res);
}
//=============================================================================
BOOST_AUTO_TEST_CASE(test_MiscCommon_smart_path1)
{
    string path("$TEST1/opt/$TEST2");
    const string path_res("/test1/demo/opt//test2/demo2");

    auto_setenv env1("TEST1", "/test1/demo");
    auto_setenv env2("TEST2", "/test2/demo2");
    smart_path(&path);
    BOOST_CHECK(path == path_res);
}
//=============================================================================
BOOST_AUTO_TEST_CASE(test_MiscCommon_smart_path2)
{
    string path("$TEST1/opt/$TEST2");
    const string path_res("$TEST1/opt/$TEST2");
    smart_path(&path);
    BOOST_CHECK(path == path_res);
}
//=============================================================================
BOOST_AUTO_TEST_CASE(test_MiscCommon_smart_path3)
{
    string path("$TEST1/opt");
    const string path_res("/test1//test2/demo2/demo/opt");

    auto_setenv env1("TEST1", "/test1/$TEST2/demo");
    auto_setenv env2("TEST2", "/test2/demo2");
    smart_path(&path);
    BOOST_CHECK(path == path_res);
}
//=============================================================================
BOOST_AUTO_TEST_CASE(test_MiscCommon_smart_path4)
{
    string path("$TEST1/opt");
    const string path_res("/test2/demo2/opt");

    auto_setenv env1("TEST1", "$TEST2");
    auto_setenv env2("TEST2", "/test2/demo2");
    smart_path(&path);
    BOOST_CHECK(path == path_res);
}
//=============================================================================
BOOST_AUTO_TEST_CASE(test_MiscCommon_smart_path5)
{
    string path("~/test/test");
    const string sTempl("/home/anar/test/test");
    smart_path(&path);
    // TODO: rewrite test, so that there will be no hard-codded "/home/anar"
    // BOOST_CHECK( sTempl == path );
}
//=============================================================================
BOOST_AUTO_TEST_CASE(test_MiscCommon_smart_path6)
{
    string path("/~test/test");
    const string sTempl("/~test/test");
    smart_path(&path);
    BOOST_CHECK(sTempl == path);
}
//=============================================================================
BOOST_AUTO_TEST_CASE(test_MiscCommon_smart_path7)
{
    string path("/test/test~/");
    const string sTempl("/test/test~/");
    smart_path(&path);
    BOOST_CHECK(sTempl == path);
}
//=============================================================================
BOOST_AUTO_TEST_CASE(test_MiscCommon_smart_path8)
{
    string path("~anar/test/test");
    const string sTempl("/home/anar/test/test");
    smart_path(&path);
    // TODO: rewrite test, so that there will be no hard-codded "/home/anar"
    //    BOOST_CHECK( sTempl == path );
}
//=============================================================================
BOOST_AUTO_TEST_CASE(test_MiscCommon_smart_path9)
{
    // regression bug test. smart_path used to remove the trailing slash.
    // check that smart_path doesn't remove the last symbol
    string path("$HOME/");
    smart_path(&path);

    string sTempl(path);
    smart_append(&sTempl, '/');
    BOOST_CHECK(sTempl == path);
}
//=============================================================================
BOOST_AUTO_TEST_CASE(test_file_size0)
{
    // Creating test file
    boost::filesystem::path tmpFilename{ boost::filesystem::temp_directory_path() / boost::filesystem::unique_path() };
    const string filename(tmpFilename.string());
    ofstream f(filename.c_str());
    const off_t size = 1000;
    const string buf(size, 'A');
    f << buf;
    f.close();
    BOOST_CHECK_EQUAL(size, file_size(filename));
    ::unlink(filename.c_str());
}
//=============================================================================
BOOST_AUTO_TEST_CASE(test_file_size1)
{
    const string filename("missing_file.txt");
    BOOST_CHECK_THROW(file_size(filename), MiscCommon::system_error);
}
//=============================================================================
BOOST_AUTO_TEST_SUITE_END();
