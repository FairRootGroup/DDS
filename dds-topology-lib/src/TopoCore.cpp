// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "TopoCore.h"
#include "TopoParserXML.h"
#include "TopoUtils.h"
#include "UserDefaults.h"
// STD
#include <string>
// MiscCommon
#include "CRC.h"
// BOOST
#include "boost/range/adaptor/map.hpp"
#include <boost/range/algorithm/copy.hpp>

using namespace std;
using namespace dds;
using namespace topology_api;
using namespace user_defaults_api;
using namespace boost;

CTopoCore::CTopoCore()
    : m_main(nullptr)
    , m_currentCollectionId(0)
    , m_bXMLValidationDisabled(false)
{
}

CTopoCore::~CTopoCore()
{
}

CTopoGroup::Ptr_t CTopoCore::getMainGroup() const
{
    return m_main;
}

void CTopoCore::init(const std::string& _fileName)
{
    init(_fileName, CUserDefaults::getTopologyXSDFilePath());
}

void CTopoCore::init(const std::string& _fileName, const std::string& _schemaFileName)
{
    string filename(_fileName);

    // Take default file for agent $DDS_LOCATION/topology.xml
    if (filename.empty())
    {
        filename = CUserDefaults::instance().getDDSPath() + "topology.xml";
    }

    if (_schemaFileName.empty() && !m_bXMLValidationDisabled)
    {
        throw runtime_error("XSD schema file not provided. Disable validation or provide a valid schema file.");
    }

    // Use empty string to disable validation in parser
    string schemaFileName = (m_bXMLValidationDisabled) ? "" : _schemaFileName;

    CTopoParserXML parser;
    m_main = std::make_shared<CTopoGroup>();
    parser.parse(filename, schemaFileName, m_main);

    m_counterMap.clear();
    m_idToRuntimeTaskMap.clear();
    m_idToRuntimeCollectionMap.clear();
    m_currentCollectionIdPath = "";
    m_currentCollectionId = 0;

    FillIdToTopoElementMap(m_main);
}

void CTopoCore::getDifference(const CTopoCore& _topology,
                              IdSet_t& _removedTasks,
                              IdSet_t& _removedCollections,
                              IdSet_t& _addedTasks,
                              IdSet_t& _addedCollections)
{
    _removedTasks.clear();
    _removedCollections.clear();
    _addedTasks.clear();
    _addedCollections.clear();

    IdSet_t tasks;
    IdSet_t newTasks;
    IdSet_t collections;
    IdSet_t newCollections;

    // Get all keys from maps as a set
    boost::copy(m_idToRuntimeTaskMap | boost::adaptors::map_keys, std::inserter(tasks, tasks.end()));
    boost::copy(_topology.getIdToRuntimeTaskMap() | boost::adaptors::map_keys, std::inserter(newTasks, newTasks.end()));
    boost::copy(m_idToRuntimeCollectionMap | boost::adaptors::map_keys, std::inserter(collections, collections.end()));
    boost::copy(_topology.getIdToRuntimeCollectionMap() | boost::adaptors::map_keys,
                std::inserter(newCollections, newCollections.end()));

    // Get difference between two sets
    std::set_difference(tasks.begin(),
                        tasks.end(),
                        newTasks.begin(),
                        newTasks.end(),
                        std::inserter(_removedTasks, _removedTasks.end()));
    std::set_difference(
        newTasks.begin(), newTasks.end(), tasks.begin(), tasks.end(), std::inserter(_addedTasks, _addedTasks.begin()));
    std::set_difference(collections.begin(),
                        collections.end(),
                        newCollections.begin(),
                        newCollections.end(),
                        std::inserter(_removedCollections, _removedCollections.end()));
    std::set_difference(newCollections.begin(),
                        newCollections.end(),
                        collections.begin(),
                        collections.end(),
                        std::inserter(_addedCollections, _addedCollections.begin()));
}

void CTopoCore::setXMLValidationDisabled(bool _val)
{
    m_bXMLValidationDisabled = _val;
}

size_t CTopoCore::getRequiredNofAgents() const
{
    return getMainGroup()->getTotalNofTasks();
}

const STopoRuntimeTask::Map_t& CTopoCore::getIdToRuntimeTaskMap() const
{
    return m_idToRuntimeTaskMap;
}

const STopoRuntimeCollection::Map_t& CTopoCore::getIdToRuntimeCollectionMap() const
{
    return m_idToRuntimeCollectionMap;
}

const STopoRuntimeTask& CTopoCore::getRuntimeTaskById(Id_t _id) const
{
    auto it = m_idToRuntimeTaskMap.find(_id);
    if (it == m_idToRuntimeTaskMap.end())
        throw runtime_error("Can not find task info with ID " + to_string(_id));
    return it->second;
}

const STopoRuntimeCollection& CTopoCore::getRuntimeCollectionById(Id_t _id) const
{
    auto it = m_idToRuntimeCollectionMap.find(_id);
    if (it == m_idToRuntimeCollectionMap.end())
        throw runtime_error("Can not find task collection with ID " + to_string(_id));
    return it->second;
}

