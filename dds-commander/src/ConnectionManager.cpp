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
    m_nofLogRequests = 0;
    m_nofRecievedLogs = 0;
    for (const auto& v : m_channels)
    {
        if (v->getType() == EAgentChannelType::AGENT && v->started())
        {
            CProtocolMessage msg;
            msg.encode<cmdGET_LOG>();
            v->pushMsg(msg);
            m_nofLogRequests++;
        }
    }
    m_uiChannel = _channel;
    return true;
}

bool CConnectionManager::binaryAttachmentLogHandler(const CProtocolMessage& _msg, CAgentChannel* _channel)
{
    // LOG(MiscCommon::debug) << "CConnectionManager::binaryAttachmentLogHandler";
    SBinaryAttachmentCmd recieved_cmd;
    recieved_cmd.convertFromData(_msg.bodyToContainer());

    //  LOG(MiscCommon::debug) << "CConnectionManager::binaryAttachmentLogHandler " << recieved_cmd;

    // Form reply command
    SBinaryDownloadStatCmd cmd;
    cmd.m_recievedCrc32 = 0; // crc32.checksum();
    cmd.m_recievedFileSize = recieved_cmd.m_fileData.size();
    cmd.m_downloadTime = 0;

    CProtocolMessage msg;
    msg.encodeWithAttachment<cmdBINARY_DOWNLOAD_STAT_LOG>(cmd);
    m_uiChannel->pushMsg(msg);

    //  LOG(MiscCommon::debug) << "CConnectionManager::binaryAttachmentLogHandler "
    //                         << " sending response " << cmd;

    m_nofRecievedLogs++;
    if (m_nofRecievedLogs == m_nofLogRequests)
    {
        m_uiChannel->pushMsg<cmdALL_LOGS_RECIEVED>();
    }

    return true;
}
