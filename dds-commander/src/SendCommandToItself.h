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
        void handle_resolve(const boost::system::error_code& err, boost::asio::ip::tcp::resolver::iterator endpoint_iterator);
        void handle_connect(const boost::system::error_code& err);
        void handle_write_request(const boost::system::error_code& err);
        void handle_read_status_line(const boost::system::error_code& err);
        void handle_read_headers(const boost::system::error_code& err);
        void handle_read_content(const boost::system::error_code& err);
        void processAdminConnection(int _serverSock);
        int processProtocolMsgs(int _serverSock, CProtocol* _protocol);

      private:
        boost::asio::ip::tcp::resolver m_resolver;
        boost::asio::ip::tcp::socket m_socket;
        boost::asio::streambuf m_request;
        boost::asio::streambuf m_response;
    };
}

#endif
