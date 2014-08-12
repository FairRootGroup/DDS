// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__CCommanderChannel__
#define __DDS__CCommanderChannel__

// DDS
#include "ConnectionImpl.h"

namespace dds
{
    class CCommanderChannel : public CConnectionImpl<CCommanderChannel>
    {
        CCommanderChannel(boost::asio::io_service& _service);

        REGISTER_DEFAULT_ON_CONNECT_CALLBACKS
        REGISTER_DEFAULT_ON_DISCONNECT_CALLBACKS

      public:
        BEGIN_MSG_MAP(CCommanderChannel)
        MESSAGE_HANDLER(cmdREPLY_HANDSHAKE_OK, on_cmdREPLY_HANDSHAKE_OK)
        MESSAGE_HANDLER(cmdSIMPLE_MSG, on_cmdSIMPLE_MSG)
        MESSAGE_HANDLER(cmdGET_HOST_INFO, on_cmdGET_HOST_INFO)
        MESSAGE_HANDLER(cmdDISCONNECT, on_cmdDISCONNECT)
        MESSAGE_HANDLER(cmdSHUTDOWN, on_cmdSHUTDOWN)
        MESSAGE_HANDLER(cmdBINARY_ATTACHMENT, on_cmdBINARY_ATTACHMENT)
        MESSAGE_HANDLER(cmdGET_UUID, on_cmdGET_UUID)
        MESSAGE_HANDLER(cmdSET_UUID, on_cmdSET_UUID)
        END_MSG_MAP()

      private:
        // Message Handlers
        int on_cmdREPLY_HANDSHAKE_OK(const CProtocolMessage& _msg);
        int on_cmdSIMPLE_MSG(const CProtocolMessage& _msg);
        int on_cmdGET_HOST_INFO(const CProtocolMessage& _msg);
        int on_cmdDISCONNECT(const CProtocolMessage& _msg);
        int on_cmdSHUTDOWN(const CProtocolMessage& _msg);
        int on_cmdBINARY_ATTACHMENT(const CProtocolMessage& _msg);
        int on_cmdGET_UUID(const CProtocolMessage& _msg);
        int on_cmdSET_UUID(const CProtocolMessage& _msg);

      private:
        void onHeaderRead();
        void readAgentUUIDFile();
        void createAgentUUIDFile() const;
        void deleteAgentUUIDFile() const;

      private:
        bool m_isHandShakeOK;
        std::chrono::steady_clock::time_point m_headerReadTime;
        boost::uuids::uuid m_id;
    };
}

#endif /* defined(__DDS__CCommanderChannel__) */
