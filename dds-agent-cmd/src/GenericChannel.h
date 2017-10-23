// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__GenericChannel__
#define __DDS__GenericChannel__
// DDS
#include "ClientChannelImpl.h"
#include "Options.h"

namespace dds
{
    namespace agent_cmd_cmd
    {
        class CGenericChannel : public protocol_api::CClientChannelImpl<CGenericChannel>
        {
            CGenericChannel(boost::asio::io_service& _service, uint64_t _protocolHeaderID = 0)
                : CClientChannelImpl<CGenericChannel>(_service, protocol_api::EChannelType::UI, _protocolHeaderID)
            {
                registerHandler<protocol_api::EChannelEvents::OnRemoteEndDissconnected>(
                    [this](const protocol_api::SSenderInfo& _sender) {
                        LOG(MiscCommon::info)
                            << "The DDS commander [" << this->socket().remote_endpoint().address().to_string()
                            << "] has closed the connection.";
                    });

                registerHandler<protocol_api::EChannelEvents::OnHandshakeOK>(
                    [this](const protocol_api::SSenderInfo& _sender) {
                        switch (m_options.m_agentCmd)
                        {
                            case EAgentCmdType::GETLOG:
                                LOG(MiscCommon::log_stdout) << "Requesting log files from agents...";
                                pushMsg<protocol_api::cmdGET_LOG>();
                                break;
                            case EAgentCmdType::UPDATE_KEY:
                            {
                                LOG(MiscCommon::log_stdout) << "Sending key update command...";
                                protocol_api::SUpdateKeyCmd cmd;
                                cmd.m_sKey = m_options.m_sUpdKey_key;
                                cmd.m_sValue = m_options.m_sUpdKey_value;
                                pushMsg<protocol_api::cmdUPDATE_KEY>(cmd);
                            }
                            break;
                            default:
                                LOG(MiscCommon::log_stderr) << "Uknown command.";
                                stop();
                        }
                    });
            }

            REGISTER_DEFAULT_REMOTE_ID_STRING

          public:
            BEGIN_MSG_MAP(CGenericChannel)
                MESSAGE_HANDLER(dds::protocol_api::cmdSIMPLE_MSG, on_cmdSIMPLE_MSG);
                MESSAGE_HANDLER(dds::protocol_api::cmdSHUTDOWN, on_cmdSHUTDOWN)
                MESSAGE_HANDLER(dds::protocol_api::cmdPROGRESS, on_cmdPROGRESS)
            END_MSG_MAP()

            void setOptions(const SOptions& _options)
            {
                m_options = _options;
            }

          private:
            // Message Handlers
            bool on_cmdSIMPLE_MSG(protocol_api::SCommandAttachmentImpl<protocol_api::cmdSIMPLE_MSG>::ptr_t _attachment,
                                  const protocol_api::SSenderInfo& _sender);
            bool on_cmdSHUTDOWN(
                dds::protocol_api::SCommandAttachmentImpl<dds::protocol_api::cmdSHUTDOWN>::ptr_t _attachment,
                const protocol_api::SSenderInfo& _sender);
            bool on_cmdPROGRESS(
                dds::protocol_api::SCommandAttachmentImpl<dds::protocol_api::cmdPROGRESS>::ptr_t _attachment,
                const protocol_api::SSenderInfo& _sender);

          private:
            SOptions m_options;
        };
    }
}
#endif /* defined(__DDS__GenericChannel__) */
