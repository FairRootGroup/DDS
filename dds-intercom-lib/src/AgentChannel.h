// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__API__CAgentChannel__
#define __DDS__API__CAgentChannel__
// DDS
#include "ClientChannelImpl.h"

namespace dds
{
    namespace internal_api
    {
        struct SSyncHelper;

        class CAgentChannel : public protocol_api::CClientChannelImpl<CAgentChannel>
        {
          public:
            BEGIN_MSG_MAP(CAgentChannel)
                MESSAGE_HANDLER(cmdSIMPLE_MSG, on_cmdSIMPLE_MSG)
                MESSAGE_HANDLER(cmdSHUTDOWN, on_cmdSHUTDOWN)
                MESSAGE_HANDLER(cmdCUSTOM_CMD, on_cmdCUSTOM_CMD)
            // MESSAGE_HANDLER(cmdUPDATE_KEY, on_cmdUPDATE_KEY)
            END_MSG_MAP()

          public:
            SSyncHelper* m_syncHelper;

          public:
            void reconnectAgentWithErrorHandler(const std::function<void(const std::string&)>& callback);

          private:
            CAgentChannel(boost::asio::io_service& _service, uint64_t _protocolHeaderID = 0);

            std::string _remoteEndIDString()
            {
                return "DDS agent";
            }

          private:
            // Message Handlers
            bool on_cmdSIMPLE_MSG(protocol_api::SCommandAttachmentImpl<protocol_api::cmdSIMPLE_MSG>::ptr_t _attachment,
                                  const protocol_api::SSenderInfo& _sender);
            bool on_cmdSHUTDOWN(protocol_api::SCommandAttachmentImpl<protocol_api::cmdSHUTDOWN>::ptr_t _attachment,
                                const protocol_api::SSenderInfo& _sender);
            bool on_cmdCUSTOM_CMD(protocol_api::SCommandAttachmentImpl<protocol_api::cmdCUSTOM_CMD>::ptr_t _attachment,
                                  const protocol_api::SSenderInfo& _sender);
            // bool on_cmdUPDATE_KEY(protocol_api::SCommandAttachmentImpl<protocol_api::cmdUPDATE_KEY>::ptr_t
            // _attachment);

            uint16_t m_connectionAttempts;
        };
    } // namespace internal_api
} // namespace dds

#endif /* defined(__DDS__API__CAgentChannel__) */
