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
#include <fstream>
#include <string>
// MiscCommon
#include "CRC.h"
// BOOST
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/range/algorithm/copy.hpp>
#include <boost/regex.hpp>

using namespace std;
using namespace dds;
using namespace topology_api;
using namespace user_defaults_api;
using namespace boost;

CTopoCore::CTopoCore()
{
}

/// copy constructor
CTopoCore::CTopoCore(CTopoCore const& _topo)
{
    // no need to lock this object because no other thread
    // will be using it until after construction
    // but we DO need to lock the other object
    std::unique_lock<std::mutex> lock_other(_topo.m_mtxTopoInit);

    m_main = _topo.m_main;
    m_idToRuntimeTaskMap = _topo.m_idToRuntimeTaskMap;
    m_idToRuntimeCollectionMap = _topo.m_idToRuntimeCollectionMap;
    m_taskIdPathToIdMap = _topo.m_taskIdPathToIdMap;
    m_collectionIdPathToIdMap = _topo.m_collectionIdPathToIdMap;
    m_counterMap = _topo.m_counterMap;
    m_currentCollectionIdPath = _topo.m_currentCollectionIdPath;
    m_currentCollectionId = _topo.m_currentCollectionId;
    m_bXMLValidationDisabled = _topo.m_bXMLValidationDisabled;
    m_name = _topo.m_name;
    m_hash = _topo.m_hash;
    m_filepath = _topo.m_filepath;
}

/// copy assignment operator
CTopoCore& CTopoCore::operator=(CTopoCore const& _topo)
{
    if (&_topo != this)
    {
        // lock both objects
        std::unique_lock<std::mutex> lock_this(m_mtxTopoInit, std::defer_lock);
        std::unique_lock<std::mutex> lock_other(_topo.m_mtxTopoInit, std::defer_lock);

        // ensure no deadlock
        std::lock(lock_this, lock_other);

        // safely copy the data
        m_main = _topo.m_main;
        m_idToRuntimeTaskMap = _topo.m_idToRuntimeTaskMap;
        m_idToRuntimeCollectionMap = _topo.m_idToRuntimeCollectionMap;
        m_taskIdPathToIdMap = _topo.m_taskIdPathToIdMap;
        m_collectionIdPathToIdMap = _topo.m_collectionIdPathToIdMap;
        m_counterMap = _topo.m_counterMap;
        m_currentCollectionIdPath = _topo.m_currentCollectionIdPath;
        m_currentCollectionId = _topo.m_currentCollectionId;
        m_bXMLValidationDisabled = _topo.m_bXMLValidationDisabled;
        m_name = _topo.m_name;
        m_hash = _topo.m_hash;
        m_filepath = _topo.m_filepath;
    }

    return *this;
}

CTopoCore::~CTopoCore()
{
}

CTopoGroup::Ptr_t CTopoCore::getMainGroup() const
{
    return m_main;
}

void CTopoCore::init(const string& _fileName)
{
    init(_fileName, CUserDefaults::getTopologyXSDFilePath());
}

void CTopoCore::init(const string& _fileName, const string& _schemaFileName)
{
    string filename(_fileName);

    // Take default file for agent $DDS_LOCATION/topology.xml
    if (filename.empty())
    {
        filename = CUserDefaults::instance().getDDSPath() + "topology.xml";
    }

    // Store path to the XML topology file
    m_filepath = boost::filesystem::canonical(filename).string();

    ifstream stream(filename);
    if (stream.is_open())
    {
        init(stream, _schemaFileName);
    }
    else
    {
        throw runtime_error("Can't open te given topo file: " + filename);
    }
}

void CTopoCore::init(istream& _stream)
{
    init(_stream, CUserDefaults::getTopologyXSDFilePath());
}

