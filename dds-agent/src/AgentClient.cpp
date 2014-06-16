// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "AgentClient.h"
#include "Logger.h"
// STD
#include <functional>

using namespace std::placeholders;
using namespace boost::asio;
using namespace boost::log::trivial;

CAgentClient::CAgentClient()
    : m_resolver(m_service)
    , m_socket(m_service)
{
}

CAgentClient::~CAgentClient()
{
}

void CAgentClient::start()
{
    LOG(info) << "Starting agent...";
    // boost::asio::ip::tcp::resolver::query query("127.0.0.1", "8001");
    // m_resolver.async_resolve(query, std::bind(&CAgentClient::resolveHandler, this));

    ip::tcp::endpoint ep(ip::address::from_string("127.0.0.1"), 8001);
    m_socket.async_connect(ep, std::bind(&CAgentClient::connectHandler, this, std::placeholders::_1));
    m_service.run();
}

void CAgentClient::stop()
{
    LOG(info) << "Stoping agent...";
    m_service.stop();
    m_resolver.cancel();
    m_socket.close();
}

void CAgentClient::readHandler(const boost::system::error_code& _ec, std::size_t _bytesTransferred)
{
    if (!_ec)
    {
        std::string msg(m_readBuffer, _bytesTransferred);
        LOG(info) << "Server response: " << msg;
        sleep(1);

        doWrite("ping\n");
    }
}

void CAgentClient::writeHandler(const boost::system::error_code& _ec, std::size_t _bytesTransferred)
{
    doRead();
}

void CAgentClient::connectHandler(const boost::system::error_code& _ec)
{
    if (!_ec)
    {
        doWrite("ping\n");
    }
}

// void CAgentClient::resolveHandler(const boost::system::error_code& _ec, boost::asio::ip::tcp::resolver::iterator _it)
//{
//    if (!_ec)
//    {
//        m_socket.async_connect(*_it, std::bind(&CAgentClient::connectHandler, this));
//    }
//}

size_t CAgentClient::readCompleteHandler(const boost::system::error_code& _ec, size_t _bytesTransferred)
{
    if (_ec)
        return 0;
    bool found = std::find(m_readBuffer, m_readBuffer + _bytesTransferred, '\n') < m_readBuffer + _bytesTransferred;
    return found ? 0 : 1;
}

void CAgentClient::doRead()
{
    async_read(m_socket,
               boost::asio::buffer(m_readBuffer),
               std::bind(&CAgentClient::readCompleteHandler, this, std::placeholders::_1, std::placeholders::_2),
               std::bind(&CAgentClient::readHandler, this, std::placeholders::_1, std::placeholders::_2));
}

void CAgentClient::doWrite(const std::string& msg)
{
    std::copy(msg.begin(), msg.end(), m_writeBuffer);
    async_write(
        m_socket, boost::asio::buffer(m_writeBuffer, msg.size()), std::bind(&CAgentClient::writeHandler, this, std::placeholders::_1, std::placeholders::_2));
}
