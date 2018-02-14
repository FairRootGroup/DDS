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
    BOOST_CHECK_THROW(pid = execute(ssCmd.str(), std::chrono::seconds(5)), runtime_error);
    boost::process::child c(pid);
    bool failed = c.running();
    if (c.running() && !c.wait_for(std::chrono::seconds(1)))
    {
        // Child didn't yet finish. Terminating it...
        if (c.running())
            c.terminate();
    }
    // the test process supposed to be terminated by the execute function
    if (failed)
        BOOST_FAIL("Test process wasn't terminated");
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
    boost::process::child c(pid);
    if (c.running() && !c.wait_for(std::chrono::seconds(5)))
    {
        // Child didn't yet finish. Terminating it...
        if (c.running())
            c.terminate();
        BOOST_FAIL("Test process didn't finish.");
    }

    // Workaround: there is always a '\n' at the end of the output after it's redirected by boost::process
    // In order to workit around we also add a '\n' to our test string.
    sTestString += '\n';

    ifstream fStdout(stdoutFile);
    BOOST_CHECK(fStdout.is_open());
    std::stringstream bufferOut;
    bufferOut << fStdout.rdbuf();
    BOOST_CHECK(sTestString == bufferOut.str());

    ifstream fStderr(sterrorFile);
    BOOST_CHECK(fStderr.is_open());
    std::stringstream bufferErr;
    bufferErr << fStderr.rdbuf();
    BOOST_CHECK(sTestString == bufferErr.str());

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
