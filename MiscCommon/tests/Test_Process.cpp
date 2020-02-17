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
BOOST_AUTO_TEST_CASE(test_MiscCommon_execute_timeout)
{
    // bash -c "echo test > test.test"
    const string sFile("/tmp/test.test");
    // clean file befor test
    boost::filesystem::remove(sFile);

    stringstream ssCmd;
    ssCmd << boost::process::search_path("bash").string()
          << " -c \"echo test > /tmp/test.test; sleep 3; echo test2 > /tmp/test.test\"";
    execute(ssCmd.str(), std::chrono::seconds(5));

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
BOOST_AUTO_TEST_CASE(test_MiscCommon_execute_terminate_exception)
{
    stringstream ssCmd;
    ssCmd << boost::process::search_path("ping").string() << " localhost";
    BOOST_CHECK_THROW(execute(ssCmd.str(), std::chrono::seconds(3)), runtime_error);
}
//=============================================================================
BOOST_AUTO_TEST_CASE(test_MiscCommon_execute_stdout_stderr)
{
    // Check stderr
    stringstream ssCmd;
    ssCmd << boost::process::search_path("bash").string() << " -c \"hostname -f 1>&2; exit 11;\"";
    string output;
    string error;
    int exitCode(-1);

    execute(ssCmd.str(), std::chrono::seconds(5), &output, &error, &exitCode, std::chrono::seconds(5));
    BOOST_TEST(output.empty());
    BOOST_TEST(!error.empty());
    BOOST_TEST(exitCode == 11);
}
//=============================================================================
BOOST_AUTO_TEST_CASE(test_MiscCommon_execute_fileout)
{
    stringstream ssCmd;
    string sTestString = "TEST1234 45";
    ssCmd << boost::process::search_path("bash").string() << " -c \"echo -n " << sTestString << "; echo -n "
          << sTestString << " >&2\"";
    string stdoutFile("/tmp/stdout");
    string sterrorFile("/tmp/stderr");

    // clean file befor test
    boost::filesystem::remove(stdoutFile);
    boost::filesystem::remove(sterrorFile);

    pid_t pid(0);
    pid = execute(ssCmd.str(), stdoutFile, sterrorFile);

    BOOST_TEST(pid != 0);
    // Wait for the preocess to finish
    int status(0);
    while (true)
    {
        pid_t ret = ::waitpid(pid, &status, WNOHANG | WUNTRACED);
        if (ret < 0 || ret == pid)
            break;
    }

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
BOOST_AUTO_TEST_CASE(test_MiscCommon_execute_bad_process)
{
    string output;
    string error;
    BOOST_CHECK_THROW(execute("1111111", std::chrono::seconds(3), &output, &error), runtime_error);
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
