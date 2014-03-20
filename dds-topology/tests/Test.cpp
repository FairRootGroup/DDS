// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// BOOST: tests
// Defines test_main function to link with actual unit test code.
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

// DDS
#include "DDSTopologyParserXML.h"
#include "DDSTopoBase.h"
#include "DDSTopoElement.h"
#include "DDSTopoProperty.h"
#include "DDSTask.h"
#include "DDSTaskCollection.h"
#include "DDSTaskGroup.h"
#include "DDSTopoUtils.h"
#include "DDSTopoFactory.h"
// BOOST
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

using namespace std;
using namespace boost::property_tree;

BOOST_AUTO_TEST_SUITE(test_dds_topology)

BOOST_AUTO_TEST_CASE(test_dds_topology_parser_xml_1)
{
    DDSTopologyParserXML parser;

    DDSTaskGroupPtr_t main = parser.parse("topology_test_1.xml");
    BOOST_CHECK(main != nullptr);

    BOOST_CHECK(main->getNofTasks() == 22);
    BOOST_CHECK(main->getN() == 7);
    BOOST_CHECK(main->getMinimumRequired() == 1);
    BOOST_CHECK(main->getName() == "main");
    BOOST_CHECK(main->getNofElements() == 4);
    BOOST_CHECK_THROW(main->getElement(4), std::out_of_range);

    DDSTopoElementPtr_t element1 = main->getElement(0);
    BOOST_CHECK(element1->getName() == "task1");
    BOOST_CHECK(element1->getType() == DDSTopoType::TASK);
    BOOST_CHECK(element1->getNofTasks() == 1);
    DDSTaskPtr_t casted1 = dynamic_pointer_cast<DDSTask>(element1);
    BOOST_CHECK(casted1->getNofPorts() == 2);
    BOOST_CHECK(casted1->getExec() == "app1");

    DDSTopoElementPtr_t element2 = main->getElement(1);
    BOOST_CHECK(element2->getName() == "collection1");
    BOOST_CHECK(element2->getType() == DDSTopoType::COLLECTION);
    BOOST_CHECK(element2->getNofTasks() == 4);
    DDSTaskCollectionPtr_t casted2 = dynamic_pointer_cast<DDSTaskCollection>(element2);
    BOOST_CHECK(casted2->getNofElements() == 4);
    BOOST_CHECK_THROW(casted2->getElement(4), std::out_of_range);
    //  BOOST_CHECK(casted2->getN() == 11);
    //  BOOST_CHECK(casted2->getMinimumRequired() == 1);

    DDSTopoElementPtr_t element3 = main->getElement(2);
    BOOST_CHECK(element3->getName() == "group1");
    BOOST_CHECK(element3->getType() == DDSTopoType::GROUP);
    BOOST_CHECK(element3->getNofTasks() == 8);
    DDSTaskGroupPtr_t casted3 = dynamic_pointer_cast<DDSTaskGroup>(element3);
    BOOST_CHECK(casted3->getNofElements() == 3);
    BOOST_CHECK_THROW(casted3->getElement(3), std::out_of_range);
    BOOST_CHECK(casted3->getN() == 10);
    BOOST_CHECK(casted3->getMinimumRequired() == 1);

    DDSTopoElementPtr_t element4 = main->getElement(3);
    BOOST_CHECK(element4->getName() == "group2");
    BOOST_CHECK(element4->getType() == DDSTopoType::GROUP);
    BOOST_CHECK(element4->getNofTasks() == 9);
    DDSTaskGroupPtr_t casted4 = dynamic_pointer_cast<DDSTaskGroup>(element4);
    BOOST_CHECK(casted4->getNofElements() == 4);
    BOOST_CHECK_THROW(casted4->getElement(4), std::out_of_range);
    BOOST_CHECK(casted4->getN() == 15);
    BOOST_CHECK(casted4->getMinimumRequired() == 3);
}

BOOST_AUTO_TEST_CASE(test_dds_topology_parser_xml_validation_1)
{
    DDSTopologyParserXML parser;
    bool result = parser.isValid("topology_test_1.xml");
    BOOST_CHECK(result == true);
}

BOOST_AUTO_TEST_CASE(test_dds_topology_parser_xml_validation_2)
{
    DDSTopologyParserXML parser;
    bool result = parser.isValid("topology_test_2.xml");
    BOOST_CHECK(result == false);
}

