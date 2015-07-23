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
        class CGenericChannel : public CClientChannelImpl<CGenericChannel>
        {
            CGenericChannel(boost::asio::io_service& _service)
                : CClientChannelImpl<CGenericChannel>(_service, EChannelType::UI)
            {
                subscribeOnEvent(EChannelEvents::OnRemoteEndDissconnected,
                                 [this](CGenericChannel* _channel)
                                 {
                                     LOG(MiscCommon::info) << "The DDS commander ["
                                                           << this->socket().remote_endpoint().address().to_string()
                                                           << "] has closed the connection.";
                                 });

                subscribeOnEvent(EChannelEvents::OnHandshakeOK,
                                 [this](CGenericChannel* _channel)
                                 {
                                     switch (m_options.m_agentCmd)
                                     {
                                         case EAgentCmdType::GETLOG:
                                             LOG(MiscCommon::log_stdout) << "Requesting log files from agents...";
                                             pushMsg<cmdGET_LOG>();
                                             break;
                                         case EAgentCmdType::UPDATE_KEY:
                                         {
                                             LOG(MiscCommon::log_stdout) << "Sending key update command...";
                                             SUpdateKeyCmd cmd;
                                             cmd.m_sKey = m_options.m_sUpdKey_key;
                                             cmd.m_sValue = m_options.m_sUpdKey_value;
                                             pushMsg<cmdUPDATE_KEY>(cmd);
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
            MESSAGE_HANDLER(cmdSIMPLE_MSG, on_cmdSIMPLE_MSG);
            MESSAGE_HANDLER(cmdSHUTDOWN, on_cmdSHUTDOWN)
            MESSAGE_HANDLER(cmdPROGRESS, on_cmdPROGRESS)
            END_MSG_MAP()

            void setOptions(const SOptions& _options)
            {
                m_options = _options;
            }

          private:
            // Message Handlers
            bool on_cmdSIMPLE_MSG(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment);
            bool on_cmdSHUTDOWN(SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment);
            bool on_cmdPROGRESS(SCommandAttachmentImpl<cmdPROGRESS>::ptr_t _attachment);

          private:
            SOptions m_options;
        };
    }
}
#endif /* defined(__DDS__GenericChannel__) */