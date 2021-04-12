// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "ConnectionManager.h"
#include "ChannelId.h"
#include "CommandAttachmentImpl.h"
#include "Intercom.h"
#include "MiscCli.h"
#include "TopoCore.h"
#include "ncf.h"
// BOOST
#include <boost/filesystem.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/regex.hpp>
// STD
#include <mutex>

using namespace dds;
using namespace dds::commander_cmd;
using namespace dds::topology_api;
using namespace dds::user_defaults_api;
using namespace dds::protocol_api;
using namespace dds::ncf;
using namespace dds::intercom_api;
using namespace dds::tools_api;
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

    // Check RMS plug-in activity
    CMonitoringThread::instance().registerCallbackFunction(
        [this, self]() -> bool {
            try
            {
                m_SubmitAgents.checkPluginFailedToStart();
            }
            catch (exception& _e)
            {
                LOG(error) << "RMS plug-in monitor: error: " << _e.what();
            }

            return true;
        },
        chrono::seconds(15));
}

void CConnectionManager::_stop()
{
}

void CConnectionManager::newClientCreated(CAgentChannel::connectionPtr_t _newClient)
{
    CAgentChannel::weakConnectionPtr_t weakClient(_newClient);

    _newClient->registerHandler<EChannelEvents::OnHandshakeOK>([this, weakClient](const SSenderInfo& _sender) {
        if (auto p = weakClient.lock())
            if (p->getChannelType() == EChannelType::UI)
            {
                LOG(info) << "Updating UI channel ID to " << p->getId();
                CConnectionManagerImpl::weakChannelInfo_t inf(weakClient, p->getId(), false);
                updateChannelProtocolHeaderID(inf);
            }
    });

    // Subscribe on protocol messages
    _newClient->registerHandler<cmdBINARY_ATTACHMENT_RECEIVED>(
        [this, weakClient](const SSenderInfo& _sender,
                           SCommandAttachmentImpl<cmdBINARY_ATTACHMENT_RECEIVED>::ptr_t _attachment) {
            this->on_cmdBINARY_ATTACHMENT_RECEIVED(_sender, _attachment, weakClient);
        });

    _newClient->registerHandler<cmdTRANSPORT_TEST>(
        [this, weakClient](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdTRANSPORT_TEST>::ptr_t _attachment) {
            this->on_cmdTRANSPORT_TEST(_sender, _attachment, weakClient);
        });

    _newClient->registerHandler<cmdREPLY>(
        [this, weakClient](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdREPLY>::ptr_t _attachment) {
            this->on_cmdREPLY(_sender, _attachment, weakClient);
        });

    _newClient->registerHandler<cmdUPDATE_KEY>(
        [this, weakClient](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment) {
            this->on_cmdUPDATE_KEY(_sender, _attachment, weakClient);
        });

    _newClient->registerHandler<cmdUSER_TASK_DONE>(
        [this, weakClient](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdUSER_TASK_DONE>::ptr_t _attachment) {
            this->on_cmdUSER_TASK_DONE(_sender, _attachment, weakClient);
        });

    _newClient->registerHandler<cmdGET_PROP_LIST>(
        [this, weakClient](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdGET_PROP_LIST>::ptr_t _attachment) {
            this->on_cmdGET_PROP_LIST(_sender, _attachment, weakClient);
        });

    _newClient->registerHandler<cmdGET_PROP_VALUES>(
        [this, weakClient](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdGET_PROP_VALUES>::ptr_t _attachment) {
            this->on_cmdGET_PROP_VALUES(_sender, _attachment, weakClient);
        });

    _newClient->registerHandler<cmdREPLY_ID>(
        [this, weakClient](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdREPLY_ID>::ptr_t _attachment) {
            this->on_cmdREPLY_ID(_sender, _attachment, weakClient);
        });

    _newClient->registerHandler<cmdCUSTOM_CMD>(
        [this, weakClient](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdCUSTOM_CMD>::ptr_t _attachment) {
            this->on_cmdCUSTOM_CMD(_sender, _attachment, weakClient);
        });
}

