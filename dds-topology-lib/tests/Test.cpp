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
#include "TopoCreator.h"
#include "TopoCreatorCore.h"
#include "TopoElement.h"
#include "TopoGroup.h"
#include "TopoParserXML.h"
#include "TopoProperty.h"
#include "TopoTask.h"
#include "TopoUtils.h"
#include "Topology.h"
#include "UserDefaults.h"
// BOOST
#include <boost/filesystem.hpp>
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

BOOST_AUTO_TEST_CASE(test_dds_topology_constructor)
{
    string topoFile("topology_test_1.xml");
    CTopology topo1(topoFile);

    ifstream topoStream(topoFile);
    CTopology topo2(topoStream);
}

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
        // std::cout << v.first << " " << v.second.m_collection->getPath() << "\n";
    }
    BOOST_CHECK(_output.match_pattern());
}

template <class T>
void check_topology_map_idpath(const T& _map, output_test_stream& _output)
{
    for (const auto& v : _map)
    {
        _output << v.first << " " << v.second << "\n";
        // std::cout << v.first << " " << v.second << "\n";
    }
    BOOST_CHECK(_output.match_pattern());
}

template <class T>
void check_topology_maps(const string& _topoName)
{
    CTopoCore topology;
    T topoInput(_topoName + ".xml");
    topology.init(topoInput);

    output_test_stream output2(_topoName + "_maps_2.txt", true);
    check_topology_map_task(topology.getIdToRuntimeTaskMap(), output2);

    output_test_stream output4(_topoName + "_maps_4.txt", true);
    check_topology_map_idpath(topology.getTaskIdPathToIdMap(), output4);

    output_test_stream output3(_topoName + "_maps_3.txt", true);
    check_topology_map_collection(topology.getIdToRuntimeCollectionMap(), output3);

    output_test_stream output5(_topoName + "_maps_5.txt", true);
    check_topology_map_idpath(topology.getCollectionIdPathToIdMap(), output5);
}

BOOST_AUTO_TEST_CASE(test_dds_topology_maps)
{
    // Test file input
    check_topology_maps<string>("topology_test_1");
    check_topology_maps<string>("topology_test_7");

    // Test stream input
    check_topology_maps<ifstream>("topology_test_1");
    check_topology_maps<ifstream>("topology_test_7");
}

template <class T>
void check_topology_iterator_task(const T& _iterator, const std::string& _topoFile)
{
    output_test_stream output(_topoFile, true);
    for (auto it = _iterator.first; it != _iterator.second; it++)
    {
        output << it->first << " " << it->second.m_task->getPath() << "\n";
        // std::cout << it->first << " " << it->second.m_task->getPath() << "\n";
    }
    BOOST_CHECK(output.match_pattern());
}

template <class T>
void check_topology_iterator_collection(const T& _iterator, const std::string& _topoFile)
{
    output_test_stream output(_topoFile, true);
    for (auto it = _iterator.first; it != _iterator.second; it++)
    {
        output << it->first << " " << it->second.m_collection->getPath() << "\n";
        // std::cout << it->first << " " << it->second.m_collection->getPath() << "\n";
    }
    BOOST_CHECK(output.match_pattern());
}

