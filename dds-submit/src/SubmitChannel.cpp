// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "SubmitChannel.h"
// BOOST
#include <boost/filesystem/operations.hpp>

using namespace MiscCommon;
using namespace dds;
using namespace dds::submit_cmd;
using namespace dds::protocol_api;
using namespace std;

CSubmitChannel::CSubmitChannel(boost::asio::io_service& _service, uint64_t _protocolHeaderID)
    : CClientChannelImpl<CSubmitChannel>(_service, EChannelType::UI, _protocolHeaderID)
    , m_number(0)
    , m_bXMLValidationDisabled(false)
{
    registerHandler<EChannelEvents::OnRemoteEndDissconnected>(
        [](const SSenderInfo& _sender) { LOG(MiscCommon::log_stderr) << "Server has closed the connection."; });

    registerHandler<EChannelEvents::OnHandshakeOK>([this](const SSenderInfo& _sender) {
        // Create the command's attachment
        SSubmitCmd cmd;
        cmd.m_sRMSType = m_sRMS;
        cmd.m_sCfgFile = m_sCfgFile;
        cmd.m_sPath = m_sPath;
        cmd.m_nNumberOfAgents = m_number;
        pushMsg<cmdSUBMIT>(cmd);
    });

    registerHandler<EChannelEvents::OnConnected>([](const SSenderInfo& _sender) {
        LOG(MiscCommon::log_stdout) << "Connection established.";
        LOG(MiscCommon::log_stdout) << "Requesting server to process job submission...";
    });

    registerHandler<EChannelEvents::OnFailedToConnect>(
        [](const SSenderInfo& _sender) { LOG(MiscCommon::log_stderr) << "Failed to connect."; });
}

void CSubmitChannel::setCfgFile(const string& _val)
{
    m_sCfgFile = _val;
}

void CSubmitChannel::setRMSType(const string& _val)
{
    m_sRMS = _val;
}

void CSubmitChannel::setPath(const string& _val)
{
    m_sPath = _val;
}

void CSubmitChannel::setNumber(const size_t _val)
{
    m_number = _val;
}

bool CSubmitChannel::on_cmdSIMPLE_MSG(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment,
                                      const protocol_api::SSenderInfo& _sender)
{
    if (!_attachment->m_sMsg.empty())
        LOG((_attachment->m_msgSeverity == fatal || _attachment->m_msgSeverity == error) ? log_stderr : log_stdout)
            << "Server reports: " << _attachment->m_sMsg;

    // stop communication if a fatal error is recieved
    if (_attachment->m_msgSeverity == fatal)
        stop();
    return true;
}

bool CSubmitChannel::on_cmdSHUTDOWN(SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t /*_attachment*/,
                                    const protocol_api::SSenderInfo& _sender)
{
    // Close communication channel
    stop();
    return true;
}