//=============================================================================
void CConnectionManager::_createWnPkg(bool _needInlineBashScript, bool _lightweightPkg, uint32_t _nSlots) const
{
    // re-create the worker package if needed
    string out;
    string err;
    try
    {
        // set submit time
        chrono::system_clock::time_point now = chrono::system_clock::now();
        stringstream ssSubmitTime;
        ssSubmitTime << " -s " << chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()).count();

        // invoking a new bash process can in some case overwrite env. vars
        // To be sure that our env is there, we call DDS_env.sh
        string cmd_env("$DDS_LOCATION/DDS_env.sh");
        smart_path(&cmd_env);

        string cmd("$DDS_LOCATION/bin/dds-prep-worker");
        smart_path(&cmd);
        cmd += ssSubmitTime.str();
        if (_needInlineBashScript)
            cmd += " -i ";
        // Session ID
        cmd += " -a ";
        cmd += CUserDefaults::instance().getCurrentSID();
        // Slots per agent
        cmd += " -t ";
        cmd += to_string(_nSlots);

        if (_lightweightPkg)
            cmd += " -l ";

        string arg("source ");
        arg += cmd_env;
        arg += " ; ";
        arg += cmd;

        stringstream ssCmd;
        ssCmd << "/bin/bash -c \"" << arg << "\"";

        LOG(debug) << "Preparing WN package: " << ssCmd.str();

        // 10 sec time-out for this command
        execute(ssCmd.str(), chrono::seconds(10), &out, &err);
        if (!err.empty())
            throw runtime_error("failed");
    }
    catch (exception& e)
    {
        stringstream ssErr;
        ssErr << "WN Package Tool: " << e.what() << "; STDOUT:" << out << "; STDERR: " << err;
        throw runtime_error(ssErr.str());
    }
    LOG(info) << "WN Package Tool: STDOUT:" << out << "; STDERR: " << err;
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
          << "port=" << _ports[0] << "\n"
          << endl;
    }

    if (_ports.size() > 1)
    {
        f << "[ui]\n"
          << "host=" << srvHost << "\n"
          << "user=" << srvUser << "\n"
          << "port=" << _ports[1] << "\n"
          << endl;
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

void CConnectionManager::on_cmdBINARY_ATTACHMENT_RECEIVED(
    const SSenderInfo& _sender,
    SCommandAttachmentImpl<cmdBINARY_ATTACHMENT_RECEIVED>::ptr_t _attachment,
    CAgentChannel::weakConnectionPtr_t _channel)
{
    switch (_attachment->m_srcCommand)
    {
        case cmdGET_LOG:
        {
            m_getLog.processMessage<SBinaryAttachmentReceivedCmd>(_sender, *_attachment, _channel);
            return;
        }

        case cmdTRANSPORT_TEST:
        {
            m_transportTest.m_totalReceived += _attachment->m_receivedFileSize;
            m_transportTest.m_totalTime += _attachment->m_downloadTime;
            m_transportTest.processMessage<SBinaryAttachmentReceivedCmd>(_sender, *_attachment, _channel);

            return;
        }
    }
}

template <protocol_api::ECmdType _cmd>
void CConnectionManager::broadcastUpdateTopologyAndWait_impl(size_t index, weakChannelInfo_t _agent)
{
    auto p = _agent.m_channel.lock();
    p->pushMsg<_cmd>(_agent.m_protocolHeaderID);
}

template <protocol_api::ECmdType _cmd>
void CConnectionManager::broadcastUpdateTopologyAndWait_impl(size_t index,
                                                             weakChannelInfo_t _agent,
                                                             typename SCommandAttachmentImpl<_cmd>::ptr_t _attachment)
{
    auto p = _agent.m_channel.lock();
    p->pushMsg<_cmd>(*_attachment, _agent.m_protocolHeaderID);
}

template <protocol_api::ECmdType _cmd>
void CConnectionManager::broadcastUpdateTopologyAndWait_impl(
    size_t index, weakChannelInfo_t _agent, const vector<typename SCommandAttachmentImpl<_cmd>::ptr_t>& _attachments)
{
    auto p = _agent.m_channel.lock();
    p->pushMsg<_cmd>(*_attachments[index], _agent.m_protocolHeaderID);
}

template <protocol_api::ECmdType _cmd>
void CConnectionManager::broadcastUpdateTopologyAndWait_impl(size_t index,
                                                             weakChannelInfo_t _agent,
                                                             const std::string& _filePath,
                                                             const std::string& _filename)
{
    auto p = _agent.m_channel.lock();
    p->pushBinaryAttachmentCmd(_filePath, _filename, _cmd, _agent.m_protocolHeaderID);
}

template <protocol_api::ECmdType _cmd>
void CConnectionManager::broadcastUpdateTopologyAndWait_impl(size_t index,
                                                             weakChannelInfo_t _agent,
                                                             const std::vector<std::string>& _filePaths,
                                                             const std::vector<std::string>& _filenames)
{
    auto p = _agent.m_channel.lock();
    string filePath = _filePaths[index];
    string filename = _filenames[index];
    p->pushBinaryAttachmentCmd(filePath, filename, _cmd, _agent.m_protocolHeaderID);
}

template <protocol_api::ECmdType _cmd, class... Args>
void CConnectionManager::broadcastUpdateTopologyAndWait(weakChannelInfo_t::container_t _agents,
                                                        CAgentChannel::weakConnectionPtr_t _channel,
                                                        const std::string& _msg,
                                                        Args&&... args)
{
    if (_agents.size() == 0)
        return;

    m_updateTopology.m_srcCommand = _cmd;
    m_updateTopology.zeroCounters();
    m_updateTopology.m_nofRequests = _agents.size();

    // Message to the UI
    sendToolsAPIMsg(_channel, m_updateTopology.m_requestID, _msg, EMsgSeverity::info);

    // Initiate the progress on the UI
    dds::tools_api::SProgressResponseData progress(_cmd, 0, m_updateTopology.m_nofRequests, 0);
    sendCustomCommandResponse(_channel, progress.toJSON());

    // Broadcast message or binary to agents
    size_t index = 0;
    for (auto& agent : _agents)
    {
        broadcastUpdateTopologyAndWait_impl<_cmd>(index, agent, args...);
        index++;
    }

    // Wait until all replies are received
    m_updateTopoCondition.reset();
    m_updateTopoCondition.wait();
}

void CConnectionManager::activateTasks(const CScheduler& _scheduler, CAgentChannel::weakConnectionPtr_t _channel)
{
    const CScheduler::ScheduleVector_t& schedule = _scheduler.getSchedule();

    // Data of user task upload to agents
    weakChannelInfo_t::container_t uploadAgents;
    vector<string> uploadFilePaths;
    vector<string> uploadFilenames;

    // Data of user task assignment
    weakChannelInfo_t::container_t assignmentAgents;
    vector<typename SCommandAttachmentImpl<cmdASSIGN_USER_TASK>::ptr_t> assignmentAttachments;
    vector<typename SCommandAttachmentImpl<cmdACTIVATE_USER_TASK>::ptr_t> activateAttachments;

    // Collecting data for broadcasting
    for (const auto& sch : schedule)
    {
        typename SCommandAttachmentImpl<cmdASSIGN_USER_TASK>::ptr_t cmd = make_shared<SAssignUserTaskCmd>();
        cmd->m_taskID = sch.m_taskID;
        cmd->m_taskIndex = sch.m_taskInfo.m_taskIndex;
        cmd->m_collectionIndex = sch.m_taskInfo.m_collectionIndex;
        cmd->m_taskPath = sch.m_taskInfo.m_taskPath;
        cmd->m_groupName = sch.m_taskInfo.m_task->getParentGroupId();
        cmd->m_collectionName = sch.m_taskInfo.m_task->getParentCollectionId();
        cmd->m_taskName = sch.m_taskInfo.m_task->getName();
        cmd->m_topoHash = m_topo.getHash();

        if (sch.m_taskInfo.m_task->isExeReachable())
        {
            cmd->m_sExeFile = sch.m_taskInfo.m_task->getExe();
        }
        else
        {
            string filePath;
            string filename;
            string cmdStr;
            parseExe(sch.m_taskInfo.m_task->getExe(), "%DDS_DEFAULT_TASK_PATH%", filePath, filename, cmdStr);

            cmd->m_sExeFile = cmdStr;

            // Upload file only if it's not reachable
            uploadFilePaths.push_back(filePath);
            uploadFilenames.push_back(filename);
            uploadAgents.push_back(sch.m_weakChannelInfo);
        }

        // attache the environment script if needed
        if (!sch.m_taskInfo.m_task->getEnv().empty())
        {
            if (sch.m_taskInfo.m_task->isEnvReachable())
            {
                cmd->m_sEnvFile = sch.m_taskInfo.m_task->getEnv();
            }
            else
            {
                string filePath;
                string filename;
                string cmdStr;
                parseExe(sch.m_taskInfo.m_task->getEnv(), "%DDS_DEFAULT_TASK_PATH%", filePath, filename, cmdStr);

                cmd->m_sEnvFile = cmdStr;

                // Upload file only if it's not reachable
                uploadFilePaths.push_back(filePath);
                uploadFilenames.push_back(filename);
                uploadAgents.push_back(sch.m_weakChannelInfo);
            }
        }

        typename SCommandAttachmentImpl<cmdACTIVATE_USER_TASK>::ptr_t activate_cmd = make_shared<SIDCmd>();
        activate_cmd->m_id = sch.m_weakChannelInfo.m_protocolHeaderID;

        assignmentAgents.push_back(sch.m_weakChannelInfo);
        assignmentAttachments.push_back(cmd);
        activateAttachments.push_back(activate_cmd);
    }

    if (uploadAgents.size() > 0)
    {
        broadcastUpdateTopologyAndWait<cmdASSIGN_USER_TASK>(
            uploadAgents, _channel, "Uploading user tasks...", uploadFilePaths, uploadFilenames);
    }

    broadcastUpdateTopologyAndWait<cmdASSIGN_USER_TASK>(
        assignmentAgents, _channel, "Assigning user tasks...", assignmentAttachments);

    // Set executing state and task ID for agent channels
    for (const auto& sch : schedule)
    {
        if (sch.m_weakChannelInfo.m_channel.expired())
            continue;
        auto ptr = sch.m_weakChannelInfo.m_channel.lock();

        SAgentInfo& inf = ptr->getAgentInfo();
        SSlotInfo& slot = inf.getSlotByID(sch.m_weakChannelInfo.m_protocolHeaderID);

        slot.m_taskID = sch.m_taskID;
        slot.m_state = EAgentState::executing;
    }

    broadcastUpdateTopologyAndWait<cmdACTIVATE_USER_TASK>(
        assignmentAgents, _channel, "Activating user tasks...", activateAttachments);
}

void CConnectionManager::on_cmdTRANSPORT_TEST(const SSenderInfo& _sender,
                                              SCommandAttachmentImpl<cmdTRANSPORT_TEST>::ptr_t _attachment,
                                              CAgentChannel::weakConnectionPtr_t _channel)
{
    lock_guard<mutex> lock(m_transportTest.m_mutexStart);

    if (!m_transportTest.m_channel.expired())
    {
        auto p = _channel.lock();
        p->pushMsg<cmdSIMPLE_MSG>(
            SSimpleMsgCmd(
                "Can not process the request. The test command is already in progress.", fatal, cmdTRANSPORT_TEST),
            _sender.m_ID);
        return;
    }
    m_transportTest.m_channel = _channel;
    m_transportTest.m_shutdownOnComplete = true;
    m_transportTest.zeroCounters();

    auto condition = [](const CConnectionManager::channelInfo_t& _v, bool& /*_stop*/) {
        return (_v.m_channel != nullptr && _v.m_channel->getChannelType() == EChannelType::AGENT &&
                _v.m_channel->started());
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
        p->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd("There are no active agents.", fatal, cmdTRANSPORT_TEST), _sender.m_ID);
    }
}

