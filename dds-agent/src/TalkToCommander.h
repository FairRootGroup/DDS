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

        REGISTER_ALL_DEFAULT_CALLBACKS

      public:
        BEGIN_MSG_MAP(CTalkToCommander)
        MESSAGE_HANDLER(cmdREPLY_HANDSHAKE_OK, on_cmdREPLY_HANDSHAKE_OK)
        MESSAGE_HANDLER(cmdSIMPLE_MSG, on_cmdSIMPLE_MSG)
        MESSAGE_HANDLER(cmdGET_HOST_INFO, on_cmdGET_HOST_INFO)
        END_MSG_MAP()

      private:
        // Message Handlers
        int on_cmdREPLY_HANDSHAKE_OK(const CProtocolMessage& _msg);
        int on_cmdSIMPLE_MSG(const CProtocolMessage& _msg);
        int on_cmdGET_HOST_INFO(const CProtocolMessage& _msg);

      private:
        bool m_isHandShakeOK;
    };
}

#endif /* defined(__DDS__CTalkToCommander__) */
