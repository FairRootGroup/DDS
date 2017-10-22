// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "CommanderChannel.h"
#include "UserDefaults.h"

using namespace MiscCommon;
using namespace dds;
using namespace dds::protocol_api;
using namespace dds::agent_cmd;
using namespace std;
using namespace user_defaults_api;

const uint16_t g_MaxConnectionAttempts = 5;

CCommanderChannel::CCommanderChannel(boost::asio::io_service& _service)
    : CClientChannelImpl<CCommanderChannel>(_service, EChannelType::AGENT)
    , m_connectionAttempts(1)
{
    // Create shared memory channel for message forwarding from the network channel
    const CUserDefaults& userDefaults = CUserDefaults::instance();
    m_SMFWChannel = CSMFWChannel::makeNew(userDefaults.getSMAgentOutputName(), userDefaults.getSMAgentInputName());

    m_SMFWChannel->registerHandler<cmdRAW_MSG>(
        [this](const protocol_api::SSenderInfo& _sender, CProtocolMessage::protocolMessagePtr_t _currentMsg) {
            LOG(debug) << "Raw message pushed to network channel: " << _currentMsg->toString();
            this->pushMsg(_currentMsg, static_cast<ECmdType>(_currentMsg->header().m_cmd));
        });

    m_SMFWChannel->start();

    registerHandler<EChannelEvents::OnRemoteEndDissconnected>([this]() {
        if (m_connectionAttempts <= g_MaxConnectionAttempts)
        {
            LOG(info) << "Commander server has dropped the connection. Trying to reconnect. Attempt "
                      << m_connectionAttempts << " out of " << g_MaxConnectionAttempts;
            this_thread::sleep_for(chrono::seconds(5));
            reconnect();
            ++m_connectionAttempts;
        }
        else
        {
            LOG(info) << "Commander server has disconnected. Sending yourself a shutdown command.";
            this->sendYourself<cmdSHUTDOWN>();
        }
    });

    registerHandler<EChannelEvents::OnFailedToConnect>([this]() {
        if (m_connectionAttempts <= g_MaxConnectionAttempts)
        {
            LOG(info) << "Failed to connect to commander server. Trying to reconnect. Attempt " << m_connectionAttempts
                      << " out of " << g_MaxConnectionAttempts;
            this_thread::sleep_for(chrono::seconds(5));
            reconnect();
            ++m_connectionAttempts;
        }
        else
        {
            LOG(info) << "Failed to connect to commander server. Sending yourself a shutdown command.";
            this->sendYourself<cmdSHUTDOWN>();
        }
    });
}

bool CCommanderChannel::on_rawMessage(CProtocolMessage::protocolMessagePtr_t _currentMsg)
{
    LOG(debug) << "Raw message pushed to shared memory channel: " << _currentMsg->toString();
    m_SMFWChannel->pushMsg(_currentMsg, static_cast<ECmdType>(_currentMsg->header().m_cmd));
    return true;
}
