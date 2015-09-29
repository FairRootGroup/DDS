// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__CAgentChannel__
#define __DDS__CAgentChannel__
// DDS
#include "ServerChannelImpl.h"
#include <chrono>
// BOOST
#include <boost/property_tree/ptree.hpp>

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

        class CAgentChannel : public protocol_api::CServerChannelImpl<CAgentChannel>
        {
            CAgentChannel(boost::asio::io_service& _service)
                : CServerChannelImpl<CAgentChannel>(
                      _service,
                      { protocol_api::EChannelType::AGENT, protocol_api::EChannelType::UI })
                , m_taskID(0)
                , m_state(EAgentState::unknown)
            {
                subscribeOnEvent(protocol_api::EChannelEvents::OnRemoteEndDissconnected,
                                 [](CAgentChannel* _channel)
                                 {
                                     LOG(MiscCommon::info) << "The Agent has closed the connection.";
                                 });

                subscribeOnEvent(protocol_api::EChannelEvents::OnHandshakeOK,
                                 [this](CAgentChannel* _channel)
                                 {
                                     switch (getChannelType())
                                     {
                                         case protocol_api::EChannelType::AGENT:
                                         {
                                             m_state = EAgentState::idle;
                                             pushMsg<protocol_api::cmdGET_ID>();
                                             pushMsg<protocol_api::cmdGET_HOST_INFO>();
                                         }
                                             return;
                                         case protocol_api::EChannelType::UI:
                                             LOG(MiscCommon::info) << "The UI agent ["
                                                                   << socket().remote_endpoint().address().to_string()
                                                                   << "] has successfully connected.";
                                             return;
                                         default:
                                             // TODO: log unknown connection attempt
                                             return;
                                     }
                                 });
            }

          public:
            BEGIN_MSG_MAP(CAgentChannel)
            MESSAGE_HANDLER(cmdREPLY_HOST_INFO, on_cmdREPLY_HOST_INFO)
            //====> replay on the "submit" command request
            MESSAGE_HANDLER(cmdSUBMIT, on_cmdSUBMIT)
            MESSAGE_HANDLER(cmdUSER_TASK_DONE, on_cmdUSER_TASK_DONE)
            //====> replay on the "info" command request
            // - get pid of the commander server
            MESSAGE_HANDLER(cmdGED_PID, on_cmdGED_PID)
            // - get Agents Info command
            MESSAGE_HANDLER(cmdGET_AGENTS_INFO, on_cmdGET_AGENTS_INFO)
            MESSAGE_HANDLER(cmdREPLY_ID, on_cmdREPLY_ID)
            MESSAGE_HANDLER(cmdBINARY_ATTACHMENT_RECEIVED, on_cmdBINARY_ATTACHMENT_RECEIVED)
            MESSAGE_HANDLER(cmdTRANSPORT_TEST, on_cmdTRANSPORT_TEST)
            MESSAGE_HANDLER(cmdSIMPLE_MSG, on_cmdSIMPLE_MSG)
            // - Topology commands
            MESSAGE_HANDLER(cmdACTIVATE_AGENT, on_cmdACTIVATE_AGENT)
            MESSAGE_HANDLER(cmdSTOP_USER_TASK, on_cmdSTOP_USER_TASK)
            MESSAGE_HANDLER(cmdSET_TOPOLOGY, on_cmdSET_TOPOLOGY)
            // - Agents commands
            MESSAGE_HANDLER(cmdGET_LOG, on_cmdGET_LOG)
            MESSAGE_HANDLER(cmdUPDATE_KEY, on_cmdUPDATE_KEY)
            // Watchdog
            MESSAGE_HANDLER(cmdWATCHDOG_HEARTBEAT, on_cmdWATCHDOG_HEARTBEAT)
            MESSAGE_HANDLER(cmdGET_PROP_LIST, on_cmdGET_PROP_LIST)
            MESSAGE_HANDLER(cmdGET_PROP_VALUES, on_cmdGET_PROP_VALUES)
            // Statistics commands
            MESSAGE_HANDLER(cmdENABLE_STAT, on_cmdENABLE_STAT)
            MESSAGE_HANDLER(cmdDISABLE_STAT, on_cmdDISABLE_STAT)
            MESSAGE_HANDLER(cmdGET_STAT, on_cmdGET_STAT)

            END_MSG_MAP()

          public:
            uint64_t getId() const;
            void setId(uint64_t _id);

            const protocol_api::SHostInfoCmd& getRemoteHostInfo() const
            {
                return m_remoteHostInfo;
            }

            uint64_t getTaskID() const;
            void setTaskID(uint64_t _taskID);

            std::chrono::milliseconds getStartupTime() const
            {
                return m_startUpTime;
            }
            EAgentState getState() const
            {
                return m_state;
            }
            void setState(EAgentState _state)
            {
                m_state = _state;
            }

            // This function only used in tests
            void setRemoteHostInfo(const protocol_api::SHostInfoCmd& _hostInfo)
            {
                m_remoteHostInfo = _hostInfo;
            }

            const boost::property_tree::ptree& getPropertyPT() const
            {
                return m_propertyPT;
            }

            std::mutex& getPropertyPTMutex()
            {
                return m_propertyPTMutex;
            }

          private:
            // Message Handlers
            bool on_cmdSUBMIT(protocol_api::SCommandAttachmentImpl<protocol_api::cmdSUBMIT>::ptr_t _attachment);
            bool on_cmdACTIVATE_AGENT(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdACTIVATE_AGENT>::ptr_t _attachment);
            bool on_cmdSTOP_USER_TASK(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdSTOP_USER_TASK>::ptr_t _attachment);
            bool on_cmdREPLY_HOST_INFO(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdREPLY_HOST_INFO>::ptr_t _attachment);
            bool on_cmdGED_PID(protocol_api::SCommandAttachmentImpl<protocol_api::cmdGED_PID>::ptr_t _attachment);
            bool on_cmdREPLY_ID(protocol_api::SCommandAttachmentImpl<protocol_api::cmdREPLY_ID>::ptr_t _attachment);
            bool on_cmdGET_LOG(protocol_api::SCommandAttachmentImpl<protocol_api::cmdGET_LOG>::ptr_t _attachment);
            bool on_cmdBINARY_ATTACHMENT_RECEIVED(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdBINARY_ATTACHMENT_RECEIVED>::ptr_t _attachment);
            bool on_cmdGET_AGENTS_INFO(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdGET_AGENTS_INFO>::ptr_t _attachment);
            bool on_cmdTRANSPORT_TEST(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdTRANSPORT_TEST>::ptr_t _attachment);
            bool on_cmdSIMPLE_MSG(protocol_api::SCommandAttachmentImpl<protocol_api::cmdSIMPLE_MSG>::ptr_t _attachment);
            bool on_cmdUPDATE_KEY(protocol_api::SCommandAttachmentImpl<protocol_api::cmdUPDATE_KEY>::ptr_t _attachment);
            bool on_cmdUSER_TASK_DONE(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdUSER_TASK_DONE>::ptr_t _attachment);
            bool on_cmdWATCHDOG_HEARTBEAT(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdWATCHDOG_HEARTBEAT>::ptr_t _attachment);
            bool on_cmdGET_PROP_LIST(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdGET_PROP_LIST>::ptr_t _attachment);
            bool on_cmdGET_PROP_VALUES(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdGET_PROP_VALUES>::ptr_t _attachment);
            bool on_cmdSET_TOPOLOGY(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdSET_TOPOLOGY>::ptr_t _attachment);
            bool on_cmdENABLE_STAT(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdENABLE_STAT>::ptr_t _attachment);
            bool on_cmdDISABLE_STAT(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdDISABLE_STAT>::ptr_t _attachment);
            bool on_cmdGET_STAT(protocol_api::SCommandAttachmentImpl<protocol_api::cmdGET_STAT>::ptr_t _attachment);

            std::string _remoteEndIDString()
            {
                if (getChannelType() == protocol_api::EChannelType::AGENT)
                    return std::to_string(m_id);
                else
                    return "UI client";
            }

          private:
            // We use unique ID because we need to identify a new agent after shutdown of the system on the same host.
            // We have to distinguish between new and old agent.
            uint64_t m_id;
            protocol_api::SHostInfoCmd m_remoteHostInfo;
            std::string m_sCurrentTopoFile;
            uint64_t m_taskID;
            std::chrono::milliseconds m_startUpTime;
            EAgentState m_state;
            boost::property_tree::ptree m_propertyPT;
            std::mutex m_propertyPTMutex;
        };
    }
}
#endif /* defined(__DDS__CAgentChannel__) */
