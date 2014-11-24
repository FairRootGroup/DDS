// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__AgentConnectionManager__
#define __DDS__AgentConnectionManager__

// DDS
#include "AgentChannel.h"
// BOOST
#include <boost/asio.hpp>

namespace dds
{
    class CAgentConnectionManager
    {
      public:
        CAgentConnectionManager(boost::asio::io_service& _io_service);
        virtual ~CAgentConnectionManager();

      public:
        void start();
        void stop();

      private:
        void doAwaitStop();
        bool on_cmdSHUTDOWN(SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment,
                            CAgentChannel::weakConnectionPtr_t _channel);

      private:
        boost::asio::io_service& m_service;
        boost::asio::signal_set m_signals;
        CAgentChannel::connectionPtrVector_t m_channels;
        bool m_bStarted;
    };
}

#endif /* defined(__DDS__AgentConnectionManager__) */
