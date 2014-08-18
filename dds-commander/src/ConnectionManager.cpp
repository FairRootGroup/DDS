// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#include "ConnectionManager.h"

using namespace dds;

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
    _newClient->registerMessageHandler(cmdGET_LOG,
                                       [this](const CProtocolMessage& _msg, CAgentChannel* _channel)->bool
                                       { return this->getLogHandler(_msg, _channel); });
    _newClient->registerMessageHandler(cmdBINARY_ATTACHMENT_LOG,
                                       [this](const CProtocolMessage& _msg, CAgentChannel* _channel)->bool
                                       { return this->binaryAttachmentLogHandler(_msg, _channel); });
}

bool CConnectionManager::getLogHandler(const CProtocolMessage& _msg, CAgentChannel* _channel)
{
    // FIXME : temporary work around to get the working version.
    for (const auto& v : m_channels)
    {
        if (v->getType() == EAgentChannelType::AGENT)
        {
            CProtocolMessage msg;
            msg.encode<cmdGET_LOG>();
            v->pushMsg(msg);
        }
    }
    m_uiChannel = _channel;
    return true;
}

bool CConnectionManager::binaryAttachmentLogHandler(const CProtocolMessage& _msg, CAgentChannel* _channel)
{

    CProtocolMessage msg;
    msg.encode<cmdGET_LOG>();
    m_uiChannel->pushMsg(msg);

    return true;
}
