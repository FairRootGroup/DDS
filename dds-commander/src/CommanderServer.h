// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__CommanderServer__
#define __DDS__CommanderServer__
// BOOST
#include <boost/asio.hpp>
// DDS
#include "TalkToAgent.h"
#include "Options.h"
#include "Topology.h"

namespace dds
{
    class CCommanderServer
    {
      public:
        CCommanderServer(const SOptions_t& _options);

        virtual ~CCommanderServer();

        void start();

        void stop();

      private:
        void acceptHandler(TalkToAgentPtr_t _agent, const boost::system::error_code& _ec);

        boost::asio::io_service* m_service;
        boost::asio::ip::tcp::acceptor* m_acceptor;
        TalkToAgentPtrVector_t m_agents;
        dds::SOptions_t m_options;
        CTopology m_topo;
    };
}
#endif /* defined(__DDS__CommanderServer__) */
