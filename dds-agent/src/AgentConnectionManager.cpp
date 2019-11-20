// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// BOOST
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>

// DDS
#include "AgentConnectionManager.h"
#include "CommanderChannel.h"
#include "Logger.h"
#include "MonitoringThread.h"

using namespace boost::asio;
namespace bi = boost::interprocess;
using namespace std;
using namespace dds::agent_cmd;
using namespace dds::user_defaults_api;
using namespace dds::protocol_api;
using namespace dds::topology_api;
using namespace MiscCommon;
namespace sp = std::placeholders;
using boost::asio::ip::tcp;

CAgentConnectionManager::CAgentConnectionManager(const SOptions_t& _options)
    : m_signals(m_io_context)
    , m_options(_options)
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

        startService(6 + CUserDefaults::getNumLeaderFW());
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

        m_io_context.stop();
    }
    catch (exception& e)
    {
        LOG(fatal) << e.what();
    }
    LOG(info) << "Shutting down DDS transport - DONE";
}

void CAgentConnectionManager::startService(size_t _numThreads)
{
    LOG(MiscCommon::info) << "Starting DDS transport engine using " << _numThreads << " concurrent threads.";
    for (int x = 0; x < _numThreads; ++x)
    {
        m_workerThreads.create_thread([this]() {
            try
            {
                m_io_context.run();
            }
            catch (exception& ex)
            {
                LOG(MiscCommon::error) << "AgentConnectionManager: " << ex.what();
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
    tcp::resolver resolver(m_io_context);
    tcp::resolver::query query(sHost, sPort);
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

    // Create new agent and push handshake message
    m_commanderChannel = CCommanderChannel::makeNew(m_io_context, _protocolHeaderID);
    m_commanderChannel->setNumberOfSlots(m_options.m_slots);

    // Subscribe to Shutdown command
    m_commanderChannel->registerHandler<cmdSHUTDOWN>(
        [this](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment) {
            this->on_cmdSHUTDOWN(_sender, _attachment, m_commanderChannel);
        });

    // Call this callback when a user process is activated
    m_commanderChannel->registerHandler<EChannelEvents::OnAssignUserTask>([/*this*/](const SSenderInfo& _sender) {
        //
    });

    // Connect to DDS commander
    m_commanderChannel->connect(endpoint_iterator);
}

void CAgentConnectionManager::createSMLeaderChannel(uint64_t _protocolHeaderID)
{
    // Shared memory channel for communication with user task
    //    const CUserDefaults& userDefaults = CUserDefaults::instance();
    //    m_SMLeaderChannel = CSMLeaderChannel::makeNew(m_io_context,
    //                                                  userDefaults.getSMAgentLeaderOutputName(),
    //                                                  userDefaults.getSMAgentLeaderOutputName(),
    //                                                  _protocolHeaderID);
    //    m_SMLeaderChannel->registerHandler<EChannelEvents::OnLobbyMemberInfo>(
    //        [this](const SSenderInfo& _sender, const string& _name) {
    //            try
    //            {
    //                // Add output for lobby members, skipping output for itself
    //                if (_sender.m_ID != m_SMLeaderChannel->getProtocolHeaderID())
    //                {
    //                    auto p = m_commanderChannel->getSMFWChannel().lock();
    //                    p->addOutput(_sender.m_ID, _name);
    //                }
    //            }
    //            catch (exception& _e)
    //            {
    //                LOG(MiscCommon::error) << "Failed to open forwarder MQ " << _name << " of the new member "
    //                                       << _sender.m_ID << " error: " << _e.what();
    //            }
    //        });
    //
    //    LOG(info) << "SM channel: Leader created";
}

void CAgentConnectionManager::createSMIntercomChannel(uint64_t _protocolHeaderID)
{
    //    // Shared memory channel for communication with user task
    //    const CUserDefaults& userDefaults = CUserDefaults::instance();
    //    m_SMIntercomChannel = CSMIntercomChannel::makeNew(
    //        m_io_context, userDefaults.getSMInputName(), userDefaults.getSMOutputName(), _protocolHeaderID);
    //
    //    // TODO: Forwarding of update key commands without decoding using raw message API
    //    // Forward messages from shared memory to the agent.
    //    // For the moment we have to replace PHID of the intercom message (which is always 0) with the real PHID of
    //    the
    //    // agent.
    //    m_SMIntercomChannel->registerHandler<cmdCUSTOM_CMD>(
    //        [this](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdCUSTOM_CMD>::ptr_t _attachment) {
    //            // m_SMCommanderChannel->pushMsg<cmdCUSTOM_CMD>(*_attachment,
    //            m_SMCommanderChannel->getProtocolHeaderID());
    //        });
    //
    //    m_SMIntercomChannel->registerHandler<cmdUPDATE_KEY>(
    //        [this](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment) {
    //            // send_cmdUPDATE_KEY(_attachment);
    //        });
    //
    //    LOG(info) << "SM channel: Intercom created";
}

// void CAgentConnectionManager::createSMCommanderChannel(uint64_t _protocolHeaderID)
//{
//    const CUserDefaults& userDefaults = CUserDefaults::instance();
//    // Create shared memory agent channel
//    m_SMCommanderChannel = CSMCommanderChannel::makeNew(m_io_context,
//                                                        userDefaults.getSMAgentInputName(),
//                                                        userDefaults.getSMAgentOutputName(_protocolHeaderID),
//                                                        _protocolHeaderID);
//    m_SMCommanderChannel->addOutput(CSMCommanderChannel::EOutputID::Leader,
//    userDefaults.getSMAgentLeaderOutputName());
//
//    // Subscribe to reply command
//    m_SMCommanderChannel->registerHandler<cmdREPLY>(
//        [this](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdREPLY>::ptr_t _attachment) {
//            if (_attachment->m_srcCommand == cmdLOBBY_MEMBER_HANDSHAKE &&
//                _attachment->m_statusCode == (uint16_t)SReplyCmd::EStatusCode::ERROR)
//            {
//                stop();
//            }
//        });
//
//    LOG(info) << "SM channel: Agent is created";
//}

void CAgentConnectionManager::on_cmdSHUTDOWN(const SSenderInfo& _sender,
                                             SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment,
                                             CCommanderChannel::weakConnectionPtr_t _channel)
{
    //    // Commander requested to shutdown.
    //    if (m_SMLeaderChannel != nullptr)
    //    {
    //        // LOG(info) << "Sending SHUTDOWN to all lobby members";
    //        m_SMLeaderChannel->syncSendShutdownAll();
    //    }
    stop();
}
