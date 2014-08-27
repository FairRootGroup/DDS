// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "ConnectionManager.h"
#include "Topology.h"
// BOOST
#include <boost/filesystem.hpp>

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
    _newClient->registerMessageHandler(cmdGET_LOG,
                                       [this](const CProtocolMessage& _msg, CAgentChannel* _channel) -> bool
                                       {
        return this->on_cmdGET_LOG(_msg, _channel);
    });

    _newClient->registerMessageHandler(cmdBINARY_ATTACHMENT_LOG,
                                       [this](const CProtocolMessage& _msg, CAgentChannel* _channel) -> bool
                                       {
        return this->on_cmdBINARY_ATTACHMENT_LOG(_msg, _channel);
    });

    _newClient->registerMessageHandler(cmdGET_LOG_ERROR,
                                       [this](const CProtocolMessage& _msg, CAgentChannel* _channel) -> bool
                                       {
        return this->on_cmdGET_LOG_ERROR(_msg, _channel);
    });

    _newClient->registerMessageHandler(cmdGET_AGENTS_INFO,
                                       [this](const CProtocolMessage& _msg, CAgentChannel* _channel) -> bool
                                       {
        return this->agentsInfoHandler(_msg, _channel);
    });

    _newClient->registerMessageHandler(cmdSUBMIT,
                                       [this](const CProtocolMessage& _msg, CAgentChannel* _channel) -> bool
                                       {
        return this->on_cmdSUBMIT(_msg, _channel);
    });

    _newClient->registerMessageHandler(cmdSUBMIT_START,
                                       [this](const CProtocolMessage& _msg, CAgentChannel* _channel) -> bool
                                       {
        return this->on_cmdSUBMIT_START(_msg, _channel);
    });
}

bool CConnectionManager::on_cmdGET_LOG(const CProtocolMessage& _msg, CAgentChannel* _channel)
{
    std::lock_guard<std::mutex> lock(m_getLog.m_mutexGetLog);

    if (m_getLog.m_channel != nullptr)
    {
        SSimpleMsgCmd cmd;
        cmd.m_sMsg = "Can not process the request. dds-getlog already in progress.";
        CProtocolMessage msg;
        msg.encodeWithAttachment<cmdGET_LOG_FATAL>(cmd);
        m_getLog.m_channel->pushMsg(msg);
        return true;
    }
    m_getLog.m_channel = _channel;
    m_getLog.zeroCounters();

    // Create directory to store logs
    const string sLogStorageDir(CUserDefaults::instance().getAgentLogStorageDir());
    fs::path dir(sLogStorageDir);
    if (!fs::exists(dir) && !fs::create_directory(dir))
    {
        SSimpleMsgCmd cmd;
        cmd.m_sMsg = "Could not create directory " + sLogStorageDir + " to save log files.";
        CProtocolMessage pm;
        pm.encodeWithAttachment<cmdGET_LOG_FATAL>(cmd);
        m_getLog.m_channel->pushMsg(pm);

        m_getLog.m_channel = nullptr;

        return true;
    }

    // Send messages to all agents
    for (const auto& v : m_channels)
    {
        if (v->getType() == EAgentChannelType::AGENT && v->started())
        {
            CProtocolMessage msg;
            msg.encode<cmdGET_LOG>();
            v->pushMsg(msg);
            m_getLog.m_nofRequests++;
        }
    }

    if (m_getLog.m_nofRequests == 0)
    {
        SSimpleMsgCmd cmd;
        cmd.m_sMsg = "There are no connecting agents.";
        CProtocolMessage pm;
        pm.encodeWithAttachment<cmdGET_LOG_FATAL>(cmd);
        m_getLog.m_channel->pushMsg(pm);
    }
    return true;
}

bool CConnectionManager::on_cmdBINARY_ATTACHMENT_LOG(const CProtocolMessage& _msg, CAgentChannel* _channel)
{
    SBinaryAttachmentCmd recieved_cmd;
    recieved_cmd.convertFromData(_msg.bodyToContainer());

    {
        std::lock_guard<std::mutex> lock(m_getLog.m_mutexReceive);

        m_getLog.m_nofRecieved++;
        string reply_msg(to_string(m_getLog.nofRecieved()) + "/" + to_string(m_getLog.m_nofRequests) +
                         " Log from agent [" + to_string(_channel->getId()) + "] saved to file " +
                         recieved_cmd.m_fileName);

        SSimpleMsgCmd cmd;
        cmd.m_sMsg = reply_msg;

        CProtocolMessage msg;
        msg.encodeWithAttachment<cmdLOG_RECIEVED>(cmd);
        m_getLog.m_channel->syncPushMsg(msg);

        checkAllRecieved();
    }

    return true;
}

