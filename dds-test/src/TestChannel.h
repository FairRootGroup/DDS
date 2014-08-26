// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__TestChannel__
#define __DDS__TestChannel__
// DDS
#include "ConnectionImpl.h"

namespace dds
{
    class CTestChannel : public CConnectionImpl<CTestChannel>
    {
        CTestChannel(boost::asio::io_service& _service)
            : CConnectionImpl<CTestChannel>(_service)
            , m_isHandShakeOK(false)
        {
        }

        REGISTER_DEFAULT_ON_CONNECT_CALLBACKS
        REGISTER_DEFAULT_ON_HEADER_READ_CALLBACKS

      public:
        BEGIN_MSG_MAP(CTestChannel)
        MESSAGE_HANDLER(cmdREPLY_HANDSHAKE_OK, on_cmdREPLY_HANDSHAKE_OK)
        MESSAGE_HANDLER(cmdDOWNLOAD_TEST_RECIEVED, on_cmdDOWNLOAD_TEST_RECIEVED)
        MESSAGE_HANDLER(cmdALL_DOWNLOAD_TESTS_RECIEVED, on_cmdALL_DOWNLOAD_TESTS_RECIEVED)
        MESSAGE_HANDLER(cmdDOWNLOAD_TEST_ERROR, on_cmdDOWNLOAD_TEST_ERROR)
        MESSAGE_HANDLER(cmdDOWNLOAD_TEST_FATAL, on_cmdDOWNLOAD_TEST_FATAL)
        END_MSG_MAP()

      private:
        // Message Handlers
        bool on_cmdREPLY_HANDSHAKE_OK(const CProtocolMessage& _msg);
        bool on_cmdDOWNLOAD_TEST_RECIEVED(const CProtocolMessage& _msg);
        bool on_cmdALL_DOWNLOAD_TESTS_RECIEVED(const CProtocolMessage& _msg);
        bool on_cmdDOWNLOAD_TEST_ERROR(const CProtocolMessage& _msg);
        bool on_cmdDOWNLOAD_TEST_FATAL(const CProtocolMessage& _msg);
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
#endif /* defined(__DDS__TalkToAgent__) */