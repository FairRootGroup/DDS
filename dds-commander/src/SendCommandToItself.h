// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__SendCommandToItself__
#define __DDS__SendCommandToItself__

#include "boost/asio.hpp"

namespace dds
{
    class CProtocol;

    class CSendCommandToItself
    {
      public:
        CSendCommandToItself(boost::asio::io_service& _io_service, boost::asio::ip::tcp::resolver::iterator _endpoint_iterator);

      private:
        void handle_connect(const boost::system::error_code& err);
        void handle_read_headers(const boost::system::error_code& err);
        void handle_read_content(const boost::system::error_code& err);
        void processAdminConnection(int _serverSock);
        int processProtocolMsgs(int _serverSock, CProtocol* _protocol);

        void writeHandler(const boost::system::error_code& _ec, std::size_t _bytesTransferred);

      private:
        boost::asio::ip::tcp::resolver m_resolver;
        boost::asio::ip::tcp::socket m_socket;
        boost::asio::streambuf m_request;
        boost::asio::streambuf m_response;
    };
}

#endif
