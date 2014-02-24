/************************************************************************/
/**
 * @file Test_Protocol.cpp
 * @brief Unit tests of Protocol
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2009-12-28
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2009 GSI GridTeam. All rights reserved.
*************************************************************************/

// BOOST: tests
// Defines test_main function to link with actual unit test code.
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include "DDSTopology.h"
#include "DDSTask.h"
#include "DDSTaskCollection.h"
#include "DDSTaskGroup.h"

BOOST_AUTO_TEST_SUITE(test_dds_topology)

BOOST_AUTO_TEST_CASE(test_dds_topology_init)
{
    DDSTopology topology;
    topology.init("/Users/andrey/Development/gsi/dds/DDS/dds-topology/tests/"
                  "test_topology.xml");
    BOOST_CHECK_EQUAL(1, 1); ///( "some error 1" );
}

BOOST_AUTO_TEST_CASE(test_case_on_file_scope)
{
    BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE(test_case2)
{
    // BOOST_ERROR( "some error 2" );
    BOOST_CHECK_EQUAL(1, 1);
}

BOOST_AUTO_TEST_SUITE_END()
