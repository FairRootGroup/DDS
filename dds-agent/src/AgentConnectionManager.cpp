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
#include "DDSIntercomGuard.h"
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
    : m_signals(m_io_service)
    , m_options(_options)
    , m_taskPid(0)
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

            LOG(info) << "Lobby status: leader";

            // To communicate with tasks
            createSMIntercomChannel(protocolHeaderID);
            createSMLeaderChannel(protocolHeaderID);
            createSMCommanderChannel(protocolHeaderID);
            // Start network channel. This is a blocking function call.
            createCommanderChannel(protocolHeaderID);

            // Start listening for messages from shared memory
            m_SMIntercomChannel->start();
            // Start listening for messages from shared memory
            m_SMLeaderChannel->start();
            // Start shared memory agent channel
            m_SMCommanderChannel->start();

            startService();

            leaderMutex->unlock();
        }
        else
        {
            // TODO: FIXME:
            // - open shared memory
            // - error processing if memory can't be opened

            // TODO: FIXME:
            // - block execution, channels are working in threads
            // leaderMutex->lock();

            LOG(info) << "Lobby status: member";

            createSMIntercomChannel(protocolHeaderID);
            // Blocking function call
            createSMCommanderChannel(protocolHeaderID);

            // Start listening for messages from shared memory
            m_SMIntercomChannel->start();
            // Start shared memory agent channel
            m_SMCommanderChannel->start();

            startService();
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
    // terminate external children processes (like user tasks, for example)
    terminateChildrenProcesses();

    try
    {
        if (m_SMCommanderChannel)
            m_SMCommanderChannel->stop();
        if (m_SMIntercomChannel)
            m_SMIntercomChannel->stop();
        if (m_SMLeaderChannel)
            m_SMLeaderChannel->stop();
        if (m_commanderChannel)
        {
            // Stop forwarder's readMessage thread
            auto pSCFW = m_commanderChannel->getSMFWChannel().lock();
            if (pSCFW)
                pSCFW->stop();
            m_commanderChannel->stop();
        }
        m_io_service.stop();
    }
    catch (exception& e)
    {
        LOG(fatal) << e.what();
    }
    LOG(info) << "Shutting down DDS transport - DONE";
}

