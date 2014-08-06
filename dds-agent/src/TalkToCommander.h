// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__CTalkToCommander__
#define __DDS__CTalkToCommander__

// DDS
#include "ConnectionImpl.h"

namespace dds
{
    class CTalkToCommander : public CConnectionImpl<CTalkToCommander>
    {
        CTalkToCommander(boost::asio::io_service& _service);

        REGISTER_DEFAULT_ON_CONNECT_CALLBACKS
        REGISTER_DEFAULT_ON_DISCONNECT_CALLBACKS

      public:
        BEGIN_MSG_MAP(CTalkToCommander)
        MESSAGE_HANDLER(cmdREPLY_HANDSHAKE_OK, on_cmdREPLY_HANDSHAKE_OK)
        MESSAGE_HANDLER(cmdSIMPLE_MSG, on_cmdSIMPLE_MSG)
        MESSAGE_HANDLER(cmdGET_HOST_INFO, on_cmdGET_HOST_INFO)
        MESSAGE_HANDLER(cmdDISCONNECT, on_cmdDISCONNECT)
        MESSAGE_HANDLER(cmdBINARY_ATTACHMENT, on_cmdBINARY_ATTACHMENT)
        END_MSG_MAP()

      private:
        // Message Handlers
        int on_cmdREPLY_HANDSHAKE_OK(const CProtocolMessage& _msg);
        int on_cmdSIMPLE_MSG(const CProtocolMessage& _msg);
        int on_cmdGET_HOST_INFO(const CProtocolMessage& _msg);
        int on_cmdDISCONNECT(const CProtocolMessage& _msg);
        int on_cmdBINARY_ATTACHMENT(const CProtocolMessage& _msg);

      private:
        void onHeaderRead();

      private:
        bool m_isHandShakeOK;
        std::chrono::steady_clock::time_point m_headerReadTime;
    };
}

#endif /* defined(__DDS__CTalkToCommander__) */
