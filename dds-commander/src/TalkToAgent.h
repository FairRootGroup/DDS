// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__TalkToAgent__
#define __DDS__TalkToAgent__
// STD
#include <deque>
// BOOST
#include "boost/noncopyable.hpp"
#include "boost/asio.hpp"
// DDS
#include "ConnectionImpl.h"

namespace dds
{
    class CTalkToAgent : public CConnectionImpl<CTalkToAgent>
    {
        CTalkToAgent(boost::asio::io_service& _service)
            : CConnectionImpl<CTalkToAgent>(_service)
        {
        }

      public:
        BEGIN_MSG_MAP(CTalkToAgent)
        MESSAGE_HANDLER(dds::cmdHANDSHAKE, on_cmdHANDSHAKE)
        END_MSG_MAP()

      private:
        // Message Handlers
        int on_cmdHANDSHAKE(const CProtocolMessage& _msg);
    };
}
#endif /* defined(__DDS__TalkToAgent__) */
