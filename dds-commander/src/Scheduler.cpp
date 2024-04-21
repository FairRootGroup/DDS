// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "Scheduler.h"
#include "TimeMeasure.h"
// STD
#include <iomanip>
#include <set>
// BOOST
#include "boost/range/adaptor/map.hpp"
#include <boost/algorithm/string/join.hpp>
#include <boost/range/algorithm/copy.hpp>
#include <boost/regex.hpp>

using namespace dds;
using namespace dds::commander_cmd;
using namespace dds::topology_api;
using namespace dds::protocol_api;
using namespace dds::misc;
using namespace std;

CScheduler::CScheduler()
{
}

CScheduler::~CScheduler()
{
}

void CScheduler::makeSchedule(CTopoCore& _topology, const weakChannelInfoVector_t& _channels)
{
    auto execTime = STimeMeasure<chrono::microseconds>::execution(
        [this, &_topology, &_channels]() { makeScheduleImpl(_topology, _channels, nullptr, nullptr); });
    LOG(info) << "Made schedule for tasks in " << execTime << " microsec.";
}

void CScheduler::makeSchedule(topology_api::CTopoCore& _topology,
                              const weakChannelInfoVector_t& _channels,
                              const CTopoCore::IdSet_t& _addedTasks,
                              const CTopoCore::IdSet_t& _addedCollections)
{
    auto execTime = STimeMeasure<chrono::microseconds>::execution(
        [this, &_topology, &_channels, &_addedTasks, &_addedCollections]()
        { makeScheduleImpl(_topology, _channels, &_addedTasks, &_addedCollections); });
    LOG(info) << "Made schedule for tasks in " << execTime << " microsec.";
}

