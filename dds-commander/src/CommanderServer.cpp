// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// STD
#include <iostream>

// DDS
#include "CommanderServer.h"
#include "TalkToAgent.h"

// BOOST
#include "boost/asio.hpp"

using namespace boost::asio;
using namespace std::placeholders;
using namespace std;

CCommanderServer::CCommanderServer()
{
    m_service = new io_service();
    m_acceptor = new ip::tcp::acceptor(*m_service, ip::tcp::endpoint(ip::tcp::v4(), 8001));
}

CCommanderServer::~CCommanderServer()
{
    if (m_service)
        delete m_service;
    if (m_acceptor)
        delete m_acceptor;
}

void CCommanderServer::start()
{
    try
    {
        m_acceptor->listen();
        TalkToAgentPtr_t client = CTalkToAgent::makeNew(*m_service);
        m_acceptor->async_accept(client->socket(), std::bind(&CCommanderServer::acceptHandler, this, client, _1));
        m_service->run();
    }
    catch (exception& e)
    {
        // FIXME: log fail
    }
}

void CCommanderServer::stop()
{
    m_acceptor->close();
    m_service->stop();
    for (const auto& v : m_agents)
    {
        v->stop();
    }
    m_agents.clear();
}

void CCommanderServer::acceptHandler(TalkToAgentPtr_t _client, const boost::system::error_code& _ec)
{
    if (!_ec) // FIXME: Add proper error processing
    {
        _client->start();
        m_agents.push_back(_client);

        TalkToAgentPtr_t newClient = CTalkToAgent::makeNew(*m_service);
        m_acceptor->async_accept(newClient->socket(), std::bind(&CCommanderServer::acceptHandler, this, newClient, _1));
    }
    else
    {
    }
}
