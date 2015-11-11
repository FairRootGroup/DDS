// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__KEY_VALUE_API__AgentConnectionManager__
#define __DDS__KEY_VALUE_API__AgentConnectionManager__
// DDS
#include "AgentChannel.h"
// BOOST
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>

namespace dds
{
    namespace key_value_api
    {
        struct SSyncHelper;

        class CAgentConnectionManager : public std::enable_shared_from_this<CAgentConnectionManager>
        {
          public:
            CAgentConnectionManager();
            virtual ~CAgentConnectionManager();

          public:
            void start();
            void stop();
            bool stopped()
            {
                return m_service.stopped();
            }
            int updateKey(const protocol_api::SUpdateKeyCmd& _cmd);

          public:
            SSyncHelper* m_syncHelper;

          private:
            void doAwaitStop();
            bool on_cmdSHUTDOWN(protocol_api::SCommandAttachmentImpl<protocol_api::cmdSHUTDOWN>::ptr_t _attachment,
                                CAgentChannel::weakConnectionPtr_t _channel);
            CAgentChannel::weakConnectionPtr_t getAgentChannel()
            {
                return m_channel;
            }

          private:
            boost::asio::io_service m_service;
            // Don't use m_channel directly, only via getAgentChannel
            // In case if channel is destoryed, there still could be user calling update key
            // TODO: need to find a way to hide m_channel from direct access
            CAgentChannel::connectionPtr_t m_channel;
            bool m_bStarted;
            boost::thread_group m_workerThreads;
        };
    }
}

#endif /* defined(__DDS__KEY_VALUE_API__AgentConnectionManager__) */
