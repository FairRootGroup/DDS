// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "SSHScheduler.h"
#include "TimeMeasure.h"
#include <set>
// BOOST
#include "boost/range/adaptor/map.hpp"
#include <boost/range/algorithm/copy.hpp>
#include <boost/regex.hpp>

using namespace dds;
using namespace dds::commander_cmd;
using namespace dds::topology_api;
using namespace dds::protocol_api;
using namespace std;
using namespace MiscCommon;

CSSHScheduler::CSSHScheduler()
{
}

CSSHScheduler::~CSSHScheduler()
{
}

void CSSHScheduler::makeSchedule(const CTopoCore& _topology, const weakChannelInfoVector_t& _channels)
{
    auto execTime = STimeMeasure<chrono::microseconds>::execution(
        [this, &_topology, &_channels]() { makeScheduleImpl(_topology, _channels, nullptr, nullptr); });
    LOG(info) << "Made schedule for tasks in " << execTime << " microsec.";
}

void CSSHScheduler::makeSchedule(const topology_api::CTopoCore& _topology,
                                 const weakChannelInfoVector_t& _channels,
                                 const CTopoCore::IdSet_t& _addedTasks,
                                 const CTopoCore::IdSet_t& _addedCollections)
{
    auto execTime = STimeMeasure<chrono::microseconds>::execution(
        [this, &_topology, &_channels, &_addedTasks, &_addedCollections]() {
            makeScheduleImpl(_topology, _channels, &_addedTasks, &_addedCollections);
        });
    LOG(info) << "Made schedule for tasks in " << execTime << " microsec.";
}

void CSSHScheduler::makeScheduleImpl(const CTopoCore& _topology,
                                     const weakChannelInfoVector_t& _channels,
                                     const CTopoCore::IdSet_t* _addedTasks,
                                     const CTopoCore::IdSet_t* _addedCollections)
{
    m_schedule.clear();

    size_t nofChannels = _channels.size();
    // Map pair<host name, worker id> to vector of channel indeces.
    // This is needed in order to reduce number of regex matches and speed up scheduling.
    hostToChannelMap_t hostToChannelMap;
    for (size_t iChannel = 0; iChannel < nofChannels; ++iChannel)
    {
        const auto& v = _channels[iChannel];
        if (v.m_channel.expired())
            continue;
        auto ptr = v.m_channel.lock();

        SAgentInfo info = ptr->getAgentInfo(v.m_protocolHeaderID);
        // Only idle DDS agents
        if (info.m_state != EAgentState::idle)
            continue;

        const SHostInfoCmd& hostInfo = info.m_remoteHostInfo;
        hostToChannelMap[make_pair(hostInfo.m_host, hostInfo.m_workerId)].push_back(iChannel);
    }

    // TODO: before scheduling the collections we have to sort them by a number of tasks in the collection.
    // TODO: for the moment we are not able to schedule collection without requirements.

    // Collect all tasks that belong to collections
    set<uint64_t> tasksInCollections;
    CollectionMap_t collectionMap;
    STopoRuntimeCollection::FilterIteratorPair_t collections = _topology.getRuntimeCollectionIterator();
    for (auto it = collections.first; it != collections.second; it++)
    {
        // Only collections that were added has to be scheduled
        if (_addedCollections != nullptr && _addedCollections->find(it->first) == _addedCollections->end())
            continue;

        const auto& taskMap = it->second.m_idToRuntimeTaskMap;
        boost::copy(taskMap | boost::adaptors::map_keys, std::inserter(tasksInCollections, tasksInCollections.end()));
        collectionMap[it->second.m_idToRuntimeTaskMap.size()].push_back(it->first);
    }

    set<uint64_t> scheduledTasks;

    scheduleCollections(_topology, _channels, hostToChannelMap, scheduledTasks, collectionMap, true);
    scheduleTasks(_topology, _channels, hostToChannelMap, scheduledTasks, tasksInCollections, true, _addedTasks);
    scheduleCollections(_topology, _channels, hostToChannelMap, scheduledTasks, collectionMap, false);
    scheduleTasks(_topology, _channels, hostToChannelMap, scheduledTasks, tasksInCollections, false, _addedTasks);

    size_t totalNofTasks =
        (_addedTasks == nullptr) ? _topology.getMainGroup()->getTotalNofTasks() : _addedTasks->size();
    if (totalNofTasks != m_schedule.size())
    {
        LOG(debug) << toString();
        stringstream ss;
        ss << "Unable to make a schedule for tasks. Number of requested tasks: " << totalNofTasks
           << ". Number of scheduled tasks: " << m_schedule.size();
        throw runtime_error(ss.str());
    }

    LOG(debug) << toString();
}

