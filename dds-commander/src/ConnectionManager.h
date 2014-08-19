// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__ConnectionManager__
#define __DDS__ConnectionManager__
// DDS
#include "ConnectionManagerImpl.h"
#include "AgentChannel.h"

namespace dds
{
    class CConnectionManager : public CConnectionManagerImpl<CAgentChannel, CConnectionManager>
    {
      public:
        CConnectionManager(const SOptions_t& _options,
                           boost::asio::io_service& _io_service,
                           boost::asio::ip::tcp::endpoint& _endpoint);

        ~CConnectionManager();

      public:
        void newClientCreated(CAgentChannel::connectionPtr_t _newClient);

      private:
        bool getLogHandler(const CProtocolMessage& _msg, CAgentChannel* _channel);
        bool binaryAttachmentLogHandler(const CProtocolMessage& _msg, CAgentChannel* _channel);

        // FIXME: Make it thread safe
        size_t m_nofLogRequests;
        size_t m_nofRecievedLogs;
        CAgentChannel* m_uiChannel;
    };
}
#endif /* defined(__DDS__ConnectionManager__) */
