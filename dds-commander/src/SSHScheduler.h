//
//  SSHScheduler.h
//  DDS
//
//  Created by Andrey Lebedev on 28/11/14.
//
//

#ifndef __DDS__SSHScheduler__
#define __DDS__SSHScheduler__

// DDS
#include "AgentChannel.h"
#include "Task.h"
#include "Topology.h"
// STD
#include <vector>

namespace dds
{
    class CSSHScheduler
    {
      public:
        struct SSchedule
        {
            uint64_t m_taskID;
            TaskPtr_t m_task;
            CAgentChannel::weakConnectionPtr_t m_channel;
        };

        typedef std::vector<SSchedule> ScheduleVector_t;
        typedef std::map<size_t, std::vector<uint64_t>, std::greater<size_t>> CollectionMap_t;

      public:
        CSSHScheduler();
        ~CSSHScheduler();

        void makeSchedule(const CTopology& _topology, const CAgentChannel::weakConnectionPtrVector_t& _channels);

        const ScheduleVector_t& getSchedule() const;

        void printSchedule();

      private:
        void makeScheduleImpl(const CTopology& _topology, const CAgentChannel::weakConnectionPtrVector_t& _channels);

        void scheduleCollections(const CTopology& _topology,
                                 const CAgentChannel::weakConnectionPtrVector_t& _channels,
                                 std::map<std::string, std::vector<size_t>>& _hostToChannelMap,
                                 std::set<uint64_t>& _scheduledTasks,
                                 const CollectionMap_t& _collectionMap,
                                 bool useRequirement);

        void scheduleTasks(const dds::CTopology& _topology,
                           const CAgentChannel::weakConnectionPtrVector_t& _channels,
                           std::map<std::string, std::vector<size_t>>& _hostToChannelMap,
                           std::set<uint64_t>& _scheduledTasks,
                           const std::set<uint64_t>& _tasksInCollections,
                           bool useRequirement);

      private:
        ScheduleVector_t m_schedule;
    };
}

#endif /* defined(__DDS__SSHScheduler__) */
