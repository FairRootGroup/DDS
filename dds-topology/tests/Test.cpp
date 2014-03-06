// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

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

BOOST_AUTO_TEST_CASE(test_dds_topology_init_1)
{
    DDSTopology topology;
    topology.init("topology_test_1.xml");
    BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE(test_dds_topology_validation_1)
{
    DDSTopology topology;
    bool result = topology.isValid("topology_test_1.xml");
    BOOST_CHECK(result == true);
}

BOOST_AUTO_TEST_CASE(test_dds_topology_validation_2)
{
    DDSTopology topology;
    bool result = topology.isValid("topology_test_2.xml");
    BOOST_CHECK(result == false);
}

BOOST_AUTO_TEST_CASE(test_dds_topology_validation_3)
{
    DDSTopology topology;
    bool result = topology.isValid("topology_test_3.xml");
    BOOST_CHECK(result == false);
}

BOOST_AUTO_TEST_CASE(test_dds_topology_validation_4)
{
    DDSTopology topology;
    bool result = topology.isValid("topology_test_4.xml");
    BOOST_CHECK(result == false);
}

BOOST_AUTO_TEST_CASE(test_dds_topology_validation_5)
{
    DDSTopology topology;
    bool result = topology.isValid("topology_test_5.xml");
    BOOST_CHECK(result == false);
}

BOOST_AUTO_TEST_CASE(test_dds_topology_validation_6)
{
    DDSTopology topology;
    bool result = topology.isValid("wrong_file.xml");
    BOOST_CHECK(result == false);
}

BOOST_AUTO_TEST_SUITE_END()
