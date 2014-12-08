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

      public:
        CSSHScheduler();
        ~CSSHScheduler();

        void makeSchedule(const CTopology& _topology, const CAgentChannel::weakConnectionPtrVector_t& _channels);

        const ScheduleVector_t& getSchedule() const;

      private:
        void printSchedule();

      private:
        ScheduleVector_t m_schedule;
    };
}

#endif /* defined(__DDS__SSHScheduler__) */
