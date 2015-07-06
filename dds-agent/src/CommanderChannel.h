// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__AGENT__CCommanderChannel__
#define __DDS__AGENT__CCommanderChannel__

// DDS
#include "ClientChannelImpl.h"

namespace dds
{
    namespace agent_cmd
    {
        class CCommanderChannel : public protocol_api::CClientChannelImpl<CCommanderChannel>
        {
            typedef std::function<void(pid_t)> handlerOnNewUserTaks_t;

          private:
            CCommanderChannel(boost::asio::io_service& _service);

            REGISTER_DEFAULT_REMOTE_ID_STRING

          public:
            BEGIN_MSG_MAP(CCommanderChannel)
            MESSAGE_HANDLER(cmdSIMPLE_MSG, on_cmdSIMPLE_MSG)
            MESSAGE_HANDLER(cmdGET_HOST_INFO, on_cmdGET_HOST_INFO)
            MESSAGE_HANDLER(cmdSHUTDOWN, on_cmdSHUTDOWN)
            MESSAGE_HANDLER(cmdBINARY_ATTACHMENT_RECEIVED, on_cmdBINARY_ATTACHMENT_RECEIVED)
            MESSAGE_HANDLER(cmdGET_ID, on_cmdGET_ID)
            MESSAGE_HANDLER(cmdSET_ID, on_cmdSET_ID)
            MESSAGE_HANDLER(cmdGET_LOG, on_cmdGET_LOG)
            MESSAGE_HANDLER(cmdASSIGN_USER_TASK, on_cmdASSIGN_USER_TASK)
            MESSAGE_HANDLER(cmdACTIVATE_AGENT, on_cmdACTIVATE_AGENT)
            MESSAGE_HANDLER(cmdSTOP_USER_TASK, on_cmdSTOP_USER_TASK)
            MESSAGE_HANDLER(cmdUPDATE_KEY, on_cmdUPDATE_KEY)
            MESSAGE_HANDLER(cmdDELETE_KEY, on_cmdDELETE_KEY)
            END_MSG_MAP()

            // gives the possibility to register a callback, which will be called when a user task is executed
            void registerOnNewUserTaskCallback(handlerOnNewUserTaks_t _callback)
            {
                m_onNewUserTaskCallback = _callback;
            }

            void updateKey(const std::string& _key, const std::string& _value);

          private:
            // Message Handlers
            bool on_cmdREPLY_HANDSHAKE_OK(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdREPLY_HANDSHAKE_OK>::ptr_t _attachment);
            bool on_cmdSIMPLE_MSG(protocol_api::SCommandAttachmentImpl<protocol_api::cmdSIMPLE_MSG>::ptr_t _attachment);
            bool on_cmdGET_HOST_INFO(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdGET_HOST_INFO>::ptr_t _attachment);
            bool on_cmdSHUTDOWN(protocol_api::SCommandAttachmentImpl<protocol_api::cmdSHUTDOWN>::ptr_t _attachment);
            bool on_cmdBINARY_ATTACHMENT_RECEIVED(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdBINARY_ATTACHMENT_RECEIVED>::ptr_t _attachment);
            bool on_cmdGET_ID(protocol_api::SCommandAttachmentImpl<protocol_api::cmdGET_ID>::ptr_t _attachment);
            bool on_cmdSET_ID(protocol_api::SCommandAttachmentImpl<protocol_api::cmdSET_ID>::ptr_t _attachment);
            bool on_cmdGET_LOG(protocol_api::SCommandAttachmentImpl<protocol_api::cmdGET_LOG>::ptr_t _attachment);
            bool on_cmdASSIGN_USER_TASK(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdASSIGN_USER_TASK>::ptr_t _attachment);
            bool on_cmdACTIVATE_AGENT(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdACTIVATE_AGENT>::ptr_t _attachment);
            bool on_cmdSTOP_USER_TASK(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdSTOP_USER_TASK>::ptr_t _attachment);
            bool on_cmdUPDATE_KEY(protocol_api::SCommandAttachmentImpl<protocol_api::cmdUPDATE_KEY>::ptr_t _attachment);
            bool on_cmdDELETE_KEY(protocol_api::SCommandAttachmentImpl<protocol_api::cmdDELETE_KEY>::ptr_t _attachment);

          private:
            void readAgentIDFile();
            void createAgentIDFile() const;
            void deleteAgentIDFile() const;

          private:
            uint64_t m_id;
            std::string m_sUsrExe;
            std::string m_sTaskId;
            size_t m_taskIndex;
            size_t m_collectionIndex;
            std::string m_taskPath;
            std::string m_groupName;
            std::string m_collectionName;
            std::string m_taskName;
            handlerOnNewUserTaks_t m_onNewUserTaskCallback;
            uint16_t m_connectionAttempts;
        };
    }
}

#endif /* defined(__DDS__AGENT__CCommanderChannel__) */
