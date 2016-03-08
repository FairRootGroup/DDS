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
    , m_deadlineTimer(std::make_shared<boost::asio::deadline_timer>(m_service, boost::posix_time::milliseconds(1000)))
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

        // Subscribe to Shutdown command
        std::function<bool(SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment, CCommanderChannel * _channel)>
            fSHUTDOWN =
                [this](SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment, CCommanderChannel* _channel) -> bool {
            // TODO: adjust the algorithm if we would need to support several agents
            // we have only one agent (newAgent) at the moment
            return this->on_cmdSHUTDOWN(_attachment, m_agent);
        };
        m_agent->registerMessageHandler<cmdSHUTDOWN>(fSHUTDOWN);

        // Subscribe for key updates
        std::function<bool(SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment, CCommanderChannel * _channel)>
            fUPDATE_KEY =
                [this](SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment, CCommanderChannel* _channel) -> bool {
            // TODO: adjust the algorithm if we would need to support several agents
            // we have only one agent (newAgent) at the moment
            return this->on_cmdUPDATE_KEY(_attachment, m_agent);
        };
        m_agent->registerMessageHandler<cmdUPDATE_KEY>(fUPDATE_KEY);

        // Subscribe for cmdSIMPLE_MSG
        std::function<bool(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment, CCommanderChannel * _channel)>
            fSIMPLE_MSG =
                [this](SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment, CCommanderChannel* _channel) -> bool {
            // TODO: adjust the algorithm if we would need to support several agents
            // we have only one agent (newAgent) at the moment
            return this->on_cmdSIMPLE_MSG(_attachment, m_agent);
        };
        m_agent->registerMessageHandler<cmdSIMPLE_MSG>(fSIMPLE_MSG);

        // Subscribe for cmdSTOP_USER_TASK
        std::function<bool(SCommandAttachmentImpl<cmdSTOP_USER_TASK>::ptr_t _attachment, CCommanderChannel * _channel)>
            fSTOP_USER_TASK = [this](SCommandAttachmentImpl<cmdSTOP_USER_TASK>::ptr_t _attachment,
                                     CCommanderChannel* _channel) -> bool {
            // TODO: adjust the algorithm if we would need to support several agents
            // we have only one agent (newAgent) at the moment
            return this->on_cmdSTOP_USER_TASK(_attachment, m_agent);
        };
        m_agent->registerMessageHandler<cmdSTOP_USER_TASK>(fSTOP_USER_TASK);

        // Subscribe for cmdCUSTOM_CMD
        std::function<bool(SCommandAttachmentImpl<cmdCUSTOM_CMD>::ptr_t _attachment, CCommanderChannel * _channel)>
            fCUSTOM_CMD =
                [this](SCommandAttachmentImpl<cmdCUSTOM_CMD>::ptr_t _attachment, CCommanderChannel* _channel) -> bool {
            // TODO: adjust the algorithm if we would need to support several agents
            // we have only one agent (newAgent) at the moment
            return this->on_cmdCUSTOM_CMD(_attachment, m_agent);
        };
        m_agent->registerMessageHandler<cmdCUSTOM_CMD>(fCUSTOM_CMD);

        // Call this callback when a user process is activated
        m_agent->registerOnNewUserTaskCallback([this](pid_t _pid) { return this->onNewUserTask(_pid); });

        m_agent->subscribeOnEvent(EChannelEvents::OnConnected, [this](CCommanderChannel* _channel) {
            // TODO: revise UIConnection manager logic
            if (m_UIConnectionMng != nullptr)
                return;

            // Start the UI agent server
            m_UIConnectionMng = make_shared<CUIConnectionManager>();
            m_UIConnectionMng->setCommanderChannel(m_agent);
            m_UIConnectionMng->start(false, 2);

        });
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
        if (m_UIConnectionMng)
            m_UIConnectionMng->stop();
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

bool CAgentConnectionManager::on_cmdSHUTDOWN(SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment,
                                             CCommanderChannel::weakConnectionPtr_t _channel)
{
    stop();
    return true;
}

