// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// BOOST: tests
// Defines test_main function to link with actual unit test code.
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/output_test_stream.hpp>
#include <boost/test/unit_test.hpp>

// DDS
#include "TopoBase.h"
#include "TopoCollection.h"
#include "TopoCore.h"
#include "TopoElement.h"
#include "TopoFactory.h"
#include "TopoGroup.h"
#include "TopoParserXML.h"
#include "TopoProperty.h"
#include "TopoTask.h"
#include "TopoUtils.h"
#include "UserDefaults.h"
// BOOST
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
// MiscCommon
#include "TimeMeasure.h"

using namespace std;
using namespace boost::property_tree;
using boost::test_tools::output_test_stream;
using namespace dds;
using namespace dds::topology_api;
using namespace dds::user_defaults_api;
using namespace MiscCommon;

BOOST_AUTO_TEST_SUITE(test_dds_topology)

template <class T>
void check_topology_map_task(const T& _map, output_test_stream& _output)
{
    for (const auto& v : _map)
    {
        _output << v.first << " " << v.second.m_task->getPath() << "\n";
        // std::cout << v.first << " " << v.second.m_task->getPath() << "\n";
    }
    BOOST_CHECK(_output.match_pattern());
}

template <class T>
void check_topology_map_collection(const T& _map, output_test_stream& _output)
{
    for (const auto& v : _map)
    {
        _output << v.first << " " << v.second.m_collection->getPath() << "\n";
        // std::cout << v.first << " " << v.second->getPath() << "\n";
    }
    BOOST_CHECK(_output.match_pattern());
}

template <class T>
void check_topology_map(const T& _map, output_test_stream& _output)
{
    for (const auto& v : _map)
    {
        _output << v.first << " " << v.second->getPath() << "\n";
        // std::cout << v.first << " " << v.second->getPath() << "\n";
    }
    BOOST_CHECK(_output.match_pattern());
}

void check_topology_maps(const string& _topoName)
{
    CTopoCore topology;
    string topoFile(_topoName + ".xml");
    topology.init(topoFile);

    output_test_stream output2(_topoName + "_maps_2.txt", true);
    check_topology_map_task(topology.getIdToRuntimeTaskMap(), output2);

    output_test_stream output3(_topoName + "_maps_3.txt", true);
    check_topology_map_collection(topology.getIdToRuntimeCollectionMap(), output3);
}

BOOST_AUTO_TEST_CASE(test_dds_topology_maps)
{
    check_topology_maps("topology_test_1");
    check_topology_maps("topology_test_7");
}

template <class T>
void check_topology_iterator_task(const T& _iterator, output_test_stream& _output)
{
    for (auto it = _iterator.first; it != _iterator.second; it++)
    {
        _output << it->first << " " << it->second.m_task->getPath() << "\n";
        // std::cout << it->first << " " << it->second.m_task->getPath() << "\n";
    }
    BOOST_CHECK(_output.match_pattern());
}

template <class T>
void check_topology_iterator_collection(const T& _iterator, output_test_stream& _output)
{
    for (auto it = _iterator.first; it != _iterator.second; it++)
    {
        _output << it->first << " " << it->second.m_collection->getPath() << "\n";
        // std::cout << it->first << " " << it->second->getPath() << "\n";
    }
    BOOST_CHECK(_output.match_pattern());
}

BOOST_AUTO_TEST_CASE(test_dds_topology_iterators)
{
    CTopoCore topology;
    topology.init("topology_test_1.xml");

    // Task iterators
    output_test_stream output1("topology_test_1_iterators_1.txt", true);
    STopoRuntimeTask::FilterIteratorPair_t taskIt1 =
        topology.getRuntimeTaskIterator([](const STopoRuntimeTask::FilterIterator_t::value_type& value) -> bool {
            CTopoTask::Ptr_t task = value.second.m_task;
            return (task->getName() == "task1");
        });
    check_topology_iterator_task(taskIt1, output1);

    output_test_stream output2("topology_test_1_iterators_2.txt", true);
    check_topology_iterator_task(topology.getRuntimeTaskIterator(), output2);

    // Task collection iterators
    output_test_stream output3("topology_test_1_iterators_3.txt", true);
    STopoRuntimeCollection::FilterIteratorPair_t tcIt1 =
        topology.getRuntimeCollectionIterator([](STopoRuntimeCollection::FilterIterator_t::value_type value) -> bool {
            CTopoCollection::Ptr_t tc = value.second.m_collection;
            return (tc->getName() == "collection1");
        });
    check_topology_iterator_collection(tcIt1, output3);

    output_test_stream output4("topology_test_1_iterators_4.txt", true);
    check_topology_iterator_collection(topology.getRuntimeCollectionIterator(), output4);

    // Task iterators for property
    output_test_stream output5("topology_test_1_iterators_5.txt", true);
    check_topology_iterator_task(topology.getRuntimeTaskIteratorForPropertyName("property4", 2918576458378016727),
                                 output5);
}

