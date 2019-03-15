// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "Topology.h"
#include "TopoUtils.h"
#include "TopologyParserXML.h"
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
using namespace boost;

CTopology::CTopology()
    : m_main(nullptr)
    , m_currentCollectionCrc(0)
    , m_bXMLValidationDisabled(false)
{
}

CTopology::~CTopology()
{
}

CTopoGroup::Ptr_t CTopology::getMainGroup() const
{
    return m_main;
}

void CTopology::init(const std::string& _fileName, bool _initForTest)
{
    CTopologyParserXML parser;
    m_main = std::make_shared<CTopoGroup>();
    parser.parse(_fileName, m_main, m_bXMLValidationDisabled);

    m_counterMap.clear();
    m_hashToRuntimeTaskMap.clear();
    m_hashToRuntimeCollectionMap.clear();
    m_currentCollectionHashPath = "";
    m_currentCollectionCrc = 0;

    FillHashToTopoElementMap(m_main);
}

void CTopology::getDifference(const CTopology& _topology,
                              HashSet_t& _removedTasks,
                              HashSet_t& _removedCollections,
                              HashSet_t& _addedTasks,
                              HashSet_t& _addedCollections)
{
    _removedTasks.clear();
    _removedCollections.clear();
    _addedTasks.clear();
    _addedCollections.clear();

    HashSet_t tasks;
    HashSet_t newTasks;
    HashSet_t collections;
    HashSet_t newCollections;

    // Get all keys from maps as a set
    boost::copy(m_hashToRuntimeTaskMap | boost::adaptors::map_keys, std::inserter(tasks, tasks.end()));
    boost::copy(_topology.getHashToRuntimeTaskMap() | boost::adaptors::map_keys,
                std::inserter(newTasks, newTasks.end()));
    boost::copy(m_hashToRuntimeCollectionMap | boost::adaptors::map_keys,
                std::inserter(collections, collections.end()));
    boost::copy(_topology.getHashToRuntimeCollectionMap() | boost::adaptors::map_keys,
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

void CTopology::setXMLValidationDisabled(bool _val)
{
    m_bXMLValidationDisabled = _val;
}

const STopoRuntimeTask::Map_t& CTopology::getHashToRuntimeTaskMap() const
{
    return m_hashToRuntimeTaskMap;
}

const STopoRuntimeCollection::Map_t& CTopology::getHashToRuntimeCollectionMap() const
{
    return m_hashToRuntimeCollectionMap;
}

const STopoRuntimeTask& CTopology::getRuntimeTaskByHash(uint64_t _hash) const
{
    auto it = m_hashToRuntimeTaskMap.find(_hash);
    if (it == m_hashToRuntimeTaskMap.end())
        throw runtime_error("Can not find task info with hash " + to_string(_hash));
    return it->second;
}

const STopoRuntimeCollection& CTopology::getRuntimeCollectionByHash(uint64_t _hash) const
{
    auto it = m_hashToRuntimeCollectionMap.find(_hash);
    if (it == m_hashToRuntimeCollectionMap.end())
        throw runtime_error("Can not find task collection with hash " + to_string(_hash));
    return it->second;
}

const STopoRuntimeTask& CTopology::getRuntimeTaskByHashPath(const std::string& _hashPath) const
{
    uint8_t hash = MiscCommon::crc64(_hashPath);
    return getRuntimeTaskByHash(hash);
}

const STopoRuntimeCollection& CTopology::getRuntimeCollectionByHashPath(const std::string& _hashPath) const
{
    uint8_t hash = MiscCommon::crc64(_hashPath);
    return getRuntimeCollectionByHash(hash);
}

STopoRuntimeTask::FilterIteratorPair_t CTopology::getRuntimeTaskIterator(const STopoRuntimeTask::Map_t& _map,
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

STopoRuntimeTask::FilterIteratorPair_t CTopology::getRuntimeTaskIterator(STopoRuntimeTask::Condition_t _condition) const
{
    return getRuntimeTaskIterator(m_hashToRuntimeTaskMap, _condition);
}

STopoRuntimeCollection::FilterIteratorPair_t CTopology::getRuntimeCollectionIterator(
    STopoRuntimeCollection::Condition_t _condition) const
{
    STopoRuntimeCollection::Condition_t condition = _condition;
    if (condition == nullptr)
    {
        condition = [](STopoRuntimeCollection::Map_t::value_type) -> bool { return true; };
    }
    STopoRuntimeCollection::FilterIterator_t begin_iterator(
        condition, m_hashToRuntimeCollectionMap.begin(), m_hashToRuntimeCollectionMap.end());
    STopoRuntimeCollection::FilterIterator_t end_iterator(
        condition, m_hashToRuntimeCollectionMap.end(), m_hashToRuntimeCollectionMap.end());
    return make_pair(begin_iterator, end_iterator);
}

STopoRuntimeTask::FilterIteratorPair_t CTopology::getRuntimeTaskIteratorForPropertyName(
    const std::string& _propertyName, uint64_t _taskHash) const
{
    auto taskIt = m_hashToRuntimeTaskMap.find(_taskHash);
    if (taskIt == m_hashToRuntimeTaskMap.end())
        throw runtime_error("Can't find task with ID" + to_string(_taskHash));

    const STopoRuntimeTask& taskInfo = taskIt->second;
    CTopoProperty::Ptr_t property = taskInfo.m_task->getProperty(_propertyName);
    if (property == nullptr)
        throw runtime_error("Property <" + _propertyName + "> for task " + to_string(_taskHash) + " doesn't exist");

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
            uint64_t collectionHash = taskInfo.m_taskCollectionHash;
            if (collectionHash == 0)
                throw runtime_error("Property <" + _propertyName + "> is set for COLLECTION scope only but task " +
                                    to_string(_taskHash) + " doesn't belong to any collection");

            auto it = m_hashToRuntimeCollectionMap.find(collectionHash);
            if (it == m_hashToRuntimeCollectionMap.end())
                throw runtime_error("Collection <" + to_string(m_currentCollectionCrc) + "> not found in map");

            const STopoRuntimeCollection& collectionInfo = it->second;

            return getRuntimeTaskIterator(
                collectionInfo.m_hashToRuntimeTaskMap,
                [&_propertyName](const STopoRuntimeTask::FilterIterator_t::value_type& value) -> bool {
                    CTopoTask::Ptr_t task = value.second.m_task;
                    CTopoProperty::Ptr_t property = task->getProperty(_propertyName);
                    return property != nullptr;
                });
        }
    }
}

void CTopology::FillHashToTopoElementMap(const CTopoElement::Ptr_t& _element)
{
    if (_element->getType() == CTopoBase::EType::TASK)
    {
        CTopoTask::Ptr_t task = dynamic_pointer_cast<CTopoTask>(_element);
        std::string path;
        size_t collectionCounter;
        if (task->getParent()->getType() == CTopoBase::EType::COLLECTION)
        {
            path = m_currentCollectionHashPath + "/" + task->getName();
            collectionCounter = m_counterMap[task->getParent()->getPath()] - 1;
        }
        else
        {
            path = _element->getPath();
            collectionCounter = std::numeric_limits<uint32_t>::max();
        }

        size_t counter = ++m_counterMap[path];
        size_t index = counter - 1;
        std::string hashPath = path + "_" + to_string(index);

        uint64_t crc = MiscCommon::crc64(hashPath);
        if (m_hashToRuntimeTaskMap.find(crc) != m_hashToRuntimeTaskMap.end())
        {
            throw std::runtime_error("Failed to create unique hash for task with path " + hashPath +
                                     ". Rename task/collection/group in the path.");
        }

        STopoRuntimeTask info;
        info.m_task = task;
        info.m_taskIndex = index;
        info.m_collectionIndex = collectionCounter;
        info.m_taskPath = hashPath;
        info.m_taskCollectionHash =
            (task->getParent()->getType() == CTopoBase::EType::COLLECTION) ? m_currentCollectionCrc : 0;
        m_hashToRuntimeTaskMap.insert(make_pair(crc, info));

        if (task->getParent()->getType() == CTopoBase::EType::COLLECTION)
        {
            auto it = m_hashToRuntimeCollectionMap.find(m_currentCollectionCrc);
            if (it == m_hashToRuntimeCollectionMap.end())
                throw runtime_error("Collection <" + to_string(m_currentCollectionCrc) + "> not found in map");

            it->second.m_hashToRuntimeTaskMap.insert(make_pair(crc, info));
        }

        return;
    }
    else if (_element->getType() == CTopoBase::EType::COLLECTION)
    {
        CTopoCollection::Ptr_t collection = dynamic_pointer_cast<CTopoCollection>(_element);

        std::string path = collection->getPath();
        size_t counter = ++m_counterMap[path];
        size_t index = counter - 1;
        m_currentCollectionHashPath = path + "_" + to_string(index);

        uint64_t crc = MiscCommon::crc64(m_currentCollectionHashPath);
        if (m_hashToRuntimeCollectionMap.find(crc) != m_hashToRuntimeCollectionMap.end())
        {
            throw std::runtime_error("Failed to create unique hash for collection with path " +
                                     m_currentCollectionHashPath + ". Rename task/collection/group in the path.");
        }

        STopoRuntimeCollection info;
        info.m_collection = collection;
        info.m_collectionIndex = index;
        info.m_collectionPath = m_currentCollectionHashPath;
        m_hashToRuntimeCollectionMap.insert(make_pair(crc, info));

        m_currentCollectionCrc = crc;

        const auto& elements = collection->getElements();
        for (const auto& v : elements)
        {
            FillHashToTopoElementMap(v);
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
                FillHashToTopoElementMap(v);
            }
        }
    }
}