const STopoRuntimeTask& CTopoCore::getRuntimeTaskByIdPath(const std::string& _idPath) const
{
    Id_t id = MiscCommon::crc64(_idPath);
    return getRuntimeTaskById(id);
}

const STopoRuntimeCollection& CTopoCore::getRuntimeCollectionByIdPath(const std::string& _idPath) const
{
    Id_t id = MiscCommon::crc64(_idPath);
    return getRuntimeCollectionById(id);
}

STopoRuntimeTask::FilterIteratorPair_t CTopoCore::getRuntimeTaskIterator(const STopoRuntimeTask::Map_t& _map,
                                                                         STopoRuntimeTask::Condition_t _condition) const
{
    STopoRuntimeTask::Condition_t condition = _condition;
    if (condition == nullptr)
    {
        condition = [](const STopoRuntimeTask::Map_t::value_type&) -> bool { return true; };
    }
    STopoRuntimeTask::FilterIterator_t begin_iterator(condition, _map.begin(), _map.end());
    STopoRuntimeTask::FilterIterator_t end_iterator(condition, _map.end(), _map.end());
    return make_pair(begin_iterator, end_iterator);
}

STopoRuntimeTask::FilterIteratorPair_t CTopoCore::getRuntimeTaskIterator(STopoRuntimeTask::Condition_t _condition) const
{
    return getRuntimeTaskIterator(m_idToRuntimeTaskMap, _condition);
}

STopoRuntimeCollection::FilterIteratorPair_t CTopoCore::getRuntimeCollectionIterator(
    STopoRuntimeCollection::Condition_t _condition) const
{
    STopoRuntimeCollection::Condition_t condition = _condition;
    if (condition == nullptr)
    {
        condition = [](STopoRuntimeCollection::Map_t::value_type) -> bool { return true; };
    }
    STopoRuntimeCollection::FilterIterator_t begin_iterator(
        condition, m_idToRuntimeCollectionMap.begin(), m_idToRuntimeCollectionMap.end());
    STopoRuntimeCollection::FilterIterator_t end_iterator(
        condition, m_idToRuntimeCollectionMap.end(), m_idToRuntimeCollectionMap.end());
    return make_pair(begin_iterator, end_iterator);
}

STopoRuntimeTask::FilterIteratorPair_t CTopoCore::getRuntimeTaskIteratorForPropertyName(
    const std::string& _propertyName, Id_t _taskId) const
{
    auto taskIt = m_idToRuntimeTaskMap.find(_taskId);
    if (taskIt == m_idToRuntimeTaskMap.end())
        throw runtime_error("Can't find task with ID" + to_string(_taskId));

    const STopoRuntimeTask& taskInfo = taskIt->second;
    CTopoProperty::Ptr_t property = taskInfo.m_task->getProperty(_propertyName);
    if (property == nullptr)
        throw runtime_error("Property <" + _propertyName + "> for task " + to_string(_taskId) + " doesn't exist");

    switch (property->getScopeType())
    {
        case CTopoProperty::EScopeType::GLOBAL:
            return getRuntimeTaskIterator(
                [&_propertyName](const STopoRuntimeTask::FilterIterator_t::value_type& value) -> bool {
                    CTopoTask::Ptr_t task = value.second.m_task;
                    CTopoProperty::Ptr_t property = task->getProperty(_propertyName);
                    return property != nullptr;
                });

        case CTopoProperty::EScopeType::COLLECTION:
        {
            Id_t collectionId = taskInfo.m_taskCollectionId;
            if (collectionId == 0)
                throw runtime_error("Property <" + _propertyName + "> is set for COLLECTION scope only but task " +
                                    to_string(_taskId) + " doesn't belong to any collection");

            auto it = m_idToRuntimeCollectionMap.find(collectionId);
            if (it == m_idToRuntimeCollectionMap.end())
                throw runtime_error("Collection <" + to_string(m_currentCollectionId) + "> not found in map");

            const STopoRuntimeCollection& collectionInfo = it->second;

            return getRuntimeTaskIterator(
                collectionInfo.m_idToRuntimeTaskMap,
                [&_propertyName](const STopoRuntimeTask::FilterIterator_t::value_type& value) -> bool {
                    CTopoTask::Ptr_t task = value.second.m_task;
                    CTopoProperty::Ptr_t property = task->getProperty(_propertyName);
                    return property != nullptr;
                });
        }
    }
    return make_pair(STopoRuntimeTask::FilterIterator_t(), STopoRuntimeTask::FilterIterator_t());
}