BOOST_AUTO_TEST_CASE(test_dds_topology_iterators_for_property)
{
    CTopoCore topology;
    topology.init("topology_test_1.xml");
}

BOOST_AUTO_TEST_CASE(test_dds_topology_parser_xml_1)
{
    CTopoCore topology;
    topology.init("topology_test_1.xml");
    // std::cout << topology.toString();
    CTopoGroup::Ptr_t main = topology.getMainGroup();

    //  CTopologyParserXML parser;
    //  TaskGroupPtr_t main = make_shared<CTaskGroup>();
    //  parser.parse("topology_test_1.xml", main);

    BOOST_CHECK(main != nullptr);

    BOOST_CHECK(main->getNofTasks() == 22);
    BOOST_CHECK(main->getTotalNofTasks() == 220);
    BOOST_CHECK(main->getN() == 1);
    BOOST_CHECK(main->getName() == "main");
    BOOST_CHECK(main->getNofElements() == 4);
    BOOST_CHECK(main->getParent() == nullptr);
    BOOST_CHECK(main->getPath() == "main");
    BOOST_CHECK_THROW(main->getElement(4), std::out_of_range);

    CTopoElement::Ptr_t element1 = main->getElement(0);
    BOOST_CHECK(element1->getName() == "task1");
    BOOST_CHECK(element1->getType() == CTopoBase::EType::TASK);
    BOOST_CHECK(element1->getParent() == main.get());
    BOOST_CHECK(element1->getPath() == "main/task1");
    BOOST_CHECK(element1->getNofTasks() == 1);
    BOOST_CHECK(element1->getTotalNofTasks() == 1);
    CTopoTask::Ptr_t casted1 = dynamic_pointer_cast<CTopoTask>(element1);
    BOOST_CHECK(casted1->getNofProperties() == 2);
    //    BOOST_CHECK(casted1->getProperty(0)->getId() == "property1");
    //    BOOST_CHECK(casted1->getProperty(1)->getId() == "property4");
    //    BOOST_CHECK(casted1->getProperty(2)->getId() == "property1");
    //    BOOST_CHECK(casted1->getProperty(0)->getAccessType() == EPropertyAccessType::READ);
    //    BOOST_CHECK(casted1->getProperty(1)->getAccessType() == EPropertyAccessType::WRITE);
    //    BOOST_CHECK(casted1->getProperty(2)->getAccessType() == EPropertyAccessType::READWRITE);
    BOOST_CHECK(casted1->getExe() == "app1 -l -n");
    BOOST_CHECK(casted1->getEnv() == "env1");
    BOOST_CHECK(casted1->isExeReachable() == true);
    BOOST_CHECK(casted1->isEnvReachable() == false);
    BOOST_CHECK(casted1->getNofRequirements() == 1);
    CTopoRequirement::Ptr_t requirement = casted1->getRequirements()[0];
    BOOST_CHECK(requirement->getName() == "requirement1");
    BOOST_CHECK(requirement->getType() == CTopoBase::EType::REQUIREMENT);
    BOOST_CHECK(requirement->getParent() == element1.get());
    BOOST_CHECK(requirement->getValue() == ".+.gsi.de");
    BOOST_CHECK(requirement->getRequirementType() == CTopoRequirement::EType::HostName);
    BOOST_CHECK(casted1->getNofTriggers() == 2);
    CTopoTrigger::Ptr_t trigger = casted1->getTriggers()[0];
    BOOST_CHECK(trigger->getName() == "trigger1");
    BOOST_CHECK(trigger->getType() == CTopoBase::EType::TRIGGER);
    BOOST_CHECK(trigger->getParent() == element1.get());
    BOOST_CHECK(trigger->getCondition() == CTopoTrigger::EConditionType::TaskCrashed);
    BOOST_CHECK(trigger->getAction() == CTopoTrigger::EActionType::RestartTask);
    BOOST_CHECK(trigger->getArgument() == "5");
    CTopoElement::Ptr_t element2 = main->getElement(1);
    BOOST_CHECK(element2->getName() == "collection1");
    BOOST_CHECK(element2->getType() == CTopoBase::EType::COLLECTION);
    BOOST_CHECK(element2->getParent() == main.get());
    BOOST_CHECK(element2->getPath() == "main/collection1");
    BOOST_CHECK(element2->getNofTasks() == 4);
    BOOST_CHECK(element2->getTotalNofTasks() == 4);
    CTopoCollection::Ptr_t casted2 = dynamic_pointer_cast<CTopoCollection>(element2);
    BOOST_CHECK(casted2->getNofElements() == 4);
    BOOST_CHECK_THROW(casted2->getElement(4), std::out_of_range);

    const auto& casted2_elements = casted2->getElements();
    for (const auto& v : casted2_elements)
    {
        BOOST_CHECK(v->getParent() == element2.get());
    }

    CTopoElement::Ptr_t element3 = main->getElement(2);
    BOOST_CHECK(element3->getName() == "group1");
    BOOST_CHECK(element3->getType() == CTopoBase::EType::GROUP);
    BOOST_CHECK(element3->getParent() == main.get());
    BOOST_CHECK(element3->getPath() == "main/group1");
    BOOST_CHECK(element3->getNofTasks() == 8);
    BOOST_CHECK(element3->getTotalNofTasks() == 80);
    CTopoGroup::Ptr_t casted3 = dynamic_pointer_cast<CTopoGroup>(element3);
    BOOST_CHECK(casted3->getNofElements() == 3);
    BOOST_CHECK_THROW(casted3->getElement(3), std::out_of_range);
    BOOST_CHECK(casted3->getN() == 10);

    CTopoElement::Ptr_t element4 = main->getElement(3);
    BOOST_CHECK(element4->getName() == "group2");
    BOOST_CHECK(element4->getType() == CTopoBase::EType::GROUP);
    BOOST_CHECK(element4->getParent() == main.get());
    BOOST_CHECK(element4->getPath() == "main/group2");
    BOOST_CHECK(element4->getNofTasks() == 9);
    BOOST_CHECK(element4->getTotalNofTasks() == 135);
    CTopoGroup::Ptr_t casted4 = dynamic_pointer_cast<CTopoGroup>(element4);
    BOOST_CHECK(casted4->getNofElements() == 4);
    BOOST_CHECK_THROW(casted4->getElement(4), std::out_of_range);
    BOOST_CHECK(casted4->getN() == 15);

    CTopoCollection::Ptr_t casted5 = dynamic_pointer_cast<CTopoCollection>(casted4->getElement(2));
    BOOST_CHECK(casted5->getName() == "collection1");
    BOOST_CHECK(casted5->getType() == CTopoBase::EType::COLLECTION);
    BOOST_CHECK(casted5->getParent() == casted4.get());
    BOOST_CHECK(casted5->getPath() == "main/group2/collection1");
    BOOST_CHECK(casted5->getNofTasks() == 4);
    BOOST_CHECK(casted5->getTotalNofTasks() == 4);
    BOOST_CHECK(casted5->getNofElements() == 4);
    BOOST_CHECK(casted5->getRequirements()[0]->getName() == "requirement1");
    BOOST_CHECK_THROW(casted5->getElement(4), std::out_of_range);
    BOOST_CHECK(casted5->getTotalCounter() == 15);

    CTopoTask::Ptr_t casted6 = dynamic_pointer_cast<CTopoTask>(casted5->getElement(0));
    BOOST_CHECK(casted6->getName() == "task1");
    BOOST_CHECK(casted6->getType() == CTopoBase::EType::TASK);
    BOOST_CHECK(casted6->getParent() == casted5.get());
    BOOST_CHECK(casted6->getPath() == "main/group2/collection1/task1");
    BOOST_CHECK(casted6->getNofTasks() == 1);
    BOOST_CHECK(casted6->getTotalNofTasks() == 1);
    BOOST_CHECK(casted6->getNofProperties() == 2);
    BOOST_CHECK(casted6->getExe() == "app1 -l -n");
    BOOST_CHECK(casted6->getEnv() == "env1");
    BOOST_CHECK(casted6->isExeReachable() == true);
    BOOST_CHECK(casted6->isEnvReachable() == false);

    // Test getElementsByType and getTotalCounter
    CTopoElement::PtrVector_t elements1 = main->getElementsByType(CTopoBase::EType::TASK);
    BOOST_CHECK(elements1.size() == 4);
    for (const auto& v : elements1)
    {
        BOOST_CHECK(v->getType() == CTopoBase::EType::TASK);
    }
    CTopoTask::Ptr_t castedTask = dynamic_pointer_cast<CTopoTask>(elements1[0]);
    BOOST_CHECK(castedTask->getName() == "task1");
    BOOST_CHECK(castedTask->getPath() == "main/task1");
    BOOST_CHECK(castedTask->getTotalCounter() == 1);
    castedTask = dynamic_pointer_cast<CTopoTask>(elements1[1]);
    BOOST_CHECK(castedTask->getName() == "task1");
    BOOST_CHECK(castedTask->getPath() == "main/group1/task1");
    BOOST_CHECK(castedTask->getTotalCounter() == 10);
    castedTask = dynamic_pointer_cast<CTopoTask>(elements1[2]);
    BOOST_CHECK(castedTask->getName() == "task3");
    BOOST_CHECK(castedTask->getPath() == "main/group2/task3");
    BOOST_CHECK(castedTask->getTotalCounter() == 15);
    castedTask = dynamic_pointer_cast<CTopoTask>(elements1[3]);
    BOOST_CHECK(castedTask->getName() == "task4");
    BOOST_CHECK(castedTask->getPath() == "main/group2/task4");
    BOOST_CHECK(castedTask->getTotalCounter() == 15);

    CTopoElement::PtrVector_t elements2 = main->getElementsByType(CTopoBase::EType::COLLECTION);
    BOOST_CHECK(elements2.size() == 5);
    for (const auto& v : elements2)
    {
        BOOST_CHECK(v->getType() == CTopoBase::EType::COLLECTION);
    }
    CTopoCollection::Ptr_t castedCollection = dynamic_pointer_cast<CTopoCollection>(elements2[0]);
    BOOST_CHECK(castedCollection->getName() == "collection1");
    BOOST_CHECK(castedCollection->getPath() == "main/collection1");
    BOOST_CHECK(castedCollection->getTotalCounter() == 1);
    castedCollection = dynamic_pointer_cast<CTopoCollection>(elements2[1]);
    BOOST_CHECK(castedCollection->getName() == "collection1");
    BOOST_CHECK(castedCollection->getPath() == "main/group1/collection1");
    BOOST_CHECK(castedCollection->getTotalCounter() == 10);
    castedCollection = dynamic_pointer_cast<CTopoCollection>(elements2[2]);
    BOOST_CHECK(castedCollection->getName() == "collection2");
    BOOST_CHECK(castedCollection->getPath() == "main/group1/collection2");
    BOOST_CHECK(castedCollection->getTotalCounter() == 10);
    castedCollection = dynamic_pointer_cast<CTopoCollection>(elements2[3]);
    BOOST_CHECK(castedCollection->getName() == "collection1");
    BOOST_CHECK(castedCollection->getPath() == "main/group2/collection1");
    BOOST_CHECK(castedCollection->getTotalCounter() == 15);
    castedCollection = dynamic_pointer_cast<CTopoCollection>(elements2[4]);
    BOOST_CHECK(castedCollection->getName() == "collection2");
    BOOST_CHECK(castedCollection->getPath() == "main/group2/collection2");
    BOOST_CHECK(castedCollection->getTotalCounter() == 15);
}

