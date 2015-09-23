// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "ConnectionManager.h"
#include "Topology.h"
#include "CommandAttachmentImpl.h"
#include "CRC.h"
// BOOST
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-register"
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#pragma clang diagnostic pop
#include <boost/filesystem.hpp>

// silance "Unused typedef" warning using clang 3.7+ and boost < 1.59
#if BOOST_VERSION < 105900
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-local-typedef"
#endif
#include <boost/property_tree/ini_parser.hpp>
#if BOOST_VERSION < 105900
#pragma clang diagnostic pop
#endif

// STD
#include <mutex>

using namespace dds;
using namespace dds::commander_cmd;
using namespace dds::topology_api;
using namespace dds::user_defaults_api;
using namespace dds::protocol_api;
using namespace std;
using namespace MiscCommon;
namespace fs = boost::filesystem;

CConnectionManager::CConnectionManager(const SOptions_t& _options)
    : CConnectionManagerImpl<CAgentChannel, CConnectionManager>(20000, 22000, true)
{
    LOG(info) << "CConnectionManager constructor";
}

CConnectionManager::~CConnectionManager()
{
}

void CConnectionManager::_start()
{
    auto self(this->shared_from_this());
    CMonitoringThread::instance().registerCallbackFunction(
        [this, self]() -> bool
        {
            try
            {
                m_propertyPT.clear();

                CAgentChannel::weakConnectionPtrVector_t channels(
                    getChannels([](CAgentChannel::connectionPtr_t _v)
                                {
                                    return (_v->getChannelType() == EChannelType::AGENT && _v->started());
                                }));

                for (const auto& v : channels)
                {
                    if (v.expired())
                        continue;
                    auto ptr = v.lock();

                    {
                        lock_guard<mutex> lock(ptr->getPropertyPTMutex());
                        const boost::property_tree::ptree& pt = ptr->getPropertyPT();
                        for (const auto& v1 : pt)
                        {
                            for (const auto& v2 : v1.second)
                            {
                                {
                                    lock_guard<mutex> lock(m_propertyPTMutex);
                                    m_propertyPT.put((v1.first + "." + v2.first), v2.second.data());
                                }
                            }
                        }
                    }
                }
                LOG(info) << "key-value lazy collector: checking available channels for new updates...";
            }
            catch (exception& _e)
            {
                LOG(fatal) << "key-value lazy collector: error: " << _e.what();
            }

            return true;
        },
        chrono::seconds(60));
}

