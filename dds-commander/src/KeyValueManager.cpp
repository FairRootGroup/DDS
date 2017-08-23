// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "KeyValueManager.h"
#include "Logger.h"
#include "UpdateKeyCmd.h"
// STD
#include <mutex>

using namespace std;
using namespace dds;
using namespace dds::commander_cmd;
using namespace dds::topology_api;
using namespace dds::protocol_api;
using namespace MiscCommon;

SKeyValueRecord::SKeyValueRecord()
    : m_mutex()
    , m_keyValue()
    , m_deleted(false)
{
}

SKeyValueRecord::~SKeyValueRecord()
{
}

EKeyUpdateResult SKeyValueRecord::updateKeyValue(const SUpdateKeyCmd& _cmd, protocol_api::SUpdateKeyCmd& _serverCmd)
{
    lock_guard<mutex> lock(m_mutex);

    // Check version and if it is correct update key-value record and increase the record's current version
    bool isVersionOK = m_keyValue.m_version == _cmd.m_version;
    if (isVersionOK)
    {
        m_keyValue = _cmd;
        m_keyValue.m_version++;
    }
    _serverCmd = m_keyValue;

    return (isVersionOK) ? EKeyUpdateResult::Correct : EKeyUpdateResult::VersionMismatchError;
}

void SKeyValueRecord::deleteKeyValue()
{
    lock_guard<mutex> lock(m_mutex);

    m_deleted = true;
}

std::string SKeyValueRecord::getKeyValueString() const
{
    lock_guard<mutex> lock(m_mutex);

    stringstream ss;
    ss << m_keyValue.m_sKey << " --> " << m_keyValue.m_sValue;
    return ss.str();
}

/////////////////////////////////////
/////////////////////////////////////
/////////////////////////////////////

SPropertyRecord::SPropertyRecord()
{
}

SPropertyRecord::~SPropertyRecord()
{
}

void SPropertyRecord::addKeyValueRecord(uint64_t _taskID, SKeyValueRecord::ptr_t _keyValueRecord)
{
    m_taskMap.insert(pair<uint64_t, SKeyValueRecord::ptr_t>(_taskID, _keyValueRecord));
}

EKeyUpdateResult SPropertyRecord::updateKeyValue(const protocol_api::SUpdateKeyCmd& _cmd, SUpdateKeyCmd& _serverCmd)
{
    uint64_t taskID = _cmd.getTaskID();

    auto it = m_taskMap.find(taskID);
    if (it == m_taskMap.end())
    {
        LOG(fatal) << "SPropertyRecord: Key-value update failed because property doesn't exists in the container.";
        return EKeyUpdateResult::KeyNotFoundError;
    }
    return it->second->updateKeyValue(_cmd, _serverCmd);
}

std::string SPropertyRecord::getKeyValueString() const
{
    stringstream ss;
    for (const auto& v : m_taskMap)
    {
        ss << v.second->getKeyValueString() << "\n";
    }
    return ss.str();
}

/////////////////////////////////////
/////////////////////////////////////
/////////////////////////////////////

CKeyValueManager::CKeyValueManager()
    : m_propertyMap()
    , m_taskMap()

{
}

CKeyValueManager::~CKeyValueManager()
{
}

std::string CKeyValueManager::getKeyValueString(const std::string _propertyID) const
{
    stringstream ss;
    if (_propertyID.empty())
    {
        for (const auto& v : m_propertyMap)
        {
            ss << "[" << v.first << "]\n" << v.second->getKeyValueString();
        }
    }
    else
    {
        const auto v = m_propertyMap.find(_propertyID);
        if (v != m_propertyMap.end())
        {
            ss << "[" << v->first << "]\n" << v->second->getKeyValueString();
        }
        else
        {
            throw runtime_error("Wrong property name: " + _propertyID);
        }
    }
    return ss.str();
}

std::string CKeyValueManager::getPropertyString() const
{
    stringstream ss;
    for (const auto& v : m_propertyMap)
    {
        ss << v.first << "\n";
    }
    return ss.str();
}

void CKeyValueManager::initWithTopology(const CTopology& _topology)
{
    m_taskMap.clear();
    m_propertyMap.clear();

    initWithTopologyImpl(_topology, nullptr);
}

void CKeyValueManager::updateWithTopology(const topology_api::CTopology& _topology,
                                          const topology_api::CTopology::HashSet_t& _removedTasks,
                                          const topology_api::CTopology::HashSet_t& _addedTasks)
{
    // Erase removed tasks
    // TODO: remove tasks also from m_propertyMap
    for (auto taskID : _removedTasks)
    {
        m_taskMap.erase(taskID);
    }

    initWithTopologyImpl(_topology, &_addedTasks);
}

void CKeyValueManager::initWithTopologyImpl(const CTopology& _topology,
                                            const topology_api::CTopology::HashSet_t* _addedTasks)
{
    // Loop over all tasks. For each task loop over tasks's properties and insert them to map.
    CTopology::TaskInfoIteratorPair_t tasks = _topology.getTaskInfoIterator();
    for (auto it = tasks.first; it != tasks.second; it++)
    {
        uint64_t taskID = it->first;

        if (_addedTasks != nullptr && _addedTasks->find(taskID) == _addedTasks->end())
            continue;

        const STaskInfo& taskInfo = it->second;

        const TopoPropertyPtrVector_t& properties = taskInfo.m_task->getProperties();
        for (const auto& property : properties)
        {
            SKeyValueRecord::ptr_t kvPtr = make_shared<SKeyValueRecord>();
            m_taskMap.insert(make_pair(taskID, kvPtr));

            auto it = m_propertyMap.find(property->getId());
            if (it != m_propertyMap.end())
            {
                it->second->addKeyValueRecord(taskID, kvPtr);
            }
            else
            {
                SPropertyRecord::ptr_t propPtr = make_shared<SPropertyRecord>();
                m_propertyMap.insert(make_pair(property->getId(), propPtr));
                propPtr->addKeyValueRecord(taskID, kvPtr);
            }
        }
    }
}

EKeyUpdateResult CKeyValueManager::updateKeyValue(const SUpdateKeyCmd& _cmd, protocol_api::SUpdateKeyCmd& _serverCmd)
{
    string propertyID = _cmd.getPropertyID();

    auto it = m_propertyMap.find(propertyID);
    if (it == m_propertyMap.end())
    {
        LOG(fatal) << "CKeyValueManager: Key-value update failed because property doesn't exists in the container.";
        return EKeyUpdateResult::KeyNotFoundError;
    }
    return it->second->updateKeyValue(_cmd, _serverCmd);
}

void CKeyValueManager::deleteKeyValue(uint64_t _taskID)
{
    auto ret = m_taskMap.equal_range(_taskID);
    for (auto it = ret.first; it != ret.second; ++it)
    {
        it->second->deleteKeyValue();
    }
}
