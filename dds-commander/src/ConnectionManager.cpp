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
#include <boost/crc.hpp>

using namespace dds;
using namespace std;
using namespace MiscCommon;
namespace fs = boost::filesystem;

CConnectionManager::CConnectionManager(const SOptions_t& _options,
                                       boost::asio::io_service& _io_service,
                                       boost::asio::ip::tcp::endpoint& _endpoint)
    : CConnectionManagerImpl<CAgentChannel, CConnectionManager>(_options, _io_service, _endpoint)
{
}

CConnectionManager::~CConnectionManager()
{
}

void CConnectionManager::newClientCreated(CAgentChannel::connectionPtr_t _newClient)
{
    // Subscribe on protocol messages
    std::function<bool(SCommandAttachmentImpl<cmdGET_LOG>::ptr_t _attachment, CAgentChannel * _channel)> fGET_LOG =
        [this](SCommandAttachmentImpl<cmdGET_LOG>::ptr_t _attachment, CAgentChannel* _channel)
            -> bool
    {
        return this->on_cmdGET_LOG(_attachment, useRawPtr(_channel));
    };
    _newClient->registerMessageHandler<cmdGET_LOG>(fGET_LOG);

    std::function<bool(SCommandAttachmentImpl<cmdBINARY_ATTACHMENT_RECEIVED>::ptr_t _attachment,
                       CAgentChannel * _channel)> fBINARY_ATTACHMENT_RECEIVED =
        [this](SCommandAttachmentImpl<cmdBINARY_ATTACHMENT_RECEIVED>::ptr_t _attachment, CAgentChannel* _channel)
            -> bool
    {
        return this->on_cmdBINARY_ATTACHMENT_RECEIVED(_attachment, useRawPtr(_channel));
    };
    _newClient->registerMessageHandler<cmdBINARY_ATTACHMENT_RECEIVED>(fBINARY_ATTACHMENT_RECEIVED);

    std::function<bool(SCommandAttachmentImpl<cmdGET_AGENTS_INFO>::ptr_t _attachment, CAgentChannel * _channel)>
        fGET_AGENTS_INFO =
            [this](SCommandAttachmentImpl<cmdGET_AGENTS_INFO>::ptr_t _attachment, CAgentChannel* _channel)
                -> bool
    {
        return this->on_cmdGET_AGENTS_INFO(_attachment, useRawPtr(_channel));
    };
    _newClient->registerMessageHandler<cmdGET_AGENTS_INFO>(fGET_AGENTS_INFO);

    std::function<bool(SCommandAttachmentImpl<cmdSUBMIT>::ptr_t _attachment, CAgentChannel * _channel)> fSUBMIT =
        [this](SCommandAttachmentImpl<cmdSUBMIT>::ptr_t _attachment, CAgentChannel* _channel)
            -> bool
    {
        return this->on_cmdSUBMIT(_attachment, useRawPtr(_channel));
    };
    _newClient->registerMessageHandler<cmdSUBMIT>(fSUBMIT);

    std::function<bool(SCommandAttachmentImpl<cmdACTIVATE_AGENT>::ptr_t _attachment, CAgentChannel * _channel)>
        fACTIVATE_AGENT = [this](SCommandAttachmentImpl<cmdACTIVATE_AGENT>::ptr_t _attachment, CAgentChannel* _channel)
                              -> bool
    {
        return this->on_cmdACTIVATE_AGENT(_attachment, useRawPtr(_channel));
    };
    _newClient->registerMessageHandler<cmdACTIVATE_AGENT>(fACTIVATE_AGENT);

    std::function<bool(SCommandAttachmentImpl<cmdTRANSPORT_TEST>::ptr_t _attachment, CAgentChannel * _channel)>
        fTRANSPORT_TEST = [this](SCommandAttachmentImpl<cmdTRANSPORT_TEST>::ptr_t _attachment, CAgentChannel* _channel)
                              -> bool
    {
        return this->on_cmdTRANSPORT_TEST(_attachment, useRawPtr(_channel));
    };
    _newClient->registerMessageHandler<cmdTRANSPORT_TEST>(fTRANSPORT_TEST);

    std::function<bool(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment, CAgentChannel * _channel)>
        fSIMPLE_MSG = [this](SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment, CAgentChannel* _channel)
                          -> bool
    {
        return this->on_cmdSIMPLE_MSG(_attachment, useRawPtr(_channel));
    };
    _newClient->registerMessageHandler<cmdSIMPLE_MSG>(fSIMPLE_MSG);
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
            CTopology topology;
            m_sCurrentTopoFile = _attachment->m_sTopoFile;
            topology.init(m_sCurrentTopoFile);
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
        // Start distributing user tasks between agents
        // TODO: We might need to create a thread here to avoid blocking a thread of the transport
        CTopology topology;
        topology.init(m_sCurrentTopoFile);

        try
        {
            // Start distributing user tasks between agents
            // TODO: We might need to create a thread here to avoid blocking a thread of the transport
            CTopology topology;
            topology.init(m_sCurrentTopoFile);

            // Send binaries of user jobs to all active agents.
            // Send activate signal to all agents. This will trigger start of user jobs on the agents.
            auto condition = [](CAgentChannel::connectionPtr_t _v)
            {
                return (_v->getType() == EAgentChannelType::AGENT && _v->started());
            };

            m_ActivateAgents.m_nofRequests = countNofChannels(condition);

            if (m_ActivateAgents.m_nofRequests == 0)
                throw runtime_error("There are no connected agents.");
            if (topology.getMainGroup()->getTotalNofTasks() > m_ActivateAgents.m_nofRequests)
                throw runtime_error("The number of active agents is not sufficient for this topology.");

            CAgentChannel::weakConnectionPtrVector_t channels(getChannels(condition));
            TopoElementPtrVector_t tasks(topology.getMainGroup()->getElementsByType(ETopoType::TASK));
            TopoElementPtrVector_t::const_iterator it_tasks = tasks.begin();
            for (const auto& v : channels)
            {
                if (v.expired())
                    continue;
                auto ptr = v.lock();

                // Assign user's tasks to agents
                if (it_tasks == tasks.end())
                    break;
                SAssignUserTaskCmd msg_cmd;
                TaskPtr_t topoTask = dynamic_pointer_cast<CTask>(*it_tasks);

                if (topoTask->isExeReachable())
                    msg_cmd.m_sExeFile = topoTask->getExe();
                else
                {
                    // Executable is not reachable by the agent.
                    // Upload it and change its path to $DDS_LOCATION on the WN
                    boost::filesystem::path exePath(topoTask->getExe());
                    const string sExeFileNameWithArgs(exePath.filename().generic_string());
                    msg_cmd.m_sExeFile += "$DDS_LOCATION/";
                    msg_cmd.m_sExeFile += sExeFileNameWithArgs;

                    // Expand the string for the program to extract exe name and command line arguments
                    wordexp_t result;
                    switch (wordexp(sExeFileNameWithArgs.c_str(), &result, 0))
                    {
                        case 0:
                        {
                            const string sExeFileName(result.we_wordv[0]);
                            boost::filesystem::path exePathWithoutArgs(exePath.parent_path());
                            exePathWithoutArgs /= sExeFileName;
                            wordfree(&result);

                            ptr->pushBinaryAttachmentCmd(exePathWithoutArgs.generic_string(), sExeFileName,
                                                         cmdASSIGN_USER_TASK);
                        }
                        break;
                        case WRDE_NOSPACE:
                            // If the error was WRDE_NOSPACE,
                            // then perhaps part of the result was allocated.
                            wordfree(&result);
                            throw runtime_error("memory error occurred while processing the user's executable path: " +
                                                topoTask->getExe());
                        default: // Some other error.
                            throw runtime_error("failed to process the user's executable path: " + topoTask->getExe());
                    }
                }
                ptr->pushMsg<cmdASSIGN_USER_TASK>(msg_cmd);
                ++it_tasks;
            }

            // Active agents.
            broadcastSimpleMsg<cmdACTIVATE_AGENT>(condition);
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

            ++cmd.m_nActiveAgents;
            ss << ptr->getId() << " " << ptr->getRemoteHostInfo().m_username << "@" << ptr->getRemoteHostInfo().m_host
               << ":" << ptr->getRemoteHostInfo().m_DDSPath << " (pid:" << ptr->getRemoteHostInfo().m_agentPid << ")\n";
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
