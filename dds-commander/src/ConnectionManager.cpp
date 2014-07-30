// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
// DDS
#include "ConnectionManager.h"
#include "MonitoringThread.h"

using namespace boost::asio;
using namespace std;
using namespace dds;
using namespace MiscCommon;
namespace sp = std::placeholders;

CConnectionManager::CConnectionManager(const SOptions_t& _options,
                                       boost::asio::io_service& _io_service,
                                       boost::asio::ip::tcp::endpoint& _endpoint)
    : m_acceptor(_io_service, _endpoint)
    , m_signals(_io_service)
    , m_options(_options)
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

CConnectionManager::~CConnectionManager()
{
    // Delete server info file
    deleteServerInfoFile();
}

void CConnectionManager::doAwaitStop()
{
    m_signals.async_wait([this](boost::system::error_code /*ec*/, int /*signo*/)
                         {
                             // The server is stopped by cancelling all outstanding asynchronous
                             // operations. Once all operations have finished the io_service::run()
                             // call will exit.
                             stop();
                         });
}

void CConnectionManager::start()
{
    try
    {
        // Start monitoring thread
        CUserDefaults ud;
        ud.init(true);
        const float maxIdleTime = std::stof(ud.getValueForKey("general.idle_time"));

        CMonitoringThread::instance().start(maxIdleTime,
                                            []()
                                            { LOG(info) << "Idle callback called"; });
        //

        m_acceptor.listen();
        CTalkToAgent::connectionPtr_t client = CTalkToAgent::makeNew(m_acceptor.get_io_service());
        m_acceptor.async_accept(client->socket(), std::bind(&CConnectionManager::acceptHandler, this, client, sp::_1));

        // Create a server info file
        createServerInfoFile();

        m_acceptor.get_io_service().run();
    }
    catch (exception& e)
    {
        LOG(fatal) << e.what();
    }
}

void CConnectionManager::stop()
{
    try
    {
        m_acceptor.close();
        m_acceptor.get_io_service().stop();
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

void CConnectionManager::acceptHandler(CTalkToAgent::connectionPtr_t _client, const boost::system::error_code& _ec)
{
    if (!_ec) // FIXME: Add proper error processing
    {
        _client->start();
        m_agents.push_back(_client);

        CTalkToAgent::connectionPtr_t newClient = CTalkToAgent::makeNew(m_acceptor.get_io_service());
        m_acceptor.async_accept(newClient->socket(),
                                std::bind(&CConnectionManager::acceptHandler, this, newClient, sp::_1));
    }
    else
    {
    }
}

void CConnectionManager::createServerInfoFile() const
{
    const string sSrvCfg(CUserDefaults::getServerInfoFile());
    LOG(info) << "Createing a server info file: " << sSrvCfg;
    ofstream f(sSrvCfg.c_str());
    if (!f.is_open() || !f.good())
    {
        string msg("Could not open a server info configuration file: ");
        msg += sSrvCfg;
        throw runtime_error(msg);
    }

    string srvHost;
    get_hostname(&srvHost);
    string srvUser;
    get_cuser_name(&srvUser);

    f << "[server]\n"
      << "host=" << srvHost << "\n"
      << "user=" << srvUser << "\n"
      << "port=" << m_acceptor.local_endpoint().port() << "\n" << endl;
}

void CConnectionManager::deleteServerInfoFile() const
{
    const string sSrvCfg(CUserDefaults::getServerInfoFile());
    if (sSrvCfg.empty())
        return;

    // TODO: check error code
    unlink(sSrvCfg.c_str());
}