void CTopoCore::init(istream& _stream, const string& _schemaFileName)
{
    lock_guard<mutex> lock(m_mtxTopoInit);

    if (_schemaFileName.empty() && !m_bXMLValidationDisabled)
    {
        throw runtime_error("XSD schema file not provided. Disable validation or provide a valid schema file.");
    }

    // Use empty string to disable validation in parser
    string schemaFileName = (m_bXMLValidationDisabled) ? "" : _schemaFileName;

    m_main = make_shared<CTopoGroup>("main");
    m_main->initFromXML(_stream, schemaFileName, &m_name);

    // Calculate topology hash
    // Function throws an exception if it fails to calculate the hash.
    m_hash = CalculateHash(_stream);

    m_counterMap.clear();
    m_idToRuntimeTaskMap.clear();
    m_idToRuntimeCollectionMap.clear();
    m_taskIdPathToIdMap.clear();
    m_collectionIdPathToIdMap.clear();
    m_currentCollectionIdPath = "";
    m_currentCollectionId = 0;

    FillIdToTopoElementMap(m_main);
}

void CTopoCore::getDifference(CTopoCore& _topology,
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
    boost::copy(m_idToRuntimeTaskMap | boost::adaptors::map_keys, inserter(tasks, tasks.end()));
    boost::copy(_topology.getIdToRuntimeTaskMap() | boost::adaptors::map_keys, inserter(newTasks, newTasks.end()));
    boost::copy(m_idToRuntimeCollectionMap | boost::adaptors::map_keys, inserter(collections, collections.end()));
    boost::copy(_topology.getIdToRuntimeCollectionMap() | boost::adaptors::map_keys,
                inserter(newCollections, newCollections.end()));

    // Get difference between two sets
    set_difference(
        tasks.begin(), tasks.end(), newTasks.begin(), newTasks.end(), inserter(_removedTasks, _removedTasks.end()));
    set_difference(
        newTasks.begin(), newTasks.end(), tasks.begin(), tasks.end(), inserter(_addedTasks, _addedTasks.begin()));
    set_difference(collections.begin(),
                   collections.end(),
                   newCollections.begin(),
                   newCollections.end(),
                   inserter(_removedCollections, _removedCollections.end()));
    set_difference(newCollections.begin(),
                   newCollections.end(),
                   collections.begin(),
                   collections.end(),
                   inserter(_addedCollections, _addedCollections.begin()));
}

string CTopoCore::getName() const
{
    if (!m_main)
    {
        throw runtime_error("Topology not initialized. Call init first.");
    }
    return m_name;
}

string CTopoCore::getFilepath() const
{
    return m_filepath;
}

uint32_t CTopoCore::getHash() const
{
    if (!m_main)
    {
        throw runtime_error("Topology not initialized. Call init first.");
    }
    return m_hash;
}

void CTopoCore::setXMLValidationDisabled(bool _val)
{
    m_bXMLValidationDisabled = _val;
}

pair<size_t, size_t> CTopoCore::getRequiredNofAgents(size_t _defaultNumSlots) const
{
    if (_defaultNumSlots == 0)
        throw invalid_argument("Default number of slots can't be zero");

    size_t numTasks = getMainGroup()->getTotalNofTasks();
    size_t maxNumSlots(_defaultNumSlots);
    auto it = getRuntimeCollectionIterator();
    for (auto i = it.first; i != it.second; i++)
    {
        const auto& collection = i->second;
        maxNumSlots = max(collection.m_collection->getTotalNofTasks(), maxNumSlots);
    }

    size_t numAgents = numTasks / maxNumSlots + ((numTasks % maxNumSlots != 0) ? 1 : 0);
    return make_pair(numAgents, maxNumSlots);
}

size_t CTopoCore::getTotalNofTasks() const
{
    return getMainGroup()->getTotalNofTasks();
}

const STopoRuntimeTask::Map_t& CTopoCore::getIdToRuntimeTaskMap()
{
    lock_guard<mutex> lock(m_mtxTopoInit);
    return m_idToRuntimeTaskMap;
}

const STopoRuntimeCollection::Map_t& CTopoCore::getIdToRuntimeCollectionMap()
{
    lock_guard<mutex> lock(m_mtxTopoInit);
    return m_idToRuntimeCollectionMap;
}

const CTopoCore::IdPathToIdMap_t& CTopoCore::getTaskIdPathToIdMap()
{
    lock_guard<mutex> lock(m_mtxTopoInit);
    return m_taskIdPathToIdMap;
}

const CTopoCore::IdPathToIdMap_t& CTopoCore::getCollectionIdPathToIdMap()
{
    lock_guard<mutex> lock(m_mtxTopoInit);
    return m_collectionIdPathToIdMap;
}

