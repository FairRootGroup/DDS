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
        typedef std::function<void(pid_t)> handlerOnNewUserTaks_t;

      private:
        CCommanderChannel(boost::asio::io_service& _service);

        REGISTER_DEFAULT_REMOTE_ID_STRING
        REGISTER_DEFAULT_ON_CONNECT_CALLBACKS

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
        MESSAGE_HANDLER(cmdGET_LOG, on_cmdGET_LOG)
        MESSAGE_HANDLER(cmdASSIGN_USER_TASK, on_cmdASSIGN_USER_TASK)
        MESSAGE_HANDLER(cmdACTIVATE_AGENT, on_cmdACTIVATE_AGENT)
        MESSAGE_HANDLER(cmdDOWNLOAD_TEST, on_cmdDOWNLOAD_TEST)
        END_MSG_MAP()

        // gives the possibility to register a callback, which will be called when a user task is executed
        void registerOnNewUserTaskCallback(handlerOnNewUserTaks_t _callback)
        {
            m_onNewUserTaskCallback = _callback;
        }

      private:
        // Message Handlers
        bool on_cmdREPLY_HANDSHAKE_OK(CProtocolMessage::protocolMessagePtr_t _msg);
        bool on_cmdSIMPLE_MSG(CProtocolMessage::protocolMessagePtr_t _msg);
        bool on_cmdGET_HOST_INFO(CProtocolMessage::protocolMessagePtr_t _msg);
        bool on_cmdDISCONNECT(CProtocolMessage::protocolMessagePtr_t _msg);
        bool on_cmdSHUTDOWN(CProtocolMessage::protocolMessagePtr_t _msg);
        bool on_cmdBINARY_ATTACHMENT(CProtocolMessage::protocolMessagePtr_t _msg);
        bool on_cmdGET_UUID(CProtocolMessage::protocolMessagePtr_t _msg);
        bool on_cmdSET_UUID(CProtocolMessage::protocolMessagePtr_t _msg);
        bool on_cmdGET_LOG(CProtocolMessage::protocolMessagePtr_t _msg);
        bool on_cmdASSIGN_USER_TASK(CProtocolMessage::protocolMessagePtr_t _msg);
        bool on_cmdACTIVATE_AGENT(CProtocolMessage::protocolMessagePtr_t _msg);
        bool on_cmdDOWNLOAD_TEST(CProtocolMessage::protocolMessagePtr_t _msg);
        void onRemoteEndDissconnected();

      private:
        void sendGetLogError(const std::string& _msg);
        void onHeaderRead();
        void readAgentUUIDFile();
        void createAgentUUIDFile() const;
        void deleteAgentUUIDFile() const;

      private:
        bool m_isHandShakeOK;
        std::chrono::steady_clock::time_point m_headerReadTime;
        boost::uuids::uuid m_id;
        std::string m_sUsrExe;
        handlerOnNewUserTaks_t m_onNewUserTaskCallback;
    };
}

#endif /* defined(__DDS__CCommanderChannel__) */