void CConnectionManager::on_cmdREPLY(const SSenderInfo& _sender,
                                     SCommandAttachmentImpl<cmdREPLY>::ptr_t _attachment,
                                     CAgentChannel::weakConnectionPtr_t _channel)
{
    switch (_attachment->m_srcCommand)
    {
        case cmdASSIGN_USER_TASK:
        {
            if (SReplyCmd::EStatusCode(_attachment->m_statusCode) == SReplyCmd::EStatusCode::OK)
            {
                m_updateTopology.processMessage<SReplyCmd>(_sender, *_attachment, _channel);
            }
            else if (SReplyCmd::EStatusCode(_attachment->m_statusCode) == SReplyCmd::EStatusCode::ERROR)
            {
                m_updateTopology.processErrorMessage<SReplyCmd>(_sender, *_attachment, _channel);
            }
            if (m_updateTopology.allReceived())
            {
                m_updateTopoCondition.notifyAll();
            }
            return;
        }

        case cmdACTIVATE_USER_TASK:
        {
            if (SReplyCmd::EStatusCode(_attachment->m_statusCode) == SReplyCmd::EStatusCode::OK)
            {
                m_updateTopology.processMessage<SReplyCmd>(_sender, *_attachment, _channel);
            }
            else if (SReplyCmd::EStatusCode(_attachment->m_statusCode) == SReplyCmd::EStatusCode::ERROR)
            {
                // In case of error set the idle state
                if (auto p = _channel.lock())
                {
                    SAgentInfo& inf = p->getAgentInfo();
                    SSlotInfo& slot = inf.getSlotByID(_sender.m_ID);

                    slot.m_taskID = 0;
                    slot.m_state = EAgentState::idle;
                }
                m_updateTopology.processErrorMessage<SReplyCmd>(_sender, *_attachment, _channel);
            }
            if (m_updateTopology.allReceived())
            {
                m_updateTopoCondition.notifyAll();
            }
            return;
        }

        case cmdSTOP_USER_TASK:
        {
            if (SReplyCmd::EStatusCode(_attachment->m_statusCode) == SReplyCmd::EStatusCode::OK)
            {
                m_updateTopology.processMessage<SReplyCmd>(_sender, *_attachment, _channel);
                {
                    // Task was successfully stopped, set the idle state
                    if (auto p = _channel.lock())
                    {
                        SAgentInfo& inf = p->getAgentInfo();
                        SSlotInfo& slot = inf.getSlotByID(_sender.m_ID);

                        slot.m_taskID = 0;
                        slot.m_state = EAgentState::idle;
                    }
                }
            }
            else if (SReplyCmd::EStatusCode(_attachment->m_statusCode) == SReplyCmd::EStatusCode::ERROR)
            {
                m_updateTopology.processErrorMessage<SReplyCmd>(_sender, *_attachment, _channel);
            }
            if (m_updateTopology.allReceived())
            {
                m_updateTopoCondition.notifyAll();
            }
            return;
        }

        case cmdUPDATE_TOPOLOGY:
        {
            if (SReplyCmd::EStatusCode(_attachment->m_statusCode) == SReplyCmd::EStatusCode::OK)
            {
                m_updateTopology.processMessage<SReplyCmd>(_sender, *_attachment, _channel);
            }
            else if (SReplyCmd::EStatusCode(_attachment->m_statusCode) == SReplyCmd::EStatusCode::ERROR)
            {
                m_updateTopology.processErrorMessage<SReplyCmd>(_sender, *_attachment, _channel);
            }
            if (m_updateTopology.allReceived())
            {
                m_updateTopoCondition.notifyAll();
            }
            return;
        }

        case cmdGET_LOG:
        {
            if (SReplyCmd::EStatusCode(_attachment->m_statusCode) == SReplyCmd::EStatusCode::ERROR)
            {
                m_getLog.processErrorMessage<SReplyCmd>(_sender, *_attachment, _channel);
            }
            return;
        }

        case cmdTRANSPORT_TEST:
        {
            if (SReplyCmd::EStatusCode(_attachment->m_statusCode) == SReplyCmd::EStatusCode::ERROR)
            {
                m_transportTest.processErrorMessage<SReplyCmd>(_sender, *_attachment, _channel);
            }
            return;
        }
    }
}

