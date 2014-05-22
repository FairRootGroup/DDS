// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__CommanderServer__
#define __DDS__CommanderServer__

// DDS
#include "TalkToAgent.h"
// BOOST
#include <boost/asio.hpp>

class CCommanderServer
{
  public:
    CCommanderServer();

    virtual ~CCommanderServer();

    void start();

    void stop();

  private:
    void acceptHandler(TalkToAgentPtr_t _agent, const boost::system::error_code& _ec);

    boost::asio::io_service* m_service;
    boost::asio::ip::tcp::acceptor* m_acceptor;
    TalkToAgentPtrVector_t m_agents;
};

#endif /* defined(__DDS__CommanderServer__) */
