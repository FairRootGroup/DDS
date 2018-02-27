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
    // clean file befor test
    boost::filesystem::remove(sFile);

    stringstream ssCmd;
    ssCmd << boost::process::search_path("bash").string()
          << " -c \"echo test > /tmp/test.test; sleep 5; echo test2 > /tmp/test.test\"";
    execute(ssCmd.str(), std::chrono::seconds(10));

    ifstream test_file(sFile);
    BOOST_CHECK(test_file.is_open());

    string test_string;
    test_file >> test_string;
    BOOST_CHECK_EQUAL(test_string, "test2");

    if (test_file.is_open())
    {
        test_file.close();
        boost::filesystem::remove(sFile);
    }

    string output;
    stringstream ssCmd2;
    ssCmd2 << boost::process::search_path("hostname").string() << " -f";
    execute(ssCmd2.str(), std::chrono::seconds(3), &output);
    BOOST_CHECK(!output.empty());
}
//=============================================================================
BOOST_AUTO_TEST_CASE(test_MiscCommon_do_execv1)
{
    stringstream ssCmd;
    ssCmd << boost::process::search_path("ping").string() << " localhost";
    pid_t pid(0);
    BOOST_CHECK_THROW(execute(ssCmd.str(), std::chrono::seconds(5)), runtime_error);
}
//=============================================================================
BOOST_AUTO_TEST_CASE(test_MiscCommon_do_execv2)
{
    // Check stderr
    stringstream ssCmd;
    ssCmd << boost::process::search_path("bash").string() << " -c \"hostname -f 1>&2\"";
    pid_t pid(0);
    string output;
    string error;
    pid = execute(ssCmd.str(), std::chrono::seconds(5), &output, &error);
    BOOST_CHECK(output.empty());
    BOOST_CHECK(!error.empty());
    boost::process::child c(pid);
    BOOST_CHECK(!c.running());
}
//=============================================================================
BOOST_AUTO_TEST_CASE(test_MiscCommon_do_execv3)
{
    stringstream ssCmd;
    string sTestString = "TEST1234 45";
    ssCmd << boost::process::search_path("bash").string() << " -c \"echo " << sTestString << "; echo " << sTestString
          << " >&2\"";
    string stdoutFile("/tmp/stdout");
    string sterrorFile("/tmp/stderr");

    // clean file befor test
    boost::filesystem::remove(stdoutFile);
    boost::filesystem::remove(sterrorFile);

    pid_t pid(0);
    pid = execute(ssCmd.str(), stdoutFile, sterrorFile);

    std::error_code ec;
    boost::process::child c(pid);
    BOOST_CHECK(c.wait_for(std::chrono::seconds(5), ec));
    BOOST_CHECK_MESSAGE(!ec, ec.message());

    // Workaround: there is always a '\n' at the end of the output after it's redirected by boost::process
    // In order to workit around we also add a '\n' to our test string.
    sTestString += '\n';

    ifstream fStdout(stdoutFile);
    BOOST_CHECK(fStdout.is_open());
    ostringstream sOut;
    copy(istreambuf_iterator<char>(fStdout), istreambuf_iterator<char>(), ostreambuf_iterator<char>(sOut));
    BOOST_CHECK(sTestString == sOut.str());

    ifstream fStderr(sterrorFile);
    BOOST_CHECK(fStderr.is_open());
    ostringstream sErr;
    copy(istreambuf_iterator<char>(fStderr), istreambuf_iterator<char>(), ostreambuf_iterator<char>(sErr));
    BOOST_CHECK(sTestString == sErr.str());

    // clean file after test
    boost::filesystem::remove(stdoutFile);
    boost::filesystem::remove(sterrorFile);
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