void CConnectionManager::on_cmdUPDATE_KEY(const SSenderInfo& _sender,
                                          SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment,
                                          CAgentChannel::weakConnectionPtr_t _channel)
{
    // Commander forwards cmdUPDATE_KEY to the proper channel
    auto channel = m_taskIDToAgentChannelMap.find(_attachment->m_receiverTaskID);
    if (channel != m_taskIDToAgentChannelMap.end())
    {
        CAgentChannel::weakConnectionPtr_t weakPtr = channel->second.m_channel;
        if (!weakPtr.expired())
        {
            auto ptr = weakPtr.lock();
            ptr->accumulativePushMsg<cmdUPDATE_KEY>(*_attachment, channel->second.m_protocolHeaderID);
        }
    }
    else
    {
        LOG(debug) << "on_cmdUPDATE_KEY task <" << _attachment->m_receiverTaskID
                   << "> not found in map. Property will not be updated.";
    }
}

void CConnectionManager::on_cmdUSER_TASK_DONE(const SSenderInfo& _sender,
                                              SCommandAttachmentImpl<cmdUSER_TASK_DONE>::ptr_t _attachment,
                                              CAgentChannel::weakConnectionPtr_t _channel)
{
    // Send a broad cast message to all agents
    auto condition = [](const CConnectionManager::channelInfo_t& _v, bool& /*_stop*/) {
        return (_v.m_channel->getChannelType() == EChannelType::AGENT && !_v.m_isSlot && _v.m_channel->started());
    };
    broadcastMsg<cmdUSER_TASK_DONE>(*_attachment, condition);

    if (!_channel.expired())
    {
        auto channelPtr = _channel.lock();
        SAgentInfo& thisInf = channelPtr->getAgentInfo();
        SSlotInfo& thisSlotInf = thisInf.getSlotByID(_sender.m_ID);
        thisSlotInf.m_state = EAgentState::idle;
        thisSlotInf.m_taskID = 0;
    }

    // remove task ID from the map
    // TODO: Temporary solution. We do not remove tasks from the map to avoid synchronization bottlenecks in
    // on_cmdUPDATE_KEY.
    //{
    //    lock_guard<mutex> lock(m_mapMutex);
    //    auto it = m_taskIDToAgentChannelMap.find(taskID);
    //    if (it != m_taskIDToAgentChannelMap.end())
    //        m_taskIDToAgentChannelMap.erase(it);
    //}

    try
    {
        auto task = m_topo.getRuntimeTaskById(_attachment->m_taskID).m_task;
        LOG(info) << "User task <" << _attachment->m_taskID << "> with path " << task->getPath() << " done";
    }
    catch (exception& _e)
    {
        LOG(info) << "User task <" << _attachment->m_taskID << "> done. Additional info: " << _e.what();
    }
}

void CConnectionManager::on_cmdGET_PROP_LIST(const SSenderInfo& _sender,
                                             SCommandAttachmentImpl<cmdGET_PROP_LIST>::ptr_t _attachment,
                                             CAgentChannel::weakConnectionPtr_t _channel)
{
    // TODO: FIXME: This command desn't work without CKeyValueManager
}

void CConnectionManager::on_cmdGET_PROP_VALUES(const SSenderInfo& _sender,
                                               SCommandAttachmentImpl<cmdGET_PROP_VALUES>::ptr_t _attachment,
                                               CAgentChannel::weakConnectionPtr_t _channel)
{
    // TODO: FIXME: This command desn't work without CKeyValueManager
}

void CConnectionManager::on_cmdREPLY_ID(const SSenderInfo& _sender,
                                        SCommandAttachmentImpl<cmdREPLY_ID>::ptr_t _attachment,
                                        CAgentChannel::weakConnectionPtr_t _channel)
{
    auto p = _channel.lock();
    try
    {
        LOG(debug) << "cmdREPLY_ID attachment [" << *_attachment << "] received from: " << p->remoteEndIDString();

        if (_attachment->m_id == 0)
        {
            uint64_t id = DDSChannelId::getChannelId();
            SIDCmd msg_cmd;
            msg_cmd.m_id = id;

            p->setId(id);

            p->pushMsg<cmdSET_ID>(msg_cmd, _sender.m_ID);
        }
        else
        {
            p->setId(_attachment->m_id);
        }

        CConnectionManagerImpl::weakChannelInfo_t inf(_channel, p->getId(), false);
        updateChannelProtocolHeaderID(inf);
    }
    catch (exception& _e)
    {
        p->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(_e.what(), fatal, cmdREPLY_ID), _sender.m_ID);
    }
}

