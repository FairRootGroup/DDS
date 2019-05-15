// Copyright 2018 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__InfoChannel__
#define __DDS__InfoChannel__
// DDS
#include "ClientChannelImpl.h"
#include "GetPropValuesCmd.h"

namespace dds
{
    namespace session_cmd
    {
        class CInfoChannel : public protocol_api::CClientChannelImpl<CInfoChannel>
        {
            CInfoChannel(boost::asio::io_context& _service, uint64_t _protocolHeaderID = 0)
                : CClientChannelImpl<CInfoChannel>(_service, protocol_api::EChannelType::UI, _protocolHeaderID)
            {
                registerHandler<protocol_api::EChannelEvents::OnHandshakeOK>(
                    [this](const protocol_api::SSenderInfo& _sender) { pushMsg<protocol_api::cmdGED_PID>(); });

                registerHandler<protocol_api::EChannelEvents::OnConnected>(
                    [](const protocol_api::SSenderInfo& _sender) {
                        LOG(MiscCommon::info) << "Connected to the commander server";
                    });

                registerHandler<protocol_api::EChannelEvents::OnFailedToConnect>(
                    [](const protocol_api::SSenderInfo& _sender) {
                        LOG(MiscCommon::log_stderr) << "Failed to connect to commander server.";
                    });
            }

            REGISTER_DEFAULT_REMOTE_ID_STRING

          public:
            BEGIN_MSG_MAP(CInfoChannel)
                MESSAGE_HANDLER(cmdSIMPLE_MSG, on_cmdSIMPLE_MSG)
                MESSAGE_HANDLER(cmdREPLY_PID, on_cmdREPLY_PID)
            END_MSG_MAP()

          private:
            // Message Handlers
            bool on_cmdSIMPLE_MSG(protocol_api::SCommandAttachmentImpl<protocol_api::cmdSIMPLE_MSG>::ptr_t _attachment,
                                  const protocol_api::SSenderInfo& _sender);
            bool on_cmdREPLY_PID(protocol_api::SCommandAttachmentImpl<protocol_api::cmdREPLY_PID>::ptr_t _attachment,
                                 const protocol_api::SSenderInfo& _sender);
        };
    } // namespace session_cmd
} // namespace dds
#endif
