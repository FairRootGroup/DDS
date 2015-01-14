// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__TestChannel__
#define __DDS__TestChannel__
// DDS
#include "ClientChannelImpl.h"

namespace dds
{
    class CTestChannel : public CClientChannelImpl<CTestChannel>
    {
        CTestChannel(boost::asio::io_service& _service)
            : CClientChannelImpl<CTestChannel>(_service, EChannelType::UI)
        {
        }

        REGISTER_DEFAULT_REMOTE_ID_STRING
        REGISTER_DEFAULT_ON_CONNECT_CALLBACKS

      public:
        BEGIN_MSG_MAP(CTestChannel)
        MESSAGE_HANDLER(cmdSIMPLE_MSG, on_cmdSIMPLE_MSG)
        MESSAGE_HANDLER(cmdSHUTDOWN, on_cmdSHUTDOWN)
        END_MSG_MAP()

      private:
        // Message Handlers
        bool on_cmdSIMPLE_MSG(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment);
        bool on_cmdSHUTDOWN(SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment);

        // On connection handles
        void onRemoteEndDissconnected()
        {
            LOG(MiscCommon::info) << "The Agent [" << socket().remote_endpoint().address().to_string()
                                  << "] has closed the connection.";
        }

        void onHandshakeOK();
        void onHandshakeERR();
    };
}
#endif /* defined(__DDS__TalkToAgent__) */
