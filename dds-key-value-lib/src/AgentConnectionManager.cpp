// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// BOOST
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
// DDS
#include "AgentConnectionManager.h"
#include "Logger.h"
#include "MonitoringThread.h"

using namespace boost::asio;
using namespace std;
using namespace dds::user_defaults_api;
using namespace dds::key_value_api;
using namespace dds::protocol_api;
using namespace MiscCommon;
namespace sp = std::placeholders;
using boost::asio::ip::tcp;

CAgentConnectionManager::CAgentConnectionManager()
    : m_syncHelper(nullptr)
    , m_signals(m_service)
    , m_bStarted(false)
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
    stop();
}

void CAgentConnectionManager::doAwaitStop()
{
    m_signals.async_wait([this](boost::system::error_code /*ec*/, int /*signo*/)
                         {
                             // Stop transport engine
                             stop();
                         });
}

void CAgentConnectionManager::start()
{
    if (m_bStarted)
        return;

    m_bStarted = true;
    try
    {
        const float maxIdleTime = CUserDefaults::instance().getOptions().m_server.m_idleTime;

        CMonitoringThread::instance().start(maxIdleTime,
                                            []()
                                            {
                                                LOG(info) << "Idle callback called";
                                            });

        // Read server info file
        const string sSrvCfg(CUserDefaults::instance().getAgentInfoFileLocation());
        LOG(info) << "Reading server info from: " << sSrvCfg;
        if (sSrvCfg.empty())
            throw runtime_error("Cannot find agent info file.");

        boost::property_tree::ptree pt;
        boost::property_tree::ini_parser::read_ini(sSrvCfg, pt);
        const string sHost(pt.get<string>("agent.host"));
        const string sPort(pt.get<string>("agent.port"));

        LOG(info) << "Contacting DDS agent on " << sHost << ":" << sPort;

        // Resolve endpoint iterator from host and port
        tcp::resolver resolver(m_service);
        tcp::resolver::query query(sHost, sPort);
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

        // Create new communication channel and push handshake message
        m_channel = CAgentChannel::makeNew(m_service);
        // Subscribe to Shutdown command
        std::function<bool(SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment, CAgentChannel * _channel)>
            fSHUTDOWN = [this](SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment, CAgentChannel* _channel) -> bool
        {
            // TODO: adjust the algorithm if we would need to support several agents
            // we have only one agent (newAgent) at the moment
            return this->on_cmdSHUTDOWN(_attachment, m_channel);
        };
        m_channel->registerMessageHandler<cmdSHUTDOWN>(fSHUTDOWN);

        m_channel->subscribeOnEvent(EChannelEvents::OnConnected,
                                    [this](CAgentChannel* _channel)
                                    {
                                        m_channel->m_syncHelper = m_syncHelper;
                                    });
        m_channel->connect(endpoint_iterator);

        // Don't block main thread, start transport service on a thread-pool
        const int nConcurrentThreads(2);
        LOG(MiscCommon::info) << "Starting DDS transport engine using " << nConcurrentThreads << " concurrent threads.";
        for (int x = 0; x < nConcurrentThreads; ++x)
        {
            m_workerThreads.create_thread(boost::bind(&boost::asio::io_service::run, &(m_service)));
        }
    }
    catch (exception& e)
    {
        LOG(fatal) << e.what();
    }
}

void CAgentConnectionManager::stop()
{
    if (!m_bStarted)
        return;

    m_bStarted = false;

    LOG(info) << "Shutting down DDS transport...";

    try
    {
        m_service.stop();
        m_workerThreads.join_all();
        if (!getAgentChannel().expired())
        {
            auto p = getAgentChannel().lock();
            p->stop();
        }
    }
    catch (exception& e)
    {
        LOG(fatal) << e.what();
    }
    LOG(info) << "Shutting down DDS transport - DONE";
}

bool CAgentConnectionManager::on_cmdSHUTDOWN(SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment,
                                             CAgentChannel::weakConnectionPtr_t _channel)
{
    stop();
    return true;
}

int CAgentConnectionManager::updateKey(const SUpdateKeyCmd& _cmd)
{
    try
    {
        if (getAgentChannel().expired())
            throw runtime_error("Agent channel is not offline");

        auto p = getAgentChannel().lock();
        p->pushMsg<cmdUPDATE_KEY>(_cmd);
        return 0;
    }
    catch (const exception& _e)
    {
        LOG(fatal) << "Fail to push the property update: " << _cmd << "; Error: " << _e.what();
    }
    LOG(fatal) << "Fail to push the property update: " << _cmd;
    return 1;
}