BOOST_AUTO_TEST_CASE(test_dds_topology_parser_xml_validation_3)
{
    DDSTopologyParserXML parser;
    bool result = parser.isValid("topology_test_3.xml");
    BOOST_CHECK(result == false);
}

BOOST_AUTO_TEST_CASE(test_dds_topology_parser_xml_validation_4)
{
    DDSTopologyParserXML parser;
    bool result = parser.isValid("topology_test_4.xml");
    BOOST_CHECK(result == false);
}

BOOST_AUTO_TEST_CASE(test_dds_topology_parser_xml_validation_5)
{
    DDSTopologyParserXML parser;
    bool result = parser.isValid("topology_test_5.xml");
    BOOST_CHECK(result == false);
}

BOOST_AUTO_TEST_CASE(test_dds_topology_parser_xml_validation_6)
{
    DDSTopologyParserXML parser;
    bool result = parser.isValid("topology_test_6.xml");
    BOOST_CHECK(result == true);
}

BOOST_AUTO_TEST_CASE(test_dds_topology_parser_xml_validation_wrong)
{
    DDSTopologyParserXML parser;
    bool result = parser.isValid("wrong_file.xml");
    BOOST_CHECK(result == false);
}

BOOST_AUTO_TEST_CASE(test_dds_topo_factory)
{
    // DDSCreateTopoBase
    BOOST_CHECK_THROW(DDSCreateTopoBase(DDSTopoType::TOPO_BASE), runtime_error);
    BOOST_CHECK_THROW(DDSCreateTopoBase(DDSTopoType::TOPO_ELEMENT), runtime_error);
    BOOST_CHECK_THROW(DDSCreateTopoBase(DDSTopoType::TOPO_PROPERTY), runtime_error);
    DDSTopoBasePtr_t baseTask = DDSCreateTopoBase(DDSTopoType::TASK);
    BOOST_CHECK(baseTask != nullptr);
    DDSTopoBasePtr_t baseCollection = DDSCreateTopoBase(DDSTopoType::COLLECTION);
    BOOST_CHECK(baseCollection != nullptr);
    DDSTopoBasePtr_t baseGroup = DDSCreateTopoBase(DDSTopoType::GROUP);
    BOOST_CHECK(baseGroup != nullptr);
    DDSTopoBasePtr_t basePort = DDSCreateTopoBase(DDSTopoType::PORT);
    BOOST_CHECK(basePort != nullptr);

    // DDSCreateTopoElement
    BOOST_CHECK_THROW(DDSCreateTopoElement(DDSTopoType::TOPO_BASE), runtime_error);
    BOOST_CHECK_THROW(DDSCreateTopoElement(DDSTopoType::TOPO_ELEMENT), runtime_error);
    BOOST_CHECK_THROW(DDSCreateTopoElement(DDSTopoType::TOPO_PROPERTY), runtime_error);
    DDSTopoElementPtr_t elementTask = DDSCreateTopoElement(DDSTopoType::TASK);
    BOOST_CHECK(elementTask != nullptr);
    DDSTopoElementPtr_t elementCollection = DDSCreateTopoElement(DDSTopoType::COLLECTION);
    BOOST_CHECK(elementCollection != nullptr);
    DDSTopoElementPtr_t elementGroup = DDSCreateTopoElement(DDSTopoType::GROUP);
    BOOST_CHECK(elementGroup != nullptr);
    BOOST_CHECK_THROW(DDSCreateTopoElement(DDSTopoType::PORT), runtime_error);

    // DDSCreateTopoProperty
    BOOST_CHECK_THROW(DDSCreateTopoProperty(DDSTopoType::TOPO_BASE), runtime_error);
    BOOST_CHECK_THROW(DDSCreateTopoProperty(DDSTopoType::TOPO_ELEMENT), runtime_error);
    BOOST_CHECK_THROW(DDSCreateTopoProperty(DDSTopoType::TOPO_PROPERTY), runtime_error);
    BOOST_CHECK_THROW(DDSCreateTopoProperty(DDSTopoType::TASK), runtime_error);
    BOOST_CHECK_THROW(DDSCreateTopoProperty(DDSTopoType::COLLECTION), runtime_error);
    BOOST_CHECK_THROW(DDSCreateTopoProperty(DDSTopoType::GROUP), runtime_error);
    DDSTopoBasePtr_t propertyPort = DDSCreateTopoProperty(DDSTopoType::PORT);
    BOOST_CHECK(propertyPort != nullptr);
}