void CTopoCore::FillIdToTopoElementMap(const CTopoElement::Ptr_t& _element)
{
    if (_element->getType() == CTopoBase::EType::TASK)
    {
        CTopoTask::Ptr_t task = dynamic_pointer_cast<CTopoTask>(_element);
        std::string path;
        size_t collectionCounter;
        if (task->getParent()->getType() == CTopoBase::EType::COLLECTION)
        {
            path = m_currentCollectionIdPath + "/" + task->getName();
            collectionCounter = m_counterMap[task->getParent()->getPath()] - 1;
        }
        else
        {
            path = _element->getPath();
            collectionCounter = std::numeric_limits<uint32_t>::max();
        }

        size_t counter = ++m_counterMap[path];
        size_t index = counter - 1;
        std::string idPath = path + "_" + to_string(index);

        Id_t crc = MiscCommon::crc64(idPath);
        if (m_idToRuntimeTaskMap.find(crc) != m_idToRuntimeTaskMap.end())
        {
            throw std::runtime_error("Failed to create unique ID for task with path " + idPath +
                                     ". Rename task/collection/group in the path.");
        }

        STopoRuntimeTask info;
        info.m_task = task;
        info.m_taskIndex = index;
        info.m_collectionIndex = collectionCounter;
        info.m_taskPath = idPath;
        info.m_taskCollectionId =
            (task->getParent()->getType() == CTopoBase::EType::COLLECTION) ? m_currentCollectionId : 0;
        m_idToRuntimeTaskMap.insert(make_pair(crc, info));

        if (task->getParent()->getType() == CTopoBase::EType::COLLECTION)
        {
            auto it = m_idToRuntimeCollectionMap.find(m_currentCollectionId);
            if (it == m_idToRuntimeCollectionMap.end())
                throw runtime_error("Collection <" + to_string(m_currentCollectionId) + "> not found in map");

            it->second.m_idToRuntimeTaskMap.insert(make_pair(crc, info));
        }

        return;
    }
    else if (_element->getType() == CTopoBase::EType::COLLECTION)
    {
        CTopoCollection::Ptr_t collection = dynamic_pointer_cast<CTopoCollection>(_element);

        std::string path = collection->getPath();
        size_t counter = ++m_counterMap[path];
        size_t index = counter - 1;
        m_currentCollectionIdPath = path + "_" + to_string(index);

        Id_t crc = MiscCommon::crc64(m_currentCollectionIdPath);
        if (m_idToRuntimeCollectionMap.find(crc) != m_idToRuntimeCollectionMap.end())
        {
            throw std::runtime_error("Failed to create unique ID for collection with path " +
                                     m_currentCollectionIdPath + ". Rename task/collection/group in the path.");
        }

        STopoRuntimeCollection info;
        info.m_collection = collection;
        info.m_collectionIndex = index;
        info.m_collectionPath = m_currentCollectionIdPath;
        m_idToRuntimeCollectionMap.insert(make_pair(crc, info));

        m_currentCollectionId = crc;

        const auto& elements = collection->getElements();
        for (const auto& v : elements)
        {
            FillIdToTopoElementMap(v);
        }
    }
    else if (_element->getType() == CTopoBase::EType::GROUP)
    {
        CTopoGroup::Ptr_t group = dynamic_pointer_cast<CTopoGroup>(_element);
        const auto& elements = group->getElements();
        size_t n = group->getN();
        for (size_t i = 0; i < n; i++)
        {
            for (const auto& v : elements)
            {
                FillIdToTopoElementMap(v);
            }
        }
    }
}

std::string CTopoCore::stringOfTasks(const IdSet_t& _ids) const
{
    set<string> tasksSet;
    multiset<string> tasksMultiset;
    for (auto taskID : _ids)
    {
        auto it = m_idToRuntimeTaskMap.find(taskID);
        if (it != m_idToRuntimeTaskMap.end())
        {
            tasksSet.insert(it->second.m_task->getPath());
            tasksMultiset.insert(it->second.m_task->getPath());
        }
    }

    stringstream ss;
    for (auto path : tasksSet)
    {
        ss << tasksMultiset.count(path) << " x " << path << "\n";
    }

    return ss.str();
}

std::string CTopoCore::stringOfCollections(const IdSet_t& _ids) const
{
    set<string> collectionsSet;
    multiset<string> collectionsMultiset;
    for (auto collectionID : _ids)
    {
        auto it = m_idToRuntimeCollectionMap.find(collectionID);
        if (it != m_idToRuntimeCollectionMap.end())
        {
            collectionsSet.insert(it->second.m_collection->getPath());
            collectionsMultiset.insert(it->second.m_collection->getPath());
        }
    }

    stringstream ss;
    for (auto path : collectionsSet)
    {
        ss << collectionsMultiset.count(path) << " x " << path << "\n";
    }

    return ss.str();
}

string CTopoCore::toString() const
{
    stringstream ss;
    ss << "CTopology:\n";
    ss << "  m_idToRuntimeTaskMap size=" << m_idToRuntimeTaskMap.size() << "\n";
    for (const auto& v : m_idToRuntimeTaskMap)
    {
        ss << "    " << v.first << " -> " << v.second.m_task->getPath() << "\n";
    }
    ss << "  m_idToRuntimeCollectionMap size=" << m_idToRuntimeCollectionMap.size() << "\n";
    for (const auto& v : m_idToRuntimeCollectionMap)
    {
        ss << "    " << v.first << " -> " << v.second.m_collection->getPath() << "\n";
    }
    return ss.str();
}

ostream& operator<<(ostream& _strm, const CTopoCore& _topology)
{
    _strm << _topology.toString();
    return _strm;
}