bool CAgentConnectionManager::on_cmdUPDATE_KEY(SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment,
                                               CCommanderChannel::weakConnectionPtr_t _channel)
{
    // We accumulate update key messages in order to prevent constant writing to shared memory for each update key
    // message. processUpdateKey is called either in deadline timer or if the queue size exceeded a certain limit.

    static const size_t maxAccumulativeWriteQueueSize = 10000;
    try
    {
        std::lock_guard<std::mutex> lock(m_updateKeyMutex);

        m_deadlineTimer->cancel();

        m_updateKeyQueue.push_back(_attachment);

        if (m_updateKeyQueue.size() > maxAccumulativeWriteQueueSize)
        {
            processUpdateKey();
        }

        m_deadlineTimer->async_wait([this](const boost::system::error_code& error) {
            if (!error)
            {
                std::lock_guard<std::mutex> lock(m_updateKeyMutex);
                if (!m_updateKeyQueue.empty())
                {
                    processUpdateKey();
                }
            }
        });
    }
    catch (std::exception& ex)
    {
        LOG(MiscCommon::error) << "Can't accumulate update key messages: " << ex.what();
    }

    return true;
}

void CAgentConnectionManager::processUpdateKey()
{
    internal_api::CDDSIntercomGuard::instance().putValues(m_updateKeyQueue);

    if (!m_UIConnectionMng)
        LOG(warning) << "UI connection manager doesn't run. Skipping key notification broadcasting.";
    else
    {
        for (const auto& v : m_updateKeyQueue)
        {
            m_UIConnectionMng->notifyAboutKeyUpdate(v);
        }
    }
    m_updateKeyQueue.clear();
};

bool CAgentConnectionManager::on_cmdSIMPLE_MSG(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment,
                                               CCommanderChannel::weakConnectionPtr_t _channel)
{
    if (_attachment->m_srcCommand == cmdUPDATE_KEY || _attachment->m_srcCommand == cmdCUSTOM_CMD)
    {
        if (_attachment->m_msgSeverity != MiscCommon::error)
        {
            if (m_UIConnectionMng)
                m_UIConnectionMng->notifyAboutSimpleMsg(_attachment);
            else
                LOG(warning) << "UI connection manager doesn't run. Skipping simple message broadcasting.";
        }
        else
        {
            // We got an error message about property propagation
            LOG(MiscCommon::error) << _attachment->m_sMsg;
            // Forward error message to UI channel
            if (m_UIConnectionMng)
                m_UIConnectionMng->notifyAboutSimpleMsg(_attachment);
            else
                LOG(warning) << "UI connection manager doesn't run. Skipping forwarding of the error message.";
        }
        return true;
    }
    else
    {

        LOG(debug) << "Received command cmdSIMPLE_MSG does not have a listener";
        return true;
    }

    return true;
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
    m_agent->pushMsg<cmdUSER_TASK_DONE>(cmd);
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
            m_agent->pushMsg<cmdWATCHDOG_HEARTBEAT>();
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

bool CAgentConnectionManager::on_cmdSTOP_USER_TASK(SCommandAttachmentImpl<cmdSTOP_USER_TASK>::ptr_t _attachment,
                                                   CCommanderChannel::weakConnectionPtr_t _channel)
{
    // TODO: add error processing, in case if user tasks won't quite
    terminateChildrenProcesses();
    auto p = _channel.lock();
    p->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd("Done", info, cmdSTOP_USER_TASK));
    return true;
}

bool CAgentConnectionManager::on_cmdCUSTOM_CMD(SCommandAttachmentImpl<cmdCUSTOM_CMD>::ptr_t _attachment,
                                               CCommanderChannel::weakConnectionPtr_t _channel)
{
    if (!m_UIConnectionMng)
        LOG(warning) << "UI connection manager doesn't run. Skipping command notification broadcasting.";
    else
    {
        m_UIConnectionMng->notifyAboutCustomCmd(_attachment);
    }
    return true;
}
