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

    void readHandler(const boost::system::error_code& _ec, std::size_t _bytesTransferred);

    void writeHandler(const boost::system::error_code& _ec, size_t bytesTransferred);

    void connectHandler(const boost::system::error_code& _ec);

    // void resolveHandler(const boost::system::error_code& _ec, boost::asio::ip::tcp::resolver::iterator _it);

    size_t readCompleteHandler(const boost::system::error_code& _ec, size_t _bytesTransferred);

    void doRead();

    void doWrite(const std::string& msg);

    boost::asio::io_service m_service;
    boost::asio::ip::tcp::resolver m_resolver;
    boost::asio::ip::tcp::socket m_socket;

    enum
    { max_msg = 1024 };
    char m_readBuffer[max_msg];
    char m_writeBuffer[max_msg];
};

#endif /* defined(__DDS__CAgentClient__) */