template <class T>
void test_topology_iterators(T& _input)
{
    CTopoCore topology;
    topology.init(_input);

    // Task iterators
    output_test_stream output1("topology_test_1_iterators_1.txt", true);
    STopoRuntimeTask::FilterIteratorPair_t taskIt1 =
        topology.getRuntimeTaskIterator([](const STopoRuntimeTask::FilterIterator_t::value_type& value) -> bool {
            CTopoTask::Ptr_t task = value.second.m_task;
            return (task->getName() == "task1");
        });
    check_topology_iterator_task(taskIt1, "topology_test_1_iterators_1.txt");
    check_topology_iterator_task(topology.getRuntimeTaskIteratorMatchingPath("main/(.*)task1_([0-9]*)"),
                                 "topology_test_1_iterators_1.txt");
    check_topology_iterator_task(topology.getRuntimeTaskIterator(), "topology_test_1_iterators_2.txt");
    check_topology_iterator_task(topology.getRuntimeTaskIteratorMatchingPath("(.*)"),
                                 "topology_test_1_iterators_2.txt");

    // Task collection iterators
    STopoRuntimeCollection::FilterIteratorPair_t tcIt1 = topology.getRuntimeCollectionIterator(
        [](const STopoRuntimeCollection::FilterIterator_t::value_type& value) -> bool {
            CTopoCollection::Ptr_t tc = value.second.m_collection;
            return (tc->getName() == "collection1");
        });
    check_topology_iterator_collection(tcIt1, "topology_test_1_iterators_3.txt");
    check_topology_iterator_collection(topology.getRuntimeCollectionIteratorMatchingPath("main/.*collection1"),
                                       "topology_test_1_iterators_3.txt");
    check_topology_iterator_collection(topology.getRuntimeCollectionIterator(), "topology_test_1_iterators_4.txt");
    check_topology_iterator_collection(topology.getRuntimeCollectionIteratorMatchingPath(".*"),
                                       "topology_test_1_iterators_4.txt");

    // Task iterators for property
    check_topology_iterator_task(topology.getRuntimeTaskIteratorForPropertyName("property4", 56611620276638591),
                                 "topology_test_1_iterators_5.txt");
}

BOOST_AUTO_TEST_CASE(test_dds_topology_iterators)
{
    // Test file input
    string topoFile("topology_test_1.xml");
    test_topology_iterators(topoFile);

    // Test stream input
    ifstream topoStream(topoFile);
    test_topology_iterators(topoStream);
}

template <class T>
void test_topology_parser_xml(const string& _filename)
{
    T input(_filename);

    CTopoCore topology;
    BOOST_CHECK_THROW(topology.getName(), std::runtime_error);
    topology.init(input);
    BOOST_CHECK(topology.getName() == "myTopology");
    if (std::is_same<T, string>::value)
    {
        BOOST_CHECK(topology.getFilepath() == boost::filesystem::canonical(_filename).string());
    }
    else
    {
        BOOST_CHECK(topology.getFilepath() == "");
    }
    BOOST_CHECK(topology.getHash() == 3727270847);
    CTopoGroup::Ptr_t main = topology.getMainGroup();

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

    auto b1(topology.getRequiredNofAgents(1) == pair<size_t, size_t>(55, 4));
    BOOST_CHECK_MESSAGE(b1, "Compare getRequiredNofAgents(1)");
    auto b2(topology.getRequiredNofAgents(10) == pair<size_t, size_t>(22, 10));
    BOOST_CHECK_MESSAGE(b2, "Compare getRequiredNofAgents(10)");
    auto b3(topology.getRequiredNofAgents(15) == pair<size_t, size_t>(15, 15));
    BOOST_CHECK_MESSAGE(b3, "Compare getRequiredNofAgents(15)");
}

BOOST_AUTO_TEST_CASE(test_dds_topology_parser_xml)
{
    string topoFile("topology_test_1.xml");
    test_topology_parser_xml<string>(topoFile);
    test_topology_parser_xml<ifstream>(topoFile);
}

template <class T>
void test_topology_validation(const string& _filename, bool _expectedResult)
{
    T input(_filename);
    bool result{ CTopoParserXML::isValid(input, CUserDefaults::getTopologyXSDFilePath()) };
    BOOST_CHECK(result == _expectedResult);
}

