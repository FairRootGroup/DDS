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
using namespace topology_api;
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
            // cmdMOVE_FILE is an exception. We have to forward it as a binary attachment.
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
            else if (cmd == cmdUPDATE_KEY)
            {
                SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t attachmentPtr =
                    SCommandAttachmentImpl<cmdUPDATE_KEY>::decode(_currentMsg);

                string propertyID(attachmentPtr->m_propertyID);
                uint64_t taskID(attachmentPtr->m_senderTaskID);
                CTopology::TaskInfoIteratorPair_t taskIt = m_topo.getTaskInfoIteratorForPropertyId(propertyID);

                for (auto it = taskIt.first; it != taskIt.second; ++it)
                {
                    uint64_t receiverTaskID(it->first);

                    // Dont't send message to itself
                    if (taskID == receiverTaskID)
                        continue;

                    auto task = m_topo.getTaskByHash(receiverTaskID);
                    auto property = task->getProperty(propertyID);
                    if (property != nullptr && (property->getAccessType() == EPropertyAccessType::READ ||
                                                property->getAccessType() == EPropertyAccessType::READWRITE))
                    {
                        SUpdateKeyCmd cmd;
                        cmd.m_propertyID = propertyID;
                        cmd.m_value = attachmentPtr->m_value;
                        cmd.m_receiverTaskID = receiverTaskID;
                        cmd.m_senderTaskID = taskID;

                        bool localPush(true);
                        uint64_t channelID(0);
                        {
                            std::lock_guard<std::mutex> lock(m_taskIDToChannelIDMapMutex);
                            auto it = m_taskIDToChannelIDMap.find(receiverTaskID);
                            localPush = (it != m_taskIDToChannelIDMap.end());
                            channelID = it->second;
                        }

                        if (localPush)
                        {
                            LOG(debug) << "Push update key via shared memory: cmd=<" << cmd
                                       << ">; protocolHeaderID=" << _currentMsg->header().m_ID
                                       << "; channelID=" << channelID;
                            m_SMFWChannel->pushMsg<cmdUPDATE_KEY>(cmd, _currentMsg->header().m_ID, channelID);
                        }
                        else
                        {
                            LOG(debug) << "Push update key via network channel: <" << cmd
                                       << ">; protocolHeaderID=" << _currentMsg->header().m_ID;
                            this->pushMsg<cmdUPDATE_KEY>(cmd, _currentMsg->header().m_ID);
                        }

                        LOG(debug) << "Property update from agent channel: <" << cmd << ">";
                    }
                }
            }
            else
            {
                // Remove task ID from map if it exited
                if (cmd == cmdUSER_TASK_DONE)
                {
                    SCommandAttachmentImpl<cmdUSER_TASK_DONE>::ptr_t attachmentPtr =
                        SCommandAttachmentImpl<cmdUSER_TASK_DONE>::decode(_currentMsg);

                    std::lock_guard<std::mutex> lock(m_taskIDToChannelIDMapMutex);
                    auto it = m_taskIDToChannelIDMap.find(attachmentPtr->m_taskID);
                    if (it != m_taskIDToChannelIDMap.end())
                        m_taskIDToChannelIDMap.erase(it);
                }

                LOG(debug) << "Raw message pushed to network channel: " << _currentMsg->toString();
                this->pushMsg(_currentMsg, cmd);
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
    ECmdType cmd = static_cast<ECmdType>(_currentMsg->header().m_cmd);
    uint64_t protocolHeaderID = _currentMsg->header().m_ID;

    if (cmd == cmdASSIGN_USER_TASK)
    {
        // Collect all task assignments for a local cache of taskID -> channelID

        SCommandAttachmentImpl<cmdASSIGN_USER_TASK>::ptr_t attachmentPtr =
            SCommandAttachmentImpl<cmdASSIGN_USER_TASK>::decode(_currentMsg);

        std::lock_guard<std::mutex> lock(m_taskIDToChannelIDMapMutex);
        m_taskIDToChannelIDMap[attachmentPtr->m_taskID] = protocolHeaderID;
    }
    else if (cmd == cmdBINARY_ATTACHMENT_RECEIVED && protocolHeaderID == getProtocolHeaderID())
    {
        // Load the topology

        SCommandAttachmentImpl<cmdBINARY_ATTACHMENT_RECEIVED>::ptr_t attachmentPtr =
            SCommandAttachmentImpl<cmdBINARY_ATTACHMENT_RECEIVED>::decode(_currentMsg);

        if (attachmentPtr->m_srcCommand == cmdUPDATE_TOPOLOGY)
        {
            topology_api::CTopology topo;
            // Topology already validated on the commander, no need to validate it again
            topo.setXMLValidationDisabled(true);
            topo.init(attachmentPtr->m_receivedFilePath);
            // Assign new topology
            m_topo = topo;
            LOG(info) << "Topology for lobby leader activated";
        }
    }

    LOG(debug) << "Raw message pushed to shared memory channel: " << _currentMsg->toString();
    m_SMFWChannel->pushMsg(_currentMsg, cmd, protocolHeaderID);
    return true;
}

CSMFWChannel::weakConnectionPtr_t CCommanderChannel::getSMFWChannel()
{
    return m_SMFWChannel;
}