void CSSHScheduler::scheduleTasks(const CTopoCore& _topology,
                                  const weakChannelInfoVector_t& _channels,
                                  hostToChannelMap_t& _hostToChannelMap,
                                  set<uint64_t>& _scheduledTasks,
                                  const set<uint64_t>& _tasksInCollections,
                                  bool useRequirement,
                                  const CTopoCore::IdSet_t* _addedTasks)
{
    STopoRuntimeTask::FilterIteratorPair_t tasks = _topology.getRuntimeTaskIterator();
    for (auto it = tasks.first; it != tasks.second; it++)
    {
        uint64_t id = it->first;

        // Check if tasks is in the added tasks
        if (_addedTasks != nullptr && _addedTasks->find(it->first) == _addedTasks->end())
            continue;

        // Check if task has to be scheduled in the collection
        if (_tasksInCollections.find(id) != _tasksInCollections.end())
            continue;

        // Check if task was already scheduled in the collection
        if (_scheduledTasks.find(id) != _scheduledTasks.end())
            continue;

        CTopoTask::Ptr_t task = it->second.m_task;

        // First path only for tasks with requirements;
        // Second path for tasks without requirements.

        // SSH scheduler doesn't support multiple requirements
        if (task->getNofRequirements() > 1)
        {
            stringstream ss;
            ss << "Unable to schedule task <" << id << "> with path " << task->getPath()
               << ": SSH scheduler doesn't support multiple requirements.";
            throw runtime_error(ss.str());
        }

        CTopoRequirement::Ptr_t requirement = (task->getNofRequirements() == 1) ? task->getRequirements()[0] : nullptr;
        if ((useRequirement && requirement == nullptr) || (!useRequirement && requirement != nullptr))
            continue;

        bool taskAssigned = false;

        for (auto& v : _hostToChannelMap)
        {
            const bool requirementOk = checkRequirement(requirement, useRequirement, v.first.first, v.first.second);
            if (requirementOk)
            {
                if (!v.second.empty())
                {
                    size_t channelIndex = v.second.back();
                    const auto& channel = _channels[channelIndex];

                    SSchedule schedule;
                    schedule.m_weakChannelInfo = channel;
                    schedule.m_taskInfo = it->second;
                    schedule.m_taskID = id;
                    m_schedule.push_back(schedule);

                    v.second.pop_back();

                    taskAssigned = true;

                    break;
                }
            }
        }
        if (!taskAssigned)
        {
            LOG(debug) << toString();
            stringstream ss;
            string requirementStr = (useRequirement)
                                        ? ("Requirement " + requirement->getName() + " couldn't be satisfied.")
                                        : "Not enough worker nodes.";
            ss << "Unable to schedule task <" << id << "> with path " << task->getPath() << ".\n" << requirementStr;
            throw runtime_error(ss.str());
        }
    }
}

