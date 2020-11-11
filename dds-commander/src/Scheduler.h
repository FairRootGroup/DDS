// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__SSHScheduler__
#define __DDS__SSHScheduler__

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

            typedef std::vector<SSchedule> ScheduleVector_t;
            typedef std::map<size_t, std::vector<uint64_t>, std::greater<size_t>> CollectionMap_t;
            typedef std::vector<dds::protocol_api::SWeakChannelInfo<CAgentChannel>> weakChannelInfoVector_t;

          private:
            // Map tuple<agent ID, host name, worker id> to vector of channel indeces.
            typedef std::map<std::tuple<uint64_t, std::string, std::string>, std::vector<size_t>> hostToChannelMap_t;
            // Map pair<host name, task/collection name> to counter.
            typedef std::map<std::pair<std::string, std::string>, size_t> hostCounterMap_t;

          public:
            CScheduler();
            ~CScheduler();

            void makeSchedule(const topology_api::CTopoCore& _topology, const weakChannelInfoVector_t& _channels);

            void makeSchedule(const topology_api::CTopoCore& _topology,
                              const weakChannelInfoVector_t& _channels,
                              const topology_api::CTopoCore::IdSet_t& _addedTasks,
                              const topology_api::CTopoCore::IdSet_t& _addedCollections);

            const ScheduleVector_t& getSchedule() const;

            static bool hostPatternMatches(const std::string& _hostPattern, const std::string& _host);

            std::string toString();

          private:
            void makeScheduleImpl(const topology_api::CTopoCore& _topology,
                                  const weakChannelInfoVector_t& _channels,
                                  const topology_api::CTopoCore::IdSet_t* _addedTasks,
                                  const topology_api::CTopoCore::IdSet_t* _addedCollections);

            void scheduleCollections(const topology_api::CTopoCore& _topology,
                                     const weakChannelInfoVector_t& _channels,
                                     hostToChannelMap_t& _hostToChannelMap,
                                     std::set<uint64_t>& _scheduledTasks,
                                     const CollectionMap_t& _collectionMap,
                                     bool useRequirement,
                                     hostCounterMap_t& _hostCounterMap);

            void scheduleTasks(const topology_api::CTopoCore& _topology,
                               const weakChannelInfoVector_t& _channels,
                               hostToChannelMap_t& _hostToChannelMap,
                               std::set<uint64_t>& _scheduledTasks,
                               const std::set<uint64_t>& _tasksInCollections,
                               bool useRequirement,
                               const topology_api::CTopoCore::IdSet_t* _addedTasks,
                               hostCounterMap_t& _hostCounterMap);

            bool checkRequirement(topology_api::CTopoRequirement::Ptr_t _requirement,
                                  bool _useRequirement,
                                  const std::string& _hostName,
                                  const std::string& _wnName,
                                  const std::string& _elementName,
                                  hostCounterMap_t& _hostCounterMap) const;

          private:
            ScheduleVector_t m_schedule;
        };
    } // namespace commander_cmd
} // namespace dds

#endif /* defined(__DDS__SSHScheduler__) */