void CConnectionManager::on_cmdCUSTOM_CMD(const SSenderInfo& _sender,
                                          SCommandAttachmentImpl<cmdCUSTOM_CMD>::ptr_t _attachment,
                                          CAgentChannel::weakConnectionPtr_t _channel)
{
    auto p = _channel.lock();
    try
    {
        // Assign sender ID of this custom command
        _attachment->m_senderId = (p->getChannelType() == EChannelType::UI) ? p->getId() : _sender.m_ID;

        CConnectionManager::weakChannelInfo_t::container_t channels;

        // First check id condition is a positive integer - channel ID - which means that custom command has to be
        // sent to a particular channel.
        try
        {
            // If condition in the attachment is not of type uint64_t function throws an exception.
            uint64_t channelId = boost::lexical_cast<uint64_t>(_attachment->m_sCondition);
            auto channel = getChannelByID(channelId);
            if (auto p = channel.m_channel.lock())
            {
                channels.push_back(channel);
            }
            else
            {
                LOG(error) << "Failed to deliver. Channel is missing. CUSTOM_CMD senderID: " << _sender.m_ID
                           << "; attachemnt: " << *_attachment;
            }
        }
        catch (boost::bad_lexical_cast&)
        {
            // Condition is not a positiove integer.

            // check if this is an agent submit request
            if (_attachment->m_sCondition == g_sRmsAgentSign)
            {
                LOG(info) << "Received a message from RMS plug-in.";
                lock_guard<mutex> lock(m_SubmitAgents.m_mutexStart);
                if (m_SubmitAgents.m_channel.expired())
                    throw runtime_error("Internal error. Submit info channel is not initialized.");

                // Remember the submit plug-in channel, which is responsible for job submissions
                // Send initial request
                if (m_SubmitAgents.m_channelSubmitPlugin.expired())
                    m_SubmitAgents.m_channelSubmitPlugin = _channel;

                // Process messages from the plug-in
                m_SubmitAgents.processCustomCommandMessage(*_attachment, _channel);
                return;
            }
            // Tools API request
            else if (_attachment->m_sCondition == g_sToolsAPISign)
            {
                LOG(info) << "Received a message from TOOLS API.";
                processToolsAPIRequests(*_attachment, _channel);
                return;
            }

            std::shared_ptr<boost::regex> pathRegex;

            // Check if we can find task for it's full id path.
            bool taskFound = true;
            try
            {
                /*const STopoRuntimeTask& runtimeTask = */
                m_topo.getRuntimeTaskByIdPath(_attachment->m_sCondition);
            }
            catch (runtime_error& _e)
            {
                taskFound = false;
                // Create regex only for broadcast
                pathRegex = make_shared<boost::regex>(_attachment->m_sCondition);
            }

            uint64_t thisTaskID(0);
            try
            {
                SAgentInfo& thisInf = p->getAgentInfo();
                SSlotInfo& thisSlotInf = thisInf.getSlotByID(_sender.m_ID);
                thisTaskID = thisSlotInf.m_taskID;
            }
            catch (exception& _e)
            {
            }

            channels = getChannels([this, taskFound, &_attachment, &thisTaskID, pathRegex](
                                       const CConnectionManager::channelInfo_t& _v, bool& _stop) {
                if (!_v.m_isSlot)
                    return false;

                SAgentInfo& inf = _v.m_channel->getAgentInfo();
                SSlotInfo& slotInf = inf.getSlotByID(_v.m_protocolHeaderID);

                // Only for Agents which are started already and executing task
                if (_v.m_channel->getChannelType() != EChannelType::AGENT || !_v.m_channel->started() ||
                    slotInf.m_state != EAgentState::executing)
                    return false;

                // Do not send command to self
                if (thisTaskID > 0 && slotInf.m_taskID == thisTaskID)
                    return false;

                // If condition is empty we broadcast command to all agents
                if (_attachment->m_sCondition.empty())
                    return true;

                const STopoRuntimeTask& taskInfo = m_topo.getRuntimeTaskById(slotInf.m_taskID);
                bool result = (taskFound) ? taskInfo.m_taskPath == _attachment->m_sCondition
                                          : boost::regex_match(taskInfo.m_task->getPath(), *pathRegex);

                _stop = (taskFound && result);

                return result;
            });
        }

        for (const auto& v : channels)
        {
            if (v.m_channel.expired())
                continue;
            auto ptr = v.m_channel.lock();

            ptr->pushMsg<cmdCUSTOM_CMD>(*_attachment, v.m_protocolHeaderID);
        }

        // Debug msg: there are too many of such messages if tasks intensivly use CC
        /*  stringstream ss;
          ss << "Send custom command to " << channels.size() << " channels." << endl;

          p->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(ss.str(), MiscCommon::info, cmdCUSTOM_CMD), _sender.m_ID);*/
    }
    catch (exception& _e)
    {
        LOG(error) << "on_cmdCUSTOM_CMD: " << _e.what();
        p->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(_e.what(), fatal, cmdCUSTOM_CMD), _sender.m_ID);
    }
}

void CConnectionManager::processToolsAPIRequests(const SCustomCmdCmd& _cmd, CAgentChannel::weakConnectionPtr_t _channel)
{
    LOG(MiscCommon::info) << "Processing Tools API message: " << _cmd.m_sCmd;
    boost::property_tree::ptree pt;

    try
    {
        istringstream ss(_cmd.m_sCmd);
        boost::property_tree::read_json(ss, pt);

        const boost::property_tree::ptree& childPT = pt.get_child("dds.tools-api");

        for (const auto& child : childPT)
        {
            const string& tag = child.first;
            if (tag == "submit")
            {
                dds::tools_api::SSubmitRequestData submitInfo;
                submitInfo.fromPT(child.second);

                submitAgents(submitInfo, _channel);
            }
            else if (tag == "topology")
            {
                dds::tools_api::STopologyRequestData topoInfo;
                topoInfo.fromPT(child.second);

                updateTopology(topoInfo, _channel);
            }
            else if (tag == "message")
            {
            }
            else if (tag == "getlog")
            {
                dds::tools_api::SGetLogRequestData getlog;
                getlog.fromPT(child.second);

                getLog(getlog, _channel);
            }
            else if (tag == "commanderInfo")
            {
                dds::tools_api::SCommanderInfoRequestData commanderInfo;
                commanderInfo.fromPT(child.second);
                LOG(info) << "ToolsAPI: processing commanderInfo with requestID = " << commanderInfo.m_requestID;
                sendUICommanderInfo(commanderInfo, _channel);
            }
            else if (tag == "agentInfo")
            {
                dds::tools_api::SAgentInfoRequestData agentInfo;
                agentInfo.fromPT(child.second);

                sendUIAgentInfo(agentInfo, _channel);
            }
            else if (tag == "agentCount")
            {
                dds::tools_api::SAgentCountRequestData agentCount;
                agentCount.fromPT(child.second);

                sendUIAgentCount(agentCount, _channel);
            }
        }
    }
    catch (const boost::property_tree::ptree_error& _e)
    {
        string msg("Failed to process Tools API message: ");
        msg += _e.what();
        LOG(MiscCommon::error) << msg;
    }
    catch (const exception& _e)
    {
        LOG(MiscCommon::error) << "CConnectionManager::processToolsAPIRequests: ";
    }
}

