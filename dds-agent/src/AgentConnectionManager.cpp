// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// BOOST
#include <boost/property_tree/ptree.hpp>

// silence "Unused typedef" warning using clang 3.7+ and boost < 1.59
#if BOOST_VERSION < 105900
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-local-typedef"
#endif
#include <boost/property_tree/ini_parser.hpp>
#if BOOST_VERSION < 105900
#pragma clang diagnostic pop
#endif

// DDS
#include "AgentConnectionManager.h"
#include "CommanderChannel.h"
#include "DDSIntercomGuard.h"
#include "Logger.h"
#include "MonitoringThread.h"
#include "SMCommanderChannel.h"

using namespace boost::asio;
using namespace std;
using namespace dds::agent_cmd;
using namespace dds::user_defaults_api;
using namespace dds::protocol_api;
using namespace MiscCommon;
namespace sp = std::placeholders;
using boost::asio::ip::tcp;

CAgentConnectionManager::CAgentConnectionManager(const SOptions_t& _options, boost::asio::io_service& _io_service)
    : m_service(_io_service)
    , m_signals(_io_service)
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

    m_bStarted = true;
    try
    {
        // Shared memory channel for communication with user task
        const CUserDefaults& userDefaults = CUserDefaults::instance();
        m_SMChannel = CSMUIChannel::makeNew(userDefaults.getSMInputName(), userDefaults.getSMOutputName());
        // Forward messages from shared memory to agent
        m_SMChannel->registerHandler<cmdRAW_MSG>(
            [this](const protocol_api::SSenderInfo& _sender,
                   protocol_api::CProtocolMessage::protocolMessagePtr_t _currentMsg) {
                m_SMAgent->pushMsg(_currentMsg, static_cast<ECmdType>(_currentMsg->header().m_cmd));
            });
        //

        const float maxIdleTime = CUserDefaults::instance().getOptions().m_server.m_idleTime;
        CMonitoringThread::instance().start(maxIdleTime, []() { LOG(info) << "Idle callback called"; });

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
        tcp::resolver resolver(m_service);
        tcp::resolver::query query(sHost, sPort);
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

        // Create new agent and push handshake message
        m_agent = CCommanderChannel::makeNew(m_service);

        // Create shared memory agent channel
        m_SMAgent =
            CSMCommanderChannel::makeNew(userDefaults.getSMAgentInputName(), userDefaults.getSMAgentOutputName());

        // Subscribe to Shutdown command
        m_SMAgent->registerHandler<cmdSHUTDOWN>(
            [this](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment) {
                this->on_cmdSHUTDOWN(_sender, _attachment, m_SMAgent);
            });

        // Subscribe for key updates
        // TODO: Forwarding of update key commands without decoding using raw mwssage API
        m_SMAgent->registerHandler<cmdUPDATE_KEY>(
            [this](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment) {
                this->on_cmdUPDATE_KEY(_sender, _attachment, m_SMAgent);
            });

        // Subscribe for key update errors
        m_SMAgent->registerHandler<cmdUPDATE_KEY_ERROR>(
            [this](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdUPDATE_KEY_ERROR>::ptr_t _attachment) {
                this->on_cmdUPDATE_KEY_ERROR(_sender, _attachment, m_SMAgent);
            });

        // Subscribe for key delete events
        m_SMAgent->registerHandler<cmdDELETE_KEY>(
            [this](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdDELETE_KEY>::ptr_t _attachment) {
                this->on_cmdDELETE_KEY(_sender, _attachment, m_SMAgent);
            });

        // Subscribe for cmdSIMPLE_MSG
        m_SMAgent->registerHandler<cmdSIMPLE_MSG>(
            [this](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment) {
                this->on_cmdSIMPLE_MSG(_sender, _attachment, m_SMAgent);
            });

        // Subscribe for cmdSTOP_USER_TASK
        m_SMAgent->registerHandler<cmdSTOP_USER_TASK>(
            [this](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdSTOP_USER_TASK>::ptr_t _attachment) {
                this->on_cmdSTOP_USER_TASK(_sender, _attachment, m_SMAgent);
            });

        // Subscribe for cmdCUSTOM_CMD
        m_SMAgent->registerHandler<cmdCUSTOM_CMD>(
            [this](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdCUSTOM_CMD>::ptr_t _attachment) {
                this->on_cmdCUSTOM_CMD(_sender, _attachment, m_SMAgent);
            });

        // Call this callback when a user process is activated
        m_SMAgent->registerHandler<EChannelEvents::OnNewUserTask>([this](pid_t _pid) { this->onNewUserTask(_pid); });

        // Start listening for messages from shared memory
        m_SMChannel->start();
        // Start shared memory agent channel
        m_SMAgent->start();
        // Connect to DDS commander
        m_agent->connect(endpoint_iterator);

        const int nConcurrentThreads(2);
        LOG(MiscCommon::info) << "Starting DDS transport engine using " << nConcurrentThreads << " concurrent threads.";
        for (int x = 0; x < nConcurrentThreads; ++x)
        {
            m_workerThreads.create_thread([this]() {
                try
                {
                    m_service.run();
                }
                catch (exception& ex)
                {
                    LOG(MiscCommon::error) << "AgentConnectionManager: " << ex.what();
                }
            });
        }

        m_workerThreads.join_all();
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
    // terminate external children processes (like user tasks, for example)
    terminateChildrenProcesses();

    try
    {
        m_service.stop();
        if (m_agent)
            m_agent->stop();
        if (m_SMAgent)
            m_SMAgent->stop();
        if (m_SMChannel)
            m_SMChannel->stop();
    }
    catch (exception& e)
    {
        LOG(fatal) << e.what();
    }
    LOG(info) << "Shutting down DDS transport - DONE";
}

