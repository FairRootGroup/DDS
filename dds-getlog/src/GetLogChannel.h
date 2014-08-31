// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__GetLogChannel__
#define __DDS__GetLogChannel__
// DDS
#include "ConnectionImpl.h"

namespace dds
{
    class CGetLogChannel : public CConnectionImpl<CGetLogChannel>
    {
        CGetLogChannel(boost::asio::io_service& _service)
            : CConnectionImpl<CGetLogChannel>(_service)
            , m_isHandShakeOK(false)
        {
        }

        REGISTER_DEFAULT_REMOTE_ID_STRING
        REGISTER_DEFAULT_ON_CONNECT_CALLBACKS
        REGISTER_DEFAULT_ON_HEADER_READ_CALLBACKS

      public:
        BEGIN_MSG_MAP(CGetLogChannel)
        MESSAGE_HANDLER(cmdREPLY_HANDSHAKE_OK, on_cmdREPLY_HANDSHAKE_OK)
        MESSAGE_HANDLER(cmdLOG_RECIEVED, on_cmdLOG_RECIEVED)
        MESSAGE_HANDLER(cmdALL_LOGS_RECIEVED, on_cmdALL_LOGS_RECIEVED)
        MESSAGE_HANDLER(cmdGET_LOG_ERROR, on_cmdGET_LOG_ERROR)
        MESSAGE_HANDLER(cmdGET_LOG_FATAL, on_cmdGET_LOG_FATAL)
        END_MSG_MAP()

      private:
        // Message Handlers
        bool on_cmdREPLY_HANDSHAKE_OK(CProtocolMessage::protocolMessagePtr_t _msg);
        bool on_cmdLOG_RECIEVED(CProtocolMessage::protocolMessagePtr_t _msg);
        bool on_cmdALL_LOGS_RECIEVED(CProtocolMessage::protocolMessagePtr_t _msg);
        bool on_cmdGET_LOG_ERROR(CProtocolMessage::protocolMessagePtr_t _msg);
        bool on_cmdGET_LOG_FATAL(CProtocolMessage::protocolMessagePtr_t _msg);
        // On connection handles
        void onRemoteEndDissconnected()
        {
            LOG(MiscCommon::info) << "The Agent [" << socket().remote_endpoint().address().to_string()
                                  << "] has closed the connection.";
        }

      private:
        bool m_isHandShakeOK;
    };
}
#endif /* defined(__DDS__GetLogChannel__) */