BOOST_AUTO_TEST_CASE(test_dds_topology_validation)
{
    vector<string> filenames{ "topology_test_1.xml", "topology_test_2.xml", "topology_test_3.xml",
                              "topology_test_4.xml", "topology_test_5.xml", "topology_test_6.xml",
                              "wrong_file.xml" };
    vector<bool> results{ false, false, false, false, false, true, false };
    for (size_t i = 0; i < filenames.size(); i++)
    {
        string filename(filenames[i]);
        bool result(results[i]);
        test_topology_validation<string>(filename, result);
        test_topology_validation<ifstream>(filename, result);
    }
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

    // PropertyAccessTypeToTag
    BOOST_CHECK(PropertyAccessTypeToTag(CTopoProperty::EAccessType::READ) == "read");
    BOOST_CHECK(PropertyAccessTypeToTag(CTopoProperty::EAccessType::WRITE) == "write");
    BOOST_CHECK(PropertyAccessTypeToTag(CTopoProperty::EAccessType::READWRITE) == "readwrite");

    // TagToPropertyScopeType
    BOOST_CHECK(TagToPropertyScopeType("global") == CTopoProperty::EScopeType::GLOBAL);
    BOOST_CHECK(TagToPropertyScopeType("collection") == CTopoProperty::EScopeType::COLLECTION);
    BOOST_CHECK_THROW(TagToPropertyScopeType("globalglobal"), runtime_error);

    // PropertyScopeTypeToTag
    BOOST_CHECK(PropertyScopeTypeToTag(CTopoProperty::EScopeType::GLOBAL) == "global");
    BOOST_CHECK(PropertyScopeTypeToTag(CTopoProperty::EScopeType::COLLECTION) == "collection");

    // TagToRequirementType
    BOOST_CHECK(TagToRequirementType("wnname") == CTopoRequirement::EType::WnName);
    BOOST_CHECK(TagToRequirementType("hostname") == CTopoRequirement::EType::HostName);
    BOOST_CHECK(TagToRequirementType("gpu") == CTopoRequirement::EType::Gpu);
    BOOST_CHECK(TagToRequirementType("maxinstances") == CTopoRequirement::EType::MaxInstancesPerHost);
    BOOST_CHECK_THROW(TagToRequirementType("wn_name"), runtime_error);

    // RequirementTypeToTag
    BOOST_CHECK(RequirementTypeToTag(CTopoRequirement::EType::WnName) == "wnname");
    BOOST_CHECK(RequirementTypeToTag(CTopoRequirement::EType::HostName) == "hostname");
    BOOST_CHECK(RequirementTypeToTag(CTopoRequirement::EType::Gpu) == "gpu");
    BOOST_CHECK(RequirementTypeToTag(CTopoRequirement::EType::MaxInstancesPerHost) == "maxinstances");

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

template <class T>
void test_topo_difference(const string& _filename, const string& _newFilename)
{
    T input(_filename);
    CTopoCore topo;
    topo.init(input);

    T newInput(_newFilename);
    CTopoCore newTopo;
    newTopo.init(newInput);

    CTopoCore::IdSet_t removedTasks;
    CTopoCore::IdSet_t removedCollections;
    CTopoCore::IdSet_t addedTasks;
    CTopoCore::IdSet_t addedCollections;

    topo.getDifference(newTopo, removedTasks, removedCollections, addedTasks, addedCollections);

    output_test_stream output1("topology_test_diff.txt", true);

    output1 << "----- Removed tasks -----\n";
    // cout << "----- Removed tasks -----\n";
    for (auto& v : removedTasks)
    {
        const STopoRuntimeTask& info = topo.getRuntimeTaskById(v);
        output1 << v << " " << info.m_task->getPath() << " " << info.m_taskPath << "\n";
        // cout << v << " " << info.m_task->getPath() << " " << info.m_taskPath << "\n";
    }

    output1 << "----- Removed collections -----\n";
    // cout << "----- Removed collections -----\n";
    for (auto& v : removedCollections)
    {
        STopoRuntimeCollection collectionInfo = topo.getRuntimeCollectionById(v);
        output1 << v << " " << collectionInfo.m_collection->getPath() << "\n";
        // cout << v << " " << collectionInfo.m_collection->getPath() << "\n";
    }

    output1 << "----- Added tasks -----\n";
    // cout << "----- Added tasks -----\n";
    for (auto& v : addedTasks)
    {
        const STopoRuntimeTask& info = newTopo.getRuntimeTaskById(v);
        output1 << v << " " << info.m_task->getPath() << " " << info.m_taskPath << "\n";
        // cout << v << " " << info.m_task->getPath() << " " << info.m_taskPath << "\n";
    }

    output1 << "----- Added collections -----\n";
    // cout << "----- Added collections -----\n";
    for (auto& v : addedCollections)
    {
        STopoRuntimeCollection collection = newTopo.getRuntimeCollectionById(v);
        output1 << v << " " << collection.m_collection->getPath() << "\n";
        // cout << v << " " << collection.m_collection->getPath() << "\n";
    }

    BOOST_CHECK(output1.match_pattern());
}

BOOST_AUTO_TEST_CASE(test_dds_topo_difference)
{
    string topoFile("topology_test_diff_1.xml");
    string newTopoFile("topology_test_diff_2.xml");
    test_topo_difference<string>(topoFile, newTopoFile);
    test_topo_difference<ifstream>(topoFile, newTopoFile);
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

template <class T>
void test_topology_property_performance(const string& _filename1, const string& _filename2)
{
    T input1(_filename1);
    CTopoCore topology1;
    topology1.init(input1);
    long long time1 = test_property(topology1);
    std::cout << "test_dds_topology_property_performance execution time1: " << time1 << " msec\n";

    T input2(_filename2);
    CTopoCore topology2;
    topology2.init(input2);
    long long time2 = test_property(topology2);
    std::cout << "test_dds_topology_property_performance execution time2: " << time2 << " msec\n";

    std::cout << "test_dds_topology_property_performance delta: " << time2 - time1 << " msec\n";
}

BOOST_AUTO_TEST_CASE(test_dds_topology_property_performance)
{
    string topoFile1("topology_test_property_1.xml");
    string topoFile2("topology_test_property_2.xml");
    test_topology_property_performance<string>(topoFile1, topoFile2);
    test_topology_property_performance<ifstream>(topoFile1, topoFile2);
}

BOOST_AUTO_TEST_CASE(test_dds_save_topo)
{
    std::string topoFile1("topology_test_creator_1.xml");
    CTopoCreatorCore topo1(topoFile1, CUserDefaults::getTopologyXSDFilePath());
    output_test_stream output1(topoFile1, true);
    topo1.save(output1);
    BOOST_CHECK(output1.match_pattern());
}

template <class T>
void test_create_topo_from_xml(T& _input)
{
    {
        // TopoProperty
        vector<string> names{ "property1", "property2", "property3", "property4" };
        for (const auto& name : names)
        {
            auto property = CTopoBase::make<CTopoProperty>(name, _input);
            BOOST_CHECK(property->getType() == CTopoBase::EType::TOPO_PROPERTY);
            BOOST_CHECK(property->getName() == name);
            BOOST_CHECK(property->getScopeType() == CTopoProperty::EScopeType::GLOBAL);
            BOOST_CHECK(property->getParent() == nullptr);
        }
        BOOST_CHECK_THROW(CTopoBase::make<CTopoProperty>("property5", _input), logic_error);
    }

    {
        // TopoRequirement
        vector<string> names{ "requirement1", "requirement2", "requirement3" };
        vector<string> values{ ".+.gsi.de", "server1.gsi.de", "node2" };
        vector<CTopoRequirement::EType> types{ CTopoRequirement::EType::HostName,
                                               CTopoRequirement::EType::HostName,
                                               CTopoRequirement::EType::WnName };
        for (size_t i = 0; i < 3; i++)
        {
            auto requirement = CTopoBase::make<CTopoRequirement>(names[i], _input);
            BOOST_CHECK(requirement->getType() == CTopoBase::EType::REQUIREMENT);
            BOOST_CHECK(requirement->getName() == names[i]);
            BOOST_CHECK(requirement->getValue() == values[i]);
            BOOST_CHECK(requirement->getRequirementType() == types[i]);
            BOOST_CHECK(requirement->getParent() == nullptr);
        }
        BOOST_CHECK_THROW(CTopoBase::make<CTopoRequirement>("requirement4", _input), logic_error);
    }

    {
        // TopoTrigger
        vector<string> names{ "trigger1", "trigger2" };
        vector<CTopoTrigger::EActionType> actions{ CTopoTrigger::EActionType::RestartTask,
                                                   CTopoTrigger::EActionType::RestartTask };
        vector<CTopoTrigger::EConditionType> conditions{ CTopoTrigger::EConditionType::TaskCrashed,
                                                         CTopoTrigger::EConditionType::TaskCrashed };
        vector<string> args{ "5", "10" };
        for (size_t i = 0; i < 2; i++)
        {
            auto trigger = CTopoBase::make<CTopoTrigger>(names[i], _input);
            BOOST_CHECK(trigger->getType() == CTopoBase::EType::TRIGGER);
            BOOST_CHECK(trigger->getName() == names[i]);
            BOOST_CHECK(trigger->getAction() == actions[i]);
            BOOST_CHECK(trigger->getCondition() == conditions[i]);
            BOOST_CHECK(trigger->getArgument() == args[i]);
            BOOST_CHECK(trigger->getParent() == nullptr);
        }
        BOOST_CHECK_THROW(CTopoBase::make<CTopoTrigger>("trigger3", _input), logic_error);
    }

    {
        // TopoTasks
        vector<string> names{ "task1", "task2", "task3", "task4", "task5" };
        vector<string> execs{ "app1 -l -n", "app2", "app3", "app4", "app5" };
        vector<bool> execsReachable{ true, true, true, true, false };
        vector<string> envs{ "env1", "env2", "env3", "env4", "env5" };
        vector<bool> envsReachable{ false, true, true, true, true };
        vector<size_t> numProperties{ 2, 2, 2, 2, 1 };
        vector<size_t> numRequirements{ 2, 2, 0, 0, 0 };
        vector<size_t> numTriggers{ 3, 0, 1, 0, 0 };
        vector<string> propNames{ "property1", "property2", "property3", "property4", "property1" };
        for (size_t i = 0; i < 5; i++)
        {
            auto task = CTopoBase::make<CTopoTask>(names[i], _input);
            BOOST_CHECK(task->getType() == CTopoBase::EType::TASK);
            BOOST_CHECK(task->getName() == names[i]);
            BOOST_CHECK(task->getExe() == execs[i]);
            BOOST_CHECK(task->getEnv() == envs[i]);
            BOOST_CHECK(task->isExeReachable() == execsReachable[i]);
            BOOST_CHECK(task->isEnvReachable() == envsReachable[i]);
            BOOST_CHECK(task->getNofProperties() == numProperties[i]);
            BOOST_CHECK(task->getNofRequirements() == numRequirements[i]);
            BOOST_CHECK(task->getNofTriggers() == numTriggers[i]);
            BOOST_CHECK(task->getProperty(propNames[i])->getParent() == task.get());
            for (const auto& r : task->getRequirements())
            {
                BOOST_CHECK(r->getParent() == task.get());
            }
            for (const auto& t : task->getTriggers())
            {
                BOOST_CHECK(t->getParent() == task.get());
            }
            BOOST_CHECK(task->getParent() == nullptr);
        }
        BOOST_CHECK_THROW(CTopoBase::make<CTopoTask>("task6", _input), logic_error);
    }

    {
        // TopoCollections
        vector<string> names{ "collection1", "collection2" };
        vector<size_t> numRequirements{ 1, 0 };
        vector<size_t> numTasks{ 4, 3 };
        for (size_t i = 0; i < 2; i++)
        {
            auto collection = CTopoBase::make<CTopoCollection>(names[i], _input);
            BOOST_CHECK(collection->getType() == CTopoBase::EType::COLLECTION);
            BOOST_CHECK(collection->getName() == names[i]);
            BOOST_CHECK(collection->getNofRequirements() == numRequirements[i]);
            BOOST_CHECK(collection->getNofElements() == numTasks[i]);
            for (const auto& r : collection->getRequirements())
            {
                BOOST_CHECK(r->getParent() == collection.get());
            }
            for (const auto& e : collection->getElements())
            {
                BOOST_CHECK(e->getParent() == collection.get());
            }
            BOOST_CHECK(collection->getParent() == nullptr);
        }
        BOOST_CHECK_THROW(CTopoBase::make<CTopoCollection>("collection3", _input), runtime_error);
    }

    {
        // TopoGroup
        vector<string> names{ "group1", "group2", "main" };
        vector<size_t> ns{ 10, 15, 1 };
        vector<size_t> numElements{ 3, 4, 4 };
        vector<size_t> numTasks{ 8, 9, 22 };
        vector<size_t> totalNumTasks{ 80, 135, 220 };
        for (size_t i = 0; i < 3; i++)
        {
            auto group = CTopoBase::make<CTopoGroup>(names[i], _input);
            BOOST_CHECK(group->getType() == CTopoBase::EType::GROUP);
            BOOST_CHECK(group->getName() == names[i]);
            BOOST_CHECK(group->getNofElements() == numElements[i]);
            BOOST_CHECK(group->getNofTasks() == numTasks[i]);
            BOOST_CHECK(group->getTotalNofTasks() == totalNumTasks[i]);
            for (const auto& e : group->getElements())
            {
                BOOST_CHECK(e->getParent() == group.get());
            }
            BOOST_CHECK(group->getParent() == nullptr);
        }
        BOOST_CHECK_THROW(CTopoBase::make<CTopoGroup>("group3", _input), runtime_error);
    }
}

BOOST_AUTO_TEST_CASE(test_dds_create_topo_from_xml)
{
    // Test file input
    std::string topoFile("topology_test_creator_1.xml");
    test_create_topo_from_xml(topoFile);

    // Test stream input
    std::ifstream topoStream(topoFile);
    test_create_topo_from_xml(topoStream);
}

template <class T>
void test_create_topo_1(T& _input1)
{
    // Initialize topology creator with existing topology
    CTopoCreator creator(_input1);

    // Add collection directly to the main group
    CTopoCollection::Ptr_t collection{ creator.getMainGroup()->addElement<CTopoCollection>("collection10") };

    // Add 4 tasks to the collection.
    // We have to create CTopoTask 4 times for it.
    for (size_t i = 0; i < 4; i++)
    {
        // Add task to the collection
        auto task{ collection->addElement<CTopoTask>("task10") };
        task->setExe("task.exe --params");
        task->setExeReachable(false);

        // Add requirement
        auto requirement{ task->addRequirement("requirement10") };
        requirement->setRequirementType(CTopoRequirement::EType::HostName);
        requirement->setValue("host.gsi.de");
    }

    // Save the topology
    creator.save("new_topology_test_creator_1.xml");

    BOOST_CHECK(creator.getMainGroup()->getNofElements() == 5);
    BOOST_CHECK(creator.getMainGroup()->getNofTasks() == 26);
    BOOST_CHECK(creator.getMainGroup()->getTotalNofTasks() == 224);
}

BOOST_AUTO_TEST_CASE(test_dds_create_topo_1)
{
    // Test file input
    std::string topoFile("topology_test_creator_1.xml");
    test_create_topo_1(topoFile);

    // Test stream input
    std::ifstream topoStream(topoFile);
    test_create_topo_1(topoStream);
}

template <class T>
void test_create_topo_2(T& _input1, T& _input2)
{
    // Initialize empty topology
    CTopoCreator creator;

    // Create new group which containes both task and collection
    auto group1{ creator.getMainGroup()->addElement<CTopoGroup>("group1") };
    group1->setN(10);
    group1->addElement<CTopoTask>("task1")->initFromXML(_input1);
    group1->addElement<CTopoCollection>("collection1")->initFromXML(_input1);

    // Add to the main group
    creator.getMainGroup()->addElement<CTopoTask>("task10")->initFromXML(_input2);
    creator.getMainGroup()->addElement<CTopoCollection>("collection10")->initFromXML(_input2);

    // Save the topology to a stream
    output_test_stream output("topology_test_creator_3.xml", true);
    creator.save(output);
    BOOST_CHECK(output.match_pattern());

    // Save the topology to a file
    creator.save("new_topology_test_creator_3.xml");

    BOOST_CHECK(creator.getMainGroup()->getNofElements() == 3);
    BOOST_CHECK(creator.getMainGroup()->getNofTasks() == 18);
    BOOST_CHECK(creator.getMainGroup()->getTotalNofTasks() == 63);
}

BOOST_AUTO_TEST_CASE(test_dds_create_topo_2)
{
    // Test file input
    std::string topoFile1("topology_test_creator_1.xml");
    std::string topoFile2("topology_test_creator_2.xml");
    test_create_topo_2(topoFile1, topoFile2);

    // Test stream input
    std::ifstream topoStream1(topoFile1);
    std::ifstream topoStream2(topoFile2);
    test_create_topo_2(topoStream1, topoStream2);
}

BOOST_AUTO_TEST_SUITE_END()
