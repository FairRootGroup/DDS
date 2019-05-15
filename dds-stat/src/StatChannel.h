// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__StatChannel__
#define __DDS__StatChannel__
// DDS
#include "ClientChannelImpl.h"
#include "Options.h"

namespace dds
{
    namespace stat_cmd
    {
        class CStatChannel : public protocol_api::CClientChannelImpl<CStatChannel>
        {
            CStatChannel(boost::asio::io_context& _service, uint64_t _protocolHeaderID = 0)
                : CClientChannelImpl<CStatChannel>(_service, protocol_api::EChannelType::UI, _protocolHeaderID)
            {
                registerHandler<protocol_api::EChannelEvents::OnHandshakeOK>(
                    [this](const protocol_api::SSenderInfo& _sender) {
                        if (m_options.m_bEnable)
                        {
                            pushMsg<protocol_api::cmdENABLE_STAT>();
                        }
                        else if (m_options.m_bDisable)
                        {
                            pushMsg<protocol_api::cmdDISABLE_STAT>();
                        }
                        else if (m_options.m_bGet)
                        {
                            pushMsg<protocol_api::cmdGET_STAT>();
                        }
                    });

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
            BEGIN_MSG_MAP(CStatChannel)
                MESSAGE_HANDLER(cmdSIMPLE_MSG, on_cmdSIMPLE_MSG)
            END_MSG_MAP()

            void setOptions(const SOptions& _options)
            {
                m_options = _options;
            }

          private:
            // Message Handlers
            bool on_cmdSIMPLE_MSG(protocol_api::SCommandAttachmentImpl<protocol_api::cmdSIMPLE_MSG>::ptr_t _attachment,
                                  const protocol_api::SSenderInfo& _sender);

          private:
            SOptions m_options;
        };
    } // namespace stat_cmd
} // namespace dds
#endif
