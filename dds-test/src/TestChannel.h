// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__TestChannel__
#define __DDS__TestChannel__
// DDS
#include "ClientChannelImpl.h"
#include "Options.h"

namespace dds
{
    namespace test_cmd
    {
        class CTestChannel : public dds::protocol_api::CClientChannelImpl<CTestChannel>
        {
            CTestChannel(boost::asio::io_service& _service)
                : CClientChannelImpl<CTestChannel>(_service, protocol_api::EChannelType::UI)
            {
                subscribeOnEvent(
                    protocol_api::EChannelEvents::OnRemoteEndDissconnected, [this](CTestChannel* _channel) {
                        LOG(MiscCommon::info) << "The Agent [" << this->socket().remote_endpoint().address().to_string()
                                              << "] has closed the connection.";
                    });

                subscribeOnEvent(protocol_api::EChannelEvents::OnHandshakeOK,
                                 [this](CTestChannel* _channel) { pushMsg<protocol_api::cmdTRANSPORT_TEST>(); });
            }

            REGISTER_DEFAULT_REMOTE_ID_STRING

          public:
            BEGIN_MSG_MAP(CTestChannel)
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
            bool on_cmdSIMPLE_MSG(protocol_api::SCommandAttachmentImpl<protocol_api::cmdSIMPLE_MSG>::ptr_t _attachment);
            bool on_cmdSHUTDOWN(protocol_api::SCommandAttachmentImpl<protocol_api::cmdSHUTDOWN>::ptr_t _attachment);
            bool on_cmdPROGRESS(protocol_api::SCommandAttachmentImpl<protocol_api::cmdPROGRESS>::ptr_t _attachment);

          private:
            SOptions m_options;
        };
    }
}
#endif /* defined(__DDS__TalkToAgent__) */