void CConnectionManager::newClientCreated(CAgentChannel::connectionPtr_t _newClient)
{
    // Subscribe on protocol messages
    function<bool(SCommandAttachmentImpl<cmdGET_LOG>::ptr_t _attachment, CAgentChannel * _channel)> fGET_LOG = [this](
        SCommandAttachmentImpl<cmdGET_LOG>::ptr_t _attachment, CAgentChannel* _channel) -> bool
    {
        return this->on_cmdGET_LOG(_attachment, getWeakPtr(_channel));
    };
    _newClient->registerMessageHandler<cmdGET_LOG>(fGET_LOG);

    function<bool(SCommandAttachmentImpl<cmdBINARY_ATTACHMENT_RECEIVED>::ptr_t _attachment, CAgentChannel * _channel)>
        fBINARY_ATTACHMENT_RECEIVED = [this](SCommandAttachmentImpl<cmdBINARY_ATTACHMENT_RECEIVED>::ptr_t _attachment,
                                             CAgentChannel* _channel) -> bool
    {
        return this->on_cmdBINARY_ATTACHMENT_RECEIVED(_attachment, getWeakPtr(_channel));
    };
    _newClient->registerMessageHandler<cmdBINARY_ATTACHMENT_RECEIVED>(fBINARY_ATTACHMENT_RECEIVED);

    function<bool(SCommandAttachmentImpl<cmdGET_AGENTS_INFO>::ptr_t _attachment, CAgentChannel * _channel)>
        fGET_AGENTS_INFO = [this](SCommandAttachmentImpl<cmdGET_AGENTS_INFO>::ptr_t _attachment,
                                  CAgentChannel* _channel) -> bool
    {
        return this->on_cmdGET_AGENTS_INFO(_attachment, getWeakPtr(_channel));
    };
    _newClient->registerMessageHandler<cmdGET_AGENTS_INFO>(fGET_AGENTS_INFO);

    function<bool(SCommandAttachmentImpl<cmdSUBMIT>::ptr_t _attachment, CAgentChannel * _channel)> fSUBMIT = [this](
        SCommandAttachmentImpl<cmdSUBMIT>::ptr_t _attachment, CAgentChannel* _channel) -> bool
    {
        return this->on_cmdSUBMIT(_attachment, getWeakPtr(_channel));
    };
    _newClient->registerMessageHandler<cmdSUBMIT>(fSUBMIT);

    function<bool(SCommandAttachmentImpl<cmdACTIVATE_AGENT>::ptr_t _attachment, CAgentChannel * _channel)>
        fACTIVATE_AGENT = [this](SCommandAttachmentImpl<cmdACTIVATE_AGENT>::ptr_t _attachment,
                                 CAgentChannel* _channel) -> bool
    {
        return this->on_cmdACTIVATE_AGENT(_attachment, getWeakPtr(_channel));
    };
    _newClient->registerMessageHandler<cmdACTIVATE_AGENT>(fACTIVATE_AGENT);

    function<bool(SCommandAttachmentImpl<cmdSTOP_USER_TASK>::ptr_t _attachment, CAgentChannel * _channel)>
        fSTOP_USER_TASK = [this](SCommandAttachmentImpl<cmdSTOP_USER_TASK>::ptr_t _attachment,
                                 CAgentChannel* _channel) -> bool
    {
        return this->on_cmdSTOP_USER_TASK(_attachment, getWeakPtr(_channel));
    };
    _newClient->registerMessageHandler<cmdSTOP_USER_TASK>(fSTOP_USER_TASK);

    function<bool(SCommandAttachmentImpl<cmdTRANSPORT_TEST>::ptr_t _attachment, CAgentChannel * _channel)>
        fTRANSPORT_TEST = [this](SCommandAttachmentImpl<cmdTRANSPORT_TEST>::ptr_t _attachment,
                                 CAgentChannel* _channel) -> bool
    {
        return this->on_cmdTRANSPORT_TEST(_attachment, getWeakPtr(_channel));
    };
    _newClient->registerMessageHandler<cmdTRANSPORT_TEST>(fTRANSPORT_TEST);

    function<bool(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment, CAgentChannel * _channel)> fSIMPLE_MSG =
        [this](SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment, CAgentChannel* _channel) -> bool
    {
        return this->on_cmdSIMPLE_MSG(_attachment, getWeakPtr(_channel));
    };
    _newClient->registerMessageHandler<cmdSIMPLE_MSG>(fSIMPLE_MSG);

    function<bool(SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment, CAgentChannel * _channel)> fUPDATE_KEY =
        [this](SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment, CAgentChannel* _channel) -> bool
    {
        return this->on_cmdUPDATE_KEY(_attachment, getWeakPtr(_channel));
    };
    _newClient->registerMessageHandler<cmdUPDATE_KEY>(fUPDATE_KEY);

    function<bool(SCommandAttachmentImpl<cmdUSER_TASK_DONE>::ptr_t _attachment, CAgentChannel * _channel)>
        fUSER_TASK_DONE = [this](SCommandAttachmentImpl<cmdUSER_TASK_DONE>::ptr_t _attachment,
                                 CAgentChannel* _channel) -> bool
    {
        return this->on_cmdUSER_TASK_DONE(_attachment, getWeakPtr(_channel));
    };
    _newClient->registerMessageHandler<cmdUSER_TASK_DONE>(fUSER_TASK_DONE);

    function<bool(SCommandAttachmentImpl<cmdGET_PROP_LIST>::ptr_t _attachment, CAgentChannel * _channel)>
        fGET_PROP_LIST = [this](SCommandAttachmentImpl<cmdGET_PROP_LIST>::ptr_t _attachment,
                                CAgentChannel* _channel) -> bool
    {
        return this->on_cmdGET_PROP_LIST(_attachment, getWeakPtr(_channel));
    };
    _newClient->registerMessageHandler<cmdGET_PROP_LIST>(fGET_PROP_LIST);

    function<bool(SCommandAttachmentImpl<cmdGET_PROP_VALUES>::ptr_t _attachment, CAgentChannel * _channel)>
        fGET_PROP_VALUES = [this](SCommandAttachmentImpl<cmdGET_PROP_VALUES>::ptr_t _attachment,
                                  CAgentChannel* _channel) -> bool
    {
        return this->on_cmdGET_PROP_VALUES(_attachment, getWeakPtr(_channel));
    };
    _newClient->registerMessageHandler<cmdGET_PROP_VALUES>(fGET_PROP_VALUES);

    function<bool(SCommandAttachmentImpl<cmdSET_TOPOLOGY>::ptr_t _attachment, CAgentChannel * _channel)> fSET_TOPOLOGY =
        [this](SCommandAttachmentImpl<cmdSET_TOPOLOGY>::ptr_t _attachment, CAgentChannel* _channel) -> bool
    {
        return this->on_cmdSET_TOPOLOGY(_attachment, getWeakPtr(_channel));
    };
    _newClient->registerMessageHandler<cmdSET_TOPOLOGY>(fSET_TOPOLOGY);

    function<bool(SCommandAttachmentImpl<cmdREPLY_ID>::ptr_t _attachment, CAgentChannel * _channel)> fREPLY_ID = [this](
        SCommandAttachmentImpl<cmdREPLY_ID>::ptr_t _attachment, CAgentChannel* _channel) -> bool
    {
        return this->on_cmdREPLY_ID(_attachment, getWeakPtr(_channel));
    };
    _newClient->registerMessageHandler<cmdREPLY_ID>(fREPLY_ID);
}

