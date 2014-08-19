// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#include "ConnectionManager.h"

using namespace dds;
using namespace std;

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
    _newClient->registerMessageHandler(cmdGET_AGENTS_INFO,
                                       [this](const CProtocolMessage& _msg, CAgentChannel* _channel) -> bool
                                       {
        return this->agentsInfoHandler(_msg, _channel);
    });

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
}

bool CConnectionManager::on_cmdGET_LOG(const CProtocolMessage& _msg, CAgentChannel* _channel)
{
    std::lock_guard<std::mutex> lock(m_getLog.m_mutex);

    if (m_getLog.m_channel != nullptr)
    {
        SSimpleMsgCmd cmd;
        cmd.m_sMsg = "Can not process the request. dds-getlog already in progress.";
        CProtocolMessage msg;
        msg.encodeWithAttachment<cmdGET_LOG_ERROR>(cmd);
        m_getLog.m_channel->pushMsg(msg);
        return true;
    }
    m_getLog.m_channel = _channel;
    m_getLog.zeroCounters();
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
    return true;
}

bool CConnectionManager::on_cmdBINARY_ATTACHMENT_LOG(const CProtocolMessage& _msg, CAgentChannel* _channel)
{
    SBinaryAttachmentCmd recieved_cmd;
    recieved_cmd.convertFromData(_msg.bodyToContainer());

    // Form reply command
    SSimpleMsgCmd cmd;
    cmd.m_sMsg = to_string(m_getLog.nofRecieved()) + "/" + to_string(m_getLog.m_nofRequests) + " Log from agent [" +
                 to_string(_channel->getId()) + "] saved to file " + recieved_cmd.m_fileName;

    CProtocolMessage msg;
    msg.encodeWithAttachment<cmdLOG_RECIEVED>(cmd);
    m_getLog.m_channel->pushMsg(msg);

    m_getLog.m_nofRecieved++;
    checkAllRecieved();

    return true;
}

bool CConnectionManager::on_cmdGET_LOG_ERROR(const CProtocolMessage& _msg, CAgentChannel* _channel)
{
    SSimpleMsgCmd recieved_cmd;
    recieved_cmd.convertFromData(_msg.bodyToContainer());

    string str("Error from agent [" + to_string(_channel->getId()) + "]: ");
    str += recieved_cmd.m_sMsg;

    SSimpleMsgCmd cmd;
    cmd.m_sMsg = str;
    CProtocolMessage msg;
    msg.encodeWithAttachment<cmdGET_LOG_ERROR>(cmd);
    m_getLog.m_channel->pushMsg(msg);

    m_getLog.m_nofRecievedErrors++;
    checkAllRecieved();

    return true;
}

bool CConnectionManager::agentsInfoHandler(const CProtocolMessage& _msg, CAgentChannel* _channel)
{
    SAgentsInfoCmd cmd;
    for (const auto& v : m_channels)
    {
        if (v->getType() == EAgentChannelType::AGENT && v->started())
            ++cmd.m_nActiveAgents;
    }
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
