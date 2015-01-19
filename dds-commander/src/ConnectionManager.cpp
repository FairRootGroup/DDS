// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "ConnectionManager.h"
#include "Topology.h"
#include "CommandAttachmentImpl.h"
// BOOST
#include <boost/filesystem.hpp>
// BOOST
#include <boost/property_tree/ini_parser.hpp>

using namespace dds;
using namespace std;
using namespace MiscCommon;
namespace fs = boost::filesystem;

CConnectionManager::CConnectionManager(const SOptions_t& _options,
                                       boost::asio::io_service& _io_service,
                                       boost::asio::ip::tcp::endpoint& _endpoint)
    : CConnectionManagerImpl<CAgentChannel, CConnectionManager>(_io_service, _endpoint)
{

    //    // create cfg file if missing
    //    boost::filesystem::path cfgFile(CUserDefaults::instance().getDDSPath());
    //    cfgFile /= "task.cfg";
    //
    //    if (fs::exists(cfgFile))
    //    {
    //        LOG(debug) << "Removing key-value storage file: " << cfgFile.generic_string();
    //        if (!fs::remove(cfgFile))
    //            LOG(fatal) << "Failed to remove key-value storage file: " << cfgFile.generic_string();
    //    }
    //
    //    if (!fs::exists(cfgFile))
    //    {
    //        LOG(debug) << "Create key-value storage file: " << cfgFile.generic_string();
    //        ofstream f(cfgFile.generic_string());
    //    }
    //    m_sCfgFilePath = cfgFile.generic_string();
    //
    //    boost::property_tree::ini_parser::read_ini(cfgFile.generic_string(), m_propertyPT);
}

CConnectionManager::~CConnectionManager()
{
}

void CConnectionManager::newClientCreated(CAgentChannel::connectionPtr_t _newClient)
{
    // Subscribe on protocol messages
    std::function<bool(SCommandAttachmentImpl<cmdGET_LOG>::ptr_t _attachment, CAgentChannel * _channel)> fGET_LOG =
        [this](SCommandAttachmentImpl<cmdGET_LOG>::ptr_t _attachment, CAgentChannel* _channel) -> bool
    {
        return this->on_cmdGET_LOG(_attachment, getWeakPtr(_channel));
    };
    _newClient->registerMessageHandler<cmdGET_LOG>(fGET_LOG);

    std::function<bool(SCommandAttachmentImpl<cmdBINARY_ATTACHMENT_RECEIVED>::ptr_t _attachment,
                       CAgentChannel * _channel)> fBINARY_ATTACHMENT_RECEIVED =
        [this](SCommandAttachmentImpl<cmdBINARY_ATTACHMENT_RECEIVED>::ptr_t _attachment, CAgentChannel* _channel)
            -> bool
    {
        return this->on_cmdBINARY_ATTACHMENT_RECEIVED(_attachment, getWeakPtr(_channel));
    };
    _newClient->registerMessageHandler<cmdBINARY_ATTACHMENT_RECEIVED>(fBINARY_ATTACHMENT_RECEIVED);

    std::function<bool(SCommandAttachmentImpl<cmdGET_AGENTS_INFO>::ptr_t _attachment, CAgentChannel * _channel)>
        fGET_AGENTS_INFO =
            [this](SCommandAttachmentImpl<cmdGET_AGENTS_INFO>::ptr_t _attachment, CAgentChannel* _channel) -> bool
    {
        return this->on_cmdGET_AGENTS_INFO(_attachment, getWeakPtr(_channel));
    };
    _newClient->registerMessageHandler<cmdGET_AGENTS_INFO>(fGET_AGENTS_INFO);

    std::function<bool(SCommandAttachmentImpl<cmdSUBMIT>::ptr_t _attachment, CAgentChannel * _channel)> fSUBMIT =
        [this](SCommandAttachmentImpl<cmdSUBMIT>::ptr_t _attachment, CAgentChannel* _channel) -> bool
    {
        return this->on_cmdSUBMIT(_attachment, getWeakPtr(_channel));
    };
    _newClient->registerMessageHandler<cmdSUBMIT>(fSUBMIT);

    std::function<bool(SCommandAttachmentImpl<cmdACTIVATE_AGENT>::ptr_t _attachment, CAgentChannel * _channel)>
        fACTIVATE_AGENT =
            [this](SCommandAttachmentImpl<cmdACTIVATE_AGENT>::ptr_t _attachment, CAgentChannel* _channel) -> bool
    {
        return this->on_cmdACTIVATE_AGENT(_attachment, getWeakPtr(_channel));
    };
    _newClient->registerMessageHandler<cmdACTIVATE_AGENT>(fACTIVATE_AGENT);

    std::function<bool(SCommandAttachmentImpl<cmdTRANSPORT_TEST>::ptr_t _attachment, CAgentChannel * _channel)>
        fTRANSPORT_TEST =
            [this](SCommandAttachmentImpl<cmdTRANSPORT_TEST>::ptr_t _attachment, CAgentChannel* _channel) -> bool
    {
        return this->on_cmdTRANSPORT_TEST(_attachment, getWeakPtr(_channel));
    };
    _newClient->registerMessageHandler<cmdTRANSPORT_TEST>(fTRANSPORT_TEST);

    std::function<bool(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment, CAgentChannel * _channel)>
        fSIMPLE_MSG = [this](SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment, CAgentChannel* _channel) -> bool
    {
        return this->on_cmdSIMPLE_MSG(_attachment, getWeakPtr(_channel));
    };
    _newClient->registerMessageHandler<cmdSIMPLE_MSG>(fSIMPLE_MSG);

    std::function<bool(SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment, CAgentChannel * _channel)>
        fUPDATE_KEY = [this](SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment, CAgentChannel* _channel) -> bool
    {
        return this->on_cmdUPDATE_KEY(_attachment, getWeakPtr(_channel));
    };
    _newClient->registerMessageHandler<cmdUPDATE_KEY>(fUPDATE_KEY);
}

