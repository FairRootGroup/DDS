// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__CScheduler__
#define __DDS__CScheduler__

// DDS
#include "AgentChannel.h"
#include "ChannelInfo.h"
#include "TopoCore.h"
#include "TopoTask.h"
// STD
#include <vector>

namespace dds
{
    namespace commander_cmd
    {
        class CScheduler
        {
          public:
            struct SSchedule
            {
                SSchedule()
                    : m_taskID(0)
                {
                }

                uint64_t m_taskID;
                topology_api::STopoRuntimeTask m_taskInfo;
                dds::protocol_api::SWeakChannelInfo<CAgentChannel> m_weakChannelInfo;
            };

            using ScheduleVector_t = std::vector<SSchedule>;
            using CollectionMap_t = std::map<size_t, std::vector<uint64_t>, std::greater<size_t>>;
            using weakChannelInfoVector_t = std::vector<dds::protocol_api::SWeakChannelInfo<CAgentChannel>>;

          private:
            // Map tuple<agent ID, host name, worker id, group name> to vector of channel indexes.
            using hostToChannelMap_t =
                std::map<std::tuple<uint64_t, std::string, std::string, std::string>, std::vector<size_t>>;
            // Map pair<host name, task/collection name> to counter.
            using hostCounterMap_t = std::map<std::pair<std::string, std::string>, size_t>;

          public:
            CScheduler();
            ~CScheduler();

            void makeSchedule(topology_api::CTopoCore& _topology, const weakChannelInfoVector_t& _channels);

            void makeSchedule(topology_api::CTopoCore& _topology,
                              const weakChannelInfoVector_t& _channels,
                              const topology_api::CTopoCore::IdSet_t& _addedTasks,
                              const topology_api::CTopoCore::IdSet_t& _addedCollections);

            const ScheduleVector_t& getSchedule() const;

            static bool hostPatternMatches(const std::string& _hostPattern, const std::string& _host);

            std::string toString();

          private:
            void makeScheduleImpl(topology_api::CTopoCore& _topology,
                                  const weakChannelInfoVector_t& _channels,
                                  const topology_api::CTopoCore::IdSet_t* _addedTasks,
                                  const topology_api::CTopoCore::IdSet_t* _addedCollections);

            void scheduleCollections(topology_api::CTopoCore& _topology,
                                     const weakChannelInfoVector_t& _channels,
                                     hostToChannelMap_t& _hostToChannelMap,
                                     std::set<uint64_t>& _scheduledTasks,
                                     const CollectionMap_t& _collectionMap,
                                     size_t _numRequirements,
                                     hostCounterMap_t& _hostCounterMap);

            void scheduleTasks(const topology_api::CTopoCore& _topology,
                               const weakChannelInfoVector_t& _channels,
                               hostToChannelMap_t& _hostToChannelMap,
                               std::set<uint64_t>& _scheduledTasks,
                               const std::set<uint64_t>& _tasksInCollections,
                               size_t _numRequirements,
                               const topology_api::CTopoCore::IdSet_t* _addedTasks,
                               hostCounterMap_t& _hostCounterMap);

            // TODO: host, wn and group names should be provided via a struct.
            bool checkRequirements(const topology_api::CTopoRequirement::PtrVector_t& _requirements,
                                   const std::string& _hostName,
                                   const std::string& _wnName,
                                   const std::string& _groupName,
                                   const std::string& _elementName,
                                   hostCounterMap_t& _hostCounterMap) const;

          private:
            ScheduleVector_t m_schedule;
        };
    } // namespace commander_cmd
} // namespace dds

#endif /* defined(__DDS__CScheduler__) */
