// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "TalkToAgent.h"
#include "Logger.h"
#include "ProtocolCommands.h"
// BOOST
#include "boost/asio.hpp"

using namespace MiscCommon;
using namespace dds;
using namespace std;

TalkToAgentPtr_t CTalkToAgent::makeNew(boost::asio::io_service& _service)
{
    TalkToAgentPtr_t newObject(new CTalkToAgent(_service));
    return newObject;
}

CTalkToAgent::CTalkToAgent(boost::asio::io_service& _service)
    : m_socket(_service)
    , m_started(false)
{
}

void CTalkToAgent::start()
{
    m_started = true;

    readHeader();
}

void CTalkToAgent::stop()
{
    if (!m_started)
        return;
    m_started = false;
    m_socket.close();
}

boost::asio::ip::tcp::socket& CTalkToAgent::socket()
{
    return m_socket;
}

void CTalkToAgent::readHeader()
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

void CTalkToAgent::readBody()
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

void CTalkToAgent::processMessage()
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