void CConnectionManager::_createInfoFile(size_t _port) const
{
    const std::string sSrvCfg(CUserDefaults::instance().getServerInfoFileLocationSrv());
    LOG(MiscCommon::info) << "Creating the server info file: " << sSrvCfg;
    std::ofstream f(sSrvCfg.c_str());
    if (!f.is_open() || !f.good())
    {
        std::string msg("Could not open the server info configuration file: ");
        msg += sSrvCfg;
        throw std::runtime_error(msg);
    }

    std::string srvHost;
    MiscCommon::get_hostname(&srvHost);
    std::string srvUser;
    MiscCommon::get_cuser_name(&srvUser);

    f << "[server]\n"
      << "host=" << srvHost << "\n"
      << "user=" << srvUser << "\n"
      << "port=" << _port << "\n" << std::endl;
}

void CConnectionManager::_deleteInfoFile() const
{
    const std::string sSrvCfg(CUserDefaults::instance().getServerInfoFileLocationSrv());
    if (sSrvCfg.empty())
        return;

    // TODO: check error code
    unlink(sSrvCfg.c_str());
}

bool CConnectionManager::on_cmdGET_LOG(SCommandAttachmentImpl<cmdGET_LOG>::ptr_t _attachment,
                                       CAgentChannel::weakConnectionPtr_t _channel)
{
    std::lock_guard<std::mutex> lock(m_getLog.m_mutexStart);
    try
    {
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
            return (_v->getType() == EAgentChannelType::AGENT && _v->started());
        };

        m_getLog.m_nofRequests = countNofChannels(condition);

        if (m_getLog.m_nofRequests == 0)
        {
            p->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd("There are no connected agents."));
            return true;
        }

        broadcastSimpleMsg<cmdGET_LOG>(condition);
    }
    catch (bad_weak_ptr& e)
    {
        // TODO: Do we need to log something here?
    }
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

            // TODO: Job submission should be moved from here to a thread
            // Resolve topology
            m_topo.init(_attachment->m_sTopoFile);
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
    std::lock_guard<std::mutex> lock(m_ActivateAgents.m_mutexStart);
    try
    {
        if (!m_ActivateAgents.m_channel.expired())
            throw runtime_error("Can not process the request. Activation of the agents is already in progress.");

        // remember the UI channel, which requested to submit the job
        m_ActivateAgents.m_channel = _channel;
        m_ActivateAgents.zeroCounters();

        auto p = m_ActivateAgents.m_channel.lock();

        try
        {
            // Start distributing user tasks between agents
            // TODO: We might need to create a thread here to avoid blocking a thread of the transport

            // Send binaries of user jobs to all active agents.
            // Send activate signal to all agents. This will trigger start of user jobs on the agents.
            auto condition = [](CAgentChannel::connectionPtr_t _v)
            {
                return (_v->getType() == EAgentChannelType::AGENT && _v->started());
            };

            m_ActivateAgents.m_nofRequests = m_topo.getMainGroup()->getTotalNofTasks();
            size_t nofAgents = countNofChannels(condition);

            if (nofAgents == 0)
                throw runtime_error("There are no connected agents.");
            if (nofAgents < m_ActivateAgents.m_nofRequests)
                throw runtime_error("The number of agents is not sufficient for this topology.");

            CAgentChannel::weakConnectionPtrVector_t channels(getChannels(condition));

            m_scheduler.makeSchedule(m_topo, channels);
            const CSSHScheduler::ScheduleVector_t& schedule = m_scheduler.getSchedule();

            for (const auto& sch : schedule)
            {
                SAssignUserTaskCmd msg_cmd;

                // Set Task ID
                msg_cmd.m_sID = to_string(sch.m_taskID);

                if (sch.m_channel.expired())
                    continue;
                auto ptr = sch.m_channel.lock();

                // Set task ID for agent and add it to map
                // TODO: Do we have to assign taskID here?
                // TODO: Probably it has to be assigned when the task is successfully activated.
                ptr->setTaskID(sch.m_taskID);
                m_taskIDToAgentChannelMap[sch.m_taskID] = sch.m_channel;

                if (sch.m_task->isExeReachable())
                {
                    msg_cmd.m_sExeFile = sch.m_task->getExe();
                }
                else
                {
                    // Executable is not reachable by the agent.
                    // Upload it and change its path to $DDS_LOCATION on the WN

                    // Expand the string for the program to extract exe name and command line arguments
                    wordexp_t result;
                    switch (wordexp(sch.m_task->getExe().c_str(), &result, 0))
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
                                                sch.m_task->getExe());

                        case WRDE_BADCHAR:
                            throw runtime_error(
                                "Illegal occurrence of newline or one of |, &, ;, <, >, (, ), {, } in " +
                                sch.m_task->getExe());
                            break;

                        case WRDE_BADVAL:
                            throw runtime_error("An undefined shell variable was referenced, and the WRDE_UNDEF flag "
                                                "told us to consider this an error in " +
                                                sch.m_task->getExe());
                            break;

                        case WRDE_CMDSUB:
                            throw runtime_error("Command substitution occurred, and the WRDE_NOCMD flag told us to "
                                                "consider this an error in " +
                                                sch.m_task->getExe());
                            break;
                        case WRDE_SYNTAX:
                            throw runtime_error(
                                "Shell syntax error, such as unbalanced parentheses or unmatched quotes in " +
                                sch.m_task->getExe());
                            break;

                        default: // Some other error.
                            throw runtime_error("failed to process the user's executable path: " +
                                                sch.m_task->getExe());
                    }
                }
                ptr->pushMsg<cmdASSIGN_USER_TASK>(msg_cmd);
            }

            // Active agents.
            broadcastSimpleMsg<cmdACTIVATE_AGENT>(
                [](CAgentChannel::connectionPtr_t _v)
                {
                    return (_v->getType() == EAgentChannelType::AGENT && _v->started() && _v->getTaskID() != 0);
                });
        }
        catch (bad_weak_ptr& _e)
        {
            // TODO: Do we need to log something here?
        }
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

