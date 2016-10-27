// Copyright 2014 GSI, Inc. All rights reserved.
//
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
    namespace commander_cmd
    {
        class CSSHScheduler
        {
          public:
            struct SSchedule
            {
                SSchedule()
                    : m_taskID(0)
                    , m_taskInfo()
                    , m_channel()
                {
                }

                uint64_t m_taskID;
                topology_api::STaskInfo m_taskInfo;
                CAgentChannel::weakConnectionPtr_t m_channel;
            };

            typedef std::vector<SSchedule> ScheduleVector_t;
            typedef std::map<size_t, std::vector<uint64_t>, std::greater<size_t>> CollectionMap_t;

          private:
            // Map pair<host name, worker id> to vector of channel indeces.
            typedef std::map<std::pair<std::string, std::string>, std::vector<size_t>> hostToChannelMap_t;

          public:
            CSSHScheduler();
            ~CSSHScheduler();

            void makeSchedule(const topology_api::CTopology& _topology,
                              const CAgentChannel::weakConnectionPtrVector_t& _channels);

            void makeSchedule(const topology_api::CTopology& _topology,
                              const CAgentChannel::weakConnectionPtrVector_t& _channels,
                              const topology_api::CTopology::HashSet_t& _addedTasks,
                              const topology_api::CTopology::HashSet_t& _addedCollections);

            const ScheduleVector_t& getSchedule() const;

            std::string toString();

          private:
            void makeScheduleImpl(const topology_api::CTopology& _topology,
                                  const CAgentChannel::weakConnectionPtrVector_t& _channels,
                                  const topology_api::CTopology::HashSet_t* _addedTasks,
                                  const topology_api::CTopology::HashSet_t* _addedCollections);

            void scheduleCollections(const topology_api::CTopology& _topology,
                                     const CAgentChannel::weakConnectionPtrVector_t& _channels,
                                     hostToChannelMap_t& _hostToChannelMap,
                                     std::set<uint64_t>& _scheduledTasks,
                                     const CollectionMap_t& _collectionMap,
                                     bool useRequirement);

            void scheduleTasks(const topology_api::CTopology& _topology,
                               const CAgentChannel::weakConnectionPtrVector_t& _channels,
                               hostToChannelMap_t& _hostToChannelMap,
                               std::set<uint64_t>& _scheduledTasks,
                               const std::set<uint64_t>& _tasksInCollections,
                               bool useRequirement,
                               const topology_api::CTopology::HashSet_t* _addedTasks);

          private:
            ScheduleVector_t m_schedule;
        };
    }
}

#endif /* defined(__DDS__SSHScheduler__) */