void CConnectionManager::_createInfoFile(const vector<size_t>& _ports) const
{
    const string sSrvCfg(CUserDefaults::instance().getServerInfoFileLocationSrv());
    LOG(MiscCommon::info) << "Creating the server info file: " << sSrvCfg;
    ofstream f(sSrvCfg.c_str());
    if (!f.is_open() || !f.good())
    {
        string msg("Could not open the server info configuration file: ");
        msg += sSrvCfg;
        throw runtime_error(msg);
    }

    string srvHost;
    MiscCommon::get_hostname(&srvHost);
    string srvUser;
    MiscCommon::get_cuser_name(&srvUser);

    if (_ports.size() > 0)
    {
        f << "[server]\n"
          << "host=" << srvHost << "\n"
          << "user=" << srvUser << "\n"
          << "port=" << _ports[0] << "\n" << endl;
    }

    if (_ports.size() > 1)
    {
        f << "[ui]\n"
          << "host=" << srvHost << "\n"
          << "user=" << srvUser << "\n"
          << "port=" << _ports[1] << "\n" << endl;
    }
}

void CConnectionManager::_deleteInfoFile() const
{
    const string sSrvCfg(CUserDefaults::instance().getServerInfoFileLocationSrv());
    if (sSrvCfg.empty())
        return;

    // TODO: check error code
    unlink(sSrvCfg.c_str());
}

bool CConnectionManager::on_cmdGET_LOG(SCommandAttachmentImpl<cmdGET_LOG>::ptr_t _attachment,
                                       CAgentChannel::weakConnectionPtr_t _channel)
{
    lock_guard<mutex> lock(m_getLog.m_mutexStart);

    if (!m_getLog.m_channel.expired())
    {
        auto p = _channel.lock();
        p->pushMsg<cmdSIMPLE_MSG>(
            SSimpleMsgCmd("Can not process the request. The getlog command is already in progress."));
        return true;
    }
    m_getLog.m_channel = _channel;
    m_getLog.zeroCounters();

    auto p = m_getLog.m_channel.lock();
    // Create directory to store logs
    const string sLogStorageDir(CUserDefaults::instance().getAgentLogStorageDir());
    fs::path dir(sLogStorageDir);
    if (!fs::exists(dir) && !fs::create_directory(dir))
    {
        stringstream ss;
        ss << "Could not create directory " << sLogStorageDir << " to save log files.";
        p->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(ss.str()));

        m_getLog.m_channel.reset();
        return true;
    }

    auto condition = [](CAgentChannel::connectionPtr_t _v)
    {
        return (_v->getChannelType() == EChannelType::AGENT && _v->started());
    };

    m_getLog.m_nofRequests = countNofChannels(condition);

    if (m_getLog.m_nofRequests == 0)
    {
        p->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd("There are no connected agents."));
        return true;
    }

    broadcastSimpleMsg<cmdGET_LOG>(condition);

    return true;
}

bool CConnectionManager::on_cmdBINARY_ATTACHMENT_RECEIVED(
    SCommandAttachmentImpl<cmdBINARY_ATTACHMENT_RECEIVED>::ptr_t _attachment,
    CAgentChannel::weakConnectionPtr_t _channel)
{
    switch (_attachment->m_srcCommand)
    {
        case cmdGET_LOG:
        {
            m_getLog.processMessage<SBinaryAttachmentReceivedCmd>(*_attachment, _channel);
            return true;
        }

        case cmdTRANSPORT_TEST:
        {
            m_transportTest.m_totalReceived += _attachment->m_receivedFileSize;
            m_transportTest.m_totalTime += _attachment->m_downloadTime;
            m_transportTest.processMessage<SBinaryAttachmentReceivedCmd>(*_attachment, _channel);

            return true;
        }
    }

    return true;
}

bool CConnectionManager::on_cmdSUBMIT(SCommandAttachmentImpl<cmdSUBMIT>::ptr_t _attachment,
                                      CAgentChannel::weakConnectionPtr_t _channel)
{
    try
    {
        auto p = _channel.lock();

        if (_attachment->m_nRMSTypeCode == SSubmitCmd::SSH)
        {
            LOG(info) << "SSH RMS is defined by: [" << _attachment->m_sSSHCfgFile << "]";
            // TODO: Compare number of job slots in the ssh (in case of ssh) config file to what topo wants from us.

            // Submitting the job
            string outPut;
            string sCommand("$DDS_LOCATION/bin/dds-ssh");
            smart_path(&sCommand);

            stringstream ssCmd;
            ssCmd << sCommand << " -c " << _attachment->m_sSSHCfgFile << " submit";
            const size_t nCmdTimeout = 60; // in sec.
            int nDdsSSHExitCode(0);

            try
            {
                do_execv(ssCmd.str(), nCmdTimeout, &outPut, nullptr, &nDdsSSHExitCode);
            }
            catch (exception& e)
            {
                if (!outPut.empty())
                {
                    ostringstream ss;
                    ss << outPut;
                    LOG(info) << ss.str();
                    p->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(ss.str()));
                }

                string sMsg("Failed to deploy agents from the given setup: ");
                sMsg += _attachment->m_sSSHCfgFile;
                throw runtime_error(sMsg);
            }
            if (!outPut.empty())
            {
                ostringstream ss;
                ss << outPut;
                LOG(info) << ss.str();
                p->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(ss.str()));
            }

            SSimpleMsgCmd msg_cmd;
            msg_cmd.m_sMsg = (0 == nDdsSSHExitCode) ? "Agents were successfully deployed."
                                                    : "Looks like we had problems to deploy agents using DDS ssh "
                                                      "plug-in. Check dds.log for more information.";
            msg_cmd.m_srcCommand = cmdSUBMIT;
            msg_cmd.m_msgSeverity = (0 == nDdsSSHExitCode) ? info : warning;
            p->pushMsg<cmdSIMPLE_MSG>(msg_cmd);
            p->pushMsg<cmdSHUTDOWN>();
        }
    }
    catch (bad_weak_ptr& e)
    {
        // TODO: Do we need to log something here?
    }
    catch (exception& e)
    {
        if (!_channel.expired())
        {
            auto p = _channel.lock();
            p->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(e.what(), fatal, cmdSUBMIT));
        }
    }

    return true;
}