bool CConnectionManager::on_cmdGET_AGENTS_INFO(SCommandAttachmentImpl<cmdGET_AGENTS_INFO>::ptr_t _attachment,
                                               CAgentChannel::weakConnectionPtr_t _channel)
{
    try
    {
        SAgentsInfoCmd cmd;
        stringstream ss;

        CAgentChannel::weakConnectionPtrVector_t channels(
            getChannels([](CAgentChannel::connectionPtr_t _v)
                        {
                            return (_v->getType() == EAgentChannelType::AGENT && _v->started());
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
               << "\nAgent startup time: " << ptr->getStartupTime().count() << " sec."
               << "\nTask ID: " << sTaskName << "\n";
        }
        cmd.m_sListOfAgents = ss.str();

        if (!_channel.expired())
        {
            auto p = _channel.lock();
            p->pushMsg<cmdREPLY_AGENTS_INFO>(cmd);
        }
    }
    catch (bad_weak_ptr& e)
    {
        // TODO: Do we need to log something here?
    }

    return true;
}

bool CConnectionManager::on_cmdTRANSPORT_TEST(SCommandAttachmentImpl<cmdTRANSPORT_TEST>::ptr_t _attachment,
                                              CAgentChannel::weakConnectionPtr_t _channel)
{
    try
    {
        std::lock_guard<std::mutex> lock(m_transportTest.m_mutexStart);

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
            return (_v != nullptr && _v->getType() == EAgentChannelType::AGENT && _v->started());
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

            string fileName = "test_data_" + std::to_string(size) + ".bin";
            broadcastBinaryAttachmentCmd(data, fileName, cmdTRANSPORT_TEST, condition);
        }

        if (m_transportTest.m_nofRequests == 0 && !m_transportTest.m_channel.expired())
        {
            auto p = m_transportTest.m_channel.lock();
            p->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd("There are no active agents.", fatal, cmdTRANSPORT_TEST));
        }
    }
    catch (bad_weak_ptr& e)
    {
        // TODO: Do we need to log something here?
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
    try
    {
        //    m_propertyPT.put(_attachment->m_sKey, _attachment->m_sValue);
        //    boost::property_tree::ini_parser::write_ini(m_sCfgFilePath, m_propertyPT);

        // If UI channel sends a property update than property key does not contain a hash.
        // In this case each agent set the property key hash himself.
        auto channelPtr = _channel.lock();
        const bool sentFromUIChannel = (channelPtr->getType() == EAgentChannelType::UI);

        string propertyID(_attachment->m_sKey);
        if (!sentFromUIChannel)
        {
            // If we get property key from agent we have to parse it to get the property ID.
            // propertyID.17621121989812
            const string propertyKey(_attachment->m_sKey);
            const size_t pos(propertyKey.find_last_of('.'));
            propertyID = propertyKey.substr(0, pos);
        }

        CTopology::TaskIteratorPair_t taskIt = m_topo.getTaskIteratorForPropertyId(propertyID);

        for (auto it = taskIt.first; it != taskIt.second; ++it)
        {
            auto iter = m_taskIDToAgentChannelMap.find(it->first);
            if (iter == m_taskIDToAgentChannelMap.end())
            {
                LOG(debug) << "on_cmdUPDATE_KEY task <" << it->first
                           << "> not found in map. Property will not be updated.";
            }
            else
            {
                if (iter->second.expired())
                    continue;
                auto ptr = iter->second.lock();

                if (ptr->getType() == EAgentChannelType::AGENT && ptr->started())
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
                            ptr->pushMsg<cmdUPDATE_KEY>(*_attachment);
                            LOG(debug) << "Property update from agent channel: <" << *_attachment << ">";
                        }
                    }
                }
            }
        }

        channelPtr->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(
            "All related agents have been advised about the key update.", MiscCommon::info, cmdUPDATE_KEY));
        // Shutdown the initial channel if it's an UI one
        if (sentFromUIChannel)
        {
            LOG(debug) << "A key-value notification has been broadcasted:" << *_attachment
                       << " There are no more requests from the UI channel, therefore shutting it down.";
            channelPtr->pushMsg<cmdSHUTDOWN>();
        }
    }
    catch (const bad_weak_ptr& e)
    {
        LOG(error) << "bad_weak_ptr exception in on_cmdUPDATE_KEY: " << e.what();
    }

    return true;
}
