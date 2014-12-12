// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__CAgentChannel__
#define __DDS__CAgentChannel__
// DDS
#include "ConnectionImpl.h"
// STD
#include <mutex>

namespace dds
{
    struct SSyncHelper;

    class CAgentChannel : public CConnectionImpl<CAgentChannel>
    {
      public:
        BEGIN_MSG_MAP(CAgentChannel)
        MESSAGE_HANDLER(cmdREPLY_HANDSHAKE_OK, on_cmdREPLY_HANDSHAKE_OK)
        MESSAGE_HANDLER(cmdSIMPLE_MSG, on_cmdSIMPLE_MSG)
        MESSAGE_HANDLER(cmdDISCONNECT, on_cmdDISCONNECT)
        MESSAGE_HANDLER(cmdSHUTDOWN, on_cmdSHUTDOWN)
        MESSAGE_HANDLER(cmdUPDATE_KEY, on_cmdUPDATE_KEY)
        END_MSG_MAP()

      public:
        SSyncHelper* m_syncHelper;
        std::mutex m_mtxChannelReady;

      private:
        CAgentChannel(boost::asio::io_service& _service);

        REGISTER_DEFAULT_ON_CONNECT_CALLBACKS

        std::string _remoteEndIDString()
        {
            return "DDS agent";
        }

      private:
        // Message Handlers
        bool on_cmdREPLY_HANDSHAKE_OK(SCommandAttachmentImpl<cmdREPLY_HANDSHAKE_OK>::ptr_t _attachment);
        bool on_cmdSIMPLE_MSG(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment);
        bool on_cmdDISCONNECT(SCommandAttachmentImpl<cmdDISCONNECT>::ptr_t _attachment);
        bool on_cmdSHUTDOWN(SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment);
        bool on_cmdUPDATE_KEY(SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment);
        void onRemoteEndDissconnected();
    };
}

#endif /* defined(__DDS__CCommanderChannel__) */
