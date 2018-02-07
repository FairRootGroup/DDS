// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "CommanderChannel.h"
#include "UserDefaults.h"
// BOOST
#include <boost/filesystem.hpp>

using namespace MiscCommon;
using namespace dds;
using namespace dds::protocol_api;
using namespace dds::agent_cmd;
using namespace std;
using namespace user_defaults_api;
namespace fs = boost::filesystem;

const uint16_t g_MaxConnectionAttempts = 5;

CCommanderChannel::CCommanderChannel(boost::asio::io_service& _service, uint64_t _ProtocolHeaderID)
    : CClientChannelImpl<CCommanderChannel>(_service, EChannelType::AGENT, _ProtocolHeaderID)
    , m_connectionAttempts(1)
{
    // Create shared memory channel for message forwarding from the network channel
    const CUserDefaults& userDefaults = CUserDefaults::instance();
    m_SMFWChannel = CSMFWChannel::makeNew(
        _service, userDefaults.getSMAgentOutputName(), userDefaults.getSMAgentInputName(), _ProtocolHeaderID);

    m_SMFWChannel->registerHandler<cmdRAW_MSG>(
        [this](const SSenderInfo& _sender, CProtocolMessage::protocolMessagePtr_t _currentMsg) {
            ECmdType cmd = static_cast<ECmdType>(_currentMsg->header().m_cmd);
            // cmdFILE_PATH is an exception. We have to forward it as a binary attachment.
            if (cmd == cmdMOVE_FILE)
            {
                LOG(debug) << "cmdMOVE_FILE pushed to network channel: " << _currentMsg->toString();
                SCommandAttachmentImpl<cmdMOVE_FILE>::ptr_t attachmentPtr =
                    SCommandAttachmentImpl<cmdMOVE_FILE>::decode(_currentMsg);
                this->pushBinaryAttachmentCmd(attachmentPtr->m_filePath,
                                              attachmentPtr->m_requestedFileName,
                                              attachmentPtr->m_srcCommand,
                                              _currentMsg->header().m_ID);

                // We take the ownership and we have to delete the file
                try
                {
                    fs::remove(attachmentPtr->m_filePath);
                }
                catch (exception& _e)
                {
                    LOG(error) << "Can't remove log archive file: " << attachmentPtr->m_filePath
                               << "; error: " << _e.what();
                }
            }
            else
            {
                LOG(debug) << "Raw message pushed to network channel: " << _currentMsg->toString();
                this->pushMsg(_currentMsg, static_cast<ECmdType>(_currentMsg->header().m_cmd));
            }
        });

    m_SMFWChannel->start();

    registerHandler<EChannelEvents::OnRemoteEndDissconnected>([this](const SSenderInfo& _sender) {
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

    registerHandler<EChannelEvents::OnFailedToConnect>([this](const SSenderInfo& _sender) {
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
    uint64_t protocolHeaderID = _currentMsg->header().m_ID;
    m_SMFWChannel->pushMsg(_currentMsg, static_cast<ECmdType>(_currentMsg->header().m_cmd), protocolHeaderID);
    return true;
}

CSMFWChannel::weakConnectionPtr_t CCommanderChannel::getSMFWChannel()
{
    return m_SMFWChannel;
}
