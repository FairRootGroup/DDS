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

// MiscCommon
#include "Process.h"
//=============================================================================
using namespace MiscCommon;
using namespace std;
using boost::unit_test::test_suite;
//=============================================================================
BOOST_AUTO_TEST_SUITE(pod_agent_MiscCommon);
//=============================================================================
#ifndef __APPLE__
BOOST_AUTO_TEST_CASE(test_MiscCommon_CProcStatus)
{
    CProcStatus p;
    pid_t pid(::getpid());
    p.Open(pid);
    // TODO: need a new algorithms for a longer app names retrieval
    BOOST_CHECK(p.GetValue("Name") == "MiscCommon_test");

    BOOST_CHECK(p.GetValue("NAME") == "MiscCommon_test");

    stringstream ss_pid;
    ss_pid << pid;
    BOOST_CHECK(p.GetValue("Pid") == ss_pid.str());
}
#endif
//=============================================================================
BOOST_AUTO_TEST_CASE(test_MiscCommon_do_execv0)
{
    // bash -c "echo test > test.test"
    const string sFile("/tmp/test.test");

    const string cmd("/bin/bash");
    stringstream ssCmd;
    ssCmd << cmd << " -c ` echo test > /tmp/test.test; sleep 5; echo test2 > /tmp/test.test`";
    do_execv(ssCmd.str(), 10, NULL);

    ifstream test_file(sFile.c_str());
    BOOST_CHECK(test_file.is_open());

    string test_string;
    test_file >> test_string;
    BOOST_CHECK_EQUAL(test_string, "test2");

    if (test_file.is_open())
    {
        test_file.close();
        ::unlink(sFile.c_str());
    }

    string output;
    stringstream ssCmd2;
    ssCmd2 << "/bin/hostname -f";
    do_execv(ssCmd2.str(), 10, &output);
    BOOST_CHECK(output.size() > 0);
}
//=============================================================================
BOOST_AUTO_TEST_CASE(test_MiscCommon_do_execv1)
{
    stringstream ssCmd;
    ssCmd << "/bin/sleep 4";
    BOOST_CHECK_THROW(do_execv(ssCmd.str(), 2, NULL), runtime_error);
}
//=============================================================================
BOOST_AUTO_TEST_CASE(test_MiscCommon_do_execv2)
{
    stringstream ssCmd;
    ssCmd << "XXXXX eee";
    BOOST_CHECK_THROW(do_execv(ssCmd.str(), 3, NULL), runtime_error);
}
//=============================================================================
BOOST_AUTO_TEST_CASE(test_MiscCommon_getprocbyname)
{
#ifdef __APPLE__
    const string name1("syslogd");
    const string name2("launchd");
#else
    const string name1("kdm");
    const string name2("sshd");
#endif
    {
        vectorPid_t container;
        getprocbyname(name1, true);
        BOOST_CHECK(container.empty());
    }
    {
        vectorPid_t container;
        getprocbyname(name1);
        BOOST_CHECK(container.empty());
    }
    {
        vectorPid_t container;
        getprocbyname(name2, true);
        BOOST_CHECK(container.empty());
    }
}

BOOST_AUTO_TEST_SUITE_END();
