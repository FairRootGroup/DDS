// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "Topology.h"
#include "TopoIndex.h"
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
    , m_currentTaskCollectionCrc(0)
    , m_bXMLValidationDisabled(false)
{
}

CTopology::~CTopology()
{
}

TaskGroupPtr_t CTopology::getMainGroup() const
{
    return m_main;
}

void CTopology::init(const std::string& _fileName, bool _initForTest)
{
    /// FIXME Use parser based on the file extension.

    CTopologyParserXML parser;
    m_main = std::make_shared<CTaskGroup>();
    parser.parse(_fileName, m_main, m_bXMLValidationDisabled);

    m_topoIndexToTopoElementMap.clear();
    FillTopoIndexToTopoElementMap(m_main);

    m_counterMap.clear();
    m_hashToTaskInfoMap.clear();
    m_hashToTaskCollectionInfoMap.clear();
    m_currentTaskCollectionHashPath = "";
    m_currentTaskCollectionCrc = 0;
    // TODO: _initForTest flag is set permanently to true.
    // We need hash path maps to be filled.
    // These maps are used to sent custom command to a particular task.
    FillHashToTopoElementMap(m_main, true); //_initForTest);
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
    boost::copy(m_hashToTaskInfoMap | boost::adaptors::map_keys, std::inserter(tasks, tasks.end()));
    boost::copy(_topology.getHashToTaskInfoMap() | boost::adaptors::map_keys, std::inserter(newTasks, newTasks.end()));
    boost::copy(m_hashToTaskCollectionInfoMap | boost::adaptors::map_keys,
                std::inserter(collections, collections.end()));
    boost::copy(_topology.getHashToTaskCollectionInfoMap() | boost::adaptors::map_keys,
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

const CTopology::TopoIndexToTopoElementMap_t& CTopology::getTopoIndexToTopoElementMap() const
{
    return m_topoIndexToTopoElementMap;
}

const HashToTaskInfoMap_t& CTopology::getHashToTaskInfoMap() const
{
    return m_hashToTaskInfoMap;
}

const HashToTaskCollectionInfoMap_t& CTopology::getHashToTaskCollectionInfoMap() const
{
    return m_hashToTaskCollectionInfoMap;
}

const CTopology::HashPathToTaskMap_t& CTopology::getHashPathToTaskMap() const
{
    return m_hashPathToTaskMap;
}

const CTopology::HashPathToTaskCollectionMap_t& CTopology::getHashPathToTaskCollectionMap() const
{
    return m_hashPathToTaskCollectionMap;
}

TopoElementPtr_t CTopology::getTopoElementByTopoIndex(const CTopoIndex& _index) const
{
    auto it = m_topoIndexToTopoElementMap.find(_index);
    if (it == m_topoIndexToTopoElementMap.end())
        throw runtime_error("Can not find element with index " + _index.getPath());
    return it->second;
}

TaskPtr_t CTopology::getTaskByHash(uint64_t _hash) const
{
    auto it = m_hashToTaskInfoMap.find(_hash);
    if (it == m_hashToTaskInfoMap.end())
        throw runtime_error("Can not find task with hash " + to_string(_hash));
    return it->second.m_task;
}

const STaskInfo& CTopology::getTaskInfoByHash(uint64_t _hash) const
{
    auto it = m_hashToTaskInfoMap.find(_hash);
    if (it == m_hashToTaskInfoMap.end())
        throw runtime_error("Can not find task info with hash " + to_string(_hash));
    return it->second;
}

const STaskCollectionInfo& CTopology::getTaskCollectionInfoByHash(uint64_t _hash) const
{
    auto it = m_hashToTaskCollectionInfoMap.find(_hash);
    if (it == m_hashToTaskCollectionInfoMap.end())
        throw runtime_error("Can not find task collection with hash " + to_string(_hash));
    return it->second;
}

TaskPtr_t CTopology::getTaskByHashPath(const std::string& _hashPath) const
{
    auto it = m_hashPathToTaskMap.find(_hashPath);
    if (it == m_hashPathToTaskMap.end())
        throw runtime_error("Can not find task for hash path " + _hashPath);
    return it->second;
}

TaskInfoIteratorPair_t CTopology::getTaskInfoIterator(const HashToTaskInfoMap_t& _map,
                                                      TaskInfoCondition_t _condition) const
{
    TaskInfoCondition_t condition = _condition;
    if (condition == nullptr)
    {
        condition = [](const HashToTaskInfoMap_t::value_type&) -> bool { return true; };
    }
    TaskInfoIterator_t begin_iterator(condition, _map.begin(), _map.end());
    TaskInfoIterator_t end_iterator(condition, _map.end(), _map.end());
    return make_pair(begin_iterator, end_iterator);
}

TaskInfoIteratorPair_t CTopology::getTaskInfoIterator(TaskInfoCondition_t _condition) const
{
    return getTaskInfoIterator(m_hashToTaskInfoMap, _condition);
}

TaskCollectionInfoIteratorPair_t CTopology::getTaskCollectionInfoIterator(
    TaskCollectionInfoCondition_t _condition) const
{
    TaskCollectionInfoCondition_t condition = _condition;
    if (condition == nullptr)
    {
        condition = [](HashToTaskCollectionInfoMap_t::value_type) -> bool { return true; };
    }
    TaskCollectionInfoIterator_t begin_iterator(
        condition, m_hashToTaskCollectionInfoMap.begin(), m_hashToTaskCollectionInfoMap.end());
    TaskCollectionInfoIterator_t end_iterator(
        condition, m_hashToTaskCollectionInfoMap.end(), m_hashToTaskCollectionInfoMap.end());
    return make_pair(begin_iterator, end_iterator);
}

TaskInfoIteratorPair_t CTopology::getTaskInfoIteratorForPropertyId(const std::string& _propertyId,
                                                                   uint64_t _taskHash) const
{
    auto taskIt = m_hashToTaskInfoMap.find(_taskHash);
    if (taskIt == m_hashToTaskInfoMap.end())
        throw runtime_error("Can't find task with ID" + to_string(_taskHash));

    const STaskInfo& taskInfo = taskIt->second;
    TopoPropertyPtr_t property = taskInfo.m_task->getProperty(_propertyId);
    if (property == nullptr)
        throw runtime_error("Property <" + _propertyId + "> for task " + to_string(_taskHash) + " doesn't exist");

    switch (property->getScopeType())
    {
        case EPropertyScopeType::GLOBAL:
            return getTaskInfoIterator([&_propertyId](const TaskInfoIterator_t::value_type& value) -> bool {
                TaskPtr_t task = value.second.m_task;
                TopoPropertyPtr_t property = task->getProperty(_propertyId);
                return property != nullptr;
            });

        case EPropertyScopeType::COLLECTION:
        {
            uint64_t collectionHash = taskInfo.m_taskCollectionHash;
            if (collectionHash == 0)
                throw runtime_error("Property <" + _propertyId + "> is set for COLLECTION scope only but task " +
                                    to_string(_taskHash) + " doesn't belong to any collection");

            auto it = m_hashToTaskCollectionInfoMap.find(collectionHash);
            if (it == m_hashToTaskCollectionInfoMap.end())
                throw runtime_error("Collection <" + to_string(m_currentTaskCollectionCrc) + "> not found in map");

            const STaskCollectionInfo& collectionInfo = it->second;

            return getTaskInfoIterator(collectionInfo.m_hashToTaskInfoMap,
                                       [&_propertyId](const TaskInfoIterator_t::value_type& value) -> bool {
                                           TaskPtr_t task = value.second.m_task;
                                           TopoPropertyPtr_t property = task->getProperty(_propertyId);
                                           return property != nullptr;
                                       });
        }
    }
}

void CTopology::FillTopoIndexToTopoElementMap(const TopoElementPtr_t& _element)
{
    if (_element->getType() == ETopoType::TASK)
    {
        m_topoIndexToTopoElementMap[_element->getPath()] = _element;
        return;
    }
    TaskContainerPtr_t container = dynamic_pointer_cast<CTaskContainer>(_element);
    const auto& elements = container->getElements();
    for (const auto& v : elements)
    {
        m_topoIndexToTopoElementMap[v->getPath()] = v;
        FillTopoIndexToTopoElementMap(v);
    }
}

void CTopology::FillHashToTopoElementMap(const TopoElementPtr_t& _element, bool _fillHashPathMaps)
{
    if (_element->getType() == ETopoType::TASK)
    {
        TaskPtr_t task = dynamic_pointer_cast<CTask>(_element);
        std::string path;
        size_t collectionCounter;
        if (task->getParent()->getType() == ETopoType::COLLECTION)
        {
            path = m_currentTaskCollectionHashPath + "/" + task->getId();
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

        if (_fillHashPathMaps)
        {
            m_hashPathToTaskMap[hashPath] = task;
        }

        uint64_t crc = MiscCommon::crc64(hashPath);
        if (m_hashToTaskInfoMap.find(crc) != m_hashToTaskInfoMap.end())
        {
            // std::stringstream ss;
            // ss << "The same hash detected <" << crc << "> for " << hashPath << std::endl;
            // throw runtime_error(ss.str());
            crc = getNextHashForTask(crc);
        }
        // m_hashToTaskMap[crc] = task;

        STaskInfo info;
        info.m_task = task;
        info.m_taskIndex = index;
        info.m_collectionIndex = collectionCounter;
        info.m_taskPath = hashPath;
        info.m_taskCollectionHash =
            (task->getParent()->getType() == ETopoType::COLLECTION) ? m_currentTaskCollectionCrc : 0;
        m_hashToTaskInfoMap.insert(make_pair(crc, info));

        if (task->getParent()->getType() == ETopoType::COLLECTION)
        {
            auto it = m_hashToTaskCollectionInfoMap.find(m_currentTaskCollectionCrc);
            if (it == m_hashToTaskCollectionInfoMap.end())
                throw runtime_error("Collection <" + to_string(m_currentTaskCollectionCrc) + "> not found in map");

            it->second.m_hashToTaskInfoMap.insert(make_pair(crc, info));
        }

        return;
    }
    else if (_element->getType() == ETopoType::COLLECTION)
    {
        TaskCollectionPtr_t collection = dynamic_pointer_cast<CTaskCollection>(_element);

        std::string path = collection->getPath();
        size_t counter = ++m_counterMap[path];
        size_t index = counter - 1;
        m_currentTaskCollectionHashPath = path + "_" + to_string(index);

        if (_fillHashPathMaps)
        {
            m_hashPathToTaskCollectionMap[m_currentTaskCollectionHashPath] = collection;
        }

        uint64_t crc = MiscCommon::crc64(m_currentTaskCollectionHashPath);
        if (m_hashToTaskCollectionInfoMap.find(crc) != m_hashToTaskCollectionInfoMap.end())
        {
            // std::stringstream ss;
            // ss << "The same hash detected <" << crc << "> for " << m_currentTaskCollectionHashPath << std::endl;
            // throw runtime_error(ss.str());
            crc = getNextHashForTaskCollection(crc);
        }

        STaskCollectionInfo info;
        info.m_collection = collection;
        info.m_collectionIndex = index;
        info.m_collectionPath = m_currentTaskCollectionHashPath;
        m_hashToTaskCollectionInfoMap.insert(make_pair(crc, info));

        m_currentTaskCollectionCrc = crc;

        const auto& elements = collection->getElements();
        for (const auto& v : elements)
        {
            FillHashToTopoElementMap(v, _fillHashPathMaps);
        }
    }
    else if (_element->getType() == ETopoType::GROUP)
    {
        TaskGroupPtr_t group = dynamic_pointer_cast<CTaskGroup>(_element);
        const auto& elements = group->getElements();
        size_t n = group->getN();
        for (size_t i = 0; i < n; i++)
        {
            for (const auto& v : elements)
            {
                FillHashToTopoElementMap(v, _fillHashPathMaps);
            }
        }
    }
}

uint64_t CTopology::getNextHashForTask(uint64_t _crc) const
{
    uint64_t crc = _crc;
    do
    {
        ++crc;
    } while (m_hashToTaskInfoMap.find(crc) == m_hashToTaskInfoMap.end());
    return crc;
}

uint64_t CTopology::getNextHashForTaskCollection(uint64_t _crc) const
{
    uint64_t crc = _crc;
    do
    {
        ++crc;
    } while (m_hashToTaskCollectionInfoMap.find(crc) == m_hashToTaskCollectionInfoMap.end());
    return crc;
}

std::string CTopology::stringOfTasks(const HashSet_t& _ids) const
{
    set<string> tasksSet;
    multiset<string> tasksMultiset;
    for (auto taskID : _ids)
    {
        auto it = m_hashToTaskInfoMap.find(taskID);
        if (it != m_hashToTaskInfoMap.end())
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
        auto it = m_hashToTaskCollectionInfoMap.find(collectionID);
        if (it != m_hashToTaskCollectionInfoMap.end())
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
    ss << "  m_topoIndexToTopoElementMap size=" << m_topoIndexToTopoElementMap.size() << "\n";
    for (const auto& v : m_topoIndexToTopoElementMap)
    {
        ss << "    " << v.first.getPath() << " -> " << v.second->getPath() << "\n";
    }
    ss << "  m_hashToTaskMap size=" << m_hashToTaskInfoMap.size() << "\n";
    for (const auto& v : m_hashToTaskInfoMap)
    {
        ss << "    " << v.first << " -> " << v.second.m_task->getPath() << "\n";
    }
    ss << "  m_hashToTaskCollectionMap size=" << m_hashToTaskCollectionInfoMap.size() << "\n";
    for (const auto& v : m_hashToTaskCollectionInfoMap)
    {
        ss << "    " << v.first << " -> " << v.second.m_collection->getPath() << "\n";
    }
    ss << "  m_hashPathToTaskMap size=" << m_hashPathToTaskMap.size() << "\n";
    for (const auto& v : m_hashPathToTaskMap)
    {
        ss << "    " << v.first << " -> " << v.second->getPath() << "\n";
    }
    ss << "  m_hashPathToTaskCollectionMap size=" << m_hashPathToTaskCollectionMap.size() << "\n";
    for (const auto& v : m_hashPathToTaskCollectionMap)
    {
        ss << "    " << v.first << " -> " << v.second->getPath() << "\n";
    }
    return ss.str();
}

ostream& operator<<(ostream& _strm, const CTopology& _topology)
{
    _strm << _topology.toString();
    return _strm;
}
