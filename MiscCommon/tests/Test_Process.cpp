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

// MiscCommon
#include "Process.h"
//=============================================================================
using namespace MiscCommon;
using namespace std;
using boost::unit_test::test_suite;
namespace fs = boost::filesystem;
//=============================================================================
BOOST_AUTO_TEST_SUITE(test_Process);
struct STestConfig
{
    static STestConfig*& instance()
    {
        static STestConfig* inst = nullptr;
        return inst;
    }

    STestConfig()
    {
        instance() = this;

        m_pathWrkDir = fs::temp_directory_path() / fs::unique_path();
        fs::create_directories(m_pathWrkDir);
        BOOST_TEST_MESSAGE("Test WrkDir: " << m_pathWrkDir);
    }
    ~STestConfig()
    {
        if (fs::exists(m_pathWrkDir))
        {
            BOOST_TEST_MESSAGE("Removing Wrk Dir: " << m_pathWrkDir);
            fs::remove_all(m_pathWrkDir);
        }
    }

    fs::path m_pathWrkDir;
};

BOOST_GLOBAL_FIXTURE(STestConfig);
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
    BOOST_CHECK_NO_THROW(execute(ssCmd.str(), std::chrono::seconds(10)));

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
    ssCmd << boost::process::search_path("bash").string() << " -c \"hostname -f; hostname -f 1>&2; exit 11;\"";
    string output;
    string error;
    int exitCode(-1);

    execute(ssCmd.str(), std::chrono::seconds(5), &output, &error, &exitCode);
    BOOST_TEST(!output.empty());
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
//=============================================================================
BOOST_AUTO_TEST_CASE(test_MiscCommon_execute_no_stdout_stderr)
{
    // Check stderr
    stringstream ssCmd;
    ssCmd << boost::process::search_path("bash").string() << " -c \"sleep 1\"";
    string output;
    string error;
    int exitCode(-1);

    execute(ssCmd.str(), std::chrono::seconds(5), &output, &error, &exitCode);
    BOOST_TEST(output.empty());
    BOOST_TEST(error.empty());
}

//=============================================================================
BOOST_AUTO_TEST_CASE(test_MiscCommon_execute_dds_daemonize)
{
    auto tmpDir{ STestConfig::instance()->m_pathWrkDir };
    // Check stderr
    stringstream ssCmd;
    ssCmd << boost::process::search_path("dds-daemonize").string() << " \"" << tmpDir.string() << "\" "
          << boost::process::search_path("hostname").string() << " -f";
    string output;
    string error;
    int exitCode(-1);

    execute(ssCmd.str(), std::chrono::seconds(5), &output, &error, &exitCode);

    // Give a chance to a child to create its log file
    this_thread::sleep_for(std::chrono::seconds(1));

    fs::path logFile{ tmpDir };
    logFile /= "hostname.out.log";
    BOOST_TEST(fs::exists(logFile));
    BOOST_TEST(fs::file_size(logFile) > 0);
}

//=============================================================================
BOOST_AUTO_TEST_CASE(test_MiscCommon_excute_in_grandchildren)
{
    auto tmpDir{ STestConfig::instance()->m_pathWrkDir };
    fs::path outputfile(tmpDir);
    outputfile /= "test_MiscCommon_excute_in_grandchildren";

    execute("MiscCommon_test_ProcessExecutable --wait=2 --output-file=" + outputfile.string());

    // let process finish
    this_thread::sleep_for(std::chrono::seconds(3));
    BOOST_TEST(fs::exists(outputfile));
    BOOST_TEST(fs::file_size(outputfile) > 0);
}

BOOST_AUTO_TEST_SUITE_END();
