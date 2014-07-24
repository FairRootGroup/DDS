// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// STD
#include <iostream>
// BOOST
#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
// DDS
#include "AgentConnectionManager.h"
#include "TalkToCommander.h"
#include "Logger.h"
// API
//#include <signal.h>

using namespace boost::asio;
using namespace std;
using namespace dds;
using namespace MiscCommon;
namespace sp = std::placeholders;

CAgentConnectionManager::CAgentConnectionManager(const SOptions_t& _options, boost::asio::io_service& _service)
    : m_service(_service)
    , m_signals(_service)
    , m_options(_options)
    , m_agents()
{
    // Register to handle the signals that indicate when the server should exit.
    // It is safe to register for the same signal multiple times in a program,
    // provided all registration for the specified signal is made through Asio.
    m_signals.add(SIGINT);
    m_signals.add(SIGTERM);
#if defined(SIGQUIT)
    m_signals.add(SIGQUIT);
#endif // defined(SIGQUIT)

    doAwaitStop();
}

CAgentConnectionManager::~CAgentConnectionManager()
{
}

void CAgentConnectionManager::doAwaitStop()
{
    m_signals.async_wait([this](boost::system::error_code /*ec*/, int /*signo*/)
                         {
                             // The server is stopped by cancelling all outstanding asynchronous
                             // operations. Once all operations have finished the io_service::run()
                             // call will exit.
                             stop();
                         });
}

void CAgentConnectionManager::start()
{
    try
    {
        // Read server info file
        const string sSrvCfg(CUserDefaults::getServerInfoFile());
        LOG(info) << "Reading server info from: " << sSrvCfg;
        if (sSrvCfg.empty())
            throw runtime_error("Can't find server info file.");

        boost::property_tree::ptree pt;
        boost::property_tree::ini_parser::read_ini(sSrvCfg, pt);
        const string sHost(pt.get<string>("server.host"));
        const string sPort(pt.get<string>("server.port"));

        LOG(info) << "Contacting DDS commander on " << sHost << ":" << sPort;

        // Resolve endpoint iterator from host and port
        boost::asio::ip::tcp::resolver resolver(m_service);
        boost::asio::ip::tcp::resolver::query query(sHost, sPort);
        boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

        // Create new agent and push hadshake message
        CTalkToCommander::connectionPtr_t newAgent = CTalkToCommander::makeNew(m_service);
        boost::asio::async_connect(newAgent->socket(),
                                   endpoint_iterator,
                                   [this, &newAgent](boost::system::error_code ec, ip::tcp::resolver::iterator)
                                   {
            if (!ec)
            {
                // Create handshake message which is the first one for all agents
                SVersionCmd ver_src;
                BYTEVector_t data_to_send;
                ver_src.convertToData(&data_to_send);
                CProtocolMessage msg;
                msg.encode_message(cmdHANDSHAKE_AGENT, data_to_send);

                newAgent->pushMsg(msg);
                newAgent->start();
            }
            else
            {
                LOG(fatal) << "Can not connect to server: " << ec.message();
            }
        });

        m_service.run();
    }
    catch (exception& e)
    {
        LOG(fatal) << e.what();
    }
}

void CAgentConnectionManager::stop()
{
    try
    {
        m_service.stop();
        for (const auto& v : m_agents)
        {
            v->stop();
        }
        m_agents.clear();
    }
    catch (exception& e)
    {
        LOG(fatal) << e.what();
    }
}
