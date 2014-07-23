// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "Connection.h"
#include "Logger.h"
#include "ProtocolCommands.h"
// BOOST
#include "boost/asio.hpp"

using namespace MiscCommon;
using namespace dds;
using namespace std;

ConnectionPtr_t CConnection::makeNew(boost::asio::io_service& _service)
{
    ConnectionPtr_t newObject(new CConnection(_service));
    return newObject;
}

CConnection::CConnection(boost::asio::io_service& _service)
    : m_socket(_service)
    , m_started(false)
    , m_currentMsg()
    , m_readMessageQueue()
{
}

void CConnection::start()
{
    m_started = true;

    readHeader();
}

void CConnection::stop()
{
    if (!m_started)
        return;
    m_started = false;
    m_socket.close();
}

// boost::asio::ip::tcp::socket& CTalkToAgent::socket()
//{
//    return m_socket;
//}

void CConnection::readHeader()
{
    auto self(shared_from_this());
    boost::asio::async_read(m_socket,
                            boost::asio::buffer(m_currentMsg.data(), CProtocolMessage::header_length),
                            [this, self](boost::system::error_code ec, std::size_t /*length*/)
                            {
        if (!ec && m_currentMsg.decode_header())
        {
            // If the header is ok, recieve the body of the message
            readBody();
        }
        else
        {
            stop();
        }
    });
}

void CConnection::readBody()
{
    auto self(shared_from_this());
    boost::asio::async_read(m_socket,
                            boost::asio::buffer(m_currentMsg.body(), m_currentMsg.body_length()),
                            [this, self](boost::system::error_code ec, std::size_t /*length*/)
                            {
        if (!ec)
        {
            stringstream ss;
            ss << "Received from Agent: ";
            m_currentMsg.printData(ss);
            LOG(debug) << ss.str();
            // processe recieved message
            processMessage();
            // Read next message
            readHeader();
        }
        else
        {
            stop();
        }
    });
}

void CConnection::writeMessage()
{
    auto writeHandler = [this](boost::system::error_code ec, std::size_t /*_bytesTransferred*/)
    {
        if (!ec)
        {
            // If the header is ok, recieve the body of the message
            // readBody();
            LOG(info) << "Data successfully sent";
        }
        else
        {
            // stop();
            LOG(info) << "Error sending data";
        }
    };

    SVersionCmd ver_src;
    // ver_src.m_version = 2;
    BYTEVector_t data_to_send;
    ver_src.convertToData(&data_to_send);
    CProtocolMessage msg;
    msg.encode_message(cmdHANDSHAKE, data_to_send);

    async_write(m_socket, boost::asio::buffer(msg.data(), msg.length()), writeHandler);
}

void CConnection::processMessage()
{
    switch (m_currentMsg.header().m_cmd)
    {
        case cmdHANDSHAKE:
            SVersionCmd ver;
            ver.convertFromData(m_currentMsg.bodyToContainer());
            // send shutdown if versions are incompatible
            if (ver != SVersionCmd())
            {
                LOG(warning) << "Client's protocol version is incompatable. Client: " << m_socket.remote_endpoint().address().to_string();
                CProtocolMessage msg;
                msg.encode_message(cmdSHUTDOWN, BYTEVector_t());

                auto self(shared_from_this());
                async_write(m_socket,
                            boost::asio::buffer(msg.data(), msg.length()),
                            [this, self](boost::system::error_code ec, std::size_t /*length*/)
                            {
                    if (!ec)
                    {
                        // write_msgs_.pop_front();
                        // if (!write_msgs_.empty())
                        // {
                        //     do_write();
                        // }
                    }
                    else
                    {
                        // room_.leave(shared_from_this());
                    }
                });
            }
            break;
    }
}