BOOST_AUTO_TEST_CASE(test_dds_topology_parser_xml_validation_1)
{
    CTopoParserXML parser;
    bool result = parser.isValid("topology_test_1.xml", CUserDefaults::getTopologyXSDFilePath());
    BOOST_CHECK(result == false);
}

BOOST_AUTO_TEST_CASE(test_dds_topology_parser_xml_validation_2)
{
    CTopoParserXML parser;
    bool result = parser.isValid("topology_test_2.xml", CUserDefaults::getTopologyXSDFilePath());
    BOOST_CHECK(result == false);
}

BOOST_AUTO_TEST_CASE(test_dds_topology_parser_xml_validation_3)
{
    CTopoParserXML parser;
    bool result = parser.isValid("topology_test_3.xml", CUserDefaults::getTopologyXSDFilePath());
    BOOST_CHECK(result == false);
}

BOOST_AUTO_TEST_CASE(test_dds_topology_parser_xml_validation_4)
{
    CTopoParserXML parser;
    bool result = parser.isValid("topology_test_4.xml", CUserDefaults::getTopologyXSDFilePath());
    BOOST_CHECK(result == false);
}

BOOST_AUTO_TEST_CASE(test_dds_topology_parser_xml_validation_5)
{
    CTopoParserXML parser;
    bool result = parser.isValid("topology_test_5.xml", CUserDefaults::getTopologyXSDFilePath());
    BOOST_CHECK(result == false);
}

