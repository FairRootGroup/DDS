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
    enum class EAgentChannelType
    {
        UNDEFINED,
        AGENT,
        UI
    };

    class CAgentChannel : public CConnectionImpl<CAgentChannel>
    {
        CAgentChannel(boost::asio::io_service& _service)
            : CConnectionImpl<CAgentChannel>(_service)
            , m_isHandShakeOK(false)
            , m_type(EAgentChannelType::UNDEFINED)
        {
        }

        REGISTER_DEFAULT_ON_CONNECT_CALLBACKS

      public:
        BEGIN_MSG_MAP(CAgentChannel)
        MESSAGE_HANDLER(cmdHANDSHAKE, on_cmdHANDSHAKE)
        MESSAGE_HANDLER(cmdHANDSHAKE_AGENT, on_cmdHANDSHAKE_AGENT)
        MESSAGE_HANDLER(cmdREPLY_HOST_INFO, on_cmdREPLY_HOST_INFO)
        //====> replay on the "submit" command request
        MESSAGE_HANDLER(cmdSUBMIT, on_cmdSUBMIT)
        MESSAGE_HANDLER(cmdSUBMIT_START, on_cmdSUBMIT_START)
        //====> replay on the "info" command request
        // - get pid of the commander server
        MESSAGE_HANDLER(cmdGED_PID, on_cmdGED_PID)
        // - get Agents Info command
        MESSAGE_HANDLER(cmdGET_AGENTS_INFO, on_cmdGET_AGENTS_INFO)

        MESSAGE_HANDLER(cmdBINARY_DOWNLOAD_STAT, on_cmdBINARY_DOWNLOAD_STAT)
        MESSAGE_HANDLER(cmdREPLY_UUID, on_cmdREPLY_UUID)
        MESSAGE_HANDLER(cmdGET_LOG, on_cmdGET_LOG)
        MESSAGE_HANDLER(cmdBINARY_ATTACHMENT_LOG, on_cmdBINARY_ATTACHMENT_LOG)
        MESSAGE_HANDLER(cmdGET_LOG_ERROR, on_cmdGET_LOG_ERROR)
        MESSAGE_HANDLER(cmdSTART_DOWNLOAD_TEST, on_cmdSTART_DOWNLOAD_TEST)
        MESSAGE_HANDLER(cmdDOWNLOAD_TEST_STAT, on_cmdDOWNLOAD_TEST_STAT)
        MESSAGE_HANDLER(cmdDOWNLOAD_TEST_ERROR, on_cmdDOWNLOAD_TEST_ERROR)
        MESSAGE_HANDLER(cmdSIMPLE_MSG, on_cmdSIMPLE_MSG)
        END_MSG_MAP()

      public:
        EAgentChannelType getType() const;
        const boost::uuids::uuid& getId() const;
        const SHostInfoCmd getRemoteHostInfo() const
        {
            return m_remoteHostInfo;
        }

      private:
        // Message Handlers
        bool on_cmdHANDSHAKE(CProtocolMessage::protocolMessagePtr_t _msg);
        bool on_cmdHANDSHAKE_AGENT(CProtocolMessage::protocolMessagePtr_t _msg);
        bool on_cmdSUBMIT(CProtocolMessage::protocolMessagePtr_t _msg);
        bool on_cmdSUBMIT_START(CProtocolMessage::protocolMessagePtr_t _msg);
        bool on_cmdREPLY_HOST_INFO(CProtocolMessage::protocolMessagePtr_t _msg);
        bool on_cmdGED_PID(CProtocolMessage::protocolMessagePtr_t _msg);
        bool on_cmdBINARY_DOWNLOAD_STAT(CProtocolMessage::protocolMessagePtr_t _msg);
        bool on_cmdREPLY_UUID(CProtocolMessage::protocolMessagePtr_t _msg);
        bool on_cmdGET_LOG(CProtocolMessage::protocolMessagePtr_t _msg);
        bool on_cmdBINARY_ATTACHMENT_LOG(CProtocolMessage::protocolMessagePtr_t _msg);
        bool on_cmdGET_AGENTS_INFO(CProtocolMessage::protocolMessagePtr_t _msg);
        bool on_cmdGET_LOG_ERROR(CProtocolMessage::protocolMessagePtr_t _msg);
        bool on_cmdSTART_DOWNLOAD_TEST(CProtocolMessage::protocolMessagePtr_t _msg);
        bool on_cmdDOWNLOAD_TEST_STAT(CProtocolMessage::protocolMessagePtr_t _msg);
        bool on_cmdDOWNLOAD_TEST_ERROR(CProtocolMessage::protocolMessagePtr_t _msg);
        bool on_cmdSIMPLE_MSG(CProtocolMessage::protocolMessagePtr_t _msg);

        // On connection handles
        void onRemoteEndDissconnected()
        {
            LOG(MiscCommon::info) << "The Agent has closed the connection.";
        }
        // On header read handle
        void onHeaderRead();

      private:
        bool m_isHandShakeOK;
        EAgentChannelType m_type;
        boost::uuids::uuid m_id;
        SHostInfoCmd m_remoteHostInfo;
        std::string m_sCurrentTopoFile;
    };
}
#endif /* defined(__DDS__CAgentChannel__) */
