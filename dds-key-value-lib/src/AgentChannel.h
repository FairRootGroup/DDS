// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__CAgentChannel__
#define __DDS__CAgentChannel__

// DDS
#include "ConnectionImpl.h"

namespace dds
{
    class CAgentChannel : public CConnectionImpl<CAgentChannel>
    {
        typedef std::function<void(pid_t)> handlerOnNewUserTaks_t;

      private:
        CAgentChannel(boost::asio::io_service& _service);

        REGISTER_DEFAULT_REMOTE_ID_STRING
        REGISTER_DEFAULT_ON_CONNECT_CALLBACKS

      public:
        BEGIN_MSG_MAP(CAgentChannel)
        MESSAGE_HANDLER(cmdREPLY_HANDSHAKE_OK, on_cmdREPLY_HANDSHAKE_OK)
        MESSAGE_HANDLER(cmdSIMPLE_MSG, on_cmdSIMPLE_MSG)
        MESSAGE_HANDLER(cmdDISCONNECT, on_cmdDISCONNECT)
        MESSAGE_HANDLER(cmdSHUTDOWN, on_cmdSHUTDOWN)
        END_MSG_MAP()

      private:
        // Message Handlers
        bool on_cmdREPLY_HANDSHAKE_OK(SCommandAttachmentImpl<cmdREPLY_HANDSHAKE_OK>::ptr_t _attachment);
        bool on_cmdSIMPLE_MSG(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment);
        bool on_cmdDISCONNECT(SCommandAttachmentImpl<cmdDISCONNECT>::ptr_t _attachment);
        bool on_cmdSHUTDOWN(SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment);
        void onRemoteEndDissconnected();

      private:
        bool m_isHandShakeOK;
    };
}

#endif /* defined(__DDS__CCommanderChannel__) */
