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

using namespace std;

BOOST_AUTO_TEST_SUITE(test_dds_topology)

BOOST_AUTO_TEST_CASE(test_dds_topology_init_1)
{
    DDSTopology topology;

    topology.init("topology_test_1.xml");
    BOOST_CHECK(true);

    DDSTaskGroupPtr_t main = topology.getMainGroup();
    BOOST_CHECK(main->getNofTasks() == 22);
    BOOST_CHECK(main->getN() == 7);
    BOOST_CHECK(main->getMinimumRequired() == 1);
    BOOST_CHECK(main->getName() == "");
    BOOST_CHECK(main->getNofElements() == 4);
    BOOST_CHECK_THROW(main->getElement(4), std::out_of_range);

    DDSTopoElementPtr_t element1 = main->getElement(0);
    BOOST_CHECK(element1->getName() == "task1");
    BOOST_CHECK(element1->getType() == DDSTopoElementType::TASK);
    BOOST_CHECK(element1->getNofTasks() == 1);
    DDSTaskPtr_t casted1 = dynamic_pointer_cast<DDSTask>(element1);
    BOOST_CHECK(casted1->getNofPorts() == 2);
    BOOST_CHECK(casted1->getExec() == "app1");

    DDSTopoElementPtr_t element2 = main->getElement(1);
    BOOST_CHECK(element2->getName() == "collection1");
    BOOST_CHECK(element2->getType() == DDSTopoElementType::COLLECTION);
    BOOST_CHECK(element2->getNofTasks() == 4);
    DDSTaskCollectionPtr_t casted2 = dynamic_pointer_cast<DDSTaskCollection>(element2);
    BOOST_CHECK(casted2->getNofElements() == 4);
    BOOST_CHECK_THROW(casted2->getElement(4), std::out_of_range);
    BOOST_CHECK(casted2->getN() == 11);
    BOOST_CHECK(casted2->getMinimumRequired() == 1);

    DDSTopoElementPtr_t element3 = main->getElement(2);
    BOOST_CHECK(element3->getName() == "group1");
    BOOST_CHECK(element3->getType() == DDSTopoElementType::GROUP);
    BOOST_CHECK(element3->getNofTasks() == 8);
    DDSTaskGroupPtr_t casted3 = dynamic_pointer_cast<DDSTaskGroup>(element3);
    BOOST_CHECK(casted3->getNofElements() == 3);
    BOOST_CHECK_THROW(casted3->getElement(3), std::out_of_range);
    BOOST_CHECK(casted3->getN() == 10);
    BOOST_CHECK(casted3->getMinimumRequired() == 1);

    DDSTopoElementPtr_t element4 = main->getElement(3);
    BOOST_CHECK(element4->getName() == "group2");
    BOOST_CHECK(element4->getType() == DDSTopoElementType::GROUP);
    BOOST_CHECK(element4->getNofTasks() == 9);
    DDSTaskGroupPtr_t casted4 = dynamic_pointer_cast<DDSTaskGroup>(element4);
    BOOST_CHECK(casted4->getNofElements() == 4);
    BOOST_CHECK_THROW(casted4->getElement(4), std::out_of_range);
    BOOST_CHECK(casted4->getN() == 15);
    BOOST_CHECK(casted4->getMinimumRequired() == 3);
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
