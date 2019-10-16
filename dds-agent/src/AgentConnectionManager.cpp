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
#include "SMCommanderChannel.h"

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
    , m_topo()
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

    typedef std::shared_ptr<bi::named_mutex> namedMutexPtr_t;
    namedMutexPtr_t leaderMutex;
    try
    {
        const float maxIdleTime = CUserDefaults::instance().getOptions().m_server.m_idleTime;
        CMonitoringThread::instance().start(maxIdleTime, []() { LOG(info) << "Idle callback called"; });

        // TODO: FIXME: Don't forget to delete mutex in scout
        // Open or create leader mutex

        const CUserDefaults& userDefaults = CUserDefaults::instance();
        string smName(userDefaults.getAgentNamedMutexName());
        try
        {
            leaderMutex = make_shared<bi::named_mutex>(bi::open_or_create, smName.c_str());
        }
        catch (bi::interprocess_exception& ex)
        {
            // TODO: FIXME: Log the error and process further
            // If we can't allocate mutex - fallback to the direct connection to the DDS commander

            LOG(fatal) << "Can't start the DDS Agent: named mutex (" << smName
                       << ") can't be created or opened: " << ex.what();
            return;
        }

        bool locked = leaderMutex->try_lock();
        // If locked then it's a lobby leader
        if (locked)
        {
            // TODO: FIXME:
            // - create shared memory
            // - error processing if memory can't be created
            // - create network channel

            LOG(info) << "Master agent";

            // To communicate with tasks
            //   createSMIntercomChannel(protocolHeaderID);
            //   createSMLeaderChannel(protocolHeaderID);
            //   createSMCommanderChannel(protocolHeaderID);
            // Start network channel. This is a blocking function call.
            createCommanderChannel(protocolHeaderID);

            // Start listening for messages from shared memory
            // m_SMIntercomChannel->start();
            // Start listening for messages from shared memory
            //  m_SMLeaderChannel->start();
            // Start shared memory agent channel
            // m_SMCommanderChannel->start();

            startService(6 + CUserDefaults::getNumLeaderFW());

            leaderMutex->unlock();
        }
        else
        {
            //            // TODO: FIXME:
            //            // - open shared memory
            //            // - error processing if memory can't be opened
            //
            //            // TODO: FIXME:
            //            // - block execution, channels are working in threads
            //            // leaderMutex->lock();
            //
            //            LOG(info) << "Lobby status: member";
            //
            //            createSMIntercomChannel(protocolHeaderID);
            //            // Blocking function call
            //            createSMCommanderChannel(protocolHeaderID);
            //
            //            // Start listening for messages from shared memory
            //            m_SMIntercomChannel->start();
            //            // Start shared memory agent channel
            //            m_SMCommanderChannel->start();
            //
            //            startService(7);
        }

        // Free mutex
        leaderMutex.reset();
    }
    catch (exception& e)
    {
        leaderMutex->unlock();
        leaderMutex.reset();
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
        //        if (m_SMCommanderChannel)
        //            m_SMCommanderChannel->stop();
        //        if (m_SMIntercomChannel)
        //            m_SMIntercomChannel->stop();
        //        if (m_SMLeaderChannel)
        //            m_SMLeaderChannel->stop();
        if (m_commanderChannel)
        {
            // Stop forwarder's readMessage thread
            //            auto pSCFW = m_commanderChannel->getSMFWChannel().lock();
            //            if (pSCFW)
            //                pSCFW->stop();
            m_commanderChannel->stopChannel();
        }
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

    // Subscribe for cmdBINARY_ATTACHMENT_RECEIVED
    m_commanderChannel->registerHandler<cmdBINARY_ATTACHMENT_RECEIVED>(
        [this](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdBINARY_ATTACHMENT_RECEIVED>::ptr_t _attachment) {
            this->on_cmdBINARY_ATTACHMENT_RECEIVED(_sender, _attachment, m_commanderChannel);
        });

    // Subscribe for cmdSIMPLE_MSG
    m_commanderChannel->registerHandler<cmdSIMPLE_MSG>(
        [this](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment) {
            this->on_cmdSIMPLE_MSG(_sender, _attachment, m_commanderChannel);
        });

    // Subscribe to Shutdown command
    m_commanderChannel->registerHandler<cmdSHUTDOWN>(
        [this](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment) {
            this->on_cmdSHUTDOWN(_sender, _attachment, m_commanderChannel);
        });

    // Subscribe for key updates. Forward message to user task.
    m_commanderChannel->registerHandler<cmdUPDATE_KEY>(
        [this](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment) {
            // m_SMIntercomChannel->pushMsg<cmdUPDATE_KEY>(*_attachment);
        });

    // Subscribe for User Task Done events
    m_commanderChannel->registerHandler<cmdUSER_TASK_DONE>(
        [this](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdUSER_TASK_DONE>::ptr_t _attachment) {
            // m_SMIntercomChannel->pushMsg<cmdUSER_TASK_DONE>(*_attachment);
        });

    // Subscribe for cmdCUSTOM_CMD
    m_commanderChannel->registerHandler<cmdCUSTOM_CMD>(
        [this](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdCUSTOM_CMD>::ptr_t _attachment) {
            //   this->on_cmdCUSTOM_CMD(_sender, _attachment, m_commanderChannel);
        });

    // Call this callback when a user process is activated
    m_commanderChannel->registerHandler<EChannelEvents::OnAssignUserTask>([this](const SSenderInfo& _sender) {
        // Stop drainning the intercom write queue
        // m_SMIntercomChannel->drainWriteQueue(false);
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
    // Shared memory channel for communication with user task
    const CUserDefaults& userDefaults = CUserDefaults::instance();
    m_SMIntercomChannel = CSMIntercomChannel::makeNew(
        m_io_context, userDefaults.getSMInputName(), userDefaults.getSMOutputName(), _protocolHeaderID);

    // TODO: Forwarding of update key commands without decoding using raw message API
    // Forward messages from shared memory to the agent.
    // For the moment we have to replace PHID of the intercom message (which is always 0) with the real PHID of the
    // agent.
    m_SMIntercomChannel->registerHandler<cmdCUSTOM_CMD>(
        [this](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdCUSTOM_CMD>::ptr_t _attachment) {
            m_SMCommanderChannel->pushMsg<cmdCUSTOM_CMD>(*_attachment, m_SMCommanderChannel->getProtocolHeaderID());
        });

    m_SMIntercomChannel->registerHandler<cmdUPDATE_KEY>(
        [this](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment) {
            send_cmdUPDATE_KEY(_attachment);
        });

    LOG(info) << "SM channel: Intercom created";
}

void CAgentConnectionManager::createSMCommanderChannel(uint64_t _protocolHeaderID)
{
    const CUserDefaults& userDefaults = CUserDefaults::instance();
    // Create shared memory agent channel
    m_SMCommanderChannel = CSMCommanderChannel::makeNew(m_io_context,
                                                        userDefaults.getSMAgentInputName(),
                                                        userDefaults.getSMAgentOutputName(_protocolHeaderID),
                                                        _protocolHeaderID);
    m_SMCommanderChannel->addOutput(CSMCommanderChannel::EOutputID::Leader, userDefaults.getSMAgentLeaderOutputName());

    // Subscribe to reply command
    m_SMCommanderChannel->registerHandler<cmdREPLY>(
        [this](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdREPLY>::ptr_t _attachment) {
            if (_attachment->m_srcCommand == cmdLOBBY_MEMBER_HANDSHAKE &&
                _attachment->m_statusCode == (uint16_t)SReplyCmd::EStatusCode::ERROR)
            {
                stop();
            }
        });

    LOG(info) << "SM channel: Agent is created";
}

void CAgentConnectionManager::on_cmdSHUTDOWN(const SSenderInfo& _sender,
                                             SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment,
                                             CCommanderChannel::weakConnectionPtr_t _channel)
{
    // Commander requested to shutdown.
    if (m_SMLeaderChannel != nullptr)
    {
        // LOG(info) << "Sending SHUTDOWN to all lobby members";
        m_SMLeaderChannel->syncSendShutdownAll();
    }
    stop();
}

void CAgentConnectionManager::on_cmdSIMPLE_MSG(const SSenderInfo& _sender,
                                               SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment,
                                               CCommanderChannel::weakConnectionPtr_t _channel)
{
    if (_attachment->m_srcCommand == cmdUPDATE_KEY || _attachment->m_srcCommand == cmdCUSTOM_CMD)
    {
        if (_attachment->m_msgSeverity == MiscCommon::error)
        {
            LOG(MiscCommon::error) << _attachment->m_sMsg;
        }
        // Forward message to user task
        // m_SMIntercomChannel->pushMsg<cmdSIMPLE_MSG>(*_attachment);
    }
    else
    {
        LOG(debug) << "Received command cmdSIMPLE_MSG does not have a listener";
    }
}

void CAgentConnectionManager::on_cmdCUSTOM_CMD(const SSenderInfo& _sender,
                                               SCommandAttachmentImpl<cmdCUSTOM_CMD>::ptr_t _attachment,
                                               CSMCommanderChannel::weakConnectionPtr_t _channel)
{
    // Forward message to user task
    m_SMIntercomChannel->pushMsg<cmdCUSTOM_CMD>(*_attachment);
}

void CAgentConnectionManager::on_cmdBINARY_ATTACHMENT_RECEIVED(
    const SSenderInfo& _sender,
    SCommandAttachmentImpl<cmdBINARY_ATTACHMENT_RECEIVED>::ptr_t _attachment,
    CCommanderChannel::weakConnectionPtr_t _channel)
{
    // Topology file path
    boost::filesystem::path destFilePath(CUserDefaults::instance().getDDSPath());
    destFilePath /= _attachment->m_requestedFileName;

    // Activating new topology
    CTopoCore topo;
    // Topology already validated on the commander, no need to validate it again
    topo.setXMLValidationDisabled(true);
    topo.init(destFilePath.string());
    // Assign new topology
    m_topo = topo;
    LOG(info) << "Topology activated";

    // Send response back to server
    if (auto p = _channel.lock())
        p->pushMsg<cmdREPLY>(SReplyCmd("File received", (uint16_t)SReplyCmd::EStatusCode::OK, 0, cmdUPDATE_TOPOLOGY));
}

void CAgentConnectionManager::send_cmdUPDATE_KEY(SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment)
{
    string propertyName(_attachment->m_propertyName);
    uint64_t taskID(m_SMCommanderChannel->getTaskID());

    auto task = m_topo.getRuntimeTaskById(taskID).m_task;
    auto property = task->getProperty(propertyName);
    // Property doesn't exists for task
    if (property == nullptr)
    {
        stringstream ss;
        ss << "Can't propagate property <" << propertyName << "> that doesn't exist for task <" << task->getName()
           << ">";
        m_SMIntercomChannel->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(ss.str(), MiscCommon::error, cmdUPDATE_KEY));
        return;
    }
    // Cant' propagate property with read access type
    if (property->getAccessType() == CTopoProperty::EAccessType::READ)
    {
        stringstream ss;
        ss << "Can't propagate property <" << property->getName() << "> which has a READ access type for task <"
           << task->getName() << ">";
        m_SMIntercomChannel->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(ss.str(), MiscCommon::error, cmdUPDATE_KEY));
        return;
    }
    // Can't send property with a collection scope if a task is outside a collection
    if ((property->getScopeType() == CTopoProperty::EScopeType::COLLECTION) &&
        (task->getParent()->getType() != CTopoBase::EType::COLLECTION))
    {
        stringstream ss;
        ss << "Can't propagate property <" << property->getName() << "> which has a COLLECTION scope type but task <"
           << task->getName() << "> is not in any collection";
        m_SMIntercomChannel->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(ss.str(), MiscCommon::error, cmdUPDATE_KEY));
        return;
    }

    SUpdateKeyCmd cmd;
    cmd.m_propertyName = propertyName;
    cmd.m_value = _attachment->m_value;
    cmd.m_senderTaskID = taskID;
    m_SMCommanderChannel->pushMsg<cmdUPDATE_KEY>(cmd, m_SMCommanderChannel->getProtocolHeaderID());

    m_SMIntercomChannel->pushMsg<cmdSIMPLE_MSG>(
        SSimpleMsgCmd("Key update messages have been sent to lobby leader.", MiscCommon::debug, cmdUPDATE_KEY));
}