bool CConnectionManager::on_cmdACTIVATE_AGENT(SCommandAttachmentImpl<cmdACTIVATE_AGENT>::ptr_t _attachment,
                                              CAgentChannel::weakConnectionPtr_t _channel)
{
    LOG(info) << "UI channel requested a topology activation...";
    lock_guard<mutex> lock(m_ActivateAgents.m_mutexStart);
    try
    {
        if (!m_ActivateAgents.m_channel.expired())
            throw runtime_error("Can not process the request. Activation of the agents is already in progress.");

        // remember the UI channel, which requested to submit the job
        m_ActivateAgents.m_channel = _channel;
        m_ActivateAgents.zeroCounters();

        auto p = m_ActivateAgents.m_channel.lock();
        // Start distributing user tasks between agents
        // TODO: We might need to create a thread here to avoid blocking a thread of the transport

        // Send binaries of user jobs to all active agents.
        // Send activate signal to all agents. This will trigger start of user jobs on the agents.
        auto condition = [](CAgentChannel::connectionPtr_t _v)
        {
            return (_v->getChannelType() == EChannelType::AGENT && _v->started());
        };

        m_ActivateAgents.m_nofRequests = m_topo.getMainGroup()->getTotalNofTasks();
        size_t nofAgents = countNofChannels(condition);

        LOG(info) << "Available agents = " << nofAgents;

        if (nofAgents == 0)
            throw runtime_error("There are no connected agents.");
        if (nofAgents < m_ActivateAgents.m_nofRequests)
            throw runtime_error("The number of agents is not sufficient for this topology.");

        // initiate UI progress
        p->pushMsg<cmdPROGRESS>(SProgressCmd(0, m_ActivateAgents.m_nofRequests, 0));

        CAgentChannel::weakConnectionPtrVector_t channels(getChannels(condition));

        m_scheduler.makeSchedule(m_topo, channels);
        const CSSHScheduler::ScheduleVector_t& schedule = m_scheduler.getSchedule();

        // Clear the map on each activation
        m_taskIDToAgentChannelMap.clear();
        // Clear property tree
        // m_propertyPT.clear();

        for (const auto& sch : schedule)
        {
            SAssignUserTaskCmd msg_cmd;
            msg_cmd.m_sID = to_string(sch.m_taskID);
            msg_cmd.m_taskIndex = sch.m_taskInfo.m_taskIndex;
            msg_cmd.m_collectionIndex = sch.m_taskInfo.m_collectionIndex;
            msg_cmd.m_taskPath = sch.m_taskInfo.m_taskPath;
            msg_cmd.m_groupName = sch.m_taskInfo.m_task->getParentGroupId();
            msg_cmd.m_collectionName = sch.m_taskInfo.m_task->getParentCollectionId();
            msg_cmd.m_taskName = sch.m_taskInfo.m_task->getId();

            if (sch.m_channel.expired())
                continue;
            auto ptr = sch.m_channel.lock();

            // Set task ID for agent and add it to map
            // TODO: Do we have to assign taskID here?
            // TODO: Probably it has to be assigned when the task is successfully activated.
            ptr->setTaskID(sch.m_taskID);
            m_taskIDToAgentChannelMap[sch.m_taskID] = sch.m_channel;

            if (sch.m_taskInfo.m_task->isExeReachable())
            {
                msg_cmd.m_sExeFile = sch.m_taskInfo.m_task->getExe();
            }
            else
            {
                // Executable is not reachable by the agent.
                // Upload it and change its path to $DDS_LOCATION on the WN

                // Expand the string for the program to extract exe name and command line arguments
                wordexp_t result;
                switch (wordexp(sch.m_taskInfo.m_task->getExe().c_str(), &result, 0))
                {
                    case 0:
                    {
                        string sExeFilePath = result.we_wordv[0];

                        boost::filesystem::path exeFilePath(sExeFilePath);
                        string sExeFileName = exeFilePath.filename().generic_string();

                        string sExeFileNameWithArgs = sExeFileName;
                        for (size_t i = 1; i < result.we_wordc; ++i)
                        {
                            sExeFileNameWithArgs += " ";
                            sExeFileNameWithArgs += result.we_wordv[i];
                        }

                        msg_cmd.m_sExeFile = "$DDS_LOCATION/";
                        msg_cmd.m_sExeFile += sExeFileNameWithArgs;

                        wordfree(&result);

                        //
                        ptr->pushBinaryAttachmentCmd(sExeFilePath, sExeFileName, cmdASSIGN_USER_TASK);
                    }
                    break;
                    case WRDE_NOSPACE:
                        // If the error was WRDE_NOSPACE,
                        // then perhaps part of the result was allocated.
                        throw runtime_error("memory error occurred while processing the user's executable path: " +
                                            sch.m_taskInfo.m_task->getExe());

                    case WRDE_BADCHAR:
                        throw runtime_error("Illegal occurrence of newline or one of |, &, ;, <, >, (, ), {, } in " +
                                            sch.m_taskInfo.m_task->getExe());
                        break;

                    case WRDE_BADVAL:
                        throw runtime_error("An undefined shell variable was referenced, and the WRDE_UNDEF flag "
                                            "told us to consider this an error in " +
                                            sch.m_taskInfo.m_task->getExe());
                        break;

                    case WRDE_CMDSUB:
                        throw runtime_error("Command substitution occurred, and the WRDE_NOCMD flag told us to "
                                            "consider this an error in " +
                                            sch.m_taskInfo.m_task->getExe());
                        break;
                    case WRDE_SYNTAX:
                        throw runtime_error(
                            "Shell syntax error, such as unbalanced parentheses or unmatched quotes in " +
                            sch.m_taskInfo.m_task->getExe());
                        break;

                    default: // Some other error.
                        throw runtime_error("failed to process the user's executable path: " +
                                            sch.m_taskInfo.m_task->getExe());
                }
            }
            ptr->pushMsg<cmdASSIGN_USER_TASK>(msg_cmd);
            ptr->setState(EAgentState::executing);
        }

        // Active agents.
        broadcastSimpleMsg<cmdACTIVATE_AGENT>(
            [](CAgentChannel::connectionPtr_t _v)
            {
                return (_v->getChannelType() == EChannelType::AGENT && _v->started() && _v->getTaskID() != 0);
            });
    }
    catch (exception& _e)
    {
        auto p = _channel.lock();
        p->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(_e.what(), fatal, cmdACTIVATE_AGENT));

        m_ActivateAgents.m_channel.reset();
        return true;
    }
    return true;
}

