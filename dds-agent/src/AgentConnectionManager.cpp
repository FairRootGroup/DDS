// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// BOOST
#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
// DDS
#include "AgentConnectionManager.h"
#include "CommanderChannel.h"
#include "Logger.h"
#include "MonitoringThread.h"

using namespace boost::asio;
using namespace std;
using namespace dds;
using namespace MiscCommon;
namespace sp = std::placeholders;
using boost::asio::ip::tcp;

CAgentConnectionManager::CAgentConnectionManager(const SOptions_t& _options, boost::asio::io_service& _io_service)
    : m_service(_io_service)
    , m_options(_options)
    , m_agents()
    , m_bStarted(false)
{
}

CAgentConnectionManager::~CAgentConnectionManager()
{
}

void CAgentConnectionManager::start()
{
    if (m_bStarted)
        return;

    m_bStarted = true;
    try
    {
        const float maxIdleTime = CUserDefaults::instance().getOptions().m_server.m_idleTime;

        CMonitoringThread::instance().start(maxIdleTime, []()
                                            {
            LOG(info) << "Idle callback called";
        });

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
        boost::asio::ip::tcp::resolver resolver(m_service);
        boost::asio::ip::tcp::resolver::query query(sHost, sPort);
        boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

        // Create new agent and push handshake message
        CCommanderChannel::connectionPtr_t newAgent = CCommanderChannel::makeNew(m_service);
        // Subscribe to Shutdown command
        newAgent->registerMessageHandler<cmdSHUTDOWN>(
            [this, newAgent](SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment, CCommanderChannel* _channel)
                -> bool
            {
                // TODO: adjust the algorithm if we would need to support several agents
                // we have only one agent (newAgent) at the moment
                return this->on_cmdSHUTDOWN(_attachment, newAgent);
            });
        // Call this callback when a user process is activated
        newAgent->registerOnNewUserTaskCallback([this](pid_t _pid)
                                                {
                                                    return this->onNewUserTask(_pid);
                                                });

        boost::asio::async_connect(newAgent->socket(), endpoint_iterator,
                                   [this, &newAgent](boost::system::error_code ec, ip::tcp::resolver::iterator)
                                   {
            if (!ec)
            {
                // Create handshake message which is the first one for all agents
                SVersionCmd ver;
                newAgent->pushMsg<cmdHANDSHAKE_AGENT>(ver);
                newAgent->start();

                // Start the UI agent server
                // Let the OS pick a random available port
                boost::asio::io_service io_service;
                tcp::endpoint endpoint(tcp::v4(), 0);
                m_UIConnectionMng = make_shared<CUIConnectionManager>(io_service, endpoint);
                m_UIConnectionMng->start(false);
            }
            else
            {
                LOG(fatal) << "Cannot connect to server: " << ec.message();
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
    if (!m_bStarted)
        return;

    m_bStarted = false;

    LOG(info) << "Shutting down DDS transport...";
    // terminate external children processes (like user tasks, for example)
    terminateChildrenProcesses();

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

        // wait 5 seconds each
        for (size_t i = 0; i < 5; ++i)
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

void CAgentConnectionManager::onNewUserTask(pid_t _pid)
{
    LOG(info) << "Starting the watchdog for user task pid = " << _pid;
    {
        // remove pid from the active children list
        std::lock_guard<std::mutex> lock(m_childrenContainerMutex);
        m_children.push_back(_pid);
    }
    // Register the user task's watchdog
    CMonitoringThread::instance().registerCallbackFunction([this, _pid]() -> bool
                                                           {
        if (!IsProcessExist(_pid))
        {
            LOG(info) << "User Tasks cannot be found. Probably it has exited. pid = " << _pid;
            LOG(info) << "Stopping the watchdog for user task pid = " << _pid;

            std::lock_guard<std::mutex> lock(m_childrenContainerMutex);
            m_children.erase(remove(m_children.begin(), m_children.end(), _pid), m_children.end());
            return false;
        }

        // We must call "wait" to check exist status of a child process, otherwise we will crate a zombie :)
        int status;
        if (_pid == ::waitpid(_pid, &status, WNOHANG))
        {
            if (WIFEXITED(status))
                LOG(info) << "User task exited" << (WCOREDUMP(status) ? " and dumped core" : "") << " with status "
                          << WEXITSTATUS(status);
            if (WIFSTOPPED(status))
                LOG(info) << "User task stopped by signal " << WSTOPSIG(status);

            LOG(info) << "Stopping the watchdog for user task pid = " << _pid;

            // remove pid from the active children list
            std::lock_guard<std::mutex> lock(m_childrenContainerMutex);
            m_children.erase(remove(m_children.begin(), m_children.end(), _pid), m_children.end());

            return false;
        }

        return true;
    });
}
