// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__AgentClient__
#define __DDS__AgentClient__

// BOOST
#include <boost/asio.hpp>

class CAgentClient
{
  public:
    CAgentClient();

    virtual ~CAgentClient();

    void start();

    void stop();

  private:
    void echo(const boost::asio::ip::tcp::endpoint& _ep, const std::string& _message);

    boost::asio::io_service m_service;
};

#endif /* defined(__DDS__CAgentClient__) */
