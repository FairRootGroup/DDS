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
                                       [this](const CProtocolMessage& _msg) -> bool
                                       {
        return this->getLogHandler(_msg);
    });
}

bool CConnectionManager::getLogHandler(const CProtocolMessage& _msg)
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
    return true;
}