void CAgentConnectionManager::terminateChildrenProcesses()
{
    // terminate user process, if any
    // TODO: Maybe it needs to be moved to the monitoring thread. Can be activated them by a
    // signal variable.
    if (m_children.empty())
    {
        LOG(info) << "There are no children processes to terminate. We are good to go...";
        return;
    }

    for (auto const& pid : m_children)
    {
        LOG(info) << "Sending graceful terminate signal to child process with pid = " << pid;
        kill(pid, SIGTERM);
    }

    LOG(info) << "Wait for child processes to exit...";
    for (auto const& pid : m_children)
    {
        if (!IsProcessExist(pid))
            continue;

        // wait 10 seconds each
        for (size_t i = 0; i < 10; ++i)
        {
            LOG(info) << "Waiting for pid = " << pid;
            int stat(0);
            if (pid == ::waitpid(pid, &stat, WNOHANG))
            {
                LOG(info) << "pid = " << pid << " - done; exit status = " << WEXITSTATUS(stat);
                break;
            }
            // TODO: Needs to be fixed! Implement time-function based timeout measurements
            // instead
            sleep(1);
        }
    }

    // kills the child
    for (auto const& pid : m_children)
    {
        if (!IsProcessExist(pid))
            continue;

        LOG(info) << "Timeout has been reached, child process with pid = " << pid << " will be forced to exit...";
        kill(pid, SIGKILL);
    }
}

void CAgentConnectionManager::on_cmdSHUTDOWN(const SSenderInfo& _sender,
                                             SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment,
                                             CSMCommanderChannel::weakConnectionPtr_t _channel)
{
    stop();
}

void CAgentConnectionManager::on_cmdUPDATE_KEY(const SSenderInfo& _sender,
                                               SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment,
                                               CSMCommanderChannel::weakConnectionPtr_t _channel)
{
    // Forward message to user task
    m_SMChannel->pushMsg<cmdUPDATE_KEY>(*_attachment);
}

void CAgentConnectionManager::on_cmdUPDATE_KEY_ERROR(
    const SSenderInfo& _sender,
    SCommandAttachmentImpl<protocol_api::cmdUPDATE_KEY_ERROR>::ptr_t _attachment,
    CSMCommanderChannel::weakConnectionPtr_t _channel)
{
    // Forward message to user task
    m_SMChannel->pushMsg<cmdUPDATE_KEY_ERROR>(*_attachment);
}

void CAgentConnectionManager::on_cmdDELETE_KEY(const SSenderInfo& _sender,
                                               SCommandAttachmentImpl<cmdDELETE_KEY>::ptr_t _attachment,
                                               CSMCommanderChannel::weakConnectionPtr_t _channel)
{
    // Forward message to user task
    m_SMChannel->pushMsg<cmdDELETE_KEY>(*_attachment);
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
        m_SMChannel->pushMsg<cmdSIMPLE_MSG>(*_attachment);
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
    m_SMChannel->pushMsg<cmdCUSTOM_CMD>(*_attachment);
}

void CAgentConnectionManager::taskExited(int _pid, int _exitCode)
{
    // remove pid from the active children list
    {
        std::lock_guard<std::mutex> lock(m_childrenContainerMutex);
        m_children.erase(remove(m_children.begin(), m_children.end(), _pid), m_children.end());
    }
    SUserTaskDoneCmd cmd;
    cmd.m_exitCode = _exitCode;
    cmd.m_taskID = m_SMAgent->getTaskID();
    m_SMAgent->pushMsg<cmdUSER_TASK_DONE>(cmd);

    m_SMChannel->reinit();
}

void CAgentConnectionManager::onNewUserTask(pid_t _pid)
{
    // watchdog
    LOG(info) << "Adding user task pid " << _pid << " to the tasks queue";
    try
    {
        // remove pid from the active children list
        std::lock_guard<std::mutex> lock(m_childrenContainerMutex);
        m_children.push_back(_pid);
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
            m_SMAgent->pushMsg<cmdWATCHDOG_HEARTBEAT>();
            CMonitoringThread::instance().updateIdle();

            try
            {
                if (!IsProcessExist(_pid))
                {
                    LOG(info) << "User Tasks cannot be found. Probably it has exited. pid = " << _pid;
                    LOG(info) << "Stopping the watchdog for user task pid = " << _pid;

                    taskExited(_pid, 0);

                    return false;
                }

                // We must call "wait" to check exist status of a child process, otherwise we will crate a
                // zombie :)
                int status;
                if (_pid == ::waitpid(_pid, &status, WNOHANG))
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
        chrono::seconds(10));

    LOG(info) << "Watchdog for task pid = " << _pid << " has been registered.";
}

void CAgentConnectionManager::on_cmdSTOP_USER_TASK(const SSenderInfo& _sender,
                                                   SCommandAttachmentImpl<cmdSTOP_USER_TASK>::ptr_t _attachment,
                                                   CSMCommanderChannel::weakConnectionPtr_t _channel)
{
    // TODO: add error processing, in case if user tasks won't quite
    terminateChildrenProcesses();
    auto p = _channel.lock();
    p->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd("Done", info, cmdSTOP_USER_TASK));
    m_SMChannel->reinit();
}
