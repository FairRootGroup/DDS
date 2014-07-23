// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__ConnectionManager__
#define __DDS__ConnectionManager__
// BOOST
#include <boost/asio.hpp>
// DDS
#include "TalkToAgent.h"
#include "Options.h"

namespace dds
{
    class CConnectionManager
    {
      public:
        CConnectionManager(const SOptions_t& _options,
                           boost::asio::io_service& io_service,
                           boost::asio::ip::tcp::endpoint& endpoint);
        virtual ~CConnectionManager();

        void start();
        void stop();

      private:
        void acceptHandler(CTalkToAgent::connectionPtr_t _agent, const boost::system::error_code& _ec);
        void createServerInfoFile() const;
        void deleteServerInfoFile() const;
        void doAwaitStop();

      private:
        boost::asio::ip::tcp::acceptor m_acceptor;
        boost::asio::ip::tcp::endpoint m_endpoint;
        /// The signal_set is used to register for process termination notifications.
        boost::asio::signal_set m_signals;

        CTalkToAgent::connectionPtrVector_t m_agents;
        dds::SOptions_t m_options;
    };
}
#endif /* defined(__DDS__ConnectionManager__) */
