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
    class CTalkToAgent : public CConnectionImpl<CTalkToAgent>
    {
        CTalkToAgent(boost::asio::io_service& _service)
            : CConnectionImpl<CTalkToAgent>(_service)
            , m_isHandShakeOK(false)
        {
        }

        REGISTER_DEFAULT_CALLBACKS

      public:
        BEGIN_MSG_MAP(CTalkToAgent)
        MESSAGE_HANDLER(cmdHANDSHAKE, on_cmdHANDSHAKE)
        MESSAGE_HANDLER(cmdHANDSHAKE_AGENT, on_cmdHANDSHAKE_AGENT)
        MESSAGE_HANDLER(cmdSUBMIT, on_cmdSUBMIT)
        MESSAGE_HANDLER(cmdREPLY_HOST_INFO, on_cmdREPLY_HOST_INFO)
        END_MSG_MAP()

      private:
        // Message Handlers
        int on_cmdHANDSHAKE(const CProtocolMessage& _msg);
        int on_cmdHANDSHAKE_AGENT(const CProtocolMessage& _msg);
        int on_cmdSUBMIT(const CProtocolMessage& _msg);
        int on_cmdREPLY_HOST_INFO(const CProtocolMessage& _msg);

      private:
        bool m_isHandShakeOK;
    };
}
#endif /* defined(__DDS__TalkToAgent__) */
