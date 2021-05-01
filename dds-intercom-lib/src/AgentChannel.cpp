// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "AgentChannel.h"

using namespace MiscCommon;
using namespace dds;
using namespace dds::internal_api;
using namespace dds::protocol_api;
using namespace std;

const uint16_t g_MaxConnectionAttempts = 12;

CAgentChannel::CAgentChannel(boost::asio::io_context& _service, uint64_t _protocolHeaderID)
    : CClientChannelImpl<CAgentChannel>(_service, EChannelType::UNKNOWN, _protocolHeaderID)
    , m_connectionAttempts(1)
{
    registerHandler<EChannelEvents::OnRemoteEndDissconnected>([this](const SSenderInfo& /*_sender*/) {
        LOG(info) << "DDS commander server has suddenly dropped the connection. Sending yourself a shutdown signal...";
        this->sendYourself<cmdSHUTDOWN>();
    });

    registerHandler<protocol_api::EChannelEvents::OnConnected>(
        [](const SSenderInfo& /*_sender*/) { LOG(MiscCommon::info) << "Connected to the commander server"; });

    registerHandler<protocol_api::EChannelEvents::OnFailedToConnect>([](const SSenderInfo& /*_sender*/) {
        LOG(MiscCommon::log_stderr) << "Failed to connect to commander server.";
    });
}

void CAgentChannel::reconnectAgentWithErrorHandler(const function<void(const string&)>& callback)
{
    if (m_connectionAttempts <= g_MaxConnectionAttempts)
    {
        LOG(log_stderr) << "Failed to connect to commander server. Trying to reconnect. Attempt "
                        << m_connectionAttempts << " out of " << g_MaxConnectionAttempts;
        this_thread::sleep_for(chrono::seconds(10));
        reconnect();
        ++m_connectionAttempts;
    }
    else
    {
        string errorMsg("Failed to connect to commander server. Sending yourself a shutdown command.");
        LOG(log_stderr) << errorMsg;
        this->sendYourself<cmdSHUTDOWN>();

        callback(errorMsg);
    }
}