bool CConnectionManager::on_cmdSTOP_USER_TASK(SCommandAttachmentImpl<cmdSTOP_USER_TASK>::ptr_t _attachment,
                                              CAgentChannel::weakConnectionPtr_t _channel)
{
    lock_guard<mutex> lock(m_StopUserTasks.m_mutexStart);
    try
    {
        if (!m_StopUserTasks.m_channel.expired())
            throw runtime_error("Can not process the request. Stopping of tasks is already in progress.");

        // remember the UI channel, which requested to submit the job
        m_StopUserTasks.m_channel = _channel;
        m_StopUserTasks.zeroCounters();

        auto p = m_StopUserTasks.m_channel.lock();
        auto condition = [](CAgentChannel::connectionPtr_t _v)
        {
            return (_v->getChannelType() == EChannelType::AGENT && _v->started() && _v->getTaskID() != 0);
        };

        m_StopUserTasks.m_nofRequests = countNofChannels(condition);

        // Stop UI if no tasks are running
        if (m_StopUserTasks.m_nofRequests <= 0)
        {
            p->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd("No running tasks. Nothing to stop.", fatal, cmdSTOP_USER_TASK));
            m_StopUserTasks.m_channel.reset();
            return true;
        }

        // initiate the progress on the UI
        p->pushMsg<cmdPROGRESS>(SProgressCmd(0, m_StopUserTasks.m_nofRequests, 0));

        CAgentChannel::weakConnectionPtrVector_t channels(getChannels(condition));
        for (const auto& v : channels)
        {
            if (v.expired())
                continue;
            auto ptr = v.lock();
            // remove task ID from the channel
            // ptr->setTaskID(0);
            // dequeue important (or expensive) messages
            ptr->dequeueMsg<cmdUPDATE_KEY>();
            ptr->dequeueMsg<cmdDELETE_KEY>();
            // send stop message
            ptr->template pushMsg<cmdSTOP_USER_TASK>();
            // ptr->setState(EAgentState::idle);
        }
    }
    catch (exception& _e)
    {
        auto p = _channel.lock();
        p->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(_e.what(), fatal, cmdSTOP_USER_TASK));

        m_StopUserTasks.m_channel.reset();
        return true;
    }
    return true;
}

bool CConnectionManager::on_cmdGET_AGENTS_INFO(SCommandAttachmentImpl<cmdGET_AGENTS_INFO>::ptr_t _attachment,
                                               CAgentChannel::weakConnectionPtr_t _channel)
{
    SAgentsInfoCmd cmd;
    stringstream ss;

    CAgentChannel::weakConnectionPtrVector_t channels(
        getChannels([](CAgentChannel::connectionPtr_t _v)
                    {
                        return (_v->getChannelType() == EChannelType::AGENT && _v->started());
                    }));

    for (const auto& v : channels)
    {
        if (v.expired())
            continue;
        auto ptr = v.lock();

        string sTaskName("no task is assigned");
        if (ptr->getTaskID() > 0)
        {
            TaskPtr_t task = m_topo.getTaskByHash(ptr->getTaskID());
            stringstream ssTaskString;
            ssTaskString << ptr->getTaskID() << " (" << task->getId() << ")";
            sTaskName = ssTaskString.str();
        }

        ++cmd.m_nActiveAgents;
        ss << " -------------->>> " << ptr->getId() << "\nHost Info: " << ptr->getRemoteHostInfo().m_username << "@"
           << ptr->getRemoteHostInfo().m_host << ":" << ptr->getRemoteHostInfo().m_DDSPath
           << "\nAgent pid: " << ptr->getRemoteHostInfo().m_agentPid
           << "\nAgent UI port: " << ptr->getRemoteHostInfo().m_agentPort
           << "\nAgent startup time: " << chrono::duration<double>(ptr->getStartupTime()).count() << " s"
           << "\nState: " << g_agentStates.at(ptr->getState()) << "\n"
           << "\nTask ID: " << sTaskName << "\n";
    }
    cmd.m_sListOfAgents = ss.str();

    if (!_channel.expired())
    {
        auto p = _channel.lock();
        p->pushMsg<cmdREPLY_AGENTS_INFO>(cmd);
    }

    return true;
}

