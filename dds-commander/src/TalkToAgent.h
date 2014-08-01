// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__TalkToAgent__
#define __DDS__TalkToAgent__
// DDS
#include "ConnectionImpl.h"

namespace dds
{
    enum class ETalkToAgentType
    {
        UNDEFINED,
        AGENT,
        UI
    };

    class CTalkToAgent : public CConnectionImpl<CTalkToAgent>
    {
        CTalkToAgent(boost::asio::io_service& _service)
            : CConnectionImpl<CTalkToAgent>(_service)
            , m_isHandShakeOK(false)
            , m_type(ETalkToAgentType::UNDEFINED)
        {
        }

        REGISTER_DEFAULT_ON_CONNECT_CALLBACKS

      public:
        BEGIN_MSG_MAP(CTalkToAgent)
        MESSAGE_HANDLER(cmdHANDSHAKE, on_cmdHANDSHAKE)
        MESSAGE_HANDLER(cmdHANDSHAKE_AGENT, on_cmdHANDSHAKE_AGENT)
        MESSAGE_HANDLER(cmdREPLY_HOST_INFO, on_cmdREPLY_HOST_INFO)
        // replay on the "submit" command request
        MESSAGE_HANDLER(cmdSUBMIT, on_cmdSUBMIT)
        // replay on the "info" command request
        // - get pid of the commander server
        MESSAGE_HANDLER(cmdGED_PID, on_cmdGED_PID)
        END_MSG_MAP()

      private:
        // Message Handlers
        int on_cmdHANDSHAKE(const CProtocolMessage& _msg);
        int on_cmdHANDSHAKE_AGENT(const CProtocolMessage& _msg);
        int on_cmdSUBMIT(const CProtocolMessage& _msg);
        int on_cmdREPLY_HOST_INFO(const CProtocolMessage& _msg);
        int on_cmdGED_PID(const CProtocolMessage& _msg);
        // On connection handles
        void onRemoteEndDissconnected()
        {
            LOG(MiscCommon::info) << "The Agent has closed the connection.";
        }

      private:
        bool m_isHandShakeOK;
        ETalkToAgentType m_type;
    };
}
#endif /* defined(__DDS__TalkToAgent__) */
