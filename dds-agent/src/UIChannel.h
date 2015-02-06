// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef DDS_UIChannel_h
#define DDS_UIChannel_h

// DDS
#include "ServerChannelImpl.h"

namespace dds
{
    class CUIChannel : public CServerChannelImpl<CUIChannel>
    {
      private:
        CUIChannel(boost::asio::io_service& _service);
        std::string _remoteEndIDString();

      public:
        BEGIN_MSG_MAP(CUIChannel)
        MESSAGE_HANDLER(cmdUPDATE_KEY, on_cmdUPDATE_KEY)
        END_MSG_MAP()

      private:
        // Message Handlers
        bool on_cmdUPDATE_KEY(SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment);
    };
}

#endif
