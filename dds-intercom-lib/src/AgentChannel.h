// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__API__CAgentChannel__
#define __DDS__API__CAgentChannel__
// DDS
#include "ClientChannelImpl.h"

namespace dds
{
    namespace internal_api
    {
        class CAgentChannel : public protocol_api::CClientChannelImpl<CAgentChannel>
        {
          public:
            BEGIN_MSG_MAP(CAgentChannel)
                MESSAGE_HANDLER_DISPATCH(cmdSHUTDOWN)
                MESSAGE_HANDLER_DISPATCH(cmdSIMPLE_MSG)
                MESSAGE_HANDLER_DISPATCH(cmdCUSTOM_CMD)
            END_MSG_MAP()

            void reconnectAgentWithErrorHandler(std::function<void(const std::string&)> callback);

          private:
            REGISTER_DEFAULT_REMOTE_ID_STRING

            CAgentChannel(boost::asio::io_context& _service, uint64_t _protocolHeaderID = 0);

            uint16_t m_connectionAttempts;
        };
    } // namespace internal_api
} // namespace dds

#endif /* defined(__DDS__API__CAgentChannel__) */