void CSSHScheduler::scheduleCollections(const CTopoCore& _topology,
                                        const weakChannelInfoVector_t& _channels,
                                        hostToChannelMap_t& _hostToChannelMap,
                                        set<uint64_t>& _scheduledTasks,
                                        const CollectionMap_t& _collectionMap,
                                        bool useRequirement)
{
    for (const auto& it_col : _collectionMap)
    {
        for (auto id : it_col.second)
        {
            const STopoRuntimeCollection& collectionInfo = _topology.getRuntimeCollectionById(id);

            // First path only for collections with requirements;
            // Second path for collections without requirements.

            // SSH scheduler doesn't support multiple requirements
            if (collectionInfo.m_collection->getNofRequirements() > 1)
            {
                stringstream ss;
                ss << "Unable to schedule collection <" << id << "> with path "
                   << collectionInfo.m_collection->getPath()
                   << ": SSH scheduler doesn't support multiple requirements.";
                throw runtime_error(ss.str());
            }

            CTopoRequirement::Ptr_t requirement = (collectionInfo.m_collection->getNofRequirements() == 1)
                                                      ? collectionInfo.m_collection->getRequirements()[0]
                                                      : nullptr;
            if ((useRequirement && requirement == nullptr) || (!useRequirement && requirement != nullptr))
                continue;

            bool collectionAssigned = false;

            for (auto& v : _hostToChannelMap)
            {
                const bool requirementOk = checkRequirement(requirement, useRequirement, v.first.first, v.first.second);
                if ((v.second.size() >= collectionInfo.m_collection->getNofTasks()) && requirementOk)
                {
                    const STopoRuntimeCollection& collectionInfo = _topology.getRuntimeCollectionById(id);

                    for (auto taskIt : collectionInfo.m_idToRuntimeTaskMap)
                    {
                        const STopoRuntimeTask& info = taskIt.second;

                        size_t channelIndex = v.second.back();
                        const auto& channel = _channels[channelIndex];

                        SSchedule schedule;
                        schedule.m_weakChannelInfo = channel;
                        schedule.m_taskInfo = info;
                        schedule.m_taskID = taskIt.first;
                        m_schedule.push_back(schedule);

                        v.second.pop_back();

                        _scheduledTasks.insert(taskIt.first);
                    }
                    collectionAssigned = true;
                    break;
                }
            }

            if (!collectionAssigned)
            {
                LOG(debug) << toString();
                stringstream ss;
                ss << "Unable to schedule collection <" << id << "> with path "
                   << collectionInfo.m_collection->getPath();
                throw runtime_error(ss.str());
            }
        }
    }
}

bool CSSHScheduler::checkRequirement(CTopoRequirement::Ptr_t _requirement,
                                     bool _useRequirement,
                                     const string& _hostName,
                                     const string& _wnName) const
{
    if (_useRequirement && (_requirement->getRequirementType() == CTopoRequirement::EType::WnName) && _wnName.empty())
    {
        LOG(warning) << "Requirement of type WnName is not supported for this RMS plug-in. Requirement: "
                     << _requirement->toString();
        return true;
    }
    return !_useRequirement ||
           (_useRequirement &&
            CSSHScheduler::hostPatternMatches(
                _requirement->getValue(),
                (_requirement->getRequirementType() == CTopoRequirement::EType::HostName) ? _hostName : _wnName));
}

const CSSHScheduler::ScheduleVector_t& CSSHScheduler::getSchedule() const
{
    return m_schedule;
}

bool CSSHScheduler::hostPatternMatches(const string& _hostPattern, const string& _host)
{
    if (_hostPattern.empty())
        return true;
    const boost::regex e(_hostPattern);
    return boost::regex_match(_host, e);
}

string CSSHScheduler::toString()
{
    stringstream ss;
    ss << "Scheduled tasks: " << m_schedule.size() << endl;
    for (const auto& s : m_schedule)
    {
        if (s.m_weakChannelInfo.m_channel.expired())
            continue;
        auto ptr = s.m_weakChannelInfo.m_channel.lock();

        SAgentInfo info = ptr->getAgentInfo(s.m_weakChannelInfo.m_protocolHeaderID);
        ss << "<" << s.m_taskID << ">"
           << " <" << s.m_taskInfo.m_task->getPath() << "> ---> " << info.m_remoteHostInfo.m_host << endl;
    }
    return ss.str();
}
