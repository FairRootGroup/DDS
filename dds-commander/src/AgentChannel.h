// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__CAgentChannel__
#define __DDS__CAgentChannel__
// DDS
#include "ServerChannelImpl.h"
#include <chrono>

namespace dds
{
    enum EAgentState
    {
        unknown = 0,
        idle,     // no tasks are assigned
        executing // assigned task is executing
    };
    const std::array<std::string, 3> g_agentStates = { { "unknown", "idle", "executing" } };

    class CAgentChannel : public CServerChannelImpl<CAgentChannel>
    {
        CAgentChannel(boost::asio::io_service& _service)
            : CServerChannelImpl<CAgentChannel>(_service, { EChannelType::AGENT, EChannelType::UI })
            , m_taskID(0)
            , m_state(EAgentState::unknown)
        {
            subscribeOnEvent(EChannelEvents::OnRemoteEndDissconnected,
                             [](CAgentChannel* _channel)
                             {
                                 LOG(MiscCommon::info) << "The Agent has closed the connection.";
                             });

            subscribeOnEvent(EChannelEvents::OnHandshakeOK,
                             [this](CAgentChannel* _channel)
                             {
                                 switch (getChannelType())
                                 {
                                     case EChannelType::AGENT:
                                     {
                                         m_state = EAgentState::idle;
                                         pushMsg<cmdGET_UUID>();
                                         pushMsg<cmdGET_HOST_INFO>();
                                     }
                                         return;
                                     case EChannelType::UI:
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
        MESSAGE_HANDLER(cmdACTIVATE_AGENT, on_cmdACTIVATE_AGENT)
        MESSAGE_HANDLER(cmdSTOP_USER_TASK, on_cmdSTOP_USER_TASK)
        //====> replay on the "info" command request
        // - get pid of the commander server
        MESSAGE_HANDLER(cmdGED_PID, on_cmdGED_PID)
        // - get Agents Info command
        MESSAGE_HANDLER(cmdGET_AGENTS_INFO, on_cmdGET_AGENTS_INFO)
        MESSAGE_HANDLER(cmdREPLY_UUID, on_cmdREPLY_UUID)
        MESSAGE_HANDLER(cmdBINARY_ATTACHMENT_RECEIVED, on_cmdBINARY_ATTACHMENT_RECEIVED)
        MESSAGE_HANDLER(cmdTRANSPORT_TEST, on_cmdTRANSPORT_TEST)
        MESSAGE_HANDLER(cmdSIMPLE_MSG, on_cmdSIMPLE_MSG)
        // - Agents commands
        MESSAGE_HANDLER(cmdGET_LOG, on_cmdGET_LOG)
        MESSAGE_HANDLER(cmdUPDATE_KEY, on_cmdUPDATE_KEY)
        END_MSG_MAP()

      public:
        const boost::uuids::uuid& getId() const;
        const SHostInfoCmd getRemoteHostInfo() const
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

      private:
        // Message Handlers
        bool on_cmdSUBMIT(SCommandAttachmentImpl<cmdSUBMIT>::ptr_t _attachment);
        bool on_cmdACTIVATE_AGENT(SCommandAttachmentImpl<cmdACTIVATE_AGENT>::ptr_t _attachment);
        bool on_cmdSTOP_USER_TASK(SCommandAttachmentImpl<cmdSTOP_USER_TASK>::ptr_t _attachment);
        bool on_cmdREPLY_HOST_INFO(SCommandAttachmentImpl<cmdREPLY_HOST_INFO>::ptr_t _attachment);
        bool on_cmdGED_PID(SCommandAttachmentImpl<cmdGED_PID>::ptr_t _attachment);
        bool on_cmdREPLY_UUID(SCommandAttachmentImpl<cmdREPLY_UUID>::ptr_t _attachment);
        bool on_cmdGET_LOG(SCommandAttachmentImpl<cmdGET_LOG>::ptr_t _attachment);
        bool on_cmdBINARY_ATTACHMENT_RECEIVED(SCommandAttachmentImpl<cmdBINARY_ATTACHMENT_RECEIVED>::ptr_t _attachment);
        bool on_cmdGET_AGENTS_INFO(SCommandAttachmentImpl<cmdGET_AGENTS_INFO>::ptr_t _attachment);
        bool on_cmdTRANSPORT_TEST(SCommandAttachmentImpl<cmdTRANSPORT_TEST>::ptr_t _attachment);
        bool on_cmdSIMPLE_MSG(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment);
        bool on_cmdUPDATE_KEY(SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment);

        std::string _remoteEndIDString()
        {
            if (getChannelType() == EChannelType::AGENT)
                return boost::uuids::to_string(m_id);
            else
                return "UI client";
        }

      private:
        boost::uuids::uuid m_id;
        SHostInfoCmd m_remoteHostInfo;
        std::string m_sCurrentTopoFile;
        uint64_t m_taskID;
        std::chrono::milliseconds m_startUpTime;
        EAgentState m_state;
    };
}
#endif /* defined(__DDS__CAgentChannel__) */
