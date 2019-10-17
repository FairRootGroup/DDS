// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__AGENT__CCommanderChannel__
#define __DDS__AGENT__CCommanderChannel__

// DDS
#include "ClientChannelImpl.h"
#include "SMLeaderChannel.h"
#include "TopoCore.h"

namespace dds
{
    namespace agent_cmd
    {
        using slotId_t = uint64_t;
        using taskId_t = uint64_t;

        struct SSlotInfo
        {
            using container_t = std::map<slotId_t, SSlotInfo>;

            slotId_t m_id{ 0 };
            std::string m_sUsrExe;
            taskId_t m_taskID{ 0 };
            size_t m_taskIndex{ 0 };
            size_t m_collectionIndex{ 0 };
            std::string m_taskPath;
            std::string m_groupName;
            std::string m_collectionName;
            std::string m_taskName;
            pid_t m_pid{ 0 };
        };
        class CCommanderChannel : public protocol_api::CClientChannelImpl<CCommanderChannel>
        {
          public:
            CCommanderChannel(boost::asio::io_context& _service, uint64_t _ProtocolHeaderID);

          public:
            REGISTER_DEFAULT_REMOTE_ID_STRING

            BEGIN_MSG_MAP(CCommanderChannel)
                MESSAGE_HANDLER(cmdREPLY, on_cmdREPLY)
                MESSAGE_HANDLER(cmdSIMPLE_MSG, on_cmdSIMPLE_MSG)
                MESSAGE_HANDLER(cmdGET_HOST_INFO, on_cmdGET_HOST_INFO)
                MESSAGE_HANDLER(cmdSHUTDOWN, on_cmdSHUTDOWN)
                MESSAGE_HANDLER(cmdBINARY_ATTACHMENT_RECEIVED, on_cmdBINARY_ATTACHMENT_RECEIVED)
                MESSAGE_HANDLER(cmdGET_ID, on_cmdGET_ID)
                MESSAGE_HANDLER(cmdSET_ID, on_cmdSET_ID)
                MESSAGE_HANDLER(cmdGET_LOG, on_cmdGET_LOG)
                MESSAGE_HANDLER(cmdASSIGN_USER_TASK, on_cmdASSIGN_USER_TASK)
                MESSAGE_HANDLER(cmdACTIVATE_USER_TASK, on_cmdACTIVATE_USER_TASK)
                MESSAGE_HANDLER(cmdSTOP_USER_TASK, on_cmdSTOP_USER_TASK)
                MESSAGE_HANDLER(cmdUPDATE_KEY, on_cmdUPDATE_KEY)
                MESSAGE_HANDLER(cmdCUSTOM_CMD, on_cmdCUSTOM_CMD)
                MESSAGE_HANDLER(cmdADD_SLOT, on_cmdADD_SLOT)
                MESSAGE_HANDLER(cmdUSER_TASK_DONE, on_cmdUSER_TASK_DONE)
            END_MSG_MAP()

            //   CSMFWChannel::weakConnectionPtr_t getSMFWChannel();

          public:
            void stopChannel();

          private:
            // Message Handlers
            bool on_cmdREPLY(protocol_api::SCommandAttachmentImpl<protocol_api::cmdREPLY>::ptr_t _attachment,
                             protocol_api::SSenderInfo& _sender);
            bool on_cmdSIMPLE_MSG(protocol_api::SCommandAttachmentImpl<protocol_api::cmdSIMPLE_MSG>::ptr_t _attachment,
                                  protocol_api::SSenderInfo& _sender);
            bool on_cmdGET_HOST_INFO(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdGET_HOST_INFO>::ptr_t _attachment,
                protocol_api::SSenderInfo& _sender);
            bool on_cmdSHUTDOWN(protocol_api::SCommandAttachmentImpl<protocol_api::cmdSHUTDOWN>::ptr_t _attachment,
                                protocol_api::SSenderInfo& _sender);
            bool on_cmdBINARY_ATTACHMENT_RECEIVED(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdBINARY_ATTACHMENT_RECEIVED>::ptr_t _attachment,
                protocol_api::SSenderInfo& _sender);
            bool on_cmdGET_ID(protocol_api::SCommandAttachmentImpl<protocol_api::cmdGET_ID>::ptr_t _attachment,
                              protocol_api::SSenderInfo& _sender);
            bool on_cmdSET_ID(protocol_api::SCommandAttachmentImpl<protocol_api::cmdSET_ID>::ptr_t _attachment,
                              protocol_api::SSenderInfo& _sender);
            bool on_cmdGET_LOG(protocol_api::SCommandAttachmentImpl<protocol_api::cmdGET_LOG>::ptr_t _attachment,
                               protocol_api::SSenderInfo& _sender);
            bool on_cmdASSIGN_USER_TASK(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdASSIGN_USER_TASK>::ptr_t _attachment,
                protocol_api::SSenderInfo& _sender);
            bool on_cmdACTIVATE_USER_TASK(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdACTIVATE_USER_TASK>::ptr_t _attachment,
                protocol_api::SSenderInfo& _sender);
            bool on_cmdSTOP_USER_TASK(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdSTOP_USER_TASK>::ptr_t _attachment,
                protocol_api::SSenderInfo& _sender);
            bool on_cmdUPDATE_KEY(protocol_api::SCommandAttachmentImpl<protocol_api::cmdUPDATE_KEY>::ptr_t _attachment,
                                  protocol_api::SSenderInfo& _sender);
            bool on_cmdCUSTOM_CMD(protocol_api::SCommandAttachmentImpl<protocol_api::cmdCUSTOM_CMD>::ptr_t _attachment,
                                  protocol_api::SSenderInfo& _sender);
            bool on_cmdADD_SLOT(protocol_api::SCommandAttachmentImpl<protocol_api::cmdADD_SLOT>::ptr_t _attachment,
                                protocol_api::SSenderInfo& _sender);
            void send_cmdUPDATE_KEY(
                const protocol_api::SSenderInfo& _sender,
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdUPDATE_KEY>::ptr_t _attachment);
            bool on_cmdUSER_TASK_DONE(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdUSER_TASK_DONE>::ptr_t _attachment,
                protocol_api::SSenderInfo& _sender);

          private:
            void readAgentIDFile();
            void createAgentIDFile() const;
            void deleteAgentIDFile() const;
            void onNewUserTask(uint64_t _slotID, pid_t _pid);
            void terminateChildrenProcesses(pid_t _parentPid = 0);
            void taskExited(uint64_t _taskID, int _exitCode);
            SSlotInfo& getSlotInfoById(const slotId_t& _slotID)
            {
                std::lock_guard<std::mutex> lock(m_mutexSlots);
                auto it = m_slots.find(_slotID);
                if (it == m_slots.end())
                {
                    std::stringstream ss;
                    ss << "No matching slot for " << _slotID;
                    throw std::runtime_error(ss.str());
                }

                return it->second;
            }

          private:
            uint64_t m_id = 0;
            uint16_t m_connectionAttempts;
            std::mutex m_taskIDToSlotIDMapMutex;
            std::map<uint64_t, uint64_t> m_taskIDToSlotIDMap;
            topology_api::CTopoCore m_topo;

            CSMLeaderChannel::connectionPtr_t m_leaderChannel;

            std::mutex m_mutexSlots;
            SSlotInfo::container_t m_slots;
        };
    } // namespace agent_cmd
} // namespace dds

#endif /* defined(__DDS__AGENT__CCommanderChannel__) */
