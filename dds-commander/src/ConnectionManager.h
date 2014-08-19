// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__ConnectionManager__
#define __DDS__ConnectionManager__
// DDS
#include "ConnectionManagerImpl.h"
#include "AgentChannel.h"
// STD
#include <mutex>

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
        bool agentsInfoHandler(const CProtocolMessage& _msg, CAgentChannel* _channel);
        bool on_cmdGET_LOG(const CProtocolMessage& _msg, CAgentChannel* _channel);
        bool on_cmdBINARY_ATTACHMENT_LOG(const CProtocolMessage& _msg, CAgentChannel* _channel);
        bool on_cmdGET_LOG_ERROR(const CProtocolMessage& _msg, CAgentChannel* _channel);

        void checkAllRecieved();

        struct SGetLogInfo
        {
            size_t nofRecieved() const
            {
                return (m_nofRecieved + m_nofRecievedErrors);
            }

            bool allRecieved() const
            {
                return (nofRecieved() == m_nofRequests);
            }

            void zeroCounters()
            {
                m_nofRequests = 0;
                m_nofRecieved = 0;
                m_nofRecievedErrors = 0;
            }
            size_t m_nofRequests;
            size_t m_nofRecieved;
            size_t m_nofRecievedErrors;
            CAgentChannel* m_channel;
            std::mutex m_mutex;
        };

        SGetLogInfo m_getLog;
    };
}
#endif /* defined(__DDS__ConnectionManager__) */