void CConnectionManager::submitAgents(const dds::tools_api::SSubmitRequestData& _submitInfo,
                                      CAgentChannel::weakConnectionPtr_t _channel)
{
    try
    {
        string pluginDir = CUserDefaults::instance().getPluginDir(_submitInfo.m_pluginPath, _submitInfo.m_rms);
        stringstream ssPluginExe;
        ssPluginExe << pluginDir << "dds-submit-" << _submitInfo.m_rms;
        if (!boost::filesystem::exists(ssPluginExe.str()))
        {
            stringstream ssErrMsg;
            ssErrMsg << "Unknown RMS plug-in requested \"" << _submitInfo.m_rms << "\" (" << ssPluginExe.str() << ")";
            throw runtime_error(ssErrMsg.str());
        }

        // Create a new submit communication info channel
        lock_guard<mutex> lock(m_SubmitAgents.m_mutexStart);
        if (!m_SubmitAgents.m_channel.expired())
            throw runtime_error("Can not process the request. Submit is already in progress.");

        // Create / re-pack WN package
        // Include inline script if present
        string inlineShellScripCmds;
        if (!_submitInfo.m_config.empty())
        {
            ifstream f(_submitInfo.m_config);
            if (!f.is_open())
            {
                string msg("can't open configuration file \"");
                msg += _submitInfo.m_config;
                msg += "\"";
                throw runtime_error(msg);
            }
            CNcf config;
            config.readFrom(f, true); // Read only bash commands if any
            inlineShellScripCmds = config.getBashEnvCmds();
            LOG(info)
                << "Agent submitter config contains an inline shell script. It will be injected it into wrk. package";

            string scriptFileName(CUserDefaults::instance().getWrkPkgDir());
            scriptFileName += "user_worker_env.sh";
            smart_path(&scriptFileName);
            ofstream f_script(scriptFileName.c_str());
            if (!f_script.is_open())
                throw runtime_error("Can't open for writing: " + scriptFileName);

            f_script << inlineShellScripCmds;
            f_script.close();
        }
        // pack worker package
        sendToolsAPIMsg(_channel, _submitInfo.m_requestID, "Creating new worker package...", EMsgSeverity::info);

        // Use a lightweightpackage when possible
        _createWnPkg(!inlineShellScripCmds.empty(), (_submitInfo.m_rms == "localhost"), _submitInfo.m_slots);

        // remember the UI channel, which requested to submit the job
        m_SubmitAgents.m_channel = _channel;
        m_SubmitAgents.m_requestID = _submitInfo.m_requestID;
        m_SubmitAgents.zeroCounters();

        SSubmit submitRequest;
        submitRequest.m_cfgFilePath = _submitInfo.m_config;
        submitRequest.m_nInstances = _submitInfo.m_instances;
        submitRequest.m_slots = _submitInfo.m_slots;
        submitRequest.m_wrkPackagePath = CUserDefaults::instance().getWrkScriptPath();
        m_SubmitAgents.m_strInitialSubmitRequest = submitRequest.toJSON();

        string sPluginInfoMsg("RMS plug-in: ");
        sPluginInfoMsg += ssPluginExe.str();
        sendToolsAPIMsg(_channel, _submitInfo.m_requestID, sPluginInfoMsg, EMsgSeverity::info);

        // Submitting the job
        stringstream ssCmd;
        ssCmd << ssPluginExe.str();
        // TODO: Send ID to the plug-in
        ssCmd << " --session " << CUserDefaults::instance().getCurrentSID() << " --id "
              << "FAKE_ID_FOR_TESTS"
              << " --path \"" << pluginDir << "\"";

        sendToolsAPIMsg(_channel, _submitInfo.m_requestID, "Initializing RMS plug-in...", EMsgSeverity::info);

        LOG(info) << "Calling RMS plug-in: " << ssCmd.str();

        // Let the submit info channel now, that plug-in is about to start
        m_SubmitAgents.initPlugin();

        try
        {
            // Execute RMS plug-in and don't wait for it.
            // Omnce plug-in is up it will connect back to the commander.
            // We will report to user if it won't connect.
            execute(ssCmd.str());
        }
        catch (exception& e)
        {
            stringstream ssMsg;
            ssMsg << "Failed to deploy agents: " << e.what();
            throw runtime_error(ssMsg.str());
        }
    }
    catch (bad_weak_ptr& e)
    {
        // TODO: Do we need to log something here?

        // In case of any plugin initialization error, reset the info channel
        m_SubmitAgents.m_channel.reset();
    }
    catch (exception& e)
    {
        sendToolsAPIMsg(_channel, _submitInfo.m_requestID, e.what(), EMsgSeverity::error);
        sendDoneResponse(_channel, _submitInfo.m_requestID);

        // In case of any plugin initialization error, reset the info channel
        m_SubmitAgents.m_channel.reset();
    }
}

