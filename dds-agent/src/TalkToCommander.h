// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__CTalkToCommander__
#define __DDS__CTalkToCommander__

// DDS
#include "ConnectionImpl.h"

namespace dds
{
    class CTalkToCommander : public CConnectionImpl<CTalkToCommander>
    {
        CTalkToCommander(boost::asio::io_service& _service)
            : CConnectionImpl<CTalkToCommander>(_service)
        {
        }

      public:
        BEGIN_MSG_MAP(CTalkToCommander)
        MESSAGE_HANDLER(cmdHANDSHAKE, on_cmdHANDSHAKE)
        END_MSG_MAP()

      private:
        // Message Handlers
        int on_cmdHANDSHAKE(const CProtocolMessage& _msg);
    };
}

#endif /* defined(__DDS__CTalkToCommander__) */
