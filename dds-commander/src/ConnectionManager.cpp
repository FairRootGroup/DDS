// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "ConnectionManager.h"
#include "Topology.h"
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
    _newClient->registerMessageHandler(
        cmdGET_LOG,
        [this](CProtocolMessage::protocolMessagePtr_t _msg, CAgentChannel* _channel) -> bool
        {
            return this->on_cmdGET_LOG(_msg, useRawPtr(_channel));
        });

    _newClient->registerMessageHandler(
        cmdBINARY_ATTACHMENT_LOG,
        [this](CProtocolMessage::protocolMessagePtr_t _msg, CAgentChannel* _channel) -> bool
        {
            return this->on_cmdBINARY_ATTACHMENT_LOG(_msg, useRawPtr(_channel));
        });

    _newClient->registerMessageHandler(
        cmdGET_AGENTS_INFO,
        [this](CProtocolMessage::protocolMessagePtr_t _msg, CAgentChannel* _channel) -> bool
        {
            return this->agentsInfoHandler(_msg, useRawPtr(_channel));
        });

    _newClient->registerMessageHandler(
        cmdSUBMIT,
        [this](CProtocolMessage::protocolMessagePtr_t _msg, CAgentChannel* _channel) -> bool
        {
            return this->on_cmdSUBMIT(_msg, useRawPtr(_channel));
        });

    _newClient->registerMessageHandler(
        cmdSUBMIT_START,
        [this](CProtocolMessage::protocolMessagePtr_t _msg, CAgentChannel* _channel) -> bool
        {
            return this->on_cmdSUBMIT_START(_msg, useRawPtr(_channel));
        });

    _newClient->registerMessageHandler(
        cmdSTART_DOWNLOAD_TEST,
        [this](CProtocolMessage::protocolMessagePtr_t _msg, CAgentChannel* _channel) -> bool
        {
            return this->on_cmdSTART_DOWNLOAD_TEST(_msg, useRawPtr(_channel));
        });

    _newClient->registerMessageHandler(
        cmdDOWNLOAD_TEST_STAT,
        [this](CProtocolMessage::protocolMessagePtr_t _msg, CAgentChannel* _channel) -> bool
        {
            return this->on_cmdDOWNLOAD_TEST_STAT(_msg, useRawPtr(_channel));
        });

    _newClient->registerMessageHandler(
        cmdSIMPLE_MSG,
        [this](CProtocolMessage::protocolMessagePtr_t _msg, CAgentChannel* _channel) -> bool
        {
            return this->on_cmdSIMPLE_MSG(_msg, useRawPtr(_channel));
        });
}

bool CConnectionManager::on_cmdGET_LOG(CProtocolMessage::protocolMessagePtr_t _msg,
                                       CAgentChannel::weakConnectionPtr_t _channel)
{
    std::lock_guard<std::mutex> lock(m_getLog.m_mutexStart);
    try
    {
        if (!m_getLog.m_channel.expired())
        {
            SSimpleMsgCmd cmd;
            // cmd.m_msgSeverity = MiscCommon::fatal;
            // cmd.m_srcCommand = cmdGET_LOG;
            cmd.m_sMsg = "Can not process the request. The getlog command is already in progress.";
            CProtocolMessage::protocolMessagePtr_t msg = make_shared<CProtocolMessage>();
            msg->encodeWithAttachment<cmdSIMPLE_MSG>(cmd);
            auto p = _channel.lock();
            p->pushMsg(msg);
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
            SSimpleMsgCmd cmd;
            // cmd.m_msgSeverity = MiscCommon::fatal;
            // cmd.m_srcCommand = cmdGET_LOG;
            cmd.m_sMsg = "Could not create directory " + sLogStorageDir + " to save log files.";
            CProtocolMessage::protocolMessagePtr_t pm = make_shared<CProtocolMessage>();
            pm->encodeWithAttachment<cmdSIMPLE_MSG>(cmd);
            p->pushMsg(pm);

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
            SSimpleMsgCmd cmd;
            // cmd.m_msgSeverity = MiscCommon::fatal;
            // cmd.m_srcCommand = cmdGET_LOG;
            cmd.m_sMsg = "There are no connected agents.";
            CProtocolMessage::protocolMessagePtr_t pm = make_shared<CProtocolMessage>();
            pm->encodeWithAttachment<cmdSIMPLE_MSG>(cmd);
            p->pushMsg(pm);

            return true;
        }

        broadcastMsg<cmdGET_LOG>(condition);
    }
    catch (bad_weak_ptr& e)
    {
        // TODO: Do we need to log something here?
    }
    return true;
}

bool CConnectionManager::on_cmdBINARY_ATTACHMENT_LOG(CProtocolMessage::protocolMessagePtr_t _msg,
                                                     CAgentChannel::weakConnectionPtr_t _channel)
{
    SBinaryAttachmentCmd recieved_cmd;
    recieved_cmd.convertFromData(_msg->bodyToContainer());
    m_getLog.processMessage<SBinaryAttachmentCmd>(recieved_cmd, _channel);

    return true;
}