void CConnectionManager::updateTopology(const dds::tools_api::STopologyRequestData& _topologyInfo,
                                        CAgentChannel::weakConnectionPtr_t _channel)
{
    LOG(info) << "UI channel requested to update/activate/stop the topology.";

    // Only a single topology update/activate/stop can be active at a time
    lock_guard<mutex> lock(m_updateTopology.m_mutexStart);
    m_updateTopology.m_requestID = _topologyInfo.m_requestID;
    m_updateTopology.m_channel = _channel;

    // Resolve environment variables
    std::string topologyFile = _topologyInfo.m_topologyFile;
    MiscCommon::smart_path(&topologyFile);

    try
    {
        STopologyRequest::request_t::EUpdateType updateType = _topologyInfo.m_updateType;

        string msg;
        switch (updateType)
        {
            case STopologyRequest::request_t::EUpdateType::UPDATE:
                msg = "Updating topology to " + topologyFile;
                break;
            case STopologyRequest::request_t::EUpdateType::ACTIVATE:
                msg = "Activating topology " + topologyFile;
                break;
            case STopologyRequest::request_t::EUpdateType::STOP:
                msg = "Stopping topology " + topologyFile;
                break;
            default:
                break;
        }

        sendToolsAPIMsg(_channel, _topologyInfo.m_requestID, msg, EMsgSeverity::info);

        //
        // Check if topology is currently active, i.e. there are executing tasks
        //
        CConnectionManager::weakChannelInfo_t::container_t channels(
            getChannels([](const CConnectionManager::channelInfo_t& _v, bool& _stop) {
                if (!_v.m_isSlot)
                    return false;

                SAgentInfo& info = _v.m_channel->getAgentInfo();
                SSlotInfo& slot = info.getSlotByID(_v.m_protocolHeaderID);
                _stop = (_v.m_channel->getChannelType() == EChannelType::AGENT && _v.m_channel->started() &&
                         slot.m_taskID > 0 && slot.m_state == EAgentState::executing);
                return _stop;
            }));
        bool topologyActive = !channels.empty();
        // Current topology is not active we reset
        if (!topologyActive)
        {
            m_topo = CTopoCore();
        }
        //

        // If topology is active we can't activate it again
        if (updateType == STopologyRequest::request_t::EUpdateType::ACTIVATE && topologyActive)
        {
            throw runtime_error("Topology is currently active, can't activate it again.");
        }

        //
        // Get new topology and calculate the difference
        //
        CTopoCore topo;
        // If topo file is empty than we stop the topology
        if (!topologyFile.empty())
        {
            topo.setXMLValidationDisabled(_topologyInfo.m_disableValidation);
            topo.init(topologyFile);
        }

        topology_api::CTopoCore::IdSet_t removedTasks;
        topology_api::CTopoCore::IdSet_t removedCollections;
        topology_api::CTopoCore::IdSet_t addedTasks;
        topology_api::CTopoCore::IdSet_t addedCollections;
        m_topo.getDifference(topo, removedTasks, removedCollections, addedTasks, addedCollections);

        stringstream ss;
        ss << "\nRemoved tasks: " << removedTasks.size() << "\n"
           << m_topo.stringOfTasks(removedTasks) << "Removed collections: " << removedCollections.size() << "\n"
           << m_topo.stringOfCollections(removedCollections) << "Added tasks: " << addedTasks.size() << "\n"
           << topo.stringOfTasks(addedTasks) << "Added collections: " << addedCollections.size() << "\n"
           << topo.stringOfCollections(addedCollections);

        sendToolsAPIMsg(_channel, _topologyInfo.m_requestID, ss.str(), EMsgSeverity::info);

        //
        // Stop removed tasks
        //
        if (removedTasks.size() > 0)
        {
            weakChannelInfo_t::container_t agents;
            for (auto taskID : removedTasks)
            {
                auto agentChannel = m_taskIDToAgentChannelMap[taskID];
                agents.push_back(agentChannel);
            }

            // TODO: FIXME: Needs to be reviewed for the current architecture
            //            for (const auto& v : agents)
            //            {
            //                if (v.m_channel.expired())
            //                    continue;
            //                auto ptr = v.m_channel.lock();
            //                // TODO: FIXME: Do we need to deque messages in new decentralized concept?
            //                // dequeue important (or expensive) messages
            //                ptr->dequeueMsg<cmdUPDATE_KEY>();
            //            }

            broadcastUpdateTopologyAndWait<cmdSTOP_USER_TASK>(agents, _channel, "Stopping removed tasks...");
        }
        //

        // Assign new topology for DDS commander
        m_topo = topo;
        //

        //
        // Update topology on the agents
        //
        if (!topologyFile.empty())
        {
            auto allCondition = [](const CConnectionManager::channelInfo_t& _v, bool& /*_stop*/) {
                return (!_v.m_isSlot && _v.m_channel->getChannelType() == EChannelType::AGENT &&
                        _v.m_channel->started());
            };
            CConnectionManager::weakChannelInfo_t::container_t allAgents(getChannels(allCondition));

            if (allAgents.size() == 0)
                throw runtime_error("There are no active agents.");

            broadcastUpdateTopologyAndWait<cmdUPDATE_TOPOLOGY>(
                allAgents, _channel, "Updating topology for agents...", topologyFile, "topology.xml");
        }

        //
        // Activate added tasks
        //
        if (addedTasks.size() > 0)
        {
            auto idleCondition = [](const CConnectionManager::channelInfo_t& _v, bool& /*_stop*/) {
                return (_v.m_isSlot && _v.m_channel->getChannelType() == EChannelType::AGENT &&
                        _v.m_channel->started() &&
                        _v.m_channel->getAgentInfo().getSlotByID(_v.m_protocolHeaderID).m_state == EAgentState::idle);
            };

            CConnectionManager::weakChannelInfo_t::container_t idleAgents(getChannels(idleCondition));

            size_t nofAgents = idleAgents.size();
            if (nofAgents == 0)
                throw runtime_error("There are no idle agents.");
            if (nofAgents < addedTasks.size())
            {
                stringstream ssMsg;
                ssMsg << "The number of agents is not sufficient for this topology (required/available "
                      << addedTasks.size() << "/" << nofAgents << ").";
                throw runtime_error(ssMsg.str());
            }

            // Schedule the tasks
            CScheduler scheduler;
            scheduler.makeSchedule(m_topo, idleAgents, addedTasks, addedCollections);
            const CScheduler::ScheduleVector_t& schedule = scheduler.getSchedule();

            // Erase removed tasks
            for (auto taskID : removedTasks)
            {
                m_taskIDToAgentChannelMap.erase(taskID);
            }
            // Add new elements
            for (const auto& sch : schedule)
            {
                m_taskIDToAgentChannelMap[sch.m_taskID] = sch.m_weakChannelInfo;
            }

            activateTasks(scheduler, _channel);
        }

        // Send shutdown to UI channel at the end
        m_updateTopology.doneWithUI();
    }
    catch (exception& _e)
    {
        sendToolsAPIMsg(_channel, _topologyInfo.m_requestID, _e.what(), EMsgSeverity::error);
        sendDoneResponse(_channel, _topologyInfo.m_requestID);
        m_updateTopology.m_channel.reset();
        return;
    }
}

void CConnectionManager::sendToolsAPIMsg(CAgentChannel::weakConnectionPtr_t _channel,
                                         requestID_t _requestID,
                                         const string& _msg,
                                         EMsgSeverity _severity)
{
    SMessageResponseData msg;
    msg.m_requestID = _requestID;
    msg.m_msg = _msg;
    msg.m_severity = _severity;

    sendCustomCommandResponse(_channel, msg.toJSON());
}