void CScheduler::makeScheduleImpl(CTopoCore& _topology,
                                  const weakChannelInfoVector_t& _channels,
                                  const CTopoCore::IdSet_t* _addedTasks,
                                  const CTopoCore::IdSet_t* _addedCollections)
{
    m_schedule.clear();

    size_t nofChannels{ _channels.size() };
    // Map pair<host name, worker id> to vector of channel indexes.
    // This is needed in order to reduce number of regex matches and speed up scheduling.
    hostToChannelMap_t hostToChannelMap;
    for (size_t iChannel = 0; iChannel < nofChannels; ++iChannel)
    {
        const auto& v = _channels[iChannel];
        auto ptr = v.m_channel.lock();
        if (ptr == nullptr)
            continue;

        SAgentInfo& info{ ptr->getAgentInfo() };
        SSlotInfo& slot{ info.getSlotByID(v.m_protocolHeaderID) };
        // Only idle DDS agents
        if (slot.m_state != EAgentState::idle)
            continue;

        const SHostInfoCmd& hostInfo = info.m_remoteHostInfo;
        hostToChannelMap[make_tuple(info.m_id, hostInfo.m_host, hostInfo.m_workerId, hostInfo.m_groupName)].push_back(
            iChannel);
    }

    // Collect all tasks that belong to collections
    set<uint64_t> tasksInCollections;
    CollectionMap_t collectionMap;
    auto collections{ _topology.getRuntimeCollectionIterator() };
    for (auto it = collections.first; it != collections.second; it++)
    {
        // Only collections that were added has to be scheduled
        if (_addedCollections != nullptr && _addedCollections->find(it->first) == _addedCollections->end())
            continue;

        const auto& taskMap = it->second.m_idToRuntimeTaskMap;
        boost::copy(taskMap | boost::adaptors::map_keys, std::inserter(tasksInCollections, tasksInCollections.end()));
        collectionMap[it->second.m_idToRuntimeTaskMap.size()].push_back(it->first);
    }

    // Maximum number of requirements per task or collection
    size_t maxRequirements{ 0 };
    auto tasks{ _topology.getRuntimeTaskIterator() };
    for (auto it = tasks.first; it != tasks.second; it++)
    {
        maxRequirements = max(maxRequirements, it->second.m_task->getNofRequirements());
    }
    for (auto it = collections.first; it != collections.second; it++)
    {
        maxRequirements = max(maxRequirements, it->second.m_collection->getNofRequirements());
    }

    // Counters of number of instances of tasks/collections on a host.
    hostCounterMap_t taskHostCounter;
    hostCounterMap_t collectionHostCounter;

    set<uint64_t> scheduledTasks;

    // First schedule collections and tasks with more requirements
    for (int i = maxRequirements; i >= 0; i--)
    {
        scheduleCollections(
            _topology, _channels, hostToChannelMap, scheduledTasks, collectionMap, i, collectionHostCounter);
        scheduleTasks(_topology,
                      _channels,
                      hostToChannelMap,
                      scheduledTasks,
                      tasksInCollections,
                      i,
                      _addedTasks,
                      taskHostCounter);
    }

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

void CScheduler::scheduleTasks(const CTopoCore& _topology,
                               const weakChannelInfoVector_t& _channels,
                               hostToChannelMap_t& _hostToChannelMap,
                               set<uint64_t>& _scheduledTasks,
                               const set<uint64_t>& _tasksInCollections,
                               size_t _numRequirements,
                               const CTopoCore::IdSet_t* _addedTasks,
                               hostCounterMap_t& _hostCounterMap)
{
    STopoRuntimeTask::FilterIteratorPair_t tasks{ _topology.getRuntimeTaskIterator() };
    for (auto it = tasks.first; it != tasks.second; it++)
    {
        uint64_t id{ it->first };

        // Check if tasks is in the added tasks
        if (_addedTasks != nullptr && _addedTasks->find(it->first) == _addedTasks->end())
            continue;

        // Check if task has to be scheduled in the collection
        if (_tasksInCollections.find(id) != _tasksInCollections.end())
            continue;

        // Check if task was already scheduled in the collection
        if (_scheduledTasks.find(id) != _scheduledTasks.end())
            continue;

        CTopoTask::Ptr_t task{ it->second.m_task };

        if (task->getNofRequirements() != _numRequirements)
            continue;

        bool taskAssigned{ false };

        for (auto& v : _hostToChannelMap)
        {
            const string hostName{ std::get<1>(v.first) };
            const string wnName{ std::get<2>(v.first) };
            const string groupName{ std::get<3>(v.first) };
            const bool requirementOk{ checkRequirements(
                task->getRequirements(), hostName, wnName, groupName, task->getName(), _hostCounterMap) };
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

                    // Increase counter of task on the host
                    _hostCounterMap[make_pair(hostName, task->getName())]++;

                    taskAssigned = true;

                    break;
                }
            }
        }
        if (!taskAssigned)
        {
            LOG(debug) << toString();
            stringstream ss;
            string requirementStr{ "Not enough worker nodes." };
            if (_numRequirements > 0)
            {
                vector<string> strs;
                transform(task->getRequirements().begin(),
                          task->getRequirements().end(),
                          back_inserter(strs),
                          [](const CTopoRequirement::Ptr_t _requirement) -> string
                          { return _requirement->toString(); });
                requirementStr = "Cannot satisfy requirements (" + boost::algorithm::join(strs, ", ") + ").";
            }
            ss << "Unable to schedule task <" << id << "> with path " << task->getPath() << ": " << requirementStr;
            throw runtime_error(ss.str());
        }
    }
}