bool CConnectionManager::on_cmdTRANSPORT_TEST(SCommandAttachmentImpl<cmdTRANSPORT_TEST>::ptr_t _attachment,
                                              CAgentChannel::weakConnectionPtr_t _channel)
{
    lock_guard<mutex> lock(m_transportTest.m_mutexStart);

    if (!m_transportTest.m_channel.expired())
    {
        auto p = _channel.lock();
        p->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(
            "Can not process the request. The test command is already in progress.", fatal, cmdTRANSPORT_TEST));
        return true;
    }
    m_transportTest.m_channel = _channel;
    m_transportTest.zeroCounters();

    auto condition = [](CAgentChannel::connectionPtr_t _v)
    {
        return (_v != nullptr && _v->getChannelType() == EChannelType::AGENT && _v->started());
    };

    vector<size_t> binarySizes{ 1000, 10000, 1000, 100000, 1000, 1000000, 1000, 10000000, 1000 };

    m_transportTest.m_nofRequests = binarySizes.size() * countNofChannels(condition);

    for (size_t size : binarySizes)
    {
        MiscCommon::BYTEVector_t data;
        for (size_t i = 0; i < size; ++i)
        {
            char c = rand() % 256;
            // char c = 'c';
            data.push_back(c);
        }

        string fileName = "test_data_" + to_string(size) + ".bin";
        broadcastBinaryAttachmentCmd(data, fileName, cmdTRANSPORT_TEST, condition);
    }

    if (m_transportTest.m_nofRequests == 0 && !m_transportTest.m_channel.expired())
    {
        auto p = m_transportTest.m_channel.lock();
        p->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd("There are no active agents.", fatal, cmdTRANSPORT_TEST));
    }

    return true;
}

bool CConnectionManager::on_cmdSIMPLE_MSG(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment,
                                          CAgentChannel::weakConnectionPtr_t _channel)
{
    switch (_attachment->m_srcCommand)
    {
        case cmdACTIVATE_AGENT:
            switch (_attachment->m_msgSeverity)
            {
                case info:
                    m_ActivateAgents.processMessage<SSimpleMsgCmd>(*_attachment, _channel);
                    break;
                case error:
                case fatal:
                    m_ActivateAgents.processErrorMessage<SSimpleMsgCmd>(*_attachment, _channel);
                    break;
            }
            return true; // let others to process this message

        case cmdSTOP_USER_TASK:
            switch (_attachment->m_msgSeverity)
            {
                case info:
                    m_StopUserTasks.processMessage<SSimpleMsgCmd>(*_attachment, _channel);
                    break;
                case error:
                case fatal:
                    m_StopUserTasks.processErrorMessage<SSimpleMsgCmd>(*_attachment, _channel);
                    break;
            }
            return true; // let others to process this message

        case cmdGET_LOG:
            return m_getLog.processErrorMessage<SSimpleMsgCmd>(*_attachment, _channel);

        case cmdTRANSPORT_TEST:
            return m_transportTest.processErrorMessage<SSimpleMsgCmd>(*_attachment, _channel);
    }
    return false;
}

