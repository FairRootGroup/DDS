//
//  SSHScheduler.cpp
//  DDS
//
//  Created by Andrey Lebedev on 28/11/14.
//
//

#include "SSHScheduler.h"
#include <set>

using namespace dds;
using namespace std;

CSSHScheduler::CSSHScheduler()
{
}

CSSHScheduler::~CSSHScheduler()
{
}

void CSSHScheduler::makeSchedule(const CTopology& _topology, const CAgentChannel::weakConnectionPtrVector_t& _channels)
{
    m_schedule.clear();

    CTopology::TaskIteratorPair_t tasks = _topology.getTaskIterator();
    set<size_t> usedChannels;

    size_t taskCounter = 0;

    // TODO: refactor this code to avoid dublication, probably move some common functionality to a function.

    // Tasks with requirements
    for (auto it = tasks.first; it != tasks.second; it++)
    {
        uint64_t id = it->first;
        TaskPtr_t task = it->second;

        // First path only for tasks with requirements;
        if (task->getRequirement() == nullptr)
            continue;

        bool taskAssigned = false;
        // Find first matched channel host
        size_t nofChannels = _channels.size();
        for (size_t iChannel = 0; iChannel < nofChannels; ++iChannel)
        {
            const auto& v = _channels[iChannel];
            if (v.expired())
                continue;
            auto ptr = v.lock();

            const SHostInfoCmd& hostInfo = ptr->getRemoteHostInfo();

            bool hostMatches =
                (task->getRequirement() == nullptr || task->getRequirement()->hostPatterMatches(hostInfo.m_host)) &&
                (usedChannels.find(iChannel) == usedChannels.end());
            if (hostMatches)
            {
                usedChannels.insert(iChannel);

                SSchedule schedule;
                schedule.m_channel = v;
                schedule.m_task = task;
                schedule.m_taskID = id;
                m_schedule.push_back(schedule);

                taskAssigned = true;

                break;
            }
        }
        if (!taskAssigned)
        {
            stringstream ss;
            ss << "Unable to schedule task <" << id << "> with path " << task->getPath();
            throw runtime_error(ss.str());
        }

        ++taskCounter;
    }

    // Tasks without requirements
    for (auto it = tasks.first; it != tasks.second; it++)
    {
        uint64_t id = it->first;
        TaskPtr_t task = it->second;

        // First path only for tasks without requirements;
        if (task->getRequirement() != nullptr)
            continue;

        bool taskAssigned = false;
        // Find first matched channel host
        size_t nofChannels = _channels.size();
        for (size_t iChannel = 0; iChannel < nofChannels; ++iChannel)
        {
            const auto& v = _channels[iChannel];
            if (v.expired())
                continue;
            auto ptr = v.lock();

            bool hostMatches = (usedChannels.find(iChannel) == usedChannels.end());
            if (hostMatches)
            {
                usedChannels.insert(iChannel);

                SSchedule schedule;
                schedule.m_channel = v;
                schedule.m_task = task;
                schedule.m_taskID = id;
                m_schedule.push_back(schedule);

                taskAssigned = true;

                break;
            }
        }
        if (!taskAssigned)
        {
            stringstream ss;
            ss << "Unable to schedule task <" << id << "> with path " << task->getPath();
            throw runtime_error(ss.str());
        }

        ++taskCounter;
    }

    if (taskCounter != m_schedule.size())
    {
        stringstream ss;
        ss << "Unable to make a schedule for tasks. Number of requested tasks: " << taskCounter
           << ". Number of scheduled tasks: " << m_schedule.size();
        throw runtime_error(ss.str());
    }
}

const CSSHScheduler::ScheduleVector_t& CSSHScheduler::getSchedule() const
{
    return m_schedule;
}