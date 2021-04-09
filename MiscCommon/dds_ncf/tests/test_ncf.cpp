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
//#include "version.h"
#include "ncf.h"
//=============================================================================
using namespace std;
using namespace dds;
using namespace dds::ncf;
using boost::unit_test::test_suite;

BOOST_AUTO_TEST_SUITE(pod_ssh_config);
//=============================================================================

//=============================================================================
BOOST_AUTO_TEST_CASE(test_readconfig)
{
    CNcf config;

    stringstream ss;
    ss << "r1, \\\"anar@lxg0527.gsi.de\\\", -p24, /tmp/test,4\n"
       << "r2, anar@lxi001.gsi.de,, /tmp/test/fff fff, 2\n"
       << "125, anar@lxg0055.gsi.de, -p22, /tmp/test, 8\n";

    config.readFrom(ss);

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
    CNcf config;

    stringstream ss;
    ss << "r1, \\\"anar@lxg0527.gsi.de\\\", -p24, /tmp/test,4\n"
       << "r2, anar@lxi001.gsi.de,/tmp/test,2\n"
       << "125, anar@lxg0055.gsi.de, -p22, /tmp/test,8\n";

    BOOST_REQUIRE_THROW(config.readFrom(ss), runtime_error);
}
BOOST_AUTO_TEST_CASE(test_duplicate_id)
{
    CNcf config;

    stringstream ss;
    ss << "r1, \\\"anar@lxg0527.gsi.de\\\", -p24, /tmp/test,4\n"
       << "r2, anar@lxi001.gsi.de,,/tmp/test,2\n"
       << "r1, anar@lxg0055.gsi.de, -p22, /tmp/test,8\n";

    BOOST_REQUIRE_THROW(config.readFrom(ss), runtime_error);
}

BOOST_AUTO_TEST_SUITE_END();