void CScheduler::scheduleCollections(CTopoCore& _topology,
                                     const weakChannelInfoVector_t& _channels,
                                     hostToChannelMap_t& _hostToChannelMap,
                                     set<uint64_t>& _scheduledTasks,
                                     const CollectionMap_t& _collectionMap,
                                     size_t _numRequirements,
                                     hostCounterMap_t& _hostCounterMap)
{
    for (const auto& it_col : _collectionMap)
    {
        for (auto id : it_col.second)
        {
            const STopoRuntimeCollection& collectionInfo = _topology.getRuntimeCollectionById(id);
            auto collection{ collectionInfo.m_collection };

            if (collection->getNofRequirements() != _numRequirements)
                continue;

            bool collectionAssigned{ false };

            for (auto& v : _hostToChannelMap)
            {
                const string hostName{ std::get<1>(v.first) };
                const string wnName{ std::get<2>(v.first) };
                const string groupName{ std::get<3>(v.first) };
                const bool requirementOk{ checkRequirements(collection->getRequirements(),
                                                            hostName,
                                                            wnName,
                                                            groupName,
                                                            collection->getName(),
                                                            _hostCounterMap) };
                if ((v.second.size() >= collectionInfo.m_collection->getNofTasks()) && requirementOk)
                {
                    const STopoRuntimeCollection& collectionInfo{ _topology.getRuntimeCollectionById(id) };

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

                    // Increase counter of collection on the host
                    _hostCounterMap[make_pair(hostName, collection->getName())]++;

                    collectionAssigned = true;
                    break;
                }
            }

            if (!collectionAssigned)
            {
                LOG(debug) << toString();
                stringstream ss;
                ss << "Unable to schedule collection <" << id << "> with path " << collection->getPath();
                throw runtime_error(ss.str());
            }
        }
    }
}

bool CScheduler::checkRequirements(const topology_api::CTopoRequirement::PtrVector_t& _requirements,
                                   const string& _hostName,
                                   const string& _wnName,
                                   const string& _groupName,
                                   const string& _elementName,
                                   hostCounterMap_t& _hostCounterMap) const
{
    if (_requirements.empty())
        return true;

    using EType = CTopoRequirement::EType;

    bool result{ false };
    for (const auto& requirement : _requirements)
    {
        const auto type{ requirement->getRequirementType() };

        if (type == EType::WnName && _wnName.empty())
        {
            LOG(warning) << "Requirement of type WnName is not supported for this RMS plug-in. Requirement: "
                         << requirement->toString();
            result = true;
        }
        else if (type == EType::WnName || type == EType::HostName)
        {
            result = CScheduler::hostPatternMatches(requirement->getValue(),
                                                    (type == EType::HostName) ? _hostName : _wnName);
        }
        else if (type == EType::GroupName)
        {
            result = CScheduler::hostPatternMatches(requirement->getValue(), _groupName);
        }
        else if (type == EType::MaxInstancesPerHost)
        {
            try
            {
                size_t value = boost::lexical_cast<size_t>(requirement->getValue());
                const auto key{ make_pair(_hostName, _elementName) };
                auto it = _hostCounterMap.find(key);
                if (it != _hostCounterMap.end())
                {
                    result = it->second < value;
                }
                else
                {
                    result = true;
                }
            }
            catch (boost::bad_lexical_cast&)
            {
                stringstream ss;
                ss << "Unable to satisfy the requirement " << requirement->getName() << ". Value "
                   << requirement->getValue() << " must be a positive number.";
                throw runtime_error(ss.str());
            }
        }
        else if (type == EType::Custom)
        {
            // ignore custom requirements
            result = true;
        }

        // All requirements must be satisfied at the same time
        if (!result)
            break;
    }

    return result;
}

const CScheduler::ScheduleVector_t& CScheduler::getSchedule() const
{
    return m_schedule;
}

bool CScheduler::hostPatternMatches(const string& _hostPattern, const string& _host)
{
    if (_hostPattern.empty())
        return true;
    const boost::regex e(_hostPattern);
    return boost::regex_match(_host, e);
}

string CScheduler::toString()
{
    stringstream ss;
    ss << "Scheduled tasks: " << m_schedule.size() << endl;
    for (const auto& s : m_schedule)
    {
        auto ptr = s.m_weakChannelInfo.m_channel.lock();
        if (ptr == nullptr)
            continue;

        const SAgentInfo& info = ptr->getAgentInfo();
        ss << "<" << s.m_taskID << ">"
           << " <" << s.m_taskInfo.m_task->getPath() << "> ---> " << info.m_remoteHostInfo.m_host << endl;
    }
    return ss.str();
}
