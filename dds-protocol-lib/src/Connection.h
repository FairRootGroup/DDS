// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__Connection__
#define __DDS__Connection__

#include <iostream>

// STD
#include <deque>
// BOOST
#include "boost/noncopyable.hpp"
#include "boost/asio.hpp"
// DDS
#include "ProtocolMessage.h"

namespace dds
{
    class CConnection;
    typedef std::shared_ptr<CConnection> ConnectionPtr_t;
    typedef std::vector<ConnectionPtr_t> ConnectionPtrVector_t;
    typedef std::deque<CProtocolMessage> MessageQueue_t;

    class CConnection : public std::enable_shared_from_this<CConnection>, boost::noncopyable
    {
        CConnection(boost::asio::io_service& _service);

      public:
        static ConnectionPtr_t makeNew(boost::asio::io_service& _service);
        void start();
        void stop();

        //      boost::asio::ip::tcp::socket& socket();

      private:
        void readHeader();
        void readBody();
        void writeMessage();
        void processMessage();

      private:
        boost::asio::ip::tcp::socket m_socket;
        bool m_started;
        CProtocolMessage m_currentMsg;

        MessageQueue_t m_readMessageQueue;
    };
}

#endif /* defined(__DDS__Connection__) */