bool CConnectionManager::on_cmdUPDATE_KEY(SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment,
                                          CAgentChannel::weakConnectionPtr_t _channel)
{
    // TODO: in general writing to m_propertyPT has to be locked with mutex
    // TODO: for the moment we only update property tree here and do not delete anything from it, otherwise we have to
    // lock, which leads to potential bottleneck.
    // m_propertyPT.put(_attachment->m_sKey, _attachment->m_sValue);
    //    boost::property_tree::ini_parser::write_ini(m_sCfgFilePath, m_propertyPT);

    // If UI channel sends a property update than property key does not contain a hash.
    // In this case each agent set the property key hash himself.
    auto channelPtr = _channel.lock();
    const bool sentFromUIChannel = (channelPtr->getChannelType() == EChannelType::UI);

    string propertyID(_attachment->m_sKey);
    if (!sentFromUIChannel)
    {
        // If we get property key from agent we have to parse it to get the property ID.
        // propertyID.17621121989812
        const string propertyKey(_attachment->m_sKey);
        const size_t pos(propertyKey.find_last_of('.'));
        propertyID = propertyKey.substr(0, pos);
    }

    // Check if the property has a write access to property.
    // If not just send back an error.
    if (!sentFromUIChannel)
    {
        auto task = m_topo.getTaskByHash(channelPtr->getTaskID());
        auto property = task->getProperty(propertyID);
        if (property == nullptr || (property != nullptr && (property->getAccessType() == EPropertyAccessType::READ)))
        {
            stringstream ss;
            if (property == nullptr)
                ss << "Can't propagate property that doesn't exist for task " << task->getId();
            else
                ss << "Can't propagate property <" << property->getId() << "> which has a READ access type for task "
                   << task->getId();
            channelPtr->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(ss.str(), MiscCommon::error, cmdUPDATE_KEY));
        }
    }

    CTopology::TaskInfoIteratorPair_t taskIt = m_topo.getTaskInfoIteratorForPropertyId(propertyID);

    for (auto it = taskIt.first; it != taskIt.second; ++it)
    {
        // We have to lock with m_mapMutex here. The map can be changed in on_cmdUSER_TASK_DONE.
        // TODO: For the moment lock is removed because it introduces the bottle neck.
        // TODO: In on_cmdUSER_TASK_DONE function we do not remove anything from m_taskIDToAgentChannelMap map.
        bool iterExists = false;
        CAgentChannel::weakConnectionPtr_t weakPtr;
        //{
        //    lock_guard<mutex> lock(m_mapMutex);
        auto iter = m_taskIDToAgentChannelMap.find(it->first);
        iterExists = (iter != m_taskIDToAgentChannelMap.end());
        if (iterExists)
            weakPtr = iter->second;
        //}

        if (!iterExists)
        {
            LOG(debug) << "on_cmdUPDATE_KEY task <" << it->first << "> not found in map. Property will not be updated.";
        }
        else
        {
            if (weakPtr.expired())
                continue;
            auto ptr = weakPtr.lock();

            if (ptr->getChannelType() == EChannelType::AGENT && ptr->started())
            {
                if (sentFromUIChannel)
                {
                    // If property changed from UI we have to change hash in the property key.
                    SUpdateKeyCmd attachment(*_attachment);
                    stringstream ss;
                    ss << propertyID << "." << ptr->getTaskID();
                    attachment.m_sKey = ss.str();
                    ptr->pushMsg<cmdUPDATE_KEY>(attachment);
                    LOG(info) << "Property update from UI channel: " << ss.str();
                }
                else
                {
                    if (ptr->getTaskID() != channelPtr->getTaskID())
                    {
                        auto task = m_topo.getTaskByHash(ptr->getTaskID());
                        auto property = task->getProperty(propertyID);
                        if (property != nullptr && (property->getAccessType() == EPropertyAccessType::READ ||
                                                    property->getAccessType() == EPropertyAccessType::READWRITE))
                        {
                            // ptr->pushMsg<cmdUPDATE_KEY>(*_attachment);
                            ptr->accumulativePushMsg<cmdUPDATE_KEY>(*_attachment);
                            LOG(debug) << "Property update from agent channel: <" << *_attachment << ">";
                        }
                    }
                }
            }
        }
    }

    channelPtr->pushMsg<cmdSIMPLE_MSG>(
        SSimpleMsgCmd("All related agents have been advised about the key update.", MiscCommon::info, cmdUPDATE_KEY));
    // Shutdown the initial channel if it's an UI one
    if (sentFromUIChannel)
    {
        LOG(debug) << "A key-value notification has been broadcasted:" << *_attachment
                   << " There are no more requests from the UI channel, therefore shutting it down.";
        channelPtr->pushMsg<cmdSHUTDOWN>();
    }

    return true;
}

bool CConnectionManager::on_cmdUSER_TASK_DONE(SCommandAttachmentImpl<cmdUSER_TASK_DONE>::ptr_t _attachment,
                                              CAgentChannel::weakConnectionPtr_t _channel)
{
    if (_channel.expired())
        return true;
    auto channelPtr = _channel.lock();

    auto taskID = channelPtr->getTaskID();
    auto task = m_topo.getTaskByHash(taskID);

    const TopoPropertyPtrVector_t& properties = task->getProperties();
    for (const auto& property : properties)
    {
        CTopology::TaskInfoIteratorPair_t taskIt = m_topo.getTaskInfoIteratorForPropertyId(property->getId());

        for (auto it = taskIt.first; it != taskIt.second; ++it)
        {
            auto iter = m_taskIDToAgentChannelMap.find(it->first);
            if (iter == m_taskIDToAgentChannelMap.end())
            {
                LOG(debug) << "on_cmdDELETE_KEY task <" << it->first
                           << "> not found in map. Property will not be deleted.";
            }
            else
            {
                if (iter->second.expired())
                    continue;
                auto ptr = iter->second.lock();

                SDeleteKeyCmd cmd;
                stringstream ss;
                ss << property->getId() << "." << taskID;
                cmd.m_sKey = ss.str();
                if (ptr->getTaskID() != 0 && ptr->getTaskID() != channelPtr->getTaskID())
                {
                    ptr->pushMsg<cmdDELETE_KEY>(cmd);
                    LOG(debug) << "Property deleted from agent channel: <" << *_attachment << ">";
                }
            }
        }
    }

    // remove task ID from the channel
    channelPtr->setTaskID(0);
    channelPtr->setState(EAgentState::idle);

    // remove task ID from the map
    // TODO: Temporary solution. We do not remove tasks from the map to avoid synchronization bottlenecks in
    // on_cmdUPDATE_KEY.
    //{
    //    lock_guard<mutex> lock(m_mapMutex);
    //    auto it = m_taskIDToAgentChannelMap.find(taskID);
    //    if (it != m_taskIDToAgentChannelMap.end())
    //        m_taskIDToAgentChannelMap.erase(it);
    //}

    LOG(info) << "User task <" << taskID << "> with path " << task->getPath() << " done";

    return true;
}