BOOST_AUTO_TEST_CASE(test_dds_topo_utils)
{
    // DDSTopoTypeToTag
    BOOST_CHECK_THROW(DDSTopoTypeToTag(DDSTopoType::TOPO_BASE), runtime_error);
    BOOST_CHECK_THROW(DDSTopoTypeToTag(DDSTopoType::TOPO_ELEMENT), runtime_error);
    BOOST_CHECK_THROW(DDSTopoTypeToTag(DDSTopoType::TOPO_PROPERTY), runtime_error);
    BOOST_CHECK(DDSTopoTypeToTag(DDSTopoType::TASK) == "task");
    BOOST_CHECK(DDSTopoTypeToTag(DDSTopoType::COLLECTION) == "collection");
    BOOST_CHECK(DDSTopoTypeToTag(DDSTopoType::GROUP) == "group");
    BOOST_CHECK(DDSTopoTypeToTag(DDSTopoType::PORT) == "port");

    // DDSCreateTopoElement
    BOOST_CHECK_THROW(DDSTagToTopoType(""), runtime_error);
    BOOST_CHECK_THROW(DDSTagToTopoType("topobase"), runtime_error);
    BOOST_CHECK_THROW(DDSTagToTopoType("topoelement"), runtime_error);
    BOOST_CHECK_THROW(DDSTagToTopoType("topoproperty"), runtime_error);
    BOOST_CHECK(DDSTagToTopoType("task") == DDSTopoType::TASK);
    BOOST_CHECK(DDSTagToTopoType("collection") == DDSTopoType::COLLECTION);
    BOOST_CHECK(DDSTagToTopoType("group") == DDSTopoType::GROUP);
    BOOST_CHECK(DDSTagToTopoType("port") == DDSTopoType::PORT);
}

BOOST_AUTO_TEST_CASE(test_dds_topo_base_find_element)
{
    ptree pt;
    read_xml("topology_test_6.xml", pt);

    const ptree& pt1 = DDSTopoBase::findElement(DDSTopoType::TASK, "task1", pt.get_child("topology"));
    BOOST_CHECK(pt1.get<string>("<xmlattr>.name") == "task1");

    const ptree& pt2 = DDSTopoBase::findElement(DDSTopoType::COLLECTION, "collection1", pt.get_child("topology"));
    BOOST_CHECK(pt2.get<string>("<xmlattr>.name") == "collection1");

    const ptree& pt3 = DDSTopoBase::findElement(DDSTopoType::GROUP, "group1", pt.get_child("topology.main"));
    BOOST_CHECK(pt3.get<string>("<xmlattr>.name") == "group1");

    const ptree& pt4 = DDSTopoBase::findElement(DDSTopoType::PORT, "port1", pt.get_child("topology"));
    BOOST_CHECK(pt4.get<string>("<xmlattr>.name") == "port1");

    // Wrong path to property tree
    BOOST_CHECK_THROW(DDSTopoBase::findElement(DDSTopoType::TASK, "task1", pt), logic_error);
    BOOST_CHECK_THROW(DDSTopoBase::findElement(DDSTopoType::GROUP, "group1", pt.get_child("topology")), logic_error);

    // Wrong element names
    BOOST_CHECK_THROW(DDSTopoBase::findElement(DDSTopoType::TASK, "NO", pt.get_child("topology")), logic_error);
    BOOST_CHECK_THROW(DDSTopoBase::findElement(DDSTopoType::COLLECTION, "NO", pt.get_child("topology")), logic_error);
    BOOST_CHECK_THROW(DDSTopoBase::findElement(DDSTopoType::GROUP, "NO", pt.get_child("topology.main")), logic_error);
    BOOST_CHECK_THROW(DDSTopoBase::findElement(DDSTopoType::PORT, "NO", pt.get_child("topology")), logic_error);

    // Dublicated names
    BOOST_CHECK_THROW(DDSTopoBase::findElement(DDSTopoType::TASK, "task2", pt.get_child("topology")), logic_error);
    BOOST_CHECK_THROW(DDSTopoBase::findElement(DDSTopoType::COLLECTION, "collection2", pt.get_child("topology")), logic_error);
    BOOST_CHECK_THROW(DDSTopoBase::findElement(DDSTopoType::GROUP, "group2", pt.get_child("topology.main")), logic_error);
    BOOST_CHECK_THROW(DDSTopoBase::findElement(DDSTopoType::PORT, "port3", pt.get_child("topology")), logic_error);
}

BOOST_AUTO_TEST_SUITE_END()
