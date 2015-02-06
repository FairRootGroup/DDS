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
    class CTestChannel : public CClientChannelImpl<CTestChannel>
    {
        CTestChannel(boost::asio::io_service& _service)
            : CClientChannelImpl<CTestChannel>(_service, EChannelType::UI)
        {
            subscribeOnEvent(EChannelEvents::OnRemoteEndDissconnected,
                             [this](CTestChannel* _channel)
                             {
                                 LOG(MiscCommon::info) << "The Agent ["
                                                       << this->socket().remote_endpoint().address().to_string()
                                                       << "] has closed the connection.";
                             });

            subscribeOnEvent(EChannelEvents::OnHandshakeOK,
                             [this](CTestChannel* _channel)
                             {
                                 pushMsg<cmdTRANSPORT_TEST>();
                             });
        }

        REGISTER_DEFAULT_REMOTE_ID_STRING

      public:
        BEGIN_MSG_MAP(CTestChannel)
        MESSAGE_HANDLER(cmdSIMPLE_MSG, on_cmdSIMPLE_MSG)
        MESSAGE_HANDLER(cmdSHUTDOWN, on_cmdSHUTDOWN)
        MESSAGE_HANDLER(cmdPROGRESS, on_cmdPROGRESS)
        END_MSG_MAP()

        void setOptions(const dds::SOptions& _options)
        {
            m_options = _options;
        }

      private:
        // Message Handlers
        bool on_cmdSIMPLE_MSG(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment);
        bool on_cmdSHUTDOWN(SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment);
        bool on_cmdPROGRESS(SCommandAttachmentImpl<cmdPROGRESS>::ptr_t _attachment);

      private:
        dds::SOptions m_options;
    };
}
#endif /* defined(__DDS__TalkToAgent__) */