void CConnectionManager::getLog(const dds::tools_api::SGetLogRequestData& _getLog,
                                CAgentChannel::weakConnectionPtr_t _channel)
{
    lock_guard<mutex> lock(m_getLog.m_mutexStart);

    if (!m_getLog.m_channel.expired())
    {
        sendToolsAPIMsg(_channel,
                        _getLog.m_requestID,
                        "Can not process the request. The getlog command is already in progress.",
                        EMsgSeverity::error);
        return;
    }

    m_getLog.m_channel = _channel;
    m_getLog.m_shutdownOnComplete = true;
    m_getLog.m_requestID = _getLog.m_requestID;
    m_getLog.zeroCounters();

    auto p = m_getLog.m_channel.lock();
    // Create directory to store logs
    const string sLogStorageDir(CUserDefaults::instance().getAgentLogStorageDir());
    fs::path dir(sLogStorageDir);
    if (!fs::exists(dir) && !fs::create_directories(dir))
    {
        stringstream ss;
        ss << "Could not create directory " << sLogStorageDir << " to save log files.";
        sendToolsAPIMsg(_channel, _getLog.m_requestID, ss.str(), EMsgSeverity::error);

        m_getLog.m_channel.reset();
        return;
    }

    auto condition = [](const CConnectionManager::channelInfo_t& _v, bool& /*_stop*/) {
        return (_v.m_channel->getChannelType() == EChannelType::AGENT && !_v.m_isSlot && _v.m_channel->started());
    };

    m_getLog.m_nofRequests = countNofChannels(condition);

    if (m_getLog.m_nofRequests == 0)
    {
        sendToolsAPIMsg(_channel, _getLog.m_requestID, "There are no connected agents.", EMsgSeverity::error);

        m_getLog.m_channel.reset();
        return;
    }

    broadcastSimpleMsg<cmdGET_LOG>(condition);
}

void CConnectionManager::sendUICommanderInfo(const dds::tools_api::SCommanderInfoRequestData& _info,
                                             CAgentChannel::weakConnectionPtr_t _channel)
{
    SCommanderInfoResponseData info;
    info.m_requestID = _info.m_requestID;

    info.m_pid = getpid();

    {
        lock_guard<mutex> lock(m_updateTopology.m_mutexStart);
        try
        {
            info.m_activeTopologyName = m_topo.getName();
            info.m_activeTopologyPath = m_topo.getFilepath();
        }
        catch (const std::runtime_error& e)
        {
            // No active topology, return empty name.
        }
    }

    sendCustomCommandResponse(_channel, info.toJSON());
    sendDoneResponse(_channel, _info.m_requestID);
}

void CConnectionManager::sendUIAgentInfo(const dds::tools_api::SAgentInfoRequestData& _info,
                                         CAgentChannel::weakConnectionPtr_t _channel)
{
    CConnectionManager::weakChannelInfo_t::container_t channels(
        getChannels([](const CConnectionManager::channelInfo_t& _v, bool& /*_stop*/) {
            return (_v.m_channel->getChannelType() == EChannelType::AGENT && !_v.m_isSlot && _v.m_channel->started());
        }));

    // Enumerate all agents
    size_t i = 0;
    for (const auto& v : channels)
    {
        SAgentInfoResponseData info;
        info.m_requestID = _info.m_requestID;

        if (v.m_channel.expired())
            continue;
        auto ptr = v.m_channel.lock();

        SAgentInfo& inf = ptr->getAgentInfo();

        string sTaskName("no task is assigned");
        //        if (inf.m_taskID > 0 && inf.m_state == EAgentState::executing)
        //        {
        //            stringstream ssTaskString;
        //            ssTaskString << inf.m_taskID << " (" << inf.m_id << ")";
        //            sTaskName = ssTaskString.str();
        //        }

        info.m_index = i++;
        info.m_agentID = inf.m_id;
        info.m_startUpTime = inf.m_startUpTime;
        info.m_username = inf.m_remoteHostInfo.m_username;
        info.m_host = inf.m_remoteHostInfo.m_host;
        info.m_DDSPath = inf.m_remoteHostInfo.m_DDSPath;
        info.m_agentPid = inf.m_remoteHostInfo.m_agentPid;
        info.m_nSlots = inf.getSlots().size();

        sendCustomCommandResponse(_channel, info.toJSON());
    }

    sendDoneResponse(_channel, _info.m_requestID);
}

void CConnectionManager::sendUIAgentCount(const dds::tools_api::SAgentCountRequestData& _info,
                                          CAgentChannel::weakConnectionPtr_t _channel)
{
    CConnectionManager::weakChannelInfo_t::container_t channels(
        getChannels([](const CConnectionManager::channelInfo_t& _v, bool& /*_stop*/) {
            if (!_v.m_isSlot)
                return false;

            return (_v.m_channel->getChannelType() == EChannelType::AGENT && _v.m_channel->started());
        }));

    // No active agents
    if (channels.empty())
    {
        SAgentCountResponseData info;
        info.m_requestID = _info.m_requestID;

        sendCustomCommandResponse(_channel, info.toJSON());
        sendDoneResponse(_channel, _info.m_requestID);

        return;
    }

    size_t activeCounter = channels.size();
    size_t idleCounter = countNofChannels([](const CConnectionManager::channelInfo_t& _v, bool& /*_stop*/) {
        if (!_v.m_isSlot)
            return false;

        const SSlotInfo& slot = _v.m_channel->getAgentInfo().getSlotByID(_v.m_protocolHeaderID);
        return (_v.m_channel->getChannelType() == EChannelType::AGENT && _v.m_channel->started() &&
                slot.m_taskID == 0 && slot.m_state == EAgentState::idle);
    });
    size_t executingCounter = countNofChannels([](const CConnectionManager::channelInfo_t& _v, bool& /*_stop*/) {
        if (!_v.m_isSlot)
            return false;

        const SSlotInfo& slot = _v.m_channel->getAgentInfo().getSlotByID(_v.m_protocolHeaderID);
        return (_v.m_channel->getChannelType() == EChannelType::AGENT && _v.m_channel->started() && slot.m_taskID > 0 &&
                slot.m_state == EAgentState::executing);
    });

    SAgentCountResponseData info;
    info.m_requestID = _info.m_requestID;
    info.m_activeSlotsCount = activeCounter;
    info.m_idleSlotsCount = idleCounter;
    info.m_executingSlotsCount = executingCounter;

    sendCustomCommandResponse(_channel, info.toJSON());
    sendDoneResponse(_channel, _info.m_requestID);
}

void CConnectionManager::sendCustomCommandResponse(CAgentChannel::weakConnectionPtr_t _channel,
                                                   const string& _json) const
{
    if (_channel.expired())
        return;

    auto p = _channel.lock();

    SCustomCmdCmd cmd;
    cmd.m_sCmd = _json;
    cmd.m_sCondition = "";
    p->template pushMsg<cmdCUSTOM_CMD>(cmd);
}

void CConnectionManager::sendDoneResponse(CAgentChannel::weakConnectionPtr_t _channel, requestID_t _requestID) const
{
    SDoneResponseData done;
    done.m_requestID = _requestID;
    sendCustomCommandResponse(_channel, done.toJSON());
}
