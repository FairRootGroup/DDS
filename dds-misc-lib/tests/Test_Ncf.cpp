/************************************************************************/
/**
 * @file test_config.cpp
 * @brief
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2010-06-07
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2010 GSI GridTeam. All rights reserved.
*************************************************************************/
// STD
#include <stdexcept>
// BOOST: tests
// Defines test_main function to link with actual unit test code.
#define BOOST_TEST_DYN_LINK
#define BOOST_AUTO_TEST_MAIN // Boost 1.33
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
// pod-ssh
#include "SSHConfigFile.h"
//=============================================================================
using namespace std;
using namespace dds;
using boost::unit_test::test_suite;

BOOST_AUTO_TEST_SUITE(pod_ssh_config);
//=============================================================================

//=============================================================================
BOOST_AUTO_TEST_CASE(test_readconfig)
{
    stringstream ss;
    ss << "r1, \\\"anar@lxg0527.gsi.de\\\", -p24, /tmp/test,4\n"
       << "r2, anar@lxi001.gsi.de,, /tmp/test/fff fff, 2\n"
       << "125, anar@lxg0055.gsi.de, -p22, /tmp/test, 8\n";

    CSSHConfigFile config(ss);

    configRecords_t recs(config.getRecords());
    BOOST_REQUIRE(!recs.empty());

    // Checking record #1
    SConfigRecord rec;
    rec.m_id = "r1";
    rec.m_addr = "\"anar@lxg0527.gsi.de\"";
    rec.m_sshOptions = "-p24";
    rec.m_wrkDir = "/tmp/test";
    rec.m_nSlots = 4;
    BOOST_REQUIRE(*recs[0] == rec);

    rec = SConfigRecord();
    rec.m_id = "r2";
    rec.m_addr = "anar@lxi001.gsi.de";
    rec.m_sshOptions = "";
    rec.m_wrkDir = "/tmp/test/fff fff";
    rec.m_nSlots = 2;
    BOOST_REQUIRE(*recs[1] == rec);

    rec = SConfigRecord();
    rec.m_id = "125";
    rec.m_addr = "anar@lxg0055.gsi.de";
    rec.m_sshOptions = "-p22";
    rec.m_wrkDir = "/tmp/test";
    rec.m_nSlots = 8;
    BOOST_REQUIRE(*recs[2] == rec);
}
BOOST_AUTO_TEST_CASE(test_readconfig_bad)
{
    stringstream ss;
    ss << "r1, \\\"anar@lxg0527.gsi.de\\\", -p24, /tmp/test,4\n"
       << "r2, anar@lxi001.gsi.de,/tmp/test,2\n"
       << "125, anar@lxg0055.gsi.de, -p22, /tmp/test,8\n";

    BOOST_REQUIRE_THROW(CSSHConfigFile config(ss), runtime_error);
}
BOOST_AUTO_TEST_CASE(test_duplicate_id)
{
    stringstream ss;
    ss << "r1, \\\"anar@lxg0527.gsi.de\\\", -p24, /tmp/test,4\n"
       << "r2, anar@lxi001.gsi.de,,/tmp/test,2\n"
       << "r1, anar@lxg0055.gsi.de, -p22, /tmp/test,8\n";

    BOOST_REQUIRE_THROW(CSSHConfigFile config(ss), runtime_error);
}

void test_make(const string& _result,
               const vector<string>& _hosts,
               const std::string& _sshOptions = "",
               const std::string& _wrkDir = "/tmp/wn_dds",
               size_t _numSlots = 1,
               const string& _bash = "")
{
    stringstream ss1;
    CSSHConfigFile::make(ss1, _hosts, _sshOptions, _wrkDir, _numSlots, _bash);
    const string bash1{ _bash.empty() ? string() : string("@bash_begin@\n") + _bash + "\n@bash_end@\n\n" };
    BOOST_CHECK(ss1.str() == (bash1 + _result));

    auto f{ CSSHConfigFile(ss1) };

    stringstream ss2;
    CSSHConfigFile::make(ss2, f.getRecords(), f.getBash());
    const string bash2{ _bash.empty() ? string() : string("@bash_begin@\n") + f.getBash() + "\n@bash_end@\n\n" };
    BOOST_CHECK(ss2.str() == (bash2 + _result));
}

BOOST_AUTO_TEST_CASE(test_make_1)
{
    const string result1{ "wn_user@host1.gsi.de, user@host1.gsi.de, , /tmp/wn_dds, 1\nwn_user@host2.gsi.de, "
                          "user@host2.gsi.de, , /tmp/wn_dds, 1\n" };
    const vector<string> hosts1{ "user@host1.gsi.de", "user@host2.gsi.de" };
    test_make(result1, hosts1);

    const string result2{ "wn_user@host1.gsi.de, user@host1.gsi.de, -p1234, "
                          "~/tmp/, 10\nwn_user@host2.gsi.de, user@host2.gsi.de, -p1234, ~/tmp/, 10\n" };
    const vector<string> hosts2{ "user@host1.gsi.de", "user@host2.gsi.de" };
    test_make(result2, hosts2, "-p1234", "~/tmp/", 10, "source env.sh");
}

BOOST_AUTO_TEST_SUITE_END();