bool CConnectionManager::on_cmdSUBMIT(CProtocolMessage::protocolMessagePtr_t _msg,
                                      CAgentChannel::weakConnectionPtr_t _channel)
{
    try
    {
        SSubmitCmd cmd;
        cmd.convertFromData(_msg->bodyToContainer());

        auto p = _channel.lock();

        if (cmd.m_nRMSTypeCode == SSubmitCmd::SSH)
        {
            LOG(info) << "SSH RMS is defined by: [" << cmd.m_sSSHCfgFile << "]";

            // TODO: Job submission should be moved from here to a thread
            // Resolve topology
            CTopology topology;
            m_sCurrentTopoFile = cmd.m_sTopoFile;
            topology.init(m_sCurrentTopoFile);
            // TODO: Compare number of job slots in the ssh (in case of ssh) config file to what topo wants from us.

            // Submitting the job
            string outPut;
            string sCommand("$DDS_LOCATION/bin/dds-ssh");
            smart_path(&sCommand);
            StringVector_t params;
            const size_t nCmdTimeout = 60; // in sec.
            params.push_back("-c" + cmd.m_sSSHCfgFile);
            params.push_back("submit");
            int nDdsSSHExitCode(0);
            try
            {
                do_execv(sCommand, params, nCmdTimeout, &outPut, nullptr, &nDdsSSHExitCode);
            }
            catch (exception& e)
            {
                if (!outPut.empty())
                {
                    ostringstream ss;
                    ss << outPut;
                    LOG(info) << ss.str();
                    p->push_SimpleMsgFromString(ss.str());
                }

                string sMsg("Failed to deploy agents from the given setup: ");
                sMsg += cmd.m_sSSHCfgFile;
                throw runtime_error(sMsg);
            }
            if (!outPut.empty())
            {
                ostringstream ss;
                ss << outPut;
                LOG(info) << ss.str();
                p->push_SimpleMsgFromString(ss.str());
            }

            SSimpleMsgCmd msg_cmd;
            msg_cmd.m_sMsg = (0 == nDdsSSHExitCode) ? "Agents were successfully deployed."
                                                    : "Looks like we had problems to deploy agents using DDS ssh "
                                                      "plug-in. Check dds.log for more information.";
            CProtocolMessage::protocolMessagePtr_t msg = make_shared<CProtocolMessage>();
            msg->encodeWithAttachment<cmdREPLY_SUBMIT_OK>(msg_cmd);
            p->pushMsg(msg);
        }
    }
    catch (bad_weak_ptr& e)
    {
        // TODO: Do we need to log something here?
    }
    catch (exception& e)
    {
        SSimpleMsgCmd msg_cmd;
        msg_cmd.m_sMsg = e.what();
        CProtocolMessage::protocolMessagePtr_t msg = make_shared<CProtocolMessage>();
        msg->encodeWithAttachment<cmdREPLY_ERR_SUBMIT>(msg_cmd);
        if (!_channel.expired())
        {
            auto p = _channel.lock();
            p->pushMsg(msg);
        }
    }

    return true;
}

bool CConnectionManager::on_cmdSUBMIT_START(CProtocolMessage::protocolMessagePtr_t _msg,
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
            size_t index(0);
            TopoElementPtrVector_t tasks(topology.getMainGroup()->getElementsByType(ETopoType::TASK));
            for (const auto& v : channels)
            {
                if (v.expired())
                    continue;
                auto ptr = v.lock();

                // Assign user's tasks to agents
                SAssignUserTaskCmd msg_cmd;
                TaskPtr_t topoTask = dynamic_pointer_cast<CTask>(tasks[index++]);
                msg_cmd.m_sExeFile = topoTask->getExec();
                CProtocolMessage::protocolMessagePtr_t msg = make_shared<CProtocolMessage>();
                msg->encodeWithAttachment<cmdASSIGN_USER_TASK>(msg_cmd);
                ptr->pushMsg(msg);
            }

            // Active agents.
            broadcastMsg<cmdACTIVATE_AGENT>(condition);
        }
        catch (bad_weak_ptr& _e)
        {
            // TODO: Do we need to log something here?
        }
    }
    catch (exception& _e)
    {
        SSimpleMsgCmd cmd;
        cmd.m_msgSeverity = MiscCommon::fatal;
        cmd.m_srcCommand = cmdSUBMIT_START;
        cmd.m_sMsg = _e.what();
        CProtocolMessage::protocolMessagePtr_t msg = make_shared<CProtocolMessage>();
        msg->encodeWithAttachment<cmdSIMPLE_MSG>(cmd);
        auto p = _channel.lock();
        p->pushMsg(msg);

        m_ActivateAgents.m_channel.reset();
        return true;
    }
    return true;
}

bool CConnectionManager::agentsInfoHandler(CProtocolMessage::protocolMessagePtr_t _msg,
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

        CProtocolMessage::protocolMessagePtr_t msg = make_shared<CProtocolMessage>();
        msg->encodeWithAttachment<cmdREPLY_AGENTS_INFO>(cmd);
        if (!_channel.expired())
        {
            auto p = _channel.lock();
            p->pushMsg(msg);
        }
    }
    catch (bad_weak_ptr& e)
    {
        // TODO: Do we need to log something here?
    }

    return true;
}

