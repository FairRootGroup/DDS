// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// BOOST
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>

// DDS
#include "AgentConnectionManager.h"
#include "Logger.h"
#include "MonitoringThread.h"

using namespace boost::asio;
using namespace std;
using namespace dds::agent_cmd;
using namespace dds::user_defaults_api;
using namespace dds::protocol_api;
using namespace MiscCommon;
using boost::asio::ip::tcp;

CAgentConnectionManager::CAgentConnectionManager(const SOptions_t& _options)
    : m_signals(m_context)
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

CAgentConnectionManager::~CAgentConnectionManager()
{
    stop();
}

void CAgentConnectionManager::doAwaitStop()
{
    m_signals.async_wait([this](boost::system::error_code /*ec*/, int /*signo*/) {
        // Stop transport engine
        stop();
    });
}

void CAgentConnectionManager::start()
{
    if (m_bStarted)
        return;

    // Generate protocol sender ID
    boost::hash<boost::uuids::uuid> uuid_hasher;
    uint64_t protocolHeaderID = uuid_hasher(boost::uuids::random_generator()());
    LOG(info) << "PROTOCOL HEADER ID: " << protocolHeaderID;

    m_bStarted = true;

    try
    {
        const float maxIdleTime = CUserDefaults::instance().getOptions().m_server.m_idleTime;
        CMonitoringThread::instance().start(maxIdleTime, []() { LOG(info) << "Idle callback called"; });

        // Start network channel. This is a blocking function call.
        createCommanderChannel(protocolHeaderID);

        startService(4, 6 + CUserDefaults::getNumLeaderFW());
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
        if (m_commanderChannel)
            m_commanderChannel->stopChannel();

        m_context.stop();
        m_intercomContext.stop();
    }
    catch (exception& e)
    {
        LOG(fatal) << e.what();
    }
    LOG(info) << "Shutting down DDS transport - DONE";
}

void CAgentConnectionManager::startService(size_t _numThreads, size_t _numIntercomThreads)
{
    LOG(MiscCommon::info) << "Starting DDS transport engine using " << _numThreads + _numIntercomThreads << " ("
                          << _numIntercomThreads << " for intercom)"
                          << " concurrent threads.";
    // Main service threads
    for (int x = 0; x < _numThreads; ++x)
    {
        m_workerThreads.create_thread([this]() {
            try
            {
                m_context.run();
            }
            catch (exception& ex)
            {
                LOG(MiscCommon::error) << "AgentConnectionManager main service: " << ex.what();
            }
        });
    }
    // Intercom service threads
    for (int x = 0; x < _numIntercomThreads; ++x)
    {
        m_workerThreads.create_thread([this]() {
            try
            {
                m_intercomContext.run();
            }
            catch (exception& ex)
            {
                LOG(MiscCommon::error) << "AgentConnectionManager intercom service: " << ex.what();
            }
        });
    }

    m_workerThreads.join_all();
}

void CAgentConnectionManager::createCommanderChannel(uint64_t _protocolHeaderID)
{
    // Read server info file
    const string sSrvCfg(CUserDefaults::instance().getServerInfoFileLocation());
    LOG(info) << "Reading server info from: " << sSrvCfg;
    if (sSrvCfg.empty())
        throw runtime_error("Cannot find server info file.");

    boost::property_tree::ptree pt;
    boost::property_tree::ini_parser::read_ini(sSrvCfg, pt);
    const string sHost(pt.get<string>("server.host"));
    const string sPort(pt.get<string>("server.port"));

    LOG(info) << "Contacting DDS commander on " << sHost << ":" << sPort;

    // Resolve endpoint iterator from host and port
    tcp::resolver resolver(m_context);
    tcp::resolver::query query(sHost, sPort);
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

    // Create new agent and push handshake message
    m_commanderChannel = CCommanderChannel::makeNew(m_context, _protocolHeaderID, m_intercomContext);
    m_commanderChannel->setNumberOfSlots(m_options.m_slots);

    // Subscribe to Shutdown command
    m_commanderChannel->registerHandler<cmdSHUTDOWN>(
        [this](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment) {
            this->on_cmdSHUTDOWN(_sender, _attachment, m_commanderChannel);
        });

    // Connect to DDS commander
    m_commanderChannel->connect(endpoint_iterator);
}

void CAgentConnectionManager::on_cmdSHUTDOWN(const SSenderInfo& _sender,
                                             SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment,
                                             CCommanderChannel::weakConnectionPtr_t _channel)
{
    stop();
}
