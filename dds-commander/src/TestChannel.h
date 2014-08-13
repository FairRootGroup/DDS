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

      public:
        BEGIN_MSG_MAP(CTestChannel)
        MESSAGE_HANDLER(cmdHANDSHAKE_AGENT, on_cmdHANDSHAKE_AGENT)
        MESSAGE_HANDLER(cmdBINARY_DOWNLOAD_STAT, on_cmdBINARY_DOWNLOAD_STAT)
        END_MSG_MAP()

      private:
        // Message Handlers
        bool on_cmdHANDSHAKE_AGENT(const CProtocolMessage& _msg);
        bool on_cmdBINARY_DOWNLOAD_STAT(const CProtocolMessage& _msg);
        // On connection handles
        void onRemoteEndDissconnected()
        {
            LOG(MiscCommon::info) << "The Agent [" << socket().remote_endpoint().address().to_string()
                                  << "] has closed the connection.";
        }
        void onHeaderRead();

      private:
        void sendTestBinaryAttachment(size_t _binarySize);

      private:
        bool m_isHandShakeOK;
    };
}
#endif /* defined(__DDS__TalkToAgent__) */