// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__AgentClient__
#define __DDS__AgentClient__

// DDS
#include "ProtocolMessage.h"
// BOOST
#include <boost/asio.hpp>

namespace dds
{
    class CAgentClient
    {
      public:
        CAgentClient(boost::asio::io_service& _service);

        virtual ~CAgentClient();

        void start();

        void stop();

      private:
        // void echo(const boost::asio::ip::tcp::endpoint& _ep, const std::string& _message);

        // void readHandler(const boost::system::error_code& _ec, std::size_t _bytesTransferred);

        // void writeHandler(const boost::system::error_code& _ec, size_t bytesTransferred);

        void connectHandler(const boost::system::error_code& _ec);

        // size_t readCompleteHandler(const boost::system::error_code& _ec, size_t _bytesTransferred);

        // void doRead();

        // void doWrite(const std::string& msg);

      private:
        void readHeader();
        void readBody();
        void processMessage();

        void writeMessage();

        boost::asio::ip::tcp::socket m_socket;
        boost::asio::io_service& m_service;
        CProtocolMessage m_currentMsg;
    };
}
#endif /* defined(__DDS__CAgentClient__) */
