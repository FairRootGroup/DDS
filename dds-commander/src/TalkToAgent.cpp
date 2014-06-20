// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "TalkToAgent.h"
#include "Logger.h"
// BOOST
#include "boost/asio.hpp"
// STD
#include <iostream>
#include <fstream>

using namespace MiscCommon;

CTalkToAgent::CTalkToAgent(boost::asio::io_service& _service)
    : m_socket(_service)
    , m_timer(_service)
    , m_started(false)
{
}

void CTalkToAgent::start()
{
    m_started = true;
    m_lastPing = boost::posix_time::microsec_clock::local_time();

    doRead();
}

TalkToAgentPtr_t CTalkToAgent::makeNew(boost::asio::io_service& _service)
{
    TalkToAgentPtr_t newObject(new CTalkToAgent(_service));
    return newObject;
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

void CTalkToAgent::readHandler(const boost::system::error_code& _ec, size_t _bytesTransferred)
{
    if (_ec)
        stop();

    if (!m_started)
        return;

    std::string msg(m_readBuffer, _bytesTransferred);
    if (msg.find("ping") == 0)
        pingRequest();
    else
        LOG(error) << "Invalid request " << msg << std::endl;
}

void CTalkToAgent::writeHandler(const boost::system::error_code& _ec, size_t bytesTransferred)
{
    doRead();
}

size_t CTalkToAgent::readCompleteHandler(const boost::system::error_code& _ec, size_t _bytesTransferred)
{
    if (_ec)
        return 0;
    bool found = std::find(m_readBuffer, m_readBuffer + _bytesTransferred, '\n') < m_readBuffer + _bytesTransferred;
    return found ? 0 : 1;
}

void CTalkToAgent::pingRequest()
{
    doWrite("ping ok\n");

    // std::ofstream fout("dds-commander-log.log", std::ios::app);
    // boost::log::sources::logger lg;
    // BOOST_LOG_SEV(lg, boost::log::trivial::info) << "ping OK";
    try
    {
        //    fout << "before first log\n";
        LOG(info) << "first ping OK";
        //    fout << "afer first log\n";
        LOG(error) << "second ping OK";
        //    fout << "after secong log\n";
    }
    catch (std::exception& e)
    {
        //    fout << "exception e.what=" << e.what() << std::endl;
        // LOG(error) << "exception e.what=" << e.what();
    }
    // fout.flush();
    // fout.close();
}

void CTalkToAgent::checkPingHandler()
{
    boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
    if ((now - m_lastPing).total_milliseconds() > 5000)
    {
        stop();
    }
    m_lastPing = boost::posix_time::microsec_clock::local_time();
}

void CTalkToAgent::checkPing()
{
    m_timer.expires_from_now(boost::posix_time::millisec(5000));
    m_timer.async_wait(std::bind(&CTalkToAgent::checkPingHandler, shared_from_this()));
}

void CTalkToAgent::doRead()
{
    async_read(m_socket,
               boost::asio::buffer(m_readBuffer),
               std::bind(&CTalkToAgent::readCompleteHandler, shared_from_this(), std::placeholders::_1, std::placeholders::_2),
               std::bind(&CTalkToAgent::readHandler, shared_from_this(), std::placeholders::_1, std::placeholders::_2));

    checkPing();
}

void CTalkToAgent::doWrite(const std::string& msg)
{
    if (!m_started)
        return;
    std::copy(msg.begin(), msg.end(), m_writeBuffer);
    async_write(m_socket,
                boost::asio::buffer(m_writeBuffer, msg.size()),
                std::bind(&CTalkToAgent::writeHandler, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
}