bool CConnectionManager::on_cmdGET_LOG_ERROR(const CProtocolMessage& _msg, CAgentChannel* _channel)
{
    SSimpleMsgCmd recieved_cmd;
    recieved_cmd.convertFromData(_msg.bodyToContainer());

    {
        std::lock_guard<std::mutex> lock(m_getLog.m_mutexReceive);

        m_getLog.m_nofRecievedErrors++;
        string reply_msg(to_string(m_getLog.nofRecieved()) + "/" + to_string(m_getLog.m_nofRequests) +
                         " Error from agent [" + to_string(_channel->getId()) + "]: ");
        reply_msg += recieved_cmd.m_sMsg;

        SSimpleMsgCmd cmd;
        cmd.m_sMsg = reply_msg;
        CProtocolMessage msg;
        msg.encodeWithAttachment<cmdGET_LOG_ERROR>(cmd);
        m_getLog.m_channel->syncPushMsg(msg);

        checkAllRecieved();
    }

    return true;
}

bool CConnectionManager::on_cmdSUBMIT(const CProtocolMessage& _msg, CAgentChannel* _channel)
{
    try
    {
        SSubmitCmd cmd;
        cmd.convertFromData(_msg.bodyToContainer());

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
            const size_t nCmdTimeout = 35; // in sec.
            params.push_back("-c" + cmd.m_sSSHCfgFile);
            params.push_back("submit");
            try
            {
                do_execv(sCommand, params, nCmdTimeout, &outPut);

                SSimpleMsgCmd msg_cmd;
                msg_cmd.m_sMsg = "Dummy job info, JOBIds";
                CProtocolMessage msg;
                msg.encodeWithAttachment<cmdREPLY_SUBMIT_OK>(msg_cmd);
                _channel->pushMsg(msg);
            }
            catch (exception& e)
            {
                string sMsg("Failed to process the task: ");
                sMsg += cmd.m_sTopoFile;
                throw runtime_error(sMsg);
            }
            if (!outPut.empty())
            {
                ostringstream ss;
                ss << "Cmnd Output: " << outPut;
                LOG(info) << ss.str();
                SSimpleMsgCmd msg_cmd;
                msg_cmd.m_sMsg = ss.str();
                CProtocolMessage msg;
                msg.encodeWithAttachment<cmdSIMPLE_MSG>(msg_cmd);
                _channel->pushMsg(msg);
            }
        }
    }
    catch (exception& e)
    {
        SSimpleMsgCmd msg_cmd;
        msg_cmd.m_sMsg = e.what();
        CProtocolMessage msg;
        msg.encodeWithAttachment<cmdREPLY_ERR_SUBMIT>(msg_cmd);
        _channel->pushMsg(msg);
    }

    return true;
}

bool CConnectionManager::on_cmdSUBMIT_START(const CProtocolMessage& _msg, CAgentChannel* _channel)
{
    // Start distirbuiting user tasks between agents
    // TODO: We might need to create a thread here to avoid blocking a thread of the transport
    CTopology topology;
    topology.init(m_sCurrentTopoFile);

    // Send binaries of user jobs to all active agents.
    // Send activate signal to all agents. This will trigger start of user jobs on the agents.
    for (const auto& v : m_channels)
    {
        if (v->getType() == EAgentChannelType::AGENT && v->started())
        {
            // Assgin user's tasks to agents
            SAssignUserTaskCmd msg_cmd;
            msg_cmd.m_sExeFile = "Test.sh";
            CProtocolMessage msg;
            msg.encodeWithAttachment<cmdASSIGN_USER_TASK>(msg_cmd);
            v->pushMsg(msg);

            // Active agents.
            v->pushMsg<cmdACTIVATE_AGENT>();
        }
    }
    return true;
}

bool CConnectionManager::agentsInfoHandler(const CProtocolMessage& _msg, CAgentChannel* _channel)
{
    SAgentsInfoCmd cmd;
    stringstream ss;
    for (const auto& v : m_channels)
    {
        if (v->getType() == EAgentChannelType::AGENT && v->started())
        {
            ++cmd.m_nActiveAgents;
            ss << v->getId() << " " << v->getRemoteHostInfo().m_username << "@" << v->getRemoteHostInfo().m_host << ":"
               << v->getRemoteHostInfo().m_DDSPath << " (pid:" << v->getRemoteHostInfo().m_agentPid << ")\n";
        }
    }
    cmd.m_sListOfAgents = ss.str();

    CProtocolMessage msg;
    msg.encodeWithAttachment<cmdREPLY_AGENTS_INFO>(cmd);
    _channel->pushMsg(msg);

    return true;
}

void CConnectionManager::checkAllRecieved()
{
    if (m_getLog.allRecieved())
    {
        SSimpleMsgCmd cmd;
        cmd.m_sMsg = "recieved: " + to_string(m_getLog.nofRecieved()) + ", total: " +
                     to_string(m_getLog.m_nofRequests) + ", errors: " + to_string(m_getLog.m_nofRecievedErrors);
        CProtocolMessage msg;
        msg.encodeWithAttachment<cmdALL_LOGS_RECIEVED>(cmd);
        m_getLog.m_channel->pushMsg(msg);

        m_getLog.m_channel = nullptr;
    }
}