const STopoRuntimeTask& CTopoCore::getRuntimeTaskById(Id_t _id)
{
    lock_guard<mutex> lock(m_mtxTopoInit);

    auto it = m_idToRuntimeTaskMap.find(_id);
    if (it == m_idToRuntimeTaskMap.end())
        throw runtime_error("Can not find task info with ID " + to_string(_id));
    return it->second;
}

const STopoRuntimeCollection& CTopoCore::getRuntimeCollectionById(Id_t _id)
{
    lock_guard<mutex> lock(m_mtxTopoInit);

    auto it = m_idToRuntimeCollectionMap.find(_id);
    if (it == m_idToRuntimeCollectionMap.end())
        throw runtime_error("Can not find task collection with ID " + to_string(_id));
    return it->second;
}

const STopoRuntimeTask& CTopoCore::getRuntimeTaskByIdPath(const string& _idPath)
{
    // TODO: Add a m_mtxTopoInit lock, but avoid deadlock in getRuntimeTaskById
    auto it = m_taskIdPathToIdMap.find(_idPath);
    if (it == m_taskIdPathToIdMap.end())
        throw runtime_error("Can not find task with path ID " + _idPath);
    return getRuntimeTaskById(it->second);
}

const STopoRuntimeCollection& CTopoCore::getRuntimeCollectionByIdPath(const string& _idPath)
{
    // TODO: Add a m_mtxTopoInit lock, but avoid deadlock in getRuntimeCollectionById
    auto it = m_collectionIdPathToIdMap.find(_idPath);
    if (it == m_collectionIdPathToIdMap.end())
        throw runtime_error("Can not find collection with path ID " + _idPath);
    return getRuntimeCollectionById(it->second);
}

const STopoRuntimeTask& CTopoCore::getRuntimeTask(const string& _path)
{
    try
    {
        // throws boost::bad_lexical_cast if path is not a number
        uint64_t taskID{ boost::lexical_cast<uint64_t>(_path) };
        // throws runtime_error if ID not found
        return getRuntimeTaskById(taskID);
    }
    catch (boost::bad_lexical_cast&)
    {
        // throws runtime_error if path not found
        return getRuntimeTaskByIdPath(_path);
    }
}

const STopoRuntimeCollection& CTopoCore::getRuntimeCollection(const string& _path)
{
    try
    {
        // throws boost::bad_lexical_cast if path is not a number
        uint64_t taskID{ boost::lexical_cast<uint64_t>(_path) };
        // throws runtime_error if ID not found
        return getRuntimeCollectionById(taskID);
    }
    catch (boost::bad_lexical_cast&)
    {
        // throws runtime_error if path not found
        return getRuntimeCollectionByIdPath(_path);
    }
}

STopoRuntimeTask::FilterIteratorPair_t CTopoCore::getRuntimeTaskIterator(const STopoRuntimeTask::Map_t& _map,
                                                                         STopoRuntimeTask::Condition_t _condition) const
{
    STopoRuntimeTask::Condition_t condition = _condition;
    if (condition == nullptr)
    {
        condition = [](const STopoRuntimeTask::FilterIterator_t::value_type&) -> bool { return true; };
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
        condition = [](const STopoRuntimeCollection::FilterIterator_t::value_type&) -> bool { return true; };
    }
    STopoRuntimeCollection::FilterIterator_t begin_iterator(
        condition, m_idToRuntimeCollectionMap.begin(), m_idToRuntimeCollectionMap.end());
    STopoRuntimeCollection::FilterIterator_t end_iterator(
        condition, m_idToRuntimeCollectionMap.end(), m_idToRuntimeCollectionMap.end());
    return make_pair(begin_iterator, end_iterator);
}

STopoRuntimeTask::FilterIteratorPair_t CTopoCore::getRuntimeTaskIteratorForPropertyName(const string& _propertyName,
                                                                                        Id_t _taskId) const
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
                [&_propertyName](const STopoRuntimeTask::FilterIterator_t::value_type& value) -> bool
                {
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
                [&_propertyName](const STopoRuntimeTask::FilterIterator_t::value_type& value) -> bool
                {
                    CTopoTask::Ptr_t task = value.second.m_task;
                    CTopoProperty::Ptr_t property = task->getProperty(_propertyName);
                    return property != nullptr;
                });
        }
    }
    return make_pair(STopoRuntimeTask::FilterIterator_t(), STopoRuntimeTask::FilterIterator_t());
}