bool CConnectionManager::on_cmdGET_PROP_LIST(SCommandAttachmentImpl<cmdGET_PROP_LIST>::ptr_t _attachment,
                                             CAgentChannel::weakConnectionPtr_t _channel)
{
    lock_guard<mutex> lock(m_propertyPTMutex);
    stringstream ss;
    for (const auto& v1 : m_propertyPT)
    {
        ss << v1.first << endl;
    }

    if (!_channel.expired())
    {
        auto ptr = _channel.lock();
        ptr->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(ss.str(), MiscCommon::info, cmdGET_PROP_LIST));
    }

    return true;
}

bool CConnectionManager::on_cmdGET_PROP_VALUES(SCommandAttachmentImpl<cmdGET_PROP_VALUES>::ptr_t _attachment,
                                               CAgentChannel::weakConnectionPtr_t _channel)
{
    stringstream ss;
    {
        try
        {
            lock_guard<mutex> lock(m_propertyPTMutex);
            if (_attachment->m_sPropertyID.empty())
            {
                for (const auto& v1 : m_propertyPT)
                {
                    ss << "[" << v1.first << "]" << endl;
                    for (const auto& v2 : v1.second)
                    {
                        m_propertyPT.put((v1.first + "." + v2.first), v2.second.data());
                        ss << (v1.first + "." + v2.first) << " --> " << v2.second.data() << endl;
                    }
                }
            }
            else
            {
                auto v1 = m_propertyPT.get_child(_attachment->m_sPropertyID);
                ss << "[" << _attachment->m_sPropertyID << "]" << endl;
                for (const auto& v2 : v1)
                {
                    m_propertyPT.put((_attachment->m_sPropertyID + "." + v2.first), v2.second.data());
                    ss << (_attachment->m_sPropertyID + "." + v2.first) << ": " << v2.second.data() << endl;
                }
            }
        }
        catch (exception& e)
        {
            if (!_channel.expired())
            {
                stringstream ss;
                ss << "Error getting values for property " << _attachment->m_sPropertyID << ": " << e.what();
                auto ptr = _channel.lock();
                ptr->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(ss.str(), MiscCommon::error, cmdGET_PROP_LIST));
            }
        }
    }

    if (!_channel.expired())
    {
        auto ptr = _channel.lock();
        ptr->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(ss.str(), MiscCommon::info, cmdGET_PROP_LIST));
    }

    return true;
}

bool CConnectionManager::on_cmdSET_TOPOLOGY(SCommandAttachmentImpl<cmdSET_TOPOLOGY>::ptr_t _attachment,
                                            CAgentChannel::weakConnectionPtr_t _channel)
{
    LOG(info) << "UI channel requested to set up a new ropology. " << *_attachment;
    auto p = _channel.lock();
    try
    {
        // Resolve topology
        m_topo.setXMLValidationDisabled(_attachment->m_nDisiableValidation);
        m_topo.init(_attachment->m_sTopologyFile);
        auto p = _channel.lock();
        p->pushMsg<cmdSIMPLE_MSG>(
            SSimpleMsgCmd("new Topology is set to: " + _attachment->m_sTopologyFile, info, cmdSET_TOPOLOGY));
    }
    catch (exception& _e)
    {
        p->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(_e.what(), fatal, cmdSET_TOPOLOGY));
    }
    p->pushMsg<cmdSHUTDOWN>();
    return true;
}

bool CConnectionManager::on_cmdREPLY_ID(SCommandAttachmentImpl<cmdREPLY_ID>::ptr_t _attachment,
                                        CAgentChannel::weakConnectionPtr_t _channel)
{
    auto p = _channel.lock();
    try
    {
        LOG(debug) << "cmdREPLY_ID attachment [" << *_attachment << "] received from: " << p->remoteEndIDString();

        if (_attachment->m_id == 0)
        {
            uint64_t agentId = getAgentId();
            p->setId(agentId);
            SIDCmd msg_cmd;
            msg_cmd.m_id = p->getId();
            p->pushMsg<cmdSET_ID>(msg_cmd);
        }
        else
        {
            p->setId(_attachment->m_id);
        }
    }
    catch (exception& _e)
    {
        p->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(_e.what(), fatal, cmdREPLY_ID));
    }

    return true;
}

uint64_t CConnectionManager::getAgentId()
{
    // TODO: We have to check that ID was not assigned to some other agent.
    // Think about if it is possible to remove this lock from here?

    do
    {
        boost::uuids::uuid id = boost::uuids::random_generator()();
        stringstream strid;
        strid << id;
        uint64_t crc = MiscCommon::crc64(strid.str());

        {
            lock_guard<mutex> lock(m_agentIdSetMutex);

            bool condition = (crc != 0 && m_agentIdSet.find(crc) == m_agentIdSet.end());
            if (condition)
            {
                m_agentIdSet.insert(crc);
                return crc;
            }
        }
    } while (true);
    return 0;
}