bool CConnectionManager::on_cmdSTART_DOWNLOAD_TEST(CProtocolMessage::protocolMessagePtr_t _msg,
                                                   CAgentChannel::weakConnectionPtr_t _channel)
{
    try
    {
        std::lock_guard<std::mutex> lock(m_downloadTest.m_mutexStart);

        if (!m_downloadTest.m_channel.expired())
        {
            SSimpleMsgCmd cmd;
            cmd.m_msgSeverity = MiscCommon::fatal;
            cmd.m_srcCommand = cmdSTART_DOWNLOAD_TEST;
            cmd.m_sMsg = "Can not process the request. The test command is already in progress.";
            CProtocolMessage::protocolMessagePtr_t msg = make_shared<CProtocolMessage>();
            msg->encodeWithAttachment<cmdSIMPLE_MSG>(cmd);
            auto p = _channel.lock();
            p->pushMsg(msg);
            return true;
        }
        m_downloadTest.m_channel = _channel;
        m_downloadTest.zeroCounters();

        auto condition = [](CAgentChannel::connectionPtr_t _v)
        {
            return (_v->getType() == EAgentChannelType::AGENT && _v->started());
        };

        vector<size_t> binarySizes{ 1000, 10000, 1000, 100000, 1000, 1000000, 1000, 10000000, 1000 };

        m_downloadTest.m_nofRequests = binarySizes.size() * countNofChannels(condition);

        for (size_t size : binarySizes)
        {
            CProtocolMessage::protocolMessagePtr_t msg = getTestBinaryAttachment(size);
            broadcastMsg(msg, condition);
        }

        if (m_downloadTest.m_nofRequests == 0)
        {
            SSimpleMsgCmd cmd;
            cmd.m_msgSeverity = MiscCommon::fatal;
            cmd.m_srcCommand = cmdSTART_DOWNLOAD_TEST;
            cmd.m_sMsg = "There are no active agents.";
            CProtocolMessage::protocolMessagePtr_t pm = make_shared<CProtocolMessage>();
            pm->encodeWithAttachment<cmdSIMPLE_MSG>(cmd);
            if (!m_downloadTest.m_channel.expired())
            {
                auto p = m_downloadTest.m_channel.lock();
                p->pushMsg(pm);
            }
        }
    }
    catch (bad_weak_ptr& e)
    {
        // TODO: Do we need to log something here?
    }

    return true;
}

CProtocolMessage::protocolMessagePtr_t CConnectionManager::getTestBinaryAttachment(size_t _binarySize)
{
    SBinaryAttachmentCmd cmd;

    for (size_t i = 0; i < _binarySize; ++i)
    {
        // char c = rand() % 256;
        char c = 1;
        cmd.m_fileData.push_back(c);
    }

    // Calculate CRC32 of the test file data
    boost::crc_32_type crc;
    crc.process_bytes(&cmd.m_fileData[0], cmd.m_fileData.size());

    cmd.m_crc32 = crc.checksum();
    cmd.m_fileName = "test_data_" + std::to_string(_binarySize) + ".bin";
    cmd.m_fileSize = cmd.m_fileData.size();

    CProtocolMessage::protocolMessagePtr_t msg = make_shared<CProtocolMessage>();
    msg->encodeWithAttachment<cmdDOWNLOAD_TEST>(cmd);

    return msg;
}

bool CConnectionManager::on_cmdDOWNLOAD_TEST_STAT(CProtocolMessage::protocolMessagePtr_t _msg,
                                                  CAgentChannel::weakConnectionPtr_t _channel)
{
    SBinaryDownloadStatCmd received_cmd;
    received_cmd.convertFromData(_msg->bodyToContainer());

    m_downloadTest.m_totalReceived += received_cmd.m_recievedFileSize;
    m_downloadTest.m_totalTime += received_cmd.m_downloadTime;

    m_downloadTest.processMessage<SBinaryDownloadStatCmd>(received_cmd, _channel);

    return true;
}

bool CConnectionManager::on_cmdSIMPLE_MSG(CProtocolMessage::protocolMessagePtr_t _msg,
                                          CAgentChannel::weakConnectionPtr_t _channel)
{
    SSimpleMsgCmd cmd;
    cmd.convertFromData(_msg->bodyToContainer());

    switch (cmd.m_srcCommand)
    {
        case cmdACTIVATE_AGENT:
            switch (cmd.m_msgSeverity)
            {
                case info:
                    m_ActivateAgents.processMessage<SSimpleMsgCmd>(cmd, _channel);
                    break;
                case error:
                case fatal:
                    m_ActivateAgents.processErrorMessage<SSimpleMsgCmd>(cmd, _channel);
                    break;
            }
            return true; // let others to process this message

        case cmdGET_LOG:
            return m_getLog.processErrorMessage<SSimpleMsgCmd>(cmd, _channel);

        case cmdSTART_DOWNLOAD_TEST:
            return m_downloadTest.processErrorMessage<SSimpleMsgCmd>(cmd, _channel);
    }
    return false;
}