STopoRuntimeTask::FilterIteratorPair_t CTopoCore::getRuntimeTaskIteratorMatchingPath(const string& _pathPattern) const
{
    // shared_ptr is needed to keep the object alive when the object is used in lambda
    std::shared_ptr<boost::regex> pathRegex = make_shared<boost::regex>(_pathPattern);
    return getRuntimeTaskIterator([pathRegex](const STopoRuntimeTask::FilterIterator_t::value_type& _value) -> bool
                                  { return boost::regex_match(_value.second.m_taskPath, *pathRegex); });
}

STopoRuntimeCollection::FilterIteratorPair_t CTopoCore::getRuntimeCollectionIteratorMatchingPath(
    const string& _pathPattern) const
{
    // shared_ptr is needed to keep the object alive when the object is used in lambda
    std::shared_ptr<boost::regex> pathRegex = make_shared<boost::regex>(_pathPattern);
    return getRuntimeCollectionIterator(
        [pathRegex](const STopoRuntimeCollection::FilterIterator_t::value_type& _value) -> bool
        { return boost::regex_match(_value.second.m_collectionPath, *pathRegex); });
}

void CTopoCore::FillIdToTopoElementMap(const CTopoElement::Ptr_t& _element)
{
    if (_element->getType() == CTopoBase::EType::TASK)
    {
        CTopoTask::Ptr_t task = dynamic_pointer_cast<CTopoTask>(_element);
        string path;
        size_t collectionCounter;
        if (task->getParent()->getType() == CTopoBase::EType::COLLECTION)
        {
            path = m_currentCollectionIdPath + "/" + task->getName();
            collectionCounter = m_counterMap[task->getParent()->getPath()] - 1;
        }
        else
        {
            path = _element->getPath();
            collectionCounter = numeric_limits<uint32_t>::max();
        }

        size_t counter = ++m_counterMap[path];
        size_t index = counter - 1;
        string idPath = path + "_" + to_string(index);

        Id_t crc{ calculateId(idPath, task->hashString()) };
        if (m_idToRuntimeTaskMap.find(crc) != m_idToRuntimeTaskMap.end())
        {
            throw runtime_error("Failed to create unique ID for task with path " + idPath +
                                ". Rename task/collection/group in the path.");
        }

        STopoRuntimeTask info;
        info.m_task = task;
        info.m_taskId = crc;
        info.m_taskIndex = index;
        info.m_collectionIndex = collectionCounter;
        info.m_taskPath = idPath;
        info.m_taskCollectionId =
            (task->getParent()->getType() == CTopoBase::EType::COLLECTION) ? m_currentCollectionId : 0;
        m_idToRuntimeTaskMap.insert(make_pair(crc, info));
        m_taskIdPathToIdMap.insert(make_pair(info.m_taskPath, crc));

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

        string path = collection->getPath();
        size_t counter = ++m_counterMap[path];
        size_t index = counter - 1;
        m_currentCollectionIdPath = path + "_" + to_string(index);

        Id_t crc{ calculateId(m_currentCollectionIdPath, collection->hashString()) };
        if (m_idToRuntimeCollectionMap.find(crc) != m_idToRuntimeCollectionMap.end())
        {
            throw runtime_error("Failed to create unique ID for collection with path " + m_currentCollectionIdPath +
                                ". Rename task/collection/group in the path.");
        }

        STopoRuntimeCollection info;
        info.m_collection = collection;
        info.m_collectionId = crc;
        info.m_collectionIndex = index;
        info.m_collectionPath = m_currentCollectionIdPath;
        m_idToRuntimeCollectionMap.insert(make_pair(crc, info));
        m_collectionIdPathToIdMap.insert(make_pair(info.m_collectionPath, crc));

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

Id_t CTopoCore::calculateId(const string& _idPath, const string& _hashString)
{
    return dds::misc::crc64(_idPath + _hashString);
}

string CTopoCore::stringOfTasks(const IdSet_t& _ids) const
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

string CTopoCore::stringOfCollections(const IdSet_t& _ids) const
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

uint32_t CTopoCore::CalculateHash(istream& _stream)
{
    return dds::misc::crc32(_stream);
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
