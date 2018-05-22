// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__ActivateChannel__
#define __DDS__ActivateChannel__

// DDS
#include "ClientChannelImpl.h"
#include "Options.h"

namespace dds
{
    namespace topology_cmd
    {
        class CActivateChannel : public protocol_api::CClientChannelImpl<CActivateChannel>
        {
            CActivateChannel(boost::asio::io_service& _service, uint64_t _protocolHeaderID = 0);

            REGISTER_DEFAULT_REMOTE_ID_STRING

          public:
            BEGIN_MSG_MAP(CActivateChannel)
                MESSAGE_HANDLER(cmdSIMPLE_MSG, on_cmdSIMPLE_MSG)
                MESSAGE_HANDLER(cmdSHUTDOWN, on_cmdSHUTDOWN)
                MESSAGE_HANDLER(cmdPROGRESS, on_cmdPROGRESS)
            END_MSG_MAP()

            void setOptions(const SOptions& _options)
            {
                m_options = _options;
            }

          private:
            // Message Handlers
            bool on_cmdSIMPLE_MSG(protocol_api::SCommandAttachmentImpl<protocol_api::cmdSIMPLE_MSG>::ptr_t _attachment,
                                  const protocol_api::SSenderInfo& _sender);
            bool on_cmdSHUTDOWN(protocol_api::SCommandAttachmentImpl<protocol_api::cmdSHUTDOWN>::ptr_t _attachment,
                                const protocol_api::SSenderInfo& _sender);
            bool on_cmdPROGRESS(protocol_api::SCommandAttachmentImpl<protocol_api::cmdPROGRESS>::ptr_t _attachment,
                                const protocol_api::SSenderInfo& _sender);

          private:
            SOptions m_options;
        };
    } // namespace topology_cmd
} // namespace dds

#endif
