// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__CAgentChannel__
#define __DDS__CAgentChannel__
// DDS
#include "ClientChannelImpl.h"
// STD
#include <mutex>

namespace dds
{
    struct SSyncHelper;

    class CAgentChannel : public CClientChannelImpl<CAgentChannel>
    {
      public:
        BEGIN_MSG_MAP(CAgentChannel)
        MESSAGE_HANDLER(cmdSIMPLE_MSG, on_cmdSIMPLE_MSG)
        MESSAGE_HANDLER(cmdSHUTDOWN, on_cmdSHUTDOWN)
        MESSAGE_HANDLER(cmdUPDATE_KEY, on_cmdUPDATE_KEY)
        END_MSG_MAP()

      public:
        SSyncHelper* m_syncHelper;

      private:
        CAgentChannel(boost::asio::io_service& _service);

        std::string _remoteEndIDString()
        {
            return "DDS agent";
        }

      private:
        // Message Handlers
        bool on_cmdSIMPLE_MSG(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment);
        bool on_cmdSHUTDOWN(SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment);
        bool on_cmdUPDATE_KEY(SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment);
    };
}

#endif /* defined(__DDS__CCommanderChannel__) */
