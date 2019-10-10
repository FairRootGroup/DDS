// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "CommanderChannel.h"
#include "UserDefaults.h"
#include "dds_env_prop.h"
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

CCommanderChannel::CCommanderChannel(boost::asio::io_context& _service, uint64_t _ProtocolHeaderID)
    : CClientChannelImpl<CCommanderChannel>(_service, EChannelType::AGENT, _ProtocolHeaderID)
    , m_connectionAttempts(1)
{
    // Create shared memory channel for message forwarding from the network channel
    const CUserDefaults& userDefaults = CUserDefaults::instance();
    //    m_SMFWChannel = CSMFWChannel::makeNew(_service,
    //                                          userDefaults.getSMAgentOutputNames(),
    //                                          userDefaults.getSMAgentInputName(),
    //                                          _ProtocolHeaderID,
    //                                          EMQOpenType::OpenOrCreate,
    //                                          EMQOpenType::OpenOrCreate);
    //
    //    m_SMFWChannel->registerHandler<cmdRAW_MSG>(
    //        [this](const SSenderInfo& _sender, CProtocolMessage::protocolMessagePtr_t _currentMsg) {
    //            ECmdType cmd = static_cast<ECmdType>(_currentMsg->header().m_cmd);
    //            // cmdMOVE_FILE is an exception. We have to forward it as a binary attachment.
    //            if (cmd == cmdMOVE_FILE)
    //            {
    //                LOG(debug) << "cmdMOVE_FILE pushed to network channel: " << _currentMsg->toString();
    //                SCommandAttachmentImpl<cmdMOVE_FILE>::ptr_t attachmentPtr =
    //                    SCommandAttachmentImpl<cmdMOVE_FILE>::decode(_currentMsg);
    //                this->pushBinaryAttachmentCmd(attachmentPtr->m_filePath,
    //                                              attachmentPtr->m_requestedFileName,
    //                                              attachmentPtr->m_srcCommand,
    //                                              _currentMsg->header().m_ID);
    //
    //                // We take the ownership and we have to delete the file
    //                try
    //                {
    //                    fs::remove(attachmentPtr->m_filePath);
    //                }
    //                catch (exception& _e)
    //                {
    //                    LOG(error) << "Can't remove log archive file: " << attachmentPtr->m_filePath
    //                               << "; error: " << _e.what();
    //                }
    //            }
    //            else if (cmd == cmdUPDATE_KEY)
    //            {
    //                SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t attachmentPtr =
    //                    SCommandAttachmentImpl<cmdUPDATE_KEY>::decode(_currentMsg);
    //
    //                string propertyName(attachmentPtr->m_propertyName);
    //                uint64_t taskID(attachmentPtr->m_senderTaskID);
    //                STopoRuntimeTask::FilterIteratorPair_t taskIt =
    //                    m_topo.getRuntimeTaskIteratorForPropertyName(propertyName, taskID);
    //
    //                for (auto it = taskIt.first; it != taskIt.second; ++it)
    //                {
    //                    uint64_t receiverTaskID(it->first);
    //
    //                    // Dont't send message to itself
    //                    if (taskID == receiverTaskID)
    //                        continue;
    //
    //                    auto task = m_topo.getRuntimeTaskById(receiverTaskID).m_task;
    //                    auto property = task->getProperty(propertyName);
    //                    if (property != nullptr && (property->getAccessType() == CTopoProperty::EAccessType::READ ||
    //                                                property->getAccessType() ==
    //                                                CTopoProperty::EAccessType::READWRITE))
    //                    {
    //                        SUpdateKeyCmd cmd;
    //                        cmd.m_propertyName = propertyName;
    //                        cmd.m_value = attachmentPtr->m_value;
    //                        cmd.m_receiverTaskID = receiverTaskID;
    //                        cmd.m_senderTaskID = taskID;
    //
    //                        bool localPush(true);
    //                        uint64_t channelID(0);
    //                        {
    //                            std::lock_guard<std::mutex> lock(m_taskIDToChannelIDMapMutex);
    //                            auto it = m_taskIDToChannelIDMap.find(receiverTaskID);
    //                            localPush = (it != m_taskIDToChannelIDMap.end());
    //                            channelID = it->second;
    //                        }
    //
    //                        if (localPush)
    //                        {
    //                            LOG(debug) << "Push update key via shared memory: cmd=<" << cmd
    //                                       << ">; protocolHeaderID=" << _currentMsg->header().m_ID
    //                                       << "; channelID=" << channelID;
    //                            m_SMFWChannel->pushMsg<cmdUPDATE_KEY>(cmd, _currentMsg->header().m_ID, channelID);
    //                        }
    //                        else
    //                        {
    //                            LOG(debug) << "Push update key via network channel: <" << cmd
    //                                       << ">; protocolHeaderID=" << _currentMsg->header().m_ID;
    //                            this->pushMsg<cmdUPDATE_KEY>(cmd, _currentMsg->header().m_ID);
    //                        }
    //
    //                        LOG(debug) << "Property update from agent channel: <" << cmd << ">";
    //                    }
    //                }
    //            }
    //            else
    //            {
    //                // Remove task ID from map if it exited
    //                if (cmd == cmdUSER_TASK_DONE)
    //                {
    //                    SCommandAttachmentImpl<cmdUSER_TASK_DONE>::ptr_t attachmentPtr =
    //                        SCommandAttachmentImpl<cmdUSER_TASK_DONE>::decode(_currentMsg);
    //
    //                    std::lock_guard<std::mutex> lock(m_taskIDToChannelIDMapMutex);
    //                    auto it = m_taskIDToChannelIDMap.find(attachmentPtr->m_taskID);
    //                    if (it != m_taskIDToChannelIDMap.end())
    //                        m_taskIDToChannelIDMap.erase(it);
    //                }
    //
    //                LOG(debug) << "Raw message pushed to network channel: " << _currentMsg->toString();
    //                this->pushMsg(_currentMsg, cmd);
    //            }
    //        });
    //
    //    m_SMFWChannel->start();

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

// bool CCommanderChannel::on_rawMessage(CProtocolMessage::protocolMessagePtr_t _currentMsg)
//{
//    ECmdType cmd = static_cast<ECmdType>(_currentMsg->header().m_cmd);
//    uint64_t protocolHeaderID = _currentMsg->header().m_ID;
//
//    if (cmd == cmdASSIGN_USER_TASK)
//    {
//        // Collect all task assignments for a local cache of taskID -> channelID
//
//        SCommandAttachmentImpl<cmdASSIGN_USER_TASK>::ptr_t attachmentPtr =
//            SCommandAttachmentImpl<cmdASSIGN_USER_TASK>::decode(_currentMsg);
//
//        std::lock_guard<std::mutex> lock(m_taskIDToChannelIDMapMutex);
//        m_taskIDToChannelIDMap[attachmentPtr->m_taskID] = protocolHeaderID;
//    }
//    else if (cmd == cmdBINARY_ATTACHMENT_RECEIVED && protocolHeaderID == getProtocolHeaderID())
//    {
//        // Load the topology
//
//        SCommandAttachmentImpl<cmdBINARY_ATTACHMENT_RECEIVED>::ptr_t attachmentPtr =
//            SCommandAttachmentImpl<cmdBINARY_ATTACHMENT_RECEIVED>::decode(_currentMsg);
//
//        if (attachmentPtr->m_srcCommand == cmdUPDATE_TOPOLOGY)
//        {
//            topology_api::CTopoCore topo;
//            // Topology already validated on the commander, no need to validate it again
//            topo.setXMLValidationDisabled(true);
//            topo.init(attachmentPtr->m_receivedFilePath);
//            // Assign new topology
//            m_topo = topo;
//            LOG(info) << "Topology for lobby leader activated";
//        }
//    }
//
//    LOG(debug) << "Raw message pushed to shared memory channel: " << _currentMsg->toString();
//    m_SMFWChannel->pushMsg(_currentMsg, cmd, protocolHeaderID);
//    return true;
//}

// CSMFWChannel::weakConnectionPtr_t CCommanderChannel::getSMFWChannel()
//{
//    return m_SMFWChannel;
//}

bool CCommanderChannel::on_cmdREPLY(protocol_api::SCommandAttachmentImpl<protocol_api::cmdREPLY>::ptr_t _attachment,
                                    protocol_api::SSenderInfo& _sender)
{
    switch (_attachment->m_srcCommand)
    {
        case cmdLOBBY_MEMBER_INFO:
        {
            if (_attachment->m_statusCode == (uint16_t)SReplyCmd::EStatusCode::OK)
            {
                LOG(info) << "Received confirmation from lobby leader";
                LOG(info) << "Sending handshake to commander with PHID: " << this->m_protocolHeaderID;

                // Prepare a hand shake message
                SVersionCmd cmd;
                cmd.m_channelType = EChannelType::AGENT;
                cmd.m_sSID = CUserDefaults::instance().getLockedSID();
                cmd.m_version = DDS_PROTOCOL_VERSION;
                pushMsg<cmdLOBBY_MEMBER_HANDSHAKE>(cmd, this->m_protocolHeaderID);

                return true;
            }
            else if (_attachment->m_statusCode == (uint16_t)SReplyCmd::EStatusCode::ERROR)
            {
                LOG(error) << "Received error from lobby leader: " << _attachment->m_sMsg;
                return true;
            }
        }
        break;

        case cmdLOBBY_MEMBER_HANDSHAKE:
        {
            if (_attachment->m_statusCode == (uint16_t)SReplyCmd::EStatusCode::OK)
            {
                LOG(info) << "SM: Handshake successfull. PHID: " << this->m_protocolHeaderID;
                return true;
            }
            else if (_attachment->m_statusCode == (uint16_t)SReplyCmd::EStatusCode::ERROR)
            {
                LOG(fatal) << "SM: Handshake failed. PHID: " << this->m_protocolHeaderID;
                return false;
            }
        }
        break;

        default:
            break;
    }

    return true;
}

bool CCommanderChannel::on_cmdSIMPLE_MSG(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment, SSenderInfo& _sender)
{
    switch (_attachment->m_srcCommand)
    {
        case cmdTRANSPORT_TEST:
        {
            pushMsg<cmdSIMPLE_MSG>(*_attachment);
            return true;
        }
        case cmdUPDATE_KEY:
            return false; // let connection manager forward this info to UI channels

        case cmdCUSTOM_CMD:
            return false; // let connection manager forward this info to UI channels

        default:
            LOG(debug) << "Received command cmdSIMPLE_MSG does not have a listener";
            return true;
    }

    return true;
}

bool CCommanderChannel::on_cmdGET_HOST_INFO(SCommandAttachmentImpl<cmdGET_HOST_INFO>::ptr_t _attachment,
                                            SSenderInfo& _sender)
{
    // pid
    pid_t pid = getpid();

    SHostInfoCmd cmd;
    get_cuser_name(&cmd.m_username);
    get_hostname(&cmd.m_host);
    cmd.m_version = PROJECT_VERSION_STRING;
    cmd.m_DDSPath = CUserDefaults::getDDSPath();
    cmd.m_agentPid = pid;

    // get worker ID
    string sWorkerId;
    char* pchWorkerId;
    pchWorkerId = getenv("DDS_WORKER_ID");
    if (NULL != pchWorkerId)
    {
        sWorkerId.assign(pchWorkerId);
        cmd.m_workerId = sWorkerId;
    }

    // get submit time
    string sSubmitTime;
    char* pchSubmitTime;
    pchSubmitTime = getenv("DDS_WN_SUBMIT_TIMESTAMP");
    if (NULL != pchSubmitTime)
    {
        sSubmitTime.assign(pchSubmitTime);
        cmd.m_submitTime = stoll(sSubmitTime);
    }

    pushMsg<cmdREPLY_HOST_INFO>(cmd);
    return true;
}

bool CCommanderChannel::on_cmdSHUTDOWN(SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment, SSenderInfo& _sender)
{
    deleteAgentIDFile();
    LOG(info) << "The SM Agent [" << m_id << "] received cmdSHUTDOWN.";
    // return false to let connection manager to catch this message as well
    return false;
}

bool CCommanderChannel::on_cmdBINARY_ATTACHMENT_RECEIVED(
    SCommandAttachmentImpl<cmdBINARY_ATTACHMENT_RECEIVED>::ptr_t _attachment, SSenderInfo& _sender)
{
    LOG(debug) << "Received command cmdBINARY_ATTACHMENT_RECEIVED";

    switch (_attachment->m_srcCommand)
    {
        case cmdTRANSPORT_TEST:
        {
            // Remove received file
            boost::filesystem::remove(_attachment->m_receivedFilePath);

            pushMsg<cmdBINARY_ATTACHMENT_RECEIVED>(*_attachment);
            return true;
        }
        case cmdASSIGN_USER_TASK:
        {
            boost::filesystem::path destFilePath(CUserDefaults::instance().getDDSPath());
            destFilePath /= _attachment->m_requestedFileName;
            boost::filesystem::rename(_attachment->m_receivedFilePath, destFilePath);
            // Add exec permissions for the users' task
            fs::permissions(destFilePath, fs::add_perms | fs::owner_exe);
            LOG(info) << "Received user executable to execute: " << destFilePath.generic_string();

            // Send response back to server
            pushMsg<cmdREPLY>(SReplyCmd("File received", (uint16_t)SReplyCmd::EStatusCode::OK, 0, cmdASSIGN_USER_TASK));
            return true;
        }
        case cmdUPDATE_TOPOLOGY:
        {
            // Copy topology file
            boost::filesystem::path destFilePath(CUserDefaults::instance().getDDSPath());
            destFilePath /= _attachment->m_requestedFileName;
            boost::filesystem::rename(_attachment->m_receivedFilePath, destFilePath);
            LOG(info) << "Received new topology file: " << destFilePath.generic_string();

            // Connection manager will activate the topology
            return false;
        }
        default:
            LOG(debug) << "Received command cmdBINARY_ATTACHMENT_RECEIVED does not have a listener";
            return true;
    }

    return true;
}

bool CCommanderChannel::on_cmdGET_ID(SCommandAttachmentImpl<cmdGET_ID>::ptr_t _attachment, SSenderInfo& _sender)
{
    LOG(info) << "cmdGET_ID received from DDS Server";

    SIDCmd msg_cmd;
    msg_cmd.m_id = m_id;
    pushMsg<cmdREPLY_ID>(msg_cmd);

    return true;
}

bool CCommanderChannel::on_cmdSET_ID(SCommandAttachmentImpl<cmdSET_ID>::ptr_t _attachment, SSenderInfo& _sender)
{
    LOG(info) << "cmdSET_ID attachment [" << *_attachment << "] from DDS Server";

    m_id = _attachment->m_id;

    createAgentIDFile();

    return true;
}

bool CCommanderChannel::on_cmdGET_LOG(SCommandAttachmentImpl<cmdGET_LOG>::ptr_t _attachment, SSenderInfo& _sender)
{
    try
    {
        string hostname;
        get_hostname(&hostname);

        time_t now = chrono::system_clock::to_time_t(chrono::system_clock::now());
        struct tm* ptm = localtime(&now);
        char buffer[20];
        strftime(buffer, 20, "%Y-%m-%d-%H-%M-%S", ptm);

        stringstream ss;
        // TODO: change the code below once on gcc5+
        // We do not use put_time for the moment as gcc4.9 does not support it.
        // ss << std::put_time(ptm, "%Y-%m-%d-%H-%M-%S") << "_" << hostname << "_" << m_id;
        ss << buffer << "_" << hostname << "_" << m_id << ".tar.gz";

        fs::path fileName(ss.str());
        fs::path logDir(CUserDefaults::getDDSPath());
        fs::path filePath(logDir);
        filePath /= fileName;

        // Find command paths in $PATH
        fs::path bashPath = bp::search_path("bash");
        fs::path tarPath = bp::search_path("tar");
        fs::path findPath = bp::search_path("find");

        stringstream ssCmd;
        ssCmd << bashPath.string() << " -c \"" << findPath.string() << " \\\"" << logDir.string()
              << "\\\" -name \\\"*.log\\\" | " << tarPath.string() << " -czf \\\"" << filePath.string()
              << "\\\" -T -\"";

        string output;
        execute(ssCmd.str(), chrono::seconds(60), &output);

        SMoveFileCmd filePathCmd;
        filePathCmd.m_filePath = filePath.string();
        filePathCmd.m_requestedFileName = fileName.string();
        filePathCmd.m_srcCommand = cmdGET_LOG;
        pushMsg<cmdMOVE_FILE>(filePathCmd);
    }
    catch (exception& e)
    {
        LOG(error) << e.what();
        pushMsg<cmdREPLY>(SReplyCmd(e.what(), (uint16_t)SReplyCmd::EStatusCode::ERROR, 0, cmdGET_LOG));
    }

    return true;
}

void CCommanderChannel::readAgentIDFile()
{
    const string sAgentIDFile(CUserDefaults::getAgentIDFilePath());
    LOG(info) << "Reading an agent ID file: " << sAgentIDFile;
    ifstream f(sAgentIDFile.c_str());
    if (!f.is_open() || !f.good())
    {
        string msg("Could not open an agent ID file: ");
        msg += sAgentIDFile;
        throw runtime_error(msg);
    }
    f >> m_id;
}

void CCommanderChannel::createAgentIDFile() const
{
    const string sAgentIDFile(CUserDefaults::getAgentIDFilePath());
    LOG(info) << "Creating an agent ID file: " << sAgentIDFile;
    ofstream f(sAgentIDFile.c_str());
    if (!f.is_open() || !f.good())
    {
        string msg("Could not open an agent ID file: ");
        msg += sAgentIDFile;
        throw runtime_error(msg);
    }

    f << m_id;
}

void CCommanderChannel::deleteAgentIDFile() const
{
    const string sAgentIDFile(CUserDefaults::getAgentIDFilePath());
    if (sAgentIDFile.empty())
        return;

    // TODO: check error code
    unlink(sAgentIDFile.c_str());
}

bool CCommanderChannel::on_cmdASSIGN_USER_TASK(SCommandAttachmentImpl<cmdASSIGN_USER_TASK>::ptr_t _attachment,
                                               SSenderInfo& _sender)
{
    LOG(info) << "Received a user task assignment. " << *_attachment;

    auto slot_iter = m_slots.find(_sender.m_ID);
    if (slot_iter == m_slots.end())
    {
        stringstream ss;
        ss << "No matching slot for " << _sender.m_ID;
        pushMsg<cmdREPLY>(SReplyCmd(ss.str(), (uint16_t)SReplyCmd::EStatusCode::ERROR, 0, cmdASSIGN_USER_TASK));
        LOG(error) << "Assign task error: " << ss.str();
        return true;
    }

    SSlotInfo& slot = slot_iter->second;

    slot.m_sUsrExe = _attachment->m_sExeFile;
    slot.m_taskID = _attachment->m_taskID;
    slot.m_taskIndex = _attachment->m_taskIndex;
    slot.m_collectionIndex = _attachment->m_collectionIndex;
    slot.m_taskPath = _attachment->m_taskPath;
    slot.m_groupName = _attachment->m_groupName;
    slot.m_collectionName = _attachment->m_collectionName;
    slot.m_taskName = _attachment->m_taskName;

    // Replace all %taskIndex% and %collectionIndex% in executable path with their values.
    boost::algorithm::replace_all(slot.m_sUsrExe, "%taskIndex%", to_string(slot.m_taskIndex));
    if (slot.m_collectionIndex != numeric_limits<uint32_t>::max())
        boost::algorithm::replace_all(slot.m_sUsrExe, "%collectionIndex%", to_string(slot.m_collectionIndex));

    try
    {
        string filePath;
        string filename;
        string cmdStr;
        smart_path(&slot.m_sUsrExe);
        parseExe(slot.m_sUsrExe, "", filePath, filename, cmdStr);
        slot.m_sUsrExe = cmdStr;
    }
    catch (exception& _e)
    {
        LOG(error) << _e.what();
        pushMsg<cmdREPLY>(SReplyCmd(_e.what(), (uint16_t)SReplyCmd::EStatusCode::ERROR, 0, cmdASSIGN_USER_TASK));
        return true;
    }

    pushMsg<cmdREPLY>(SReplyCmd("User task assigned", (uint16_t)SReplyCmd::EStatusCode::OK, 0, cmdASSIGN_USER_TASK));

    return true;
}

bool CCommanderChannel::on_cmdACTIVATE_USER_TASK(SCommandAttachmentImpl<cmdACTIVATE_USER_TASK>::ptr_t _attachment,
                                                 SSenderInfo& _sender)
{
    // take slot ID from the command attachment
    auto slot_it = m_slots.find(_attachment->m_id);
    if (slot_it == m_slots.end())
    {
        LOG(error) << "Received activation command. Wrong slot ID.";
        // Send response back to server
        pushMsg<cmdREPLY>(SReplyCmd("Received activation command. Wrong slot ID.",
                                    (uint16_t)SReplyCmd::EStatusCode::ERROR,
                                    0,
                                    cmdACTIVATE_USER_TASK));
        return true;
    }

    const SSlotInfo& slot = slot_it->second;

    string sUsrExe(slot.m_sUsrExe);

    if (sUsrExe.empty())
    {
        LOG(info) << "Received activation command. Ignoring the command, since no task is assigned.";
        // Send response back to server
        pushMsg<cmdREPLY>(SReplyCmd("No task is assigned. Activation is ignored.",
                                    (uint16_t)SReplyCmd::EStatusCode::OK,
                                    0,
                                    cmdACTIVATE_USER_TASK));
        return true;
    }

    StringVector_t params;
    pid_t pidUsrTask(0);

    try
    {
        // set task's environment
        LOG(info) << "Setting up task's environment: "
                  << "DDS_TASK_ID:" << slot.m_taskID << " DDS_TASK_INDEX:" << slot.m_taskIndex
                  << " DDS_COLLECTION_INDEX:" << slot.m_collectionIndex << " DDS_TASK_PATH:" << slot.m_taskPath
                  << " DDS_GROUP_NAME:" << slot.m_groupName << " DDS_COLLECTION_NAME:" << slot.m_collectionName
                  << " DDS_TASK_NAME:" << slot.m_taskName
                  << " DDS_SESSION_ID: " << dds::env_prop<dds::EEnvProp::dds_session_id>();
        if (::setenv("DDS_TASK_ID", to_string(slot.m_taskID).c_str(), 1) == -1)
            throw MiscCommon::system_error("Failed to set up $DDS_TASK_ID");
        if (::setenv("DDS_TASK_INDEX", to_string(slot.m_taskIndex).c_str(), 1) == -1)
            throw MiscCommon::system_error("Failed to set up $DDS_TASK_INDEX");
        if (slot.m_collectionIndex != numeric_limits<uint32_t>::max())
            if (::setenv("DDS_COLLECTION_INDEX", to_string(slot.m_collectionIndex).c_str(), 1) == -1)
                throw MiscCommon::system_error("Failed to set up $DDS_COLLECTION_INDEX");
        if (::setenv("DDS_TASK_PATH", slot.m_taskPath.c_str(), 1) == -1)
            throw MiscCommon::system_error("Failed to set up $DDS_TASK_PATH");
        if (::setenv("DDS_GROUP_NAME", slot.m_groupName.c_str(), 1) == -1)
            throw MiscCommon::system_error("Failed to set up $DDS_GROUP_NAME");
        if (::setenv("DDS_COLLECTION_NAME", slot.m_collectionName.c_str(), 1) == -1)
            throw MiscCommon::system_error("Failed to set up $DDS_COLLECTION_NAME");
        if (::setenv("DDS_TASK_NAME", slot.m_taskName.c_str(), 1) == -1)
            throw MiscCommon::system_error("Failed to set up $DDS_TASK_NAME");

        dispatchHandlers<>(EChannelEvents::OnAssignUserTask, _sender);

        // execute the task
        LOG(info) << "Executing user task: " << sUsrExe;

        // Task output files: <user_task_name>_<datetime>_<task_id>_<out/err>.log
        stringstream ssTaskOutput;
        ssTaskOutput << CUserDefaults::getDDSPath() << slot.m_taskName;

        // TODO: Change the code below once on gcc5+. GCC 4.9 doesn't support put_time
        //
        // current time
        // auto now = std::chrono::system_clock::now();
        // auto in_time_t = std::chrono::system_clock::to_time_t(now);
        // ssTaskOutput << std::put_time(std::localtime(&in_time_t), "_%F_%H-%M-%S");

        time_t now = chrono::system_clock::to_time_t(chrono::system_clock::now());
        struct tm* ptm = localtime(&now);
        char buffer[20];
        strftime(buffer, 20, "%Y-%m-%d-%H-%M-%S", ptm);
        ssTaskOutput << "_" << buffer;

        // task id
        ssTaskOutput << "_" << slot.m_taskID;

        string sTaskStdOut(ssTaskOutput.str() + "_out.log");
        string sTaskStdErr(ssTaskOutput.str() + "_err.log");

        pidUsrTask = execute(sUsrExe, sTaskStdOut, sTaskStdErr);
    }
    catch (exception& _e)
    {
        LOG(error) << _e.what();
        // Send response back to server
        pushMsg<cmdREPLY>(SReplyCmd(_e.what(), (uint16_t)SReplyCmd::EStatusCode::ERROR, 0, cmdACTIVATE_USER_TASK));
        return true;
    }

    stringstream ss;
    ss << "User task (pid:" << pidUsrTask << ") is activated.";
    LOG(info) << ss.str();

    dispatchHandlers<>(EChannelEvents::OnNewUserTask, _sender, slot.m_id, pidUsrTask);

    // Send response back to server
    pushMsg<cmdREPLY>(SReplyCmd(ss.str(), (uint16_t)SReplyCmd::EStatusCode::OK, 0, cmdACTIVATE_USER_TASK));

    return true;
}

bool CCommanderChannel::on_cmdSTOP_USER_TASK(SCommandAttachmentImpl<cmdSTOP_USER_TASK>::ptr_t _attachment,
                                             SSenderInfo& _sender)
{
    //    if (m_taskID == 0)
    //    {
    //        // No running tasks, nothing to stop
    //        // Send response back to server
    //        pushMsg<cmdREPLY>(SReplyCmd(
    //            "No tasks is running. Nothing to stop.", (uint16_t)SReplyCmd::EStatusCode::OK, 0, cmdSTOP_USER_TASK));
    //        return true;
    //    }

    // Let the parent should terminate user tasks
    return false;
}

bool CCommanderChannel::on_cmdADD_SLOT(SCommandAttachmentImpl<cmdADD_SLOT>::ptr_t _attachment, SSenderInfo& _sender)
{
    LOG(info) << "Received a ADD SLOT request, id = " << _attachment->m_id;
    // Add new Task slot
    SSlotInfo info;
    info.m_id = _attachment->m_id;

    {
        std::lock_guard<std::mutex> lock(m_mutexSlots);
        m_slots.insert(make_pair(info.m_id, info));
    }

    // Confirm to commander the new slot
    SIDCmd msg_cmd;
    msg_cmd.m_id = info.m_id;
    pushMsg<cmdREPLY_ADD_SLOT>(msg_cmd);

    return true;
}

bool CCommanderChannel::on_cmdUPDATE_KEY(SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment, SSenderInfo& _sender)
{
    LOG(debug) << "Received a key update notifications: " << *_attachment;
    return false;
}

bool CCommanderChannel::on_cmdCUSTOM_CMD(SCommandAttachmentImpl<cmdCUSTOM_CMD>::ptr_t _attachment, SSenderInfo& _sender)
{
    LOG(debug) << "Received custom command: " << *_attachment;
    return false;
}

void CCommanderChannel::sendTaskDone(uint64_t _slotID, int _exitCode)
{
    SSlotInfo::container_t::const_iterator slot_iter;
    {
        lock_guard<std::mutex> lock(m_mutexSlots);
        slot_iter = m_slots.find(_slotID);
        if (slot_iter == m_slots.end())
        {
            LOG(error) << "Can't send TASK_DONE. The coresponding slot is missing";
            return;
        }
    }
    SUserTaskDoneCmd cmd;
    cmd.m_exitCode = _exitCode;
    cmd.m_taskID = slot_iter->second.m_taskID;
    pushMsg<cmdUSER_TASK_DONE>(cmd);
}
