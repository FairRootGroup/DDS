// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "SubmitChannel.h"

using namespace MiscCommon;
using namespace dds;
using namespace dds::submit_cmd;
using namespace dds::protocol_api;
using namespace std;

CSubmitChannel::CSubmitChannel(boost::asio::io_service& _service)
    : CClientChannelImpl<CSubmitChannel>(_service, EChannelType::UI)
    , m_RMS(SSubmitCmd::UNKNOWN)
{
    subscribeOnEvent(EChannelEvents::OnRemoteEndDissconnected,
                     [](CSubmitChannel* _channel)
                     {
                         LOG(MiscCommon::log_stderr) << "Server has closed the connection.";
                     });

    subscribeOnEvent(EChannelEvents::OnHandshakeOK,
                     [this](CSubmitChannel* _channel)
                     {
                         if (SSubmitCmd::UNKNOWN != m_RMS)
                         {
                             // Create the command's attachment
                             SSubmitCmd cmd;
                             cmd.m_nRMSTypeCode = m_RMS;
                             cmd.m_sSSHCfgFile = m_sSSHCfgFile;
                             pushMsg<cmdSUBMIT>(cmd);
                         }
                     });

    subscribeOnEvent(EChannelEvents::OnConnected,
                     [](CSubmitChannel* _channel)
                     {
                         LOG(MiscCommon::log_stdout) << "Connection established.";
                         LOG(MiscCommon::log_stdout) << "Requesting server to process job submission...";
                     });

    subscribeOnEvent(EChannelEvents::OnFailedToConnect,
                     [](CSubmitChannel* _channel)
                     {
                         LOG(MiscCommon::log_stdout) << "Failed to connect.";
                     });
}

void CSubmitChannel::setSSHCfgFile(const string& _val)
{
    m_sSSHCfgFile = _val;
}

void CSubmitChannel::setRMSTypeCode(const SSubmitCmd::ERmsType& _val)
{
    m_RMS = _val;
}

bool CSubmitChannel::on_cmdSIMPLE_MSG(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment)
{
    if (!_attachment->m_sMsg.empty())
        LOG((_attachment->m_msgSeverity == fatal || _attachment->m_msgSeverity == error) ? log_stderr : log_stdout)
            << "Server reports: " << _attachment->m_sMsg;

    // stop communication if a fatal error is recieved
    if (_attachment->m_msgSeverity == fatal)
        stop();
    return true;
}

bool CSubmitChannel::on_cmdSHUTDOWN(SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t /*_attachment*/)
{
    // Close communication channel
    stop();
    return true;
}