BOOST_AUTO_TEST_CASE(test_dds_topology_parser_xml_validation_6)
{
    CTopoParserXML parser;
    bool result = parser.isValid("topology_test_6.xml", CUserDefaults::getTopologyXSDFilePath());
    BOOST_CHECK(result == true);
}

BOOST_AUTO_TEST_CASE(test_dds_topology_parser_xml_validation_wrong)
{
    CTopoParserXML parser;
    bool result = parser.isValid("wrong_file.xml", CUserDefaults::getTopologyXSDFilePath());
    BOOST_CHECK(result == false);
}

BOOST_AUTO_TEST_CASE(test_dds_topo_factory)
{
    // DDSCreateTopoBase
    BOOST_CHECK_THROW(CreateTopoBase(CTopoBase::EType::TOPO_BASE), runtime_error);
    BOOST_CHECK_THROW(CreateTopoBase(CTopoBase::EType::TOPO_ELEMENT), runtime_error);
    CTopoBase::Ptr_t baseTopoProperty = CreateTopoBase(CTopoBase::EType::TOPO_PROPERTY);
    BOOST_CHECK(baseTopoProperty != nullptr);
    CTopoBase::Ptr_t baseTask = CreateTopoBase(CTopoBase::EType::TASK);
    BOOST_CHECK(baseTask != nullptr);
    CTopoBase::Ptr_t baseCollection = CreateTopoBase(CTopoBase::EType::COLLECTION);
    BOOST_CHECK(baseCollection != nullptr);
    CTopoBase::Ptr_t baseGroup = CreateTopoBase(CTopoBase::EType::GROUP);
    BOOST_CHECK(baseGroup != nullptr);

    // DDSCreateTopoElement
    BOOST_CHECK_THROW(CreateTopoElement(CTopoBase::EType::TOPO_BASE), runtime_error);
    BOOST_CHECK_THROW(CreateTopoElement(CTopoBase::EType::TOPO_ELEMENT), runtime_error);
    BOOST_CHECK_THROW(CreateTopoElement(CTopoBase::EType::TOPO_PROPERTY), runtime_error);
    CTopoElement::Ptr_t elementTask = CreateTopoElement(CTopoBase::EType::TASK);
    BOOST_CHECK(elementTask != nullptr);
    CTopoElement::Ptr_t elementCollection = CreateTopoElement(CTopoBase::EType::COLLECTION);
    BOOST_CHECK(elementCollection != nullptr);
    CTopoElement::Ptr_t elementGroup = CreateTopoElement(CTopoBase::EType::GROUP);
    BOOST_CHECK(elementGroup != nullptr);
}

