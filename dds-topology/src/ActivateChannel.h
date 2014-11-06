// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__ActivateChannel__
#define __DDS__ActivateChannel__
// DDS
#include "ConnectionImpl.h"

namespace dds
{
    class CActivateChannel : public CConnectionImpl<CActivateChannel>
    {
        CActivateChannel(boost::asio::io_service& _service)
            : CConnectionImpl<CActivateChannel>(_service)
            , m_isHandShakeOK(false)
        {
        }

        REGISTER_DEFAULT_REMOTE_ID_STRING

      public:
        BEGIN_MSG_MAP(CActivateChannel)
        MESSAGE_HANDLER(cmdREPLY_HANDSHAKE_OK, on_cmdREPLY_HANDSHAKE_OK)
        MESSAGE_HANDLER(cmdSIMPLE_MSG, on_cmdSIMPLE_MSG)
        MESSAGE_HANDLER(cmdSHUTDOWN, on_cmdSHUTDOWN)
        END_MSG_MAP()

      private:
        // Message Handlers
        bool on_cmdREPLY_HANDSHAKE_OK(SCommandAttachmentImpl<cmdREPLY_HANDSHAKE_OK>::ptr_t _attachment);
        bool on_cmdSIMPLE_MSG(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment);
        bool on_cmdSHUTDOWN(SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment);
        // On connection handles
        void onConnected()
        {
            LOG(MiscCommon::log_stdout) << "Connection established.";
            LOG(MiscCommon::log_stdout) << "Requesting server to activate tasks...";
        }
        void onFailedToConnect()
        {
            LOG(MiscCommon::log_stdout) << "Failed to connect.";
        }
        void onRemoteEndDissconnected()
        {
            LOG(MiscCommon::log_stderr) << "Server has closed the coinnection.";
        }
        void onHeaderRead()
        {
        }

      private:
        bool m_isHandShakeOK;
    };
}

#endif