std::string CTopology::stringOfTasks(const HashSet_t& _ids) const
{
    set<string> tasksSet;
    multiset<string> tasksMultiset;
    for (auto taskID : _ids)
    {
        auto it = m_hashToRuntimeTaskMap.find(taskID);
        if (it != m_hashToRuntimeTaskMap.end())
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

std::string CTopology::stringOfCollections(const HashSet_t& _ids) const
{
    set<string> collectionsSet;
    multiset<string> collectionsMultiset;
    for (auto collectionID : _ids)
    {
        auto it = m_hashToRuntimeCollectionMap.find(collectionID);
        if (it != m_hashToRuntimeCollectionMap.end())
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

string CTopology::toString() const
{
    stringstream ss;
    ss << "CTopology:\n";
    ss << "  m_hashToTaskMap size=" << m_hashToRuntimeTaskMap.size() << "\n";
    for (const auto& v : m_hashToRuntimeTaskMap)
    {
        ss << "    " << v.first << " -> " << v.second.m_task->getPath() << "\n";
    }
    ss << "  m_hashToTaskCollectionMap size=" << m_hashToRuntimeCollectionMap.size() << "\n";
    for (const auto& v : m_hashToRuntimeCollectionMap)
    {
        ss << "    " << v.first << " -> " << v.second.m_collection->getPath() << "\n";
    }
    return ss.str();
}

ostream& operator<<(ostream& _strm, const CTopology& _topology)
{
    _strm << _topology.toString();
    return _strm;
}