BOOST_AUTO_TEST_CASE(test_dds_topo_utils)
{
    // TopoTypeToUseTag
    BOOST_CHECK_THROW(TopoTypeToUseTag(CTopoBase::EType::TOPO_BASE), runtime_error);
    BOOST_CHECK_THROW(TopoTypeToUseTag(CTopoBase::EType::TOPO_ELEMENT), runtime_error);
    BOOST_CHECK(TopoTypeToUseTag(CTopoBase::EType::TASK) == "task");
    BOOST_CHECK(TopoTypeToUseTag(CTopoBase::EType::COLLECTION) == "collection");
    BOOST_CHECK(TopoTypeToUseTag(CTopoBase::EType::GROUP) == "group");
    BOOST_CHECK(TopoTypeToUseTag(CTopoBase::EType::TOPO_PROPERTY) == "property");
    BOOST_CHECK(TopoTypeToUseTag(CTopoBase::EType::REQUIREMENT) == "requirement");
    BOOST_CHECK(TopoTypeToUseTag(CTopoBase::EType::TRIGGER) == "trigger");

    // UseTagToTopoType
    BOOST_CHECK_THROW(UseTagToTopoType(""), runtime_error);
    BOOST_CHECK_THROW(UseTagToTopoType("topobase"), runtime_error);
    BOOST_CHECK_THROW(UseTagToTopoType("topoelement"), runtime_error);
    BOOST_CHECK(UseTagToTopoType("task") == CTopoBase::EType::TASK);
    BOOST_CHECK(UseTagToTopoType("collection") == CTopoBase::EType::COLLECTION);
    BOOST_CHECK(UseTagToTopoType("group") == CTopoBase::EType::GROUP);
    BOOST_CHECK(UseTagToTopoType("property") == CTopoBase::EType::TOPO_PROPERTY);
    BOOST_CHECK(UseTagToTopoType("requirement") == CTopoBase::EType::REQUIREMENT);
    BOOST_CHECK(UseTagToTopoType("trigger") == CTopoBase::EType::TRIGGER);

    // TopoTypeToDeclTag
    BOOST_CHECK_THROW(TopoTypeToDeclTag(CTopoBase::EType::TOPO_BASE), runtime_error);
    BOOST_CHECK_THROW(TopoTypeToDeclTag(CTopoBase::EType::TOPO_ELEMENT), runtime_error);
    BOOST_CHECK(TopoTypeToDeclTag(CTopoBase::EType::TASK) == "decltask");
    BOOST_CHECK(TopoTypeToDeclTag(CTopoBase::EType::COLLECTION) == "declcollection");
    BOOST_CHECK(TopoTypeToDeclTag(CTopoBase::EType::GROUP) == "group");
    BOOST_CHECK(TopoTypeToDeclTag(CTopoBase::EType::TOPO_PROPERTY) == "property");
    BOOST_CHECK(TopoTypeToDeclTag(CTopoBase::EType::REQUIREMENT) == "declrequirement");
    BOOST_CHECK(TopoTypeToDeclTag(CTopoBase::EType::TOPO_VARS) == "var");
    BOOST_CHECK(TopoTypeToDeclTag(CTopoBase::EType::TRIGGER) == "decltrigger");

    // TagToPropertyAccessType
    BOOST_CHECK(TagToPropertyAccessType("read") == CTopoProperty::EAccessType::READ);
    BOOST_CHECK(TagToPropertyAccessType("write") == CTopoProperty::EAccessType::WRITE);
    BOOST_CHECK(TagToPropertyAccessType("readwrite") == CTopoProperty::EAccessType::READWRITE);
    BOOST_CHECK_THROW(TagToPropertyAccessType("readread"), runtime_error);

    // TagToPropertyScopeType
    BOOST_CHECK(TagToPropertyScopeType("global") == CTopoProperty::EScopeType::GLOBAL);
    BOOST_CHECK(TagToPropertyScopeType("collection") == CTopoProperty::EScopeType::COLLECTION);
    BOOST_CHECK_THROW(TagToPropertyScopeType("globalglobal"), runtime_error);

    // TagToRequirementType
    BOOST_CHECK(TagToRequirementType("wnname") == CTopoRequirement::EType::WnName);
    BOOST_CHECK(TagToRequirementType("hostname") == CTopoRequirement::EType::HostName);
    BOOST_CHECK(TagToRequirementType("gpu") == CTopoRequirement::EType::Gpu);
    BOOST_CHECK_THROW(TagToRequirementType("wn_name"), runtime_error);

    // RequirementTypeToTag
    BOOST_CHECK(RequirementTypeToTag(CTopoRequirement::EType::WnName) == "WnName");
    BOOST_CHECK(RequirementTypeToTag(CTopoRequirement::EType::HostName) == "HostName");
    BOOST_CHECK(RequirementTypeToTag(CTopoRequirement::EType::Gpu) == "Gpu");

    // TagToConditionType
    BOOST_CHECK(TagToConditionType("TaskCrashed") == CTopoTrigger::EConditionType::TaskCrashed);

    // ConditionTypeToTag
    BOOST_CHECK(ConditionTypeToTag(CTopoTrigger::EConditionType::TaskCrashed) == "TaskCrashed");

    // TagToActionType
    BOOST_CHECK(TagToActionType("RestartTask") == CTopoTrigger::EActionType::RestartTask);

    // ActionTypeToTag
    BOOST_CHECK(ActionTypeToTag(CTopoTrigger::EActionType::RestartTask) == "RestartTask");

    // DeclTagToTopoType
    BOOST_CHECK_THROW(DeclTagToTopoType(""), runtime_error);
    BOOST_CHECK_THROW(DeclTagToTopoType("topobase"), runtime_error);
    BOOST_CHECK_THROW(DeclTagToTopoType("topoelement"), runtime_error);
    BOOST_CHECK(DeclTagToTopoType("decltask") == CTopoBase::EType::TASK);
    BOOST_CHECK(DeclTagToTopoType("declcollection") == CTopoBase::EType::COLLECTION);
    BOOST_CHECK(DeclTagToTopoType("group") == CTopoBase::EType::GROUP);
    BOOST_CHECK(DeclTagToTopoType("property") == CTopoBase::EType::TOPO_PROPERTY);
    BOOST_CHECK(DeclTagToTopoType("declrequirement") == CTopoBase::EType::REQUIREMENT);
    BOOST_CHECK(DeclTagToTopoType("var") == CTopoBase::EType::TOPO_VARS);
    BOOST_CHECK(DeclTagToTopoType("decltrigger") == CTopoBase::EType::TRIGGER);
}

