// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "SendCommandToItself.h"
#include "Logger.h"
// BOOST
#include "boost/asio.hpp"
#include "boost/bind.hpp"

using namespace MiscCommon;
using namespace std;
using boost::asio::ip::tcp;

CSendCommandToItself::CSendCommandToItself(boost::asio::io_service& _io_service, tcp::resolver::iterator _endpoint_iterator)
    : m_resolver(_io_service)
    , m_socket(_io_service)
{
    std::ostream request_stream(&m_request);
    request_stream << "TEST_CMD";

    boost::asio::async_connect(m_socket, _endpoint_iterator, boost::bind(&CSendCommandToItself::handle_connect, this, boost::asio::placeholders::error));
}

void CSendCommandToItself::handle_resolve(const boost::system::error_code& err, tcp::resolver::iterator endpoint_iterator)
{
    LOG(debug) << "CSendCommandToItself::handle_resolve";
    if (!err)
    {
        // Attempt a connection to each endpoint in the list until we
        // successfully establish a connection.
        boost::asio::async_connect(m_socket, endpoint_iterator, boost::bind(&CSendCommandToItself::handle_connect, this, boost::asio::placeholders::error));
    }
    else
    {
        LOG(log_stderr) << "Error: " << err.message();
    }
}

void CSendCommandToItself::handle_connect(const boost::system::error_code& err)
{
    LOG(debug) << "CSendCommandToItself::handle_connect";
    if (!err)
    {
        // The connection was successful. Send the request.
        boost::asio::async_write(m_socket, m_request, boost::bind(&CSendCommandToItself::handle_write_request, this, boost::asio::placeholders::error));
    }
    else
    {
        LOG(log_stderr) << "Error: " << err.message();
    }
}

void CSendCommandToItself::handle_write_request(const boost::system::error_code& err)
{
    LOG(debug) << "CSendCommandToItself::handle_write_request";
    if (!err)
    {
        // Read the response status line. The response_ streambuf will
        // automatically grow to accommodate the entire line. The growth may be
        // limited by passing a maximum size to the streambuf constructor.
        boost::asio::async_read_until(
            m_socket, m_response, "\r\n", boost::bind(&CSendCommandToItself::handle_read_status_line, this, boost::asio::placeholders::error));
    }
    else
    {
        LOG(log_stderr) << "Error: " << err.message();
    }
}

void CSendCommandToItself::handle_read_status_line(const boost::system::error_code& err)
{
    LOG(debug) << "CSendCommandToItself::handle_read_status_line";
    if (!err)
    {
        // Check that response is OK.
        std::istream response_stream(&m_response);
        std::string http_version;
        response_stream >> http_version;
        unsigned int status_code;
        response_stream >> status_code;
        std::string status_message;
        std::getline(response_stream, status_message);
        if (!response_stream || http_version.substr(0, 5) != "HTTP/")
        {
            LOG(log_stderr) << "Invalid response\n";
            return;
        }
        if (status_code != 200)
        {
            LOG(log_stderr) << "Response returned with status code ";
            LOG(log_stderr) << status_code;
            return;
        }

        // Read the response headers, which are terminated by a blank line.
        boost::asio::async_read_until(
            m_socket, m_response, "\r\n\r\n", boost::bind(&CSendCommandToItself::handle_read_headers, this, boost::asio::placeholders::error));
    }
    else
    {
        LOG(log_stderr) << "Error: " << err;
    }
}

void CSendCommandToItself::handle_read_headers(const boost::system::error_code& err)
{
    LOG(debug) << "CSendCommandToItself::handle_read_headers";
    if (!err)
    {
        // Process the response headers.
        std::istream response_stream(&m_response);
        std::string header;
        while (std::getline(response_stream, header) && header != "\r")
            std::cout << header << "\n";
        std::cout << "\n";

        // Write whatever content we already have to output.
        if (m_response.size() > 0)
            std::cout << &m_response;

        // Start reading remaining data until EOF.
        boost::asio::async_read(m_socket,
                                m_response,
                                boost::asio::transfer_at_least(1),
                                boost::bind(&CSendCommandToItself::handle_read_content, this, boost::asio::placeholders::error));
    }
    else
    {
        LOG(log_stderr) << "Error: " << err;
    }
}

void CSendCommandToItself::handle_read_content(const boost::system::error_code& err)
{
    LOG(debug) << "CSendCommandToItself::handle_read_content";
    if (!err)
    {
        // Write all of the data that has been read so far.
        std::cout << &m_response;

        // Continue reading remaining data until EOF.
        boost::asio::async_read(m_socket,
                                m_response,
                                boost::asio::transfer_at_least(1),
                                boost::bind(&CSendCommandToItself::handle_read_content, this, boost::asio::placeholders::error));
    }
    else if (err != boost::asio::error::eof)
    {
        LOG(log_stderr) << "Error: " << err;
    }
}
