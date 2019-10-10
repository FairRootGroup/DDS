// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__CAgentChannel__
#define __DDS__CAgentChannel__
// DDS
#include "ServerChannelImpl.h"
// STD
#include <chrono>

namespace dds
{
    namespace commander_cmd
    {
        enum EAgentState
        {
            unknown = 0,
            idle,     // no tasks are assigned
            executing // assigned task is executing
        };
        const std::array<std::string, 3> g_agentStates = { { "unknown", "idle", "executing" } };

        using slotID_t = uint64_t;
        using taskID_t = uint64_t;
        struct SSlotInfo
        {
            slotID_t m_id{ 0 }; /// slot ID
            taskID_t m_taskID{ 0 };
            EAgentState m_state{ EAgentState::unknown };
        };

        struct SAgentInfo
        {
            using SlotContainer_t = std::map<slotID_t, SSlotInfo>;

            SAgentInfo()
                : m_id(0)
                , m_startUpTime(0)
            {
            }
            void addSlot(const SSlotInfo& _slot)
            {
                std::lock_guard<std::mutex> lock(m_mtxSlot);
                m_slots.insert(std::make_pair(_slot.m_id, _slot));
            }
            const SlotContainer_t& getSlots() const
            {
                return m_slots;
            }

            const SSlotInfo& getSlotByID(slotID_t _slotID) const
            {
                auto it = m_slots.find(_slotID);
                if (it == m_slots.end())
                {
                    std::stringstream ss;
                    ss << "getSlotByID: slot " << _slotID << " does not exist.";
                    throw std::runtime_error(ss.str());
                }

                return it->second;
            }

            // We use unique ID because we need to identify a new agent after shutdown of the system on the same host.
            // We have to distinguish between new and old agent.
            uint64_t m_id;
            protocol_api::SHostInfoCmd m_remoteHostInfo;
            std::chrono::milliseconds m_startUpTime;

          private:
            std::mutex m_mtxSlot;
            SlotContainer_t m_slots;
        };

        typedef std::vector<uint64_t> LobbyProtocolHeaderIdContainer_t;

        class CAgentChannel : public protocol_api::CServerChannelImpl<CAgentChannel>
        {
            CAgentChannel(boost::asio::io_context& _context, uint64_t _protocolHeaderID = 0);

          public:
            BEGIN_MSG_MAP(CAgentChannel)
                MESSAGE_HANDLER(cmdREPLY_HOST_INFO, on_cmdREPLY_HOST_INFO)
                //====> replay on the "submit" command request
                MESSAGE_HANDLER(cmdSUBMIT, on_cmdSUBMIT)
                MESSAGE_HANDLER_DISPATCH(cmdUSER_TASK_DONE)
                //====> replay on the "info" command request
                // - get Agents Info command
                MESSAGE_HANDLER_DISPATCH(cmdGET_AGENTS_INFO)
                MESSAGE_HANDLER_DISPATCH(cmdGET_IDLE_AGENTS_COUNT)
                //
                MESSAGE_HANDLER_DISPATCH(cmdREPLY_ID)
                MESSAGE_HANDLER(cmdBINARY_ATTACHMENT_RECEIVED, on_cmdBINARY_ATTACHMENT_RECEIVED)
                MESSAGE_HANDLER_DISPATCH(cmdTRANSPORT_TEST)
                MESSAGE_HANDLER(cmdREPLY, on_cmdREPLY)
                // - Topology commands
                MESSAGE_HANDLER_DISPATCH(cmdUPDATE_TOPOLOGY)
                // - Agents commands
                MESSAGE_HANDLER_DISPATCH(cmdGET_LOG)
                MESSAGE_HANDLER_DISPATCH(cmdUPDATE_KEY)
                // Watchdog
                MESSAGE_HANDLER(cmdWATCHDOG_HEARTBEAT, on_cmdWATCHDOG_HEARTBEAT)
                MESSAGE_HANDLER_DISPATCH(cmdGET_PROP_LIST)
                MESSAGE_HANDLER_DISPATCH(cmdGET_PROP_VALUES)
                // Statistics commands
                MESSAGE_HANDLER_DISPATCH(cmdENABLE_STAT)
                MESSAGE_HANDLER_DISPATCH(cmdDISABLE_STAT)
                MESSAGE_HANDLER_DISPATCH(cmdGET_STAT)
                // custom command
                MESSAGE_HANDLER_DISPATCH(cmdCUSTOM_CMD)
                // TASK SLOTS
                MESSAGE_HANDLER(cmdREPLY_ADD_SLOT, on_cmdREPLY_ADD_SLOT)
            END_MSG_MAP()

          public:
            uint64_t getId() const;
            void setId(uint64_t _id);

            protocol_api::SHostInfoCmd getRemoteHostInfo(const dds::protocol_api::SSenderInfo& _sender);
            // This function only used in tests
            void setRemoteHostInfo(const dds::protocol_api::SSenderInfo& _sender,
                                   const protocol_api::SHostInfoCmd& _hostInfo);

            std::chrono::milliseconds getStartupTime(const dds::protocol_api::SSenderInfo& _sender);

            EAgentState getState(const dds::protocol_api::SSenderInfo& _sender);
            void setState(const dds::protocol_api::SSenderInfo& _sender, EAgentState _state);

            uint64_t getTaskID(const dds::protocol_api::SSenderInfo& _sender);
            void setTaskID(const dds::protocol_api::SSenderInfo& _sender, uint64_t _taskID);

            // AgentInfo operations
            /// add new or update existing Agent info
            //    void updateAgentInfo(const dds::protocol_api::SSenderInfo& _sender, const SAgentInfo& _info);
            //    void updateAgentInfo(uint64_t _protocolHeaderID, const SAgentInfo& _info);
            /// Get a copy of the agent info
            // FIXME: This function makes a copy of the info struct. Find a solution to avoid copy operations. But the
            // function and the info struct still must be thread safe.
            const SAgentInfo& getAgentInfo() const;
            //   SAgentInfo getAgentInfo(uint64_t _protocolHeaderID);
            LobbyProtocolHeaderIdContainer_t getLobbyPHID() const;

          private:
            // Message Handlers
            bool on_cmdSUBMIT(protocol_api::SCommandAttachmentImpl<protocol_api::cmdSUBMIT>::ptr_t _attachment,
                              const protocol_api::SSenderInfo& _sender);
            bool on_cmdREPLY_HOST_INFO(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdREPLY_HOST_INFO>::ptr_t _attachment,
                const protocol_api::SSenderInfo& _sender);
            bool on_cmdBINARY_ATTACHMENT_RECEIVED(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdBINARY_ATTACHMENT_RECEIVED>::ptr_t _attachment,
                const protocol_api::SSenderInfo& _sender);
            bool on_cmdREPLY(protocol_api::SCommandAttachmentImpl<protocol_api::cmdREPLY>::ptr_t _attachment,
                             const protocol_api::SSenderInfo& _sender);
            bool on_cmdWATCHDOG_HEARTBEAT(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdWATCHDOG_HEARTBEAT>::ptr_t _attachment,
                const protocol_api::SSenderInfo& _sender);
            bool on_cmdREPLY_ADD_SLOT(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdREPLY_ADD_SLOT>::ptr_t _attachment,
                const protocol_api::SSenderInfo& _sender);

            std::string _remoteEndIDString();

          private:
            std::string m_sCurrentTopoFile;
            SAgentInfo m_info;
            std::mutex m_mtxInfo;
        };
    } // namespace commander_cmd
} // namespace dds
#endif /* defined(__DDS__CAgentChannel__) */