BOOST_AUTO_TEST_CASE(test_dds_find_element_in_property_tree)
{
    ptree pt;
    read_xml("topology_test_6.xml", pt);

    const ptree& pt1 = FindElementInPropertyTree(CTopoBase::EType::TASK, "task1", pt.get_child("topology"));
    BOOST_CHECK(pt1.get<string>("<xmlattr>.name") == "task1");

    const ptree& pt2 = FindElementInPropertyTree(CTopoBase::EType::COLLECTION, "collection1", pt.get_child("topology"));
    BOOST_CHECK(pt2.get<string>("<xmlattr>.name") == "collection1");

    const ptree& pt3 = FindElementInPropertyTree(CTopoBase::EType::GROUP, "group1", pt.get_child("topology.main"));
    BOOST_CHECK(pt3.get<string>("<xmlattr>.name") == "group1");

    const ptree& pt4 =
        FindElementInPropertyTree(CTopoBase::EType::TOPO_PROPERTY, "property1", pt.get_child("topology"));
    BOOST_CHECK(pt4.get<string>("<xmlattr>.name") == "property1");

    const ptree& pt5 =
        FindElementInPropertyTree(CTopoBase::EType::REQUIREMENT, "requirement1", pt.get_child("topology"));
    BOOST_CHECK(pt5.get<string>("<xmlattr>.name") == "requirement1");

    // Wrong path to property tree
    BOOST_CHECK_THROW(FindElementInPropertyTree(CTopoBase::EType::TASK, "task1", pt), logic_error);
    BOOST_CHECK_THROW(FindElementInPropertyTree(CTopoBase::EType::GROUP, "group1", pt.get_child("topology")),
                      logic_error);

    // Wrong element names
    BOOST_CHECK_THROW(FindElementInPropertyTree(CTopoBase::EType::TASK, "NO", pt.get_child("topology")), logic_error);
    BOOST_CHECK_THROW(FindElementInPropertyTree(CTopoBase::EType::COLLECTION, "NO", pt.get_child("topology")),
                      logic_error);
    BOOST_CHECK_THROW(FindElementInPropertyTree(CTopoBase::EType::GROUP, "NO", pt.get_child("topology.main")),
                      logic_error);
    BOOST_CHECK_THROW(FindElementInPropertyTree(CTopoBase::EType::TOPO_PROPERTY, "NO", pt.get_child("topology")),
                      logic_error);

    // Dublicated names
    BOOST_CHECK_THROW(FindElementInPropertyTree(CTopoBase::EType::TASK, "task2", pt.get_child("topology")),
                      logic_error);
    BOOST_CHECK_THROW(FindElementInPropertyTree(CTopoBase::EType::COLLECTION, "collection2", pt.get_child("topology")),
                      logic_error);
    BOOST_CHECK_THROW(FindElementInPropertyTree(CTopoBase::EType::GROUP, "group2", pt.get_child("topology.main")),
                      logic_error);
    BOOST_CHECK_THROW(FindElementInPropertyTree(CTopoBase::EType::TOPO_PROPERTY, "property3", pt.get_child("topology")),
                      logic_error);
}

