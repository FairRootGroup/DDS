// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "SendCommandToItself.h"
#include "Logger.h"
#include "Protocol.h"
#include "ProtocolCommands.h"
// BOOST
#include "boost/asio.hpp"
#include "boost/bind.hpp"

using namespace MiscCommon;
using namespace std;
using namespace dds;
using boost::asio::ip::tcp;
//=============================================================================
CSendCommandToItself::CSendCommandToItself(boost::asio::io_service& _io_service, tcp::resolver::iterator _endpoint_iterator)
    : m_resolver(_io_service)
    , m_socket(_io_service)
{
    std::ostream request_stream(&m_request);
    request_stream << "TEST_CMD";

    boost::asio::async_connect(m_socket, _endpoint_iterator, boost::bind(&CSendCommandToItself::handle_connect, this, boost::asio::placeholders::error));
}
//=============================================================================
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
//=============================================================================
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
//=============================================================================
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
//=============================================================================
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
//=============================================================================
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
//=============================================================================
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
//=============================================================================
void CSendCommandToItself::processAdminConnection(int _serverSock)
{
    LOG(debug) << "receiving server commands";
    CProtocol protocol;
    CProtocol::EStatus_t ret = protocol.read(_serverSock);
    /*    switch (ret)
        {
            case CProtocol::stDISCONNECT:
                throw runtime_error("a disconnect has been detected on the adminChannel");
            case CProtocol::stAGAIN:
            case CProtocol::stOK:
            {
                while (protocol.checkoutNextMsg())
                {
                    processProtocolMsgs(_serverSock, &protocol);
                }
            }
            break;
        }*/
}
//=============================================================================
int CSendCommandToItself::processProtocolMsgs(int _serverSock, CProtocol* _protocol)
{
    /* BYTEVector_t data;
     SMessageHeader header = _protocol->getMsg( &data );
     stringstream ss;
     ss << "CMD: " <<  header.m_cmd;
     InfoLog( ss.str() );
     switch( static_cast<ECmdType>( header.m_cmd ) )
     {
             //case cmdVERSION_BAD:
             //    break;
         case cmdGET_HOST_INFO:
         {
             InfoLog( "Server requests host information." );
             SHostInfoCmd h;
             get_cuser_name( &h.m_username );
             get_hostname( &h.m_host );
             h.m_xpdPort = m_xpdPort;
             // retrieve submit time
             string sSubmitTime;
             get_env( "POD_WN_SUBMIT_TIMESTAMP", &sSubmitTime );
             if( !sSubmitTime.empty() )
             {
                 stringstream ssBuf( sSubmitTime );
                 ssBuf >> h.m_timeStamp;
             }

             BYTEVector_t data_to_send;
             h.convertToData( &data_to_send );
             _protocol->write( _serverSock, static_cast<uint16_t>( cmdHOST_INFO ), data_to_send );
         }
             break;
         case cmdGET_ID:
         {
             SIdCmd id;
             id.m_id = m_id;
             BYTEVector_t data_to_send;
             id.convertToData( &data_to_send );
             _protocol->write( _serverSock, static_cast<uint16_t>( cmdID ), data_to_send );
         }
             break;
         case cmdSET_ID:
         {
             SIdCmd id;
             id.convertFromData( data );
             m_id = id.m_id;
             // saving ID to the ID file
             string id_file( g_wnIDFile );
             smart_path( &id_file );
             ofstream f( id_file.c_str() );
             if( !f.is_open() )
                 FaultLog( 1, "Can't write to id file: " + id_file );
             else
                 f << m_id;

             stringstream ss;
             ss << "Server has assigned ID = " << m_id << " to this worker.";
             InfoLog( ss.str() );
         }
             break;
         case cmdUSE_PACKETFORWARDING_PROOF:
             // going out of the admin channel and start the packet forwarding
             InfoLog( "Server requests to use a packet forwarding for PROOF packages." );
             m_isDirect = false;
             return 1;
         case cmdUSE_DIRECT_PROOF:
             // TODO: we keep admin channel open and start the monitoring (proof status) thread
             m_isDirect = true;
             InfoLog( "Server requests to use a direct connection for PROOF packages." );
             break;
         case cmdGET_WRK_NUM:
         {
             // reuse SIdCmd
             SIdCmd wn_num;
             wn_num.m_id = m_numberOfPROOFWorkers;
             BYTEVector_t data_to_send;
             wn_num.convertToData( &data_to_send );
             _protocol->write( _serverSock, static_cast<uint16_t>( cmdWRK_NUM ), data_to_send );
             stringstream ss;
             ss << "A number of PROOF workers [" << m_numberOfPROOFWorkers << "] has been sent to server.";
             InfoLog( ss.str() );
         }
             break;
         case cmdSHUTDOWN:
             InfoLog( "Server requests to shut down..." );
             graceful_quit = true;
             throw runtime_error( "stop admin channel." );
         default:
             WarningLog( 0, "Unexpected message in the admin channel" );
             break;
     }*/
    return 0;
}
