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
    namespace agent
    {
        class CCommanderChannel : public CClientChannelImpl<CCommanderChannel>
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
            MESSAGE_HANDLER(cmdGET_UUID, on_cmdGET_UUID)
            MESSAGE_HANDLER(cmdSET_UUID, on_cmdSET_UUID)
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
            bool on_cmdREPLY_HANDSHAKE_OK(SCommandAttachmentImpl<cmdREPLY_HANDSHAKE_OK>::ptr_t _attachment);
            bool on_cmdSIMPLE_MSG(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment);
            bool on_cmdGET_HOST_INFO(SCommandAttachmentImpl<cmdGET_HOST_INFO>::ptr_t _attachment);
            bool on_cmdSHUTDOWN(SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment);
            bool on_cmdBINARY_ATTACHMENT_RECEIVED(
                SCommandAttachmentImpl<cmdBINARY_ATTACHMENT_RECEIVED>::ptr_t _attachment);
            bool on_cmdGET_UUID(SCommandAttachmentImpl<cmdGET_UUID>::ptr_t _attachment);
            bool on_cmdSET_UUID(SCommandAttachmentImpl<cmdSET_UUID>::ptr_t _attachment);
            bool on_cmdGET_LOG(SCommandAttachmentImpl<cmdGET_LOG>::ptr_t _attachment);
            bool on_cmdASSIGN_USER_TASK(SCommandAttachmentImpl<cmdASSIGN_USER_TASK>::ptr_t _attachment);
            bool on_cmdACTIVATE_AGENT(SCommandAttachmentImpl<cmdACTIVATE_AGENT>::ptr_t _attachment);
            bool on_cmdSTOP_USER_TASK(SCommandAttachmentImpl<cmdSTOP_USER_TASK>::ptr_t _attachment);
            bool on_cmdUPDATE_KEY(SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment);
            bool on_cmdDELETE_KEY(SCommandAttachmentImpl<cmdDELETE_KEY>::ptr_t _attachment);

          private:
            void readAgentUUIDFile();
            void createAgentUUIDFile() const;
            void deleteAgentUUIDFile() const;

          private:
            boost::uuids::uuid m_id;
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