BOOST_AUTO_TEST_CASE(test_dds_topo_difference)
{
    CTopoCore topo;
    topo.init("topology_test_diff_1.xml");

    CTopoCore newTopo;
    newTopo.init("topology_test_diff_2.xml");

    CTopoCore::IdSet_t removedTasks;
    CTopoCore::IdSet_t removedCollections;
    CTopoCore::IdSet_t addedTasks;
    CTopoCore::IdSet_t addedCollections;

    topo.getDifference(newTopo, removedTasks, removedCollections, addedTasks, addedCollections);

    output_test_stream output1("topology_test_diff.txt", true);

    output1 << "----- Removed tasks -----\n";
    for (auto& v : removedTasks)
    {
        const STopoRuntimeTask& info = topo.getRuntimeTaskById(v);
        output1 << v << " " << info.m_task->getPath() << " " << info.m_taskPath << "\n";
    }

    output1 << "----- Removed collections -----\n";
    for (auto& v : removedCollections)
    {
        STopoRuntimeCollection collectionInfo = topo.getRuntimeCollectionById(v);
        output1 << v << " " << collectionInfo.m_collection->getPath() << "\n";
    }

    output1 << "----- Added tasks -----\n";
    for (auto& v : addedTasks)
    {
        const STopoRuntimeTask& info = newTopo.getRuntimeTaskById(v);
        output1 << v << " " << info.m_task->getPath() << " " << info.m_taskPath << "\n";
    }

    output1 << "----- Added collections -----\n";
    for (auto& v : addedCollections)
    {
        STopoRuntimeCollection collection = newTopo.getRuntimeCollectionById(v);
        output1 << v << " " << collection.m_collection->getPath() << "\n";
    }

    BOOST_CHECK(output1.match_pattern());
}

