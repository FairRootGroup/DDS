// Copyright 2016 GSI, Inc. All rights reserved.
//
//
//
#ifndef DDS_OCTOPUS_TASK_H
#define DDS_OCTOPUS_TASK_H

// DDS
#include "OctopusProtocol.h"
#include "OctopusTestImpl.h"
#include "dds_intercom.h"

namespace dds
{
    namespace dds_octopus_task
    {
        class COctopusTask
        {
          public:
            COctopusTask();
            ~COctopusTask();

            BEGIN_OCTOPUS_MSG_MAP("OctopusTask")
                OCTOPUS_MSG_HANDLER(onGetPingCmd, dds_octopus::SOctopusProtocol_GetPing)
                OCTOPUS_MSG_HANDLER(onBigCmdCmd, dds_octopus::SOctopusProtocol_BigCmd)
            END_OCTOPUS_MSG_MAP

          public:
            void init();
            void onGetPingCmd(const dds_octopus::SOctopusProtocol_GetPing& _cmd, uint64_t _senderId);
            void onBigCmdCmd(const dds_octopus::SOctopusProtocol_BigCmd& _cmd, uint64_t _senderId);

          private:
            dds::intercom_api::CIntercomService m_intercomService;
            dds::intercom_api::CCustomCmd m_customCmd;
            std::mutex m_waitMutex;
            std::condition_variable m_waitCondition;
        };
    }
}
#endif