void CAgentConnectionManager::startService()
{
    const int nConcurrentThreads(7);
    LOG(MiscCommon::info) << "Starting DDS transport engine using " << nConcurrentThreads << " concurrent threads.";
    for (int x = 0; x < nConcurrentThreads; ++x)
    {
        m_workerThreads.create_thread([this]() {
            try
            {
                m_io_service.run();
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
    tcp::resolver resolver(m_io_service);
    tcp::resolver::query query(sHost, sPort);
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

    // Create new agent and push handshake message
    m_commanderChannel = CCommanderChannel::makeNew(m_io_service, _protocolHeaderID);

    // Connect to DDS commander
    m_commanderChannel->connect(endpoint_iterator);
}

void CAgentConnectionManager::createSMLeaderChannel(uint64_t _protocolHeaderID)
{
    // Shared memory channel for communication with user task
    const CUserDefaults& userDefaults = CUserDefaults::instance();
    m_SMLeaderChannel = CSMLeaderChannel::makeNew(m_io_service,
                                                  userDefaults.getSMAgentLeaderOutputName(),
                                                  userDefaults.getSMAgentLeaderOutputName(),
                                                  _protocolHeaderID);
    m_SMLeaderChannel->registerHandler<EChannelEvents::OnLobbyMemberInfo>(
        [this](const SSenderInfo& _sender, const string& _name) {
            try
            {
                // Add output for lobby members, skipping output for itself
                if (_sender.m_ID != m_SMLeaderChannel->getProtocolHeaderID())
                {
                    auto p = m_commanderChannel->getSMFWChannel().lock();
                    p->addOutput(_sender.m_ID, _name);
                }
            }
            catch (exception& _e)
            {
                LOG(MiscCommon::error) << "Failed to open forwarder MQ " << _name << " of the new member "
                                       << _sender.m_ID << " error: " << _e.what();
            }
        });

    LOG(info) << "SM channel: Leader created";
}

void CAgentConnectionManager::createSMIntercomChannel(uint64_t _protocolHeaderID)
{
    // Shared memory channel for communication with user task
    const CUserDefaults& userDefaults = CUserDefaults::instance();
    m_SMIntercomChannel = CSMIntercomChannel::makeNew(
        m_io_service, userDefaults.getSMInputName(), userDefaults.getSMOutputName(), _protocolHeaderID);

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
    m_SMCommanderChannel = CSMCommanderChannel::makeNew(
        m_io_service, userDefaults.getSMAgentInputName(), userDefaults.getSMAgentOutputName(), _protocolHeaderID);
    m_SMCommanderChannel->addOutput(CSMCommanderChannel::EOutputID::Leader, userDefaults.getSMAgentLeaderOutputName());

    // Subscribe to Shutdown command
    m_SMCommanderChannel->registerHandler<cmdSHUTDOWN>(
        [this](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment) {
            this->on_cmdSHUTDOWN(_sender, _attachment, m_SMCommanderChannel);
        });

    // Subscribe to reply command
    m_SMCommanderChannel->registerHandler<cmdREPLY>(
        [this](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdREPLY>::ptr_t _attachment) {
            if (_attachment->m_srcCommand == cmdLOBBY_MEMBER_HANDSHAKE &&
                _attachment->m_statusCode == (uint16_t)SReplyCmd::EStatusCode::ERROR)
            {
                stop();
            }
        });

    // Subscribe for key updates. Forward message to user task.
    // TODO: Forwarding of update key commands without decoding using raw mwssage API
    m_SMCommanderChannel->registerHandler<cmdUPDATE_KEY>(
        [this](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment) {
            m_SMIntercomChannel->pushMsg<cmdUPDATE_KEY>(*_attachment);
        });

    // Subscribe for key delete events
    m_SMCommanderChannel->registerHandler<cmdDELETE_KEY>(
        [this](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdDELETE_KEY>::ptr_t _attachment) {
            m_SMIntercomChannel->pushMsg<cmdDELETE_KEY>(*_attachment);
        });

    // Subscribe for User Task Done events
    m_SMCommanderChannel->registerHandler<cmdUSER_TASK_DONE>(
        [this](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdUSER_TASK_DONE>::ptr_t _attachment) {
            m_SMIntercomChannel->pushMsg<cmdUSER_TASK_DONE>(*_attachment);
        });

    // Subscribe for cmdSIMPLE_MSG
    m_SMCommanderChannel->registerHandler<cmdSIMPLE_MSG>(
        [this](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment) {
            this->on_cmdSIMPLE_MSG(_sender, _attachment, m_SMCommanderChannel);
        });

    // Subscribe for cmdSTOP_USER_TASK
    m_SMCommanderChannel->registerHandler<cmdSTOP_USER_TASK>(
        [this](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdSTOP_USER_TASK>::ptr_t _attachment) {
            this->on_cmdSTOP_USER_TASK(_sender, _attachment, m_SMCommanderChannel);
        });

    // Subscribe for cmdCUSTOM_CMD
    m_SMCommanderChannel->registerHandler<cmdCUSTOM_CMD>(
        [this](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdCUSTOM_CMD>::ptr_t _attachment) {
            this->on_cmdCUSTOM_CMD(_sender, _attachment, m_SMCommanderChannel);
        });

    // Call this callback when a user process is activated
    m_SMCommanderChannel->registerHandler<EChannelEvents::OnNewUserTask>(
        [this](const SSenderInfo& _sender, pid_t _pid) { this->onNewUserTask(_pid); });

    // Subscribe for cmdBINARY_ATTACHMENT_RECEIVED
    m_SMCommanderChannel->registerHandler<cmdBINARY_ATTACHMENT_RECEIVED>(
        [this](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdBINARY_ATTACHMENT_RECEIVED>::ptr_t _attachment) {
            this->on_cmdBINARY_ATTACHMENT_RECEIVED(_sender, _attachment, m_SMCommanderChannel);
        });

    LOG(info) << "SM channel: Agent is created";
}

void CAgentConnectionManager::terminateChildrenProcesses()
{
    // terminate user process, if any
    // TODO: Maybe it needs to be moved to the monitoring thread. Can be activated them by a
    // signal variable.
    {
        lock_guard<mutex> lock(m_taskPidMutex);
        if (m_taskPid == 0)
        {
            LOG(info) << "There is no task to terminate. We are good to go...";
            return;
        }
    }

    LOG(info) << "Getting a list of child processes of the task with pid = " << m_taskPid;
    std::vector<std::string> vecChildren;
    try
    {
        // a pgrep command is used to find out the list of child processes of the task
        stringstream ssCmd;
        ssCmd << boost::process::search_path("pgrep").string() << " -P " << m_taskPid;
        string output;
        execute(ssCmd.str(), std::chrono::seconds(10), &output);
        boost::split(vecChildren, output, boost::is_any_of(" \n"), boost::token_compress_on);
        vecChildren.erase(
            remove_if(vecChildren.begin(), vecChildren.end(), [](const string& _val) { return _val.empty(); }),
            vecChildren.end());
    }
    catch (...)
    {
    }
    string sChildren;
    for (const auto i : vecChildren)
    {
        if (!sChildren.empty())
            sChildren += ", ";
        sChildren += i;
    }
    LOG(info) << "Process " << m_taskPid << " has " << vecChildren.size() << " child process(es)"
              << (sChildren.empty() ? "." : " " + sChildren);

    LOG(info) << "Sending graceful terminate signal to a parent process " << m_taskPid;
    {
        lock_guard<mutex> lock(m_taskPidMutex);
        if (m_taskPid > 0)
            kill(m_taskPid, SIGTERM);
    }
    for (const auto i : vecChildren)
    {
        LOG(info) << "Sending graceful terminate signal to a child process " << i;
        kill(stol(i), SIGTERM);
    }

    LOG(info) << "Wait for task to exit...";
    if (IsProcessRunning(m_taskPid))
    {
        // wait 10 seconds each
        for (size_t i = 0; i < 10; ++i)
        {
            LOG(info) << "Waiting for pid = " << m_taskPid;
            int stat(0);
            if (m_taskPid == ::waitpid(m_taskPid, &stat, WNOHANG | WUNTRACED))
            {
                LOG(info) << "pid = " << m_taskPid << " - done; exit status = " << WEXITSTATUS(stat);
                break;
            }
            // TODO: Needs to be fixed! Implement time-function based timeout measurements
            // instead
            sleep(1);
        }
    }

    // kill all child process of tasks if there are any
    // We do it before terminating tasks to give parenrt task processes a change to read state of children - otherwise
    // we will get zombies if user tasks don't manage their children properly
    for (auto const& pid : vecChildren)
    {
        if (!IsProcessRunning(stol(pid)))
            continue;

        LOG(info) << "Child process with pid = " << pid << " will be forced to exit...";
        kill(stol(pid), SIGKILL);
    }

    // Force kill of the tasks
    if (IsProcessRunning(m_taskPid))
    {
        LOG(info) << "Timeout has been reached, child process with pid = " << m_taskPid << " will be forced to exit...";
        lock_guard<mutex> lock(m_taskPidMutex);
        if (m_taskPid > 0)
            kill(m_taskPid, SIGKILL);
    }
}

void CAgentConnectionManager::on_cmdSHUTDOWN(const SSenderInfo& _sender,
                                             SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment,
                                             CSMCommanderChannel::weakConnectionPtr_t _channel)
{
    // Commander requested to shutdown.
    // Shutting down all members of the lobby.
    if (m_SMLeaderChannel != nullptr)
    {
        LOG(info) << "Sending SHUTDOWN to all lobby members";
        m_SMLeaderChannel->syncSendShutdownAll();
    }
    stop();
}

void CAgentConnectionManager::on_cmdSIMPLE_MSG(const SSenderInfo& _sender,
                                               SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment,
                                               CSMCommanderChannel::weakConnectionPtr_t _channel)
{
    if (_attachment->m_srcCommand == cmdUPDATE_KEY || _attachment->m_srcCommand == cmdCUSTOM_CMD)
    {
        if (_attachment->m_msgSeverity == MiscCommon::error)
        {
            LOG(MiscCommon::error) << _attachment->m_sMsg;
        }
        // Forward message to user task
        m_SMIntercomChannel->pushMsg<cmdSIMPLE_MSG>(*_attachment);
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

void CAgentConnectionManager::taskExited(int _pid, int _exitCode)
{
    // remove pid from the active children list
    {
        lock_guard<mutex> lock(m_taskPidMutex);
        m_taskPid = 0;
    }
    SUserTaskDoneCmd cmd;
    cmd.m_exitCode = _exitCode;
    cmd.m_taskID = m_SMCommanderChannel->getTaskID();
    m_SMCommanderChannel->pushMsg<cmdUSER_TASK_DONE>(cmd);
}

void CAgentConnectionManager::onNewUserTask(pid_t _pid)
{
    // watchdog
    LOG(info) << "Adding user task pid " << _pid << " to the tasks queue";
    try
    {
        // remove pid from the active children list
        lock_guard<mutex> lock(m_taskPidMutex);
        m_taskPid = _pid;
    }
    catch (exception& _e)
    {
        LOG(fatal) << "Can't add new user task to the list of children: " << _e.what();
    }

    // Register the user task's watchdog

    LOG(info) << "Starting the watchdog for user task pid = " << _pid;

    auto self(shared_from_this());
    CMonitoringThread::instance().registerCallbackFunction(
        [this, self, _pid]() -> bool {
            // Send commander server the watchdog heartbeat.
            // It indicates that the agent is executing a task and is not idle
            m_SMCommanderChannel->pushMsg<cmdWATCHDOG_HEARTBEAT>();
            CMonitoringThread::instance().updateIdle();

            try
            {
                // NOTE: We don't use boost::process because it returned an evaluated exit status, but we need a raw to
                // be able to detect how exactly the child exited.
                // boost::process  only checks that the child ended because of a call to ::exit() and does not check for
                // exiting via signal (WIFSIGNALED()).

                // We must call "wait" to check exist status of a child process, otherwise we will crate a
                // zombie :)
                int status;
                pid_t ret = ::waitpid(_pid, &status, WNOHANG | WUNTRACED);
                if (ret < 0)
                {
                    switch (errno)
                    {
                        case ECHILD:
                            LOG(MiscCommon::error) << "Watchdog: The process or process group specified by pid "
                                                      "does not exist or is not a child of the calling process.";
                            break;
                        case EFAULT:
                            LOG(MiscCommon::error) << "Watchdog: stat_loc is not a writable address.";
                            break;
                        case EINTR:
                            LOG(MiscCommon::error) << "Watchdog: The function was interrupted by a signal. The "
                                                      "value of the location pointed to by stat_loc is undefined.";
                            break;
                        case EINVAL:
                            LOG(MiscCommon::error) << "Watchdog: The options argument is not valid.";
                            break;
                        case ENOSYS:
                            LOG(MiscCommon::error) << "Watchdog: pid specifies a process group (0 or less than "
                                                      "-1), which is not currently supported.";
                            break;
                    }
                    LOG(info) << "User Tasks cannot be found. Probably it has exited. pid = " << _pid;
                    LOG(info) << "Stopping the watchdog for user task pid = " << _pid;

                    taskExited(_pid, 0);

                    return false;
                }
                else if (ret == _pid)
                {
                    if (WIFEXITED(status))
                        LOG(info) << "User task exited" << (WCOREDUMP(status) ? " and dumped core" : "")
                                  << " with status " << WEXITSTATUS(status);
                    else if (WIFSTOPPED(status))
                        LOG(info) << "User task stopped by signal " << WSTOPSIG(status);
                    else if (WIFSIGNALED(status))
                        LOG(info) << "User task killed by signal " << WTERMSIG(status)
                                  << (WCOREDUMP(status) ? "; (core dumped)" : "");
                    else
                        LOG(info) << "User task exited with unexpected status: " << status;

                    LOG(info) << "Stopping the watchdog for user task pid = " << _pid;

                    taskExited(_pid, status);
                    return false;
                }
            }
            catch (exception& _e)
            {
                LOG(fatal) << "User processe monitoring thread received an exception: " << _e.what();
            }

            return true;
        },
        std::chrono::seconds(5));

    LOG(info) << "Watchdog for task pid = " << _pid << " has been registered.";
}

void CAgentConnectionManager::on_cmdSTOP_USER_TASK(const SSenderInfo& _sender,
                                                   SCommandAttachmentImpl<cmdSTOP_USER_TASK>::ptr_t _attachment,
                                                   CSMCommanderChannel::weakConnectionPtr_t _channel)
{
    // TODO: add error processing, in case if user tasks won't quite
    terminateChildrenProcesses();
    auto p = _channel.lock();
    p->pushMsg<cmdREPLY>(SReplyCmd("Done", (uint16_t)SReplyCmd::EStatusCode::OK, 0, cmdSTOP_USER_TASK));
}

void CAgentConnectionManager::on_cmdBINARY_ATTACHMENT_RECEIVED(
    const SSenderInfo& _sender,
    SCommandAttachmentImpl<cmdBINARY_ATTACHMENT_RECEIVED>::ptr_t _attachment,
    CSMCommanderChannel::weakConnectionPtr_t _channel)
{
    // Topology file path
    boost::filesystem::path destFilePath(CUserDefaults::instance().getDDSPath());
    destFilePath /= _attachment->m_requestedFileName;

    // Activating new topology
    CTopology topo;
    // Topology already validated on the commander, no need to validate it again
    topo.setXMLValidationDisabled(true);
    topo.init(destFilePath.string());
    // Assign new topology
    m_topo = topo;
    LOG(info) << "Topology activated";

    // Send response back to server
    auto p = _channel.lock();
    p->pushMsg<cmdREPLY>(SReplyCmd("File received", (uint16_t)SReplyCmd::EStatusCode::OK, 0, cmdUPDATE_TOPOLOGY));
}

void CAgentConnectionManager::send_cmdUPDATE_KEY(SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment)
{
    string propertyID(_attachment->m_propertyID);
    uint64_t taskID(m_SMCommanderChannel->getTaskID());

    auto task = m_topo.getTaskByHash(taskID);
    auto property = task->getProperty(propertyID);
    // Property doesn't exists for task
    if (property == nullptr)
    {
        stringstream ss;
        ss << "Can't propagate property <" << propertyID << "> that doesn't exist for task <" << task->getId() << ">";
        m_SMIntercomChannel->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(ss.str(), MiscCommon::error, cmdUPDATE_KEY));
        return;
    }
    // Cant' propagate property with read access type
    if (property->getAccessType() == EPropertyAccessType::READ)
    {
        stringstream ss;
        ss << "Can't propagate property <" << property->getId() << "> which has a READ access type for task <"
           << task->getId() << ">";
        m_SMIntercomChannel->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(ss.str(), MiscCommon::error, cmdUPDATE_KEY));
        return;
    }
    // Can't send property with a collection scope if a task is outside a collection
    if ((property->getScopeType() == EPropertyScopeType::COLLECTION) &&
        (task->getParent()->getType() != ETopoType::COLLECTION))
    {
        stringstream ss;
        ss << "Can't propagate property <" << property->getId() << "> which has a COLLECTION scope type but task <"
           << task->getId() << "> is not in any collection";
        m_SMIntercomChannel->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(ss.str(), MiscCommon::error, cmdUPDATE_KEY));
        return;
    }

    SUpdateKeyCmd cmd;
    cmd.m_propertyID = propertyID;
    cmd.m_value = _attachment->m_value;
    cmd.m_senderTaskID = taskID;
    m_SMCommanderChannel->pushMsg<cmdUPDATE_KEY>(cmd, m_SMCommanderChannel->getProtocolHeaderID());

    m_SMIntercomChannel->pushMsg<cmdSIMPLE_MSG>(
        SSimpleMsgCmd("Key update messages have been sent to lobby leader.", MiscCommon::debug, cmdUPDATE_KEY));
}