long long test_property(const CTopoCore& _topology)
{
    auto execTime = STimeMeasure<>::execution([&_topology]() {
        for (size_t i = 0; i < 1000; i++)
        {
            const STopoRuntimeTask::Map_t& taskMap = _topology.getIdToRuntimeTaskMap();

            for (const auto& v : taskMap)
            {
                uint64_t taskID = v.first;
                const STopoRuntimeTask& taskInfo = v.second;

                const CTopoProperty::PtrMap_t& properties = taskInfo.m_task->getProperties();
                for (const auto& property : properties)
                {
                    _topology.getRuntimeTaskIteratorForPropertyName(property.first, taskID);
                }
            }
        }
    });
    return execTime;
}

BOOST_AUTO_TEST_CASE(test_dds_topology_property_performance)
{
    CTopoCore topology1;
    topology1.init("topology_test_property_1.xml");
    long long time1 = test_property(topology1);
    std::cout << "test_dds_topology_property_performance execution time1: " << time1 << " msec\n";

    CTopoCore topology2;
    topology2.init("topology_test_property_2.xml");
    long long time2 = test_property(topology2);
    std::cout << "test_dds_topology_property_performance execution time2: " << time2 << " msec\n";

    std::cout << "test_dds_topology_property_performance delta: " << time2 - time1 << " msec\n";
}

BOOST_AUTO_TEST_SUITE_END()
