// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__GetLogChannel__
#define __DDS__GetLogChannel__
// DDS
#include "ConnectionImpl.h"
#include "Options.h"

namespace dds
{
    class CGenericChannel : public CConnectionImpl<CGenericChannel>
    {
        CGenericChannel(boost::asio::io_service& _service)
            : CConnectionImpl<CGenericChannel>(_service)
            , m_isHandShakeOK(false)
        {
        }

        REGISTER_DEFAULT_REMOTE_ID_STRING
        REGISTER_DEFAULT_ON_CONNECT_CALLBACKS

      public:
        BEGIN_MSG_MAP(CGenericChannel)
        MESSAGE_HANDLER(cmdREPLY_HANDSHAKE_OK, on_cmdREPLY_HANDSHAKE_OK)
        MESSAGE_HANDLER(cmdSIMPLE_MSG, on_cmdSIMPLE_MSG);
        MESSAGE_HANDLER(cmdSHUTDOWN, on_cmdSHUTDOWN)
        END_MSG_MAP()

        void setOptions(const SOptions& _options)
        {
            m_options = _options;
        }

      private:
        // Message Handlers
        bool on_cmdREPLY_HANDSHAKE_OK(SCommandAttachmentImpl<cmdREPLY_HANDSHAKE_OK>::ptr_t _attachment);
        bool on_cmdSIMPLE_MSG(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment);
        bool on_cmdSHUTDOWN(SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment);
        // On connection handles
        void onRemoteEndDissconnected()
        {
            LOG(MiscCommon::info) << "The DDS commander [" << socket().remote_endpoint().address().to_string()
                                  << "] has closed the connection.";
        }

      private:
        bool m_isHandShakeOK;
        SOptions m_options;
    };
}
#endif /* defined(__DDS__GetLogChannel__) */