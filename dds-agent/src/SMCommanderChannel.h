// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__CSMCommanderChannel__
#define __DDS__CSMCommanderChannel__
// DDS
#include "BaseSMChannelImpl.h"

namespace dds
{
    class CSMCommanderChannel : public protocol_api::CBaseSMChannelImpl<CSMCommanderChannel>
    {
      public:
        enum EOutputID
        {
            Leader = 1
        };

      protected:
        CSMCommanderChannel(boost::asio::io_service& _service,
                            const std::string& _inputName,
                            const std::string& _outputName,
                            uint64_t _protocolHeaderID,
                            protocol_api::EMQOpenType _inputOpenType,
                            protocol_api::EMQOpenType _outputOpenType);

      public:
        ~CSMCommanderChannel();

      public:
        BEGIN_SM_MSG_MAP(CSMCommanderChannel)
            SM_MESSAGE_HANDLER(cmdREPLY, on_cmdREPLY)
            SM_MESSAGE_HANDLER(cmdSIMPLE_MSG, on_cmdSIMPLE_MSG)
            SM_MESSAGE_HANDLER(cmdGET_HOST_INFO, on_cmdGET_HOST_INFO)
            SM_MESSAGE_HANDLER(cmdSHUTDOWN, on_cmdSHUTDOWN)
            SM_MESSAGE_HANDLER(cmdBINARY_ATTACHMENT_RECEIVED, on_cmdBINARY_ATTACHMENT_RECEIVED)
            SM_MESSAGE_HANDLER(cmdGET_ID, on_cmdGET_ID)
            SM_MESSAGE_HANDLER(cmdSET_ID, on_cmdSET_ID)
            SM_MESSAGE_HANDLER(cmdGET_LOG, on_cmdGET_LOG)
            SM_MESSAGE_HANDLER(cmdASSIGN_USER_TASK, on_cmdASSIGN_USER_TASK)
            SM_MESSAGE_HANDLER(cmdACTIVATE_USER_TASK, on_cmdACTIVATE_USER_TASK)
            SM_MESSAGE_HANDLER(cmdSTOP_USER_TASK, on_cmdSTOP_USER_TASK)
            SM_MESSAGE_HANDLER(cmdUPDATE_KEY, on_cmdUPDATE_KEY)
            SM_MESSAGE_HANDLER(cmdDELETE_KEY, on_cmdDELETE_KEY)
            SM_MESSAGE_HANDLER(cmdCUSTOM_CMD, on_cmdCUSTOM_CMD)
        END_SM_MSG_MAP()

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
        bool on_cmdDELETE_KEY(protocol_api::SCommandAttachmentImpl<protocol_api::cmdDELETE_KEY>::ptr_t _attachment,
                              protocol_api::SSenderInfo& _sender);
        bool on_cmdCUSTOM_CMD(protocol_api::SCommandAttachmentImpl<protocol_api::cmdCUSTOM_CMD>::ptr_t _attachment,
                              protocol_api::SSenderInfo& _sender);

      public:
        uint64_t getTaskID() const
        {
            return m_taskID;
        }

      private:
        void readAgentIDFile();
        void createAgentIDFile() const;
        void deleteAgentIDFile() const;

      private:
        uint64_t m_id;
        std::string m_sUsrExe;
        uint64_t m_taskID;
        size_t m_taskIndex;
        size_t m_collectionIndex;
        std::string m_taskPath;
        std::string m_groupName;
        std::string m_collectionName;
        std::string m_taskName;
    };
} // namespace dds
#endif
