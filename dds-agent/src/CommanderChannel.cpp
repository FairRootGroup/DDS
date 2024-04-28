// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "CommanderChannel.h"
#include "ConditionEvent.h"
#include "EnvProp.h"
#include "UserDefaults.h"
#include "Version.h"
// BOOST
#include <boost/bind/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/process.hpp>

using namespace std;
using namespace dds::misc;
using namespace dds;
using namespace dds::protocol_api;
using namespace dds::agent_cmd;
using namespace user_defaults_api;
using namespace topology_api;

namespace fs = boost::filesystem;
namespace ba = boost::algorithm;
namespace bp = boost::process;

const uint16_t g_MaxConnectionAttempts = 5;
// 1MB = 1048576 Bytes
const uint g_bytesInMB{ 1048576 };

CCommanderChannel::CCommanderChannel(boost::asio::io_context& _service,
                                     uint64_t _ProtocolHeaderID,
                                     boost::asio::io_context& _intercomService)
    : CClientChannelImpl<CCommanderChannel>(_service, EChannelType::AGENT, _ProtocolHeaderID)
{
    // Create shared memory channel for message forwarding from the network channel
    const CUserDefaults& userDefaults = CUserDefaults::instance();

    m_intercomChannel = CSMIntercomChannel::makeNew(_intercomService,
                                                    userDefaults.getSMLeaderInputNames(),
                                                    userDefaults.getSMLeaderOutputName(_ProtocolHeaderID),
                                                    _ProtocolHeaderID,
                                                    EMQOpenType::OpenOrCreate,
                                                    EMQOpenType::OpenOrCreate);

    m_intercomChannel->registerHandler<cmdCUSTOM_CMD>(
        [this](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdCUSTOM_CMD>::ptr_t _attachment)
        { pushMsg<cmdCUSTOM_CMD>(*_attachment, _sender.m_ID); });

    m_intercomChannel->registerHandler<cmdUPDATE_KEY>(
        [this](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment)
        { send_cmdUPDATE_KEY(_sender, _attachment); });

    m_intercomChannel->start();

    registerHandler<EChannelEvents::OnRemoteEndDissconnected>(
        [this](const SSenderInfo& /*_sender*/)
        {
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

    registerHandler<EChannelEvents::OnFailedToConnect>(
        [this](const SSenderInfo& /*_sender*/)
        {
            if (m_connectionAttempts <= g_MaxConnectionAttempts)
            {
                LOG(info) << "Failed to connect to commander server. Trying to reconnect. Attempt "
                          << m_connectionAttempts << " out of " << g_MaxConnectionAttempts;
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

    // Check free disk space (GH-392)
    // Report available disk space at start
    uintmax_t nAvailable{ 0 };
    isLowDiskSpace(&nAvailable);
    LOG(info) << "Available disk space: " << dds::misc::HumanReadable{ nAvailable };
    // create async timer
    m_resourceMonitorTimer = make_unique<timer_t>(_service);
    startResourceMonitor(_service, chrono::seconds(30));
}

void CCommanderChannel::startResourceMonitor(boost::asio::io_context& _service, const chrono::seconds& _interval)
{
    m_resourceMonitorTimer->expires_after(_interval);
    // Check free disk space (GH-392)
    m_resourceMonitorTimer->async_wait(
        [this, &_service, _interval](const boost::system::error_code& /*_error*/) mutable
        {
            uintmax_t nAvailable{ 0 };
            if (isLowDiskSpace(&nAvailable))
            {
                unsigned long nLimit = stoul(CUserDefaults::instance().getValueForKey("agent.disk_space_threshold"));
                LOG(fatal) << "Triggering a shutdown due to low disk space. Required: "
                           << dds::misc::HumanReadable{ nLimit * g_bytesInMB }
                           << " Available: " << dds::misc::HumanReadable{ nAvailable };
                this->sendYourself<cmdSHUTDOWN>();
                return;
            }

            // reschedule
            startResourceMonitor(_service, _interval);
        });
}

void CCommanderChannel::setNumberOfSlots(uint32_t _nSlots)
{
    m_nSlots = _nSlots;
}

void CCommanderChannel::setGroupName(const std::string& _groupName)
{
    m_groupName = _groupName;
}

bool CCommanderChannel::on_cmdREPLY(SCommandAttachmentImpl<cmdREPLY>::ptr_t _attachment, SSenderInfo& /*_sender*/)
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
                LOG(info) << "SM: Handshake successful. PHID: " << this->m_protocolHeaderID;
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
        case cmdCUSTOM_CMD:
        {
            if (_attachment->m_msgSeverity == error)
            {
                LOG(error) << _attachment->m_sMsg;
            }
            // Forward message to user task
            m_intercomChannel->pushMsg<cmdSIMPLE_MSG>(*_attachment, _sender.m_ID, _sender.m_ID);
            return true;
        }

        default:
            LOG(debug) << "Received command cmdSIMPLE_MSG does not have a listener";
            return true;
    }

    return true;
}

bool CCommanderChannel::on_cmdGET_HOST_INFO(SCommandAttachmentImpl<cmdGET_HOST_INFO>::ptr_t /*_attachment*/,
                                            SSenderInfo& /*_sender*/)
{
    // pid
    const pid_t pid{ getpid() };

    SHostInfoCmd cmd;
    get_cuser_name(&cmd.m_username);
    get_hostname(&cmd.m_host);
    cmd.m_version = DDS_VERSION_STRING;
    cmd.m_DDSPath = CUserDefaults::getDDSPath();
    cmd.m_agentPid = pid;
    cmd.m_slots = m_nSlots;
    cmd.m_groupName = m_groupName;

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

    LOG(info) << cmd;

    pushMsg<cmdREPLY_HOST_INFO>(cmd);
    return true;
}

bool CCommanderChannel::on_cmdSHUTDOWN(SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t /*_attachment*/,
                                       SSenderInfo& /*_sender*/)
{
    deleteAgentIDFile();
    LOG(info) << "The Agent [" << m_id << "] received cmdSHUTDOWN.";

    // Remove global Assets
    {
        lock_guard<mutex> lock(m_mutexGlobalAssets);
        for (const auto& asset : m_globalAssets)
        {
            try
            {
                if (!fs::exists(asset) || !fs::is_regular_file(asset))
                    continue;

                fs::remove(asset);
                LOG(info) << "Removing global asset: " << asset.generic_string();
            }
            catch (...)
            {
            }
        }
        m_globalAssets.clear();
    }

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
            fs::remove(_attachment->m_receivedFilePath);

            pushMsg<cmdBINARY_ATTACHMENT_RECEIVED>(*_attachment);
            return true;
        }
        case cmdASSIGN_USER_TASK:
        {
            fs::path destFilePath(CUserDefaults::instance().getSlotsRootDir());
            destFilePath /= to_string(_sender.m_ID);
            destFilePath /= _attachment->m_requestedFileName;
            fs::rename(_attachment->m_receivedFilePath, destFilePath);
            // Add exec permissions for the users' task
            fs::permissions(destFilePath, fs::add_perms | fs::owner_exe);
            LOG(info) << "Received user executable to execute: " << destFilePath.generic_string();

            // Send response back to server
            pushMsg<cmdREPLY>(SReplyCmd("File received", (uint16_t)SReplyCmd::EStatusCode::OK, 0, cmdASSIGN_USER_TASK),
                              _sender.m_ID);
            return true;
        }
        case cmdUPDATE_TOPOLOGY:
        {
            // Copy topology file
            fs::path destFilePath(CUserDefaults::instance().getDDSPath());
            destFilePath /= _attachment->m_requestedFileName;
            fs::rename(_attachment->m_receivedFilePath, destFilePath);
            LOG(info) << "Received new topology file: " << destFilePath.generic_string();

            // Decompressing the topology file
            if (destFilePath.extension() == ".gz")
            {
                const fs::path gzipPath{ bp::search_path("gzip") };
                stringstream ssCmd;
                ssCmd << gzipPath.string() << " -df " << destFilePath;
                string output;
                execute(ssCmd.str(), chrono::seconds(60), &output);
                // remove ".gz" extension
                destFilePath = destFilePath.stem();
            }

            // Activating new topology
            CTopoCore::Ptr_t topo{ make_shared<CTopoCore>() };
            // Topology already validated on the commander, no need to validate it again
            topo->setXMLValidationDisabled(true);
            topo->init(destFilePath.string());
            // Assign new topology
            {
                lock_guard<mutex> lock(m_topoMutex);
                m_topo = topo;
            }
            LOG(info) << "Topology activated";

            // Send response back to server
            pushMsg<cmdREPLY>(SReplyCmd("File received", (uint16_t)SReplyCmd::EStatusCode::OK, 0, cmdUPDATE_TOPOLOGY));

            return true;
        }
        default:
            LOG(debug) << "Received command cmdBINARY_ATTACHMENT_RECEIVED does not have a listener";
            return true;
    }

    return true;
}

bool CCommanderChannel::on_cmdGET_ID(SCommandAttachmentImpl<cmdGET_ID>::ptr_t /*_attachment*/, SSenderInfo& /*_sender*/)
{
    LOG(info) << "cmdGET_ID received from DDS Server";

    SIDCmd msg_cmd;
    msg_cmd.m_id = m_id;
    pushMsg<cmdREPLY_ID>(msg_cmd);

    return true;
}

bool CCommanderChannel::on_cmdSET_ID(SCommandAttachmentImpl<cmdSET_ID>::ptr_t _attachment, SSenderInfo& /*_sender*/)
{
    LOG(info) << "cmdSET_ID attachment [" << *_attachment << "] from DDS Server";

    m_id = _attachment->m_id;

    createAgentIDFile();

    return true;
}

bool CCommanderChannel::on_cmdGET_LOG(SCommandAttachmentImpl<cmdGET_LOG>::ptr_t /*_attachment*/, SSenderInfo& _sender)
{
    try
    {
        string hostname;
        get_hostname(&hostname);

        const time_t now{ chrono::system_clock::to_time_t(chrono::system_clock::now()) };
        stringstream ss;
        ss << put_time(localtime(&now), "%Y-%m-%d-%H-%M-%S") << "_" << hostname << "_" << m_id << ".tar.gz";

        const fs::path fileName(ss.str());
        const fs::path logDir(CUserDefaults::getDDSPath());
        const fs::path filePath(logDir / fileName);

        // Find command paths in $PATH
        const fs::path bashPath{ bp::search_path("bash") };
        const fs::path tarPath{ bp::search_path("tar") };
        const fs::path findPath{ bp::search_path("find") };

        stringstream ssCmd;
        ssCmd << bashPath.string() << " -c \"" << findPath.string() << " \\\"" << logDir.string()
              << "\\\" -maxdepth 1 -type f -name \\\"*.log\\\" -exec basename {} \\; | " << tarPath.string()
              << " -czf \\\"" << filePath.string() << "\\\" --directory \\\"" << logDir.string() << "\\\" -T -\"";

        string output;
        execute(ssCmd.str(), chrono::seconds(60), &output);

        pushBinaryAttachmentCmd(filePath.string(), fileName.string(), cmdGET_LOG, _sender.m_ID);
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

    // Check that topology is the same before task assignment
    try
    {
        uint32_t topoHash{ 0 };
        {
            lock_guard<mutex> lock(m_topoMutex);
            topoHash = m_topo->getHash();
        }
        if (topoHash != _attachment->m_topoHash)
        {
            stringstream ss;
            ss << "Topology hash check failed: " << topoHash << " (must be " << _attachment->m_topoHash << ")";
            throw runtime_error(ss.str());
        }
    }
    catch (exception& _e)
    {
        pushMsg<cmdREPLY>(SReplyCmd(_e.what(), (uint16_t)SReplyCmd::EStatusCode::ERROR, 0, cmdASSIGN_USER_TASK),
                          _sender.m_ID);
        LOG(error) << "Assign task error: " << _e.what();
        return true;
    }

    SSlotInfo::SSlotInfoPtr_t slot;
    try
    {
        slot = getSlotInfoById(_sender.m_ID);
    }
    catch (exception& _e)
    {
        pushMsg<cmdREPLY>(SReplyCmd(_e.what(), (uint16_t)SReplyCmd::EStatusCode::ERROR, 0, cmdASSIGN_USER_TASK),
                          _sender.m_ID);
        LOG(error) << "Assign task error: " << _e.what();
        return true;
    }

    if (slot->m_taskID > 0)
    {
        stringstream ssError;
        ssError << "Assign task error: The slot " << _sender.m_ID << " already running task with id " << slot->m_taskID;
        pushMsg<cmdREPLY>(SReplyCmd(ssError.str(), (uint16_t)SReplyCmd::EStatusCode::ERROR, 0, cmdASSIGN_USER_TASK),
                          _sender.m_ID);
        LOG(error) << ssError.str();
        return true;
    }

    slot->m_sUsrExe = _attachment->m_sExeFile;
    slot->m_sUsrEnv = _attachment->m_sEnvFile;
    slot->m_taskID = _attachment->m_taskID;
    slot->m_taskIndex = _attachment->m_taskIndex;
    slot->m_collectionIndex = _attachment->m_collectionIndex;
    slot->m_taskPath = _attachment->m_taskPath;
    slot->m_groupName = _attachment->m_groupName;
    slot->m_collectionName = _attachment->m_collectionName;
    slot->m_taskName = _attachment->m_taskName;

    {
        lock_guard<mutex> lock(m_taskIDToSlotIDMapMutex);
        m_taskIDToSlotIDMap.insert(make_pair(slot->m_taskID, slot->m_id));
    }

    // Replace all %taskIndex% and %collectionIndex% in executable path with their values.
    ba::replace_all(slot->m_sUsrExe, "%taskIndex%", to_string(slot->m_taskIndex));
    if (slot->m_collectionIndex != numeric_limits<uint32_t>::max())
        ba::replace_all(slot->m_sUsrExe, "%collectionIndex%", to_string(slot->m_collectionIndex));

    // If the user task was transferred, than replace "%DDS_DEFAULT_TASK_PATH%" with the real path
    fs::path dir(CUserDefaults::instance().getSlotsRootDir());
    dir /= to_string(_sender.m_ID);
    dir += fs::path::preferred_separator;
    ba::replace_all(slot->m_sUsrExe, "%DDS_DEFAULT_TASK_PATH%", dir.generic_string());
    // If the user custom environment was transferred, than replace "%DDS_DEFAULT_TASK_PATH%" with the real path
    ba::replace_all(slot->m_sUsrEnv, "%DDS_DEFAULT_TASK_PATH%", dir.generic_string());

    // Revoke drain of the write queue to start accept messages
    m_intercomChannel->drainWriteQueue(false, slot->m_id);

    pushMsg<cmdREPLY>(SReplyCmd("User task assigned", (uint16_t)SReplyCmd::EStatusCode::OK, 0, cmdASSIGN_USER_TASK),
                      _sender.m_ID);

    // Creating task assets, if needed
    CTopoTask::Ptr_t task;
    {
        lock_guard<mutex> lock(m_topoMutex);
        task = m_topo->getRuntimeTaskById(slot->m_taskID).m_task;
    }
    auto assets = task->getAssets();
    for (const auto& asset : assets)
    {
        stringstream assetFileName;
        assetFileName << asset->getName() << ".asset";
        fs::path pathAsset;
        switch (asset->getAssetVisibility())
        {
            case CTopoAsset::EVisibility::Task:
                pathAsset = dir;
                pathAsset /= assetFileName.str();

                // If local asset exists, we will overwrite it. No skipping.

                slot->m_taskAssets.push_back(pathAsset);
                break;
            case CTopoAsset::EVisibility::Global:
                pathAsset = CUserDefaults::instance().getDDSPath();
                pathAsset /= assetFileName.str();

                // Create global Asset only once. Skip if exists.
                if (fs::exists(pathAsset))
                    continue;

                lock_guard<mutex> lock(m_mutexGlobalAssets);
                m_globalAssets.push_back(pathAsset);
                break;
        }

        LOG(info) << "Creating task ASSET for taskID " << slot->m_taskID << ": " << pathAsset.generic_string();
        ofstream f(pathAsset.generic_string());
        if (!f.is_open())
        {
            LOG(error) << "Failed to create task ASSET for taskID " << slot->m_taskID << ": "
                       << pathAsset.generic_string();
            continue;
        }
        f << asset->getValue();
        f.flush();
    }

    return true;
}

bool CCommanderChannel::on_cmdACTIVATE_USER_TASK(SCommandAttachmentImpl<cmdACTIVATE_USER_TASK>::ptr_t _attachment,
                                                 SSenderInfo& _sender)
{

    SSlotInfo::SSlotInfoPtr_t slot;
    try
    {
        slot = getSlotInfoById(_attachment->m_id);
    }
    catch (exception& _e)
    {
        LOG(error) << "Received activation command. Wrong slot ID.";
        // Send response back to server
        pushMsg<cmdREPLY>(SReplyCmd("Received activation command. Wrong slot ID.",
                                    (uint16_t)SReplyCmd::EStatusCode::ERROR,
                                    0,
                                    cmdACTIVATE_USER_TASK),
                          _sender.m_ID);
        return true;
    }

    const string sUsrExe(slot->m_sUsrExe);

    if (sUsrExe.empty())
    {
        LOG(info) << "Received activation command. Ignoring the command, since no task is assigned.";
        // Send response back to server
        pushMsg<cmdREPLY>(SReplyCmd("No task is assigned. Activation is ignored.",
                                    (uint16_t)SReplyCmd::EStatusCode::OK,
                                    0,
                                    cmdACTIVATE_USER_TASK),
                          _sender.m_ID);
        return true;
    }

    StringVector_t params;
    pid_t pidUsrTask(0);

    try
    {
        // set task's environment
        LOG(info) << "Setting up task's environment: "
                  << "DDS_TASK_ID:" << slot->m_taskID << " DDS_TASK_INDEX:" << slot->m_taskIndex
                  << " DDS_COLLECTION_INDEX:" << slot->m_collectionIndex << " DDS_TASK_PATH:" << slot->m_taskPath
                  << " DDS_GROUP_NAME:" << slot->m_groupName << " DDS_COLLECTION_NAME:" << slot->m_collectionName
                  << " DDS_TASK_NAME:" << slot->m_taskName << " DDS_SLOT_ID:" << slot->m_id
                  << " DDS_SESSION_ID: " << dds::env_prop<dds::EEnvProp::dds_session_id>();
        if (::setenv("DDS_TASK_ID", to_string(slot->m_taskID).c_str(), 1) == -1)
            throw dds::misc::system_error("Failed to set up $DDS_TASK_ID");
        if (::setenv("DDS_TASK_INDEX", to_string(slot->m_taskIndex).c_str(), 1) == -1)
            throw dds::misc::system_error("Failed to set up $DDS_TASK_INDEX");
        if (slot->m_collectionIndex != numeric_limits<uint32_t>::max())
            if (::setenv("DDS_COLLECTION_INDEX", to_string(slot->m_collectionIndex).c_str(), 1) == -1)
                throw dds::misc::system_error("Failed to set up $DDS_COLLECTION_INDEX");
        if (::setenv("DDS_TASK_PATH", slot->m_taskPath.c_str(), 1) == -1)
            throw dds::misc::system_error("Failed to set up $DDS_TASK_PATH");
        if (::setenv("DDS_GROUP_NAME", slot->m_groupName.c_str(), 1) == -1)
            throw dds::misc::system_error("Failed to set up $DDS_GROUP_NAME");
        if (::setenv("DDS_COLLECTION_NAME", slot->m_collectionName.c_str(), 1) == -1)
            throw dds::misc::system_error("Failed to set up $DDS_COLLECTION_NAME");
        if (::setenv("DDS_TASK_NAME", slot->m_taskName.c_str(), 1) == -1)
            throw dds::misc::system_error("Failed to set up $DDS_TASK_NAME");
        if (::setenv("DDS_SLOT_ID", to_string(slot->m_id).c_str(), 1) == -1)
            throw dds::misc::system_error("Failed to set up $DDS_SLOT_ID");

        // execute the task
        LOG(info) << "Executing user task: " << sUsrExe;

        // Task output files: <user_task_name>_<datetime>_<task_id>_<out/err>.log
        const time_t now{ chrono::system_clock::to_time_t(chrono::system_clock::now()) };
        stringstream ssTaskOutput;
        ssTaskOutput << CUserDefaults::getDDSPath() << slot->m_taskName << "_"
                     << put_time(localtime(&now), "%Y-%m-%d-%H-%M-%S") << "_" << slot->m_taskID;
        const string sTaskStdOut(ssTaskOutput.str() + "_out.log");
        const string sTaskStdErr(ssTaskOutput.str() + "_err.log");

        fs::path pathSlotDir(CUserDefaults::instance().getSlotsRootDir());
        pathSlotDir /= to_string(_sender.m_ID);
        const fs::path pathTaskWrapperIn("dds_user_task_wrapper.sh.in");
        const fs::path pathTaskWrapper(pathSlotDir / "dds_user_task_wrapper.sh");

        // Replace placeholders in the task wrapper
        std::ifstream fTaskWrapper(pathTaskWrapperIn.native());
        if (!fTaskWrapper.is_open())
            throw runtime_error("Failed to open task wrapper template.");
        string sTaskWrapperContent((istreambuf_iterator<char>(fTaskWrapper)), istreambuf_iterator<char>());
        // JOB SCRIPT ---  Custom environment
        if (!slot->m_sUsrEnv.empty())
        {
            boost::replace_all(sTaskWrapperContent, "# %DDS_USER_ENVIRONMENT%", "source " + slot->m_sUsrEnv);
        }
        // JOB SCRIPT --- Task Executable
        boost::replace_all(sTaskWrapperContent, "# %DDS_USER_TASK%", sUsrExe);

        std::ofstream fTaskWrapperOut(pathTaskWrapper.native());
        if (!fTaskWrapperOut.is_open())
            throw runtime_error("Failed to create task wrapper script.");

        fTaskWrapperOut << sTaskWrapperContent;
        fTaskWrapperOut.flush();
        fTaskWrapperOut.close();

        // Apply execute access on the wrapper script
        fs::permissions(pathTaskWrapper, fs::add_perms | fs::owner_all);

        stringstream ssCmd;
        ssCmd << bp::search_path("bash").string();
        ssCmd << " -c \" " << pathTaskWrapper.native() << " \"";

        // retrieve the user defined access permissions
        string sAccessPermissions = CUserDefaults::instance().getValueForKey("agent.access_permissions");
        if (!sAccessPermissions.empty())
            LOG(info) << "Using user defined access permissions on output files. Mode: " << sAccessPermissions;

        pidUsrTask = execute(ssCmd.str(), sTaskStdOut, sTaskStdErr, &sAccessPermissions);
    }
    catch (exception& _e)
    {
        LOG(error) << _e.what();
        // Send response back to server
        pushMsg<cmdREPLY>(SReplyCmd(_e.what(), (uint16_t)SReplyCmd::EStatusCode::ERROR, 0, cmdACTIVATE_USER_TASK),
                          _sender.m_ID);
        return true;
    }

    stringstream ss;
    ss << "User task (pid:" << pidUsrTask << ") is activated.";
    LOG(info) << ss.str();

    onNewUserTask(slot->m_id, pidUsrTask);

    // Send response back to server
    pushMsg<cmdREPLY>(SReplyCmd(ss.str(), (uint16_t)SReplyCmd::EStatusCode::OK, 0, cmdACTIVATE_USER_TASK),
                      _sender.m_ID);

    return true;
}

bool CCommanderChannel::on_cmdSTOP_USER_TASK(SCommandAttachmentImpl<cmdSTOP_USER_TASK>::ptr_t /*_attachment*/,
                                             SSenderInfo& _sender)
{
    try
    {
        LOG(info) << "Received a STOP_USER_TASK request, slot id = " << _sender.m_ID;
        SSlotInfo::SSlotInfoPtr_t slot = getSlotInfoById(_sender.m_ID);

        if (slot->m_taskID == 0)
        {
            // No running tasks, nothing to stop
            // Send response back to server
            LOG(info) << "No tasks is running. Nothing to stop. Slot id = " << _sender.m_ID;
            pushMsg<cmdREPLY>(SReplyCmd("No tasks is running. Nothing to stop.",
                                        (uint16_t)SReplyCmd::EStatusCode::OK,
                                        0,
                                        cmdSTOP_USER_TASK),
                              _sender.m_ID);
            return true;
        }

        if (slot->m_pid > 0)
        {
            // Prevent blocking of the current thread.
            // The term-kill logic is posted to a different free thread in the queue.
            LOG(info) << "Scheduling a task stop for Slot id = " << _sender.m_ID << "; pid = " << slot->m_pid;
            m_ioContext.post(
                [this, slot, id = _sender.m_ID]
                {
                    terminateChildrenProcesses(
                        slot->m_pid,
                        [this, id]()
                        {
                            // Once child termination is finished, send User task "Done" to the commander
                            pushMsg<cmdREPLY>(
                                SReplyCmd("Done", (uint16_t)SReplyCmd::EStatusCode::OK, 0, cmdSTOP_USER_TASK), id);
                        });
                });
        }
    }
    catch (exception& _e)
    {
        LOG(warning) << "Can't find user task on slot " << _sender.m_ID << ": " << _e.what();
    }

    return true;
}

bool CCommanderChannel::on_cmdADD_SLOT(SCommandAttachmentImpl<cmdADD_SLOT>::ptr_t _attachment, SSenderInfo& /*_sender*/)
{
    LOG(info) << "Received a ADD SLOT request, id = " << _attachment->m_id;

    // Add new Task slot
    SSlotInfo::SSlotInfoPtr_t info = make_shared<SSlotInfo::SSlotInfoPtr_t::element_type>();
    info->m_id = _attachment->m_id;

    // TODO: catch exception if directory can not be created
    fs::path dir(CUserDefaults::instance().getSlotsRootDir());
    dir /= to_string(info->m_id);
    fs::create_directories(dir);

    // Add shared memory output for intercom API task
    m_intercomChannel->addOutput(info->m_id, CUserDefaults::instance().getSMLeaderOutputName(info->m_id));

    {
        lock_guard<mutex> lock(m_mutexSlots);
        m_slots.insert(make_pair(info->m_id, info));
    }

    // Confirm to commander the new slot
    SIDCmd msg_cmd;
    msg_cmd.m_id = info->m_id;
    pushMsg<cmdREPLY_ADD_SLOT>(msg_cmd);

    return true;
}

bool CCommanderChannel::on_cmdUPDATE_KEY(SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment, SSenderInfo& _sender)
{
    LOG(debug) << "Received key value update: " << *_attachment;

    // Forward message to user task
    m_intercomChannel->pushMsg<cmdUPDATE_KEY>(*_attachment, _sender.m_ID, _sender.m_ID);

    return true;
}

bool CCommanderChannel::on_cmdCUSTOM_CMD(SCommandAttachmentImpl<cmdCUSTOM_CMD>::ptr_t _attachment, SSenderInfo& _sender)
{
    LOG(debug) << "Received custom command: " << *_attachment;

    // Forward message to user task
    m_intercomChannel->pushMsg<cmdCUSTOM_CMD>(*_attachment, _sender.m_ID, _sender.m_ID);

    return true;
}

void CCommanderChannel::onNewUserTask(uint64_t _slotID, pid_t _pid)
{
    // watchdog
    LOG(info) << "Adding user task on slot " << _slotID << " with pid " << _pid << " to tasks queue";
    try
    {
        // Add a new task to the watchdog list
        SSlotInfo::SSlotInfoPtr_t slot = getSlotInfoById(_slotID);
        slot->m_pid = _pid;
    }
    catch (exception& _e)
    {
        LOG(fatal) << "Can't add new user task on slot " << _slotID << " to the list of children: " << _e.what();
    }

    // Register the user task's watchdog

    LOG(info) << "Starting the watchdog for user task on slot " << _slotID << " pid = " << _pid;

    auto self(shared_from_this());
    CMonitoringThread::instance().registerCallbackFunction(
        [this, self, _slotID, _pid]() -> bool
        {
            // Send commander server the watchdog heartbeat.
            // It indicates that the agent is executing a task and is not idle
            pushMsg<cmdWATCHDOG_HEARTBEAT>();
            CMonitoringThread::instance().updateIdle();

            try
            {
                // NOTE: We don't use boost::process because it returned an evaluated exit status, but we need a raw to
                // be able to detect how exactly the child exited.
                // boost::process  only checks that the child ended because of a call to ::exit() and does not check for
                // exiting via signal (WIFSIGNALED()).

                // We must call "wait" to check exist status of a child process, otherwise we will crate a
                // zombie :)
                int status;
                pid_t ret = ::waitpid(_pid, &status, WNOHANG | WUNTRACED);
                if (ret < 0)
                {
                    switch (errno)
                    {
                        case ECHILD:
                            LOG(error) << "Watchdog " << _slotID
                                       << ": The process or process group specified by pid "
                                          "does not exist or is not a child of the calling process.";
                            break;
                        case EFAULT:
                            LOG(error) << "Watchdog " << _slotID << ": stat_loc is not a writable address.";
                            break;
                        case EINTR:
                            LOG(error) << "Watchdog " << _slotID
                                       << ": The function was interrupted by a signal. The "
                                          "value of the location pointed to by stat_loc is undefined.";
                            break;
                        case EINVAL:
                            LOG(error) << "Watchdog " << _slotID << ": The options argument is not valid.";
                            break;
                        case ENOSYS:
                            LOG(error) << "Watchdog " << _slotID
                                       << ": pid specifies a process group (0 or less than "
                                          "-1), which is not currently supported.";
                            break;
                    }
                    LOG(info) << "User Tasks on slot " << _slotID
                              << " cannot be found. Probably it has exited. pid = " << _pid;
                    LOG(info) << "Stopping the watchdog for slot " << _slotID << " pid = " << _pid;

                    taskExited(_slotID, 0);

                    return false;
                }
                else if (ret == _pid)
                {
                    if (WIFEXITED(status))
                    {
                        // NOTE: We are using a bash wrapper script for user tasks.
                        // According to bash, the exist status of child processes can be interpreted in the following
                        // way:
                        // - For the shell’s purposes, a command which exits with a zero exit status has succeeded.
                        // - A non-zero exit status indicates failure.
                        //   This seemingly counter-intuitive scheme is used so there is one well-defined way to
                        //   indicate success and a variety of ways to indicate various failure modes. When a command
                        //   terminates on a fatal signal whose number is N, Bash uses the value 128+N as the exit
                        //   status.
                        // - If a command is not found, the child process created to execute it returns a status of 127.
                        // - If a command is found but is not executable, the return status is 126.
                        if (WEXITSTATUS(status) <= 128)
                            LOG(info) << "User task on slot " << _slotID << " exited"
                                      << (WCOREDUMP(status) ? " and dumped core" : "") << " with status "
                                      << WEXITSTATUS(status);
                        else
                            LOG(info) << "User task on slot " << _slotID << " killed by signal "
                                      << (WEXITSTATUS(status) - 128);
                    }
                    else if (WIFSTOPPED(status))
                        LOG(info) << "User task on slot " << _slotID << " stopped by signal " << WSTOPSIG(status);
                    else if (WIFSIGNALED(status))
                        LOG(info) << "User task on slot " << _slotID << " killed by signal " << WTERMSIG(status)
                                  << (WCOREDUMP(status) ? "; (core dumped)" : "");
                    else
                        LOG(info) << "User task on slot " << _slotID << " exited with unexpected status: " << status;

                    LOG(info) << "Stopping the watchdog for slot " << _slotID << " pid = " << _pid;

                    // Note: The value from WEXITSTATUS(status) is valid only if WIFEXITED returned true.
                    LOG(info) << "slot = " << _slotID << " pid = " << _pid
                              << " - done; exit status = " << WEXITSTATUS(status);

                    taskExited(_slotID, status);
                    return false;
                }
            }
            catch (exception& _e)
            {
                LOG(fatal) << "User processe monitoring thread received an exception: " << _e.what();
            }

            return true;
        },
        chrono::seconds(5));

    LOG(info) << "Watchdog for task on slot " << _slotID << " pid = " << _pid << " has been registered.";
}

void CCommanderChannel::enumChildProcesses(pid_t _forPid, CCommanderChannel::stringContainer_t& _children)
{
    CCommanderChannel::stringContainer_t tmpContainer;
    try
    {
        // a pgrep command is used to find out the list of child processes of the task
        stringstream ssCmd;
        ssCmd << bp::search_path("pgrep").string() << " -P " << _forPid;
        string output;
        execute(ssCmd.str(), chrono::seconds(5), &output);
        boost::split(tmpContainer, output, boost::is_any_of(" \n"), boost::token_compress_on);
        tmpContainer.erase(
            remove_if(tmpContainer.begin(), tmpContainer.end(), [](const string& _val) { return _val.empty(); }),
            tmpContainer.end());

        if (tmpContainer.empty())
            return;

        // Add all found children
        _children.insert(_children.end(), tmpContainer.begin(), tmpContainer.end());

        // Look for child processes of the children
        for (const auto& i : tmpContainer)
        {
            pid_t pid{ 0 };
            try
            {
                pid = stoi(i);
            }
            catch (invalid_argument& _e)
            {
                LOG(error) << "Invalid pid: " << i;
                continue;
            }
            enumChildProcesses(pid, _children);
        }
    }
    catch (...)
    {
    }
}

void CCommanderChannel::terminateChildrenProcesses(
    pid_t _parentPid, const CCommanderChannel::terminateChildrenOnComplete_t& _onCompleteSlot)
{
    // terminate all child processes of the given parent
    // Either tasks or all processes of the agent.
    pid_t mainPid((_parentPid > 0) ? _parentPid : getpid());

    LOG(info) << "Stopping child processes for parent pid " << mainPid;

    LOG(info) << "Getting a list of child processes of the " << (_parentPid > 0 ? "task" : "agent") << " with pid "
              << mainPid;

    stringContainer_t vecChildren;
    enumChildProcesses(mainPid, vecChildren);

    // the mainPid is never included to the list
    // In case of the agent the reason is obvious.
    // In case of a task, since it is running via the DDS task wrapper it will exit automatically once children are out
    string sChildren;
    pidContainer_t pidChildren;
    for (const auto& i : vecChildren)
    {
        pid_t pid{ 0 };
        try
        {
            pid = stoi(i);
            pidChildren.push_back(pid);
        }
        catch (invalid_argument& _e)
        {
            LOG(error) << "Can't insert pid: " << i;
        }

        if (!sChildren.empty())
            sChildren += ", ";
        sChildren += i;
    }
    LOG(info) << "The parent process " << mainPid << " has " << vecChildren.size()
              << " children: " << (sChildren.empty() ? "." : " " + sChildren);

    LOG(info) << "Sending graceful terminate signal to child processes.";
    for (const auto i : pidChildren)
    {
        LOG(info) << "Sending graceful terminate signal to child process " << i;
        kill(i, SIGTERM);
    }
    // 5 seconds timeout until sending the final sigkill
    chrono::steady_clock::time_point tpWaitUntil(chrono::steady_clock::now() + chrono::milliseconds(5000));

    LOG(info) << "Wait for children of " << mainPid << " to exit...";

    // create async timer
    timerPtr_t timer = make_unique<timer_t>(m_ioContext, chrono::milliseconds(100));

    // Prevent blocking of the current thread.
    // The term-kill logic is posted to a different free thread in the queue.
    // To prevent this algorithm to spin too fast and block the thread pool, we put it on a short timer.
    auto self(this->shared_from_this());
    timer->async_wait([this, self, pidChildren, tpWaitUntil, _onCompleteSlot, timer{ std::move(timer) }](
                          const boost::system::error_code& _error) mutable
                      { terminateChildrenProcesses(timer, pidChildren, tpWaitUntil, _onCompleteSlot, _error); });
}

void CCommanderChannel::terminateChildrenProcesses(
    CCommanderChannel::timerPtr_t& _timer,
    const CCommanderChannel::pidContainer_t& _children,
    const chrono::steady_clock::time_point& _wait_until,
    const CCommanderChannel::terminateChildrenOnComplete_t& _onCompleteSlot,
    const boost::system::error_code& _error)
{
    bool bAllDone(true);
    for (auto const& pid : _children)
    {
        if (pid > 0 && IsProcessRunning(pid))
        {
            bAllDone = false;
            break;
        }
    }

    if (bAllDone)
    {
        LOG(info) << "All child processes have exited.";
        // Report back to the user of the API that termination is completed
        if (_onCompleteSlot != nullptr)
            _onCompleteSlot();
        return;
    }

    auto duration = chrono::duration_cast<chrono::milliseconds>(_wait_until - chrono::steady_clock::now());
    if (!_error && duration.count() > 0)
    {
        // Prevent blocking of the current thread.
        // The term-kill logic is posted to a different free thread in the queue.
        // To prevent this algorithm to spin too fast and block the thread pool, we put it on a short timer.
        auto self(this->shared_from_this());
        _timer->async_wait([this, self, _children, _wait_until, _onCompleteSlot, timer{ std::move(_timer) }](
                               const boost::system::error_code& _error) mutable
                           { terminateChildrenProcesses(timer, _children, _wait_until, _onCompleteSlot, _error); });
    }
    else
    {
        // kill all child process of tasks if there are any
        // We do it before terminating tasks to give the parent task processes a chance to read state of children -
        // otherwise we will get zombies if user tasks don't manage their children properly
        LOG(info) << "Timeout is reached. Sending unconditional kill signal to all existing child processes...";
        for (auto const& pid : _children)
        {
            if (!IsProcessRunning(pid))
                continue;

            LOG(info) << "Child process with pid = " << pid << " will be forced to exit...";
            kill(pid, SIGKILL);
        }

        // Report back to the user of the API that termination is completed
        if (_onCompleteSlot != nullptr)
            _onCompleteSlot();
    }
}

void CCommanderChannel::taskExited(uint64_t _slotID, int _exitCode)
{
    // remove pid from the active children list
    try
    {
        // Add a new task to the watchdog list
        SSlotInfo::SSlotInfoPtr_t slot = getSlotInfoById(_slotID);

        {
            lock_guard<mutex> lock(m_taskIDToSlotIDMapMutex);
            m_taskIDToSlotIDMap.erase(slot->m_taskID);
        }

        // Remove tasks assets
        for (const auto& asset : slot->m_taskAssets)
        {
            try
            {
                if (fs::exists(asset) && fs::is_regular_file(asset))
                    fs::remove(asset);
            }
            catch (...)
            {
            }
        }
        slot->m_taskAssets.clear();

        // Save values before we reset them
        SUserTaskDoneCmd cmd;
        cmd.m_exitCode = _exitCode;
        cmd.m_taskID = slot->m_taskID;

        // reset slot info
        slot->m_pid = 0;
        slot->m_taskID = 0;

        // Draining the Intercom write queue
        m_intercomChannel->drainWriteQueue(true, _slotID);

        // Notify DDS commander
        pushMsg<cmdUSER_TASK_DONE>(cmd, _slotID);
    }
    catch (exception& _e)
    {
        LOG(fatal) << "Failed to remove user task on slot " << _slotID << " from the list of children: " << _e.what();
        LOG(error) << "Can't send TASK_DONE. The corrsponding slot is missing";
    }
}

dds::agent_cmd::SSlotInfo::SSlotInfoPtr_t CCommanderChannel::getSlotInfoById(const slotId_t& _slotID)
{
    lock_guard<mutex> lock(m_mutexSlots);
    auto it = m_slots.find(_slotID);
    if (it == m_slots.end())
    {
        stringstream ss;
        ss << "No matching slot for " << _slotID;
        throw runtime_error(ss.str());
    }

    return it->second;
}

void CCommanderChannel::stopChannel()
{
    CConditionEvent waitCondition;
    // terminate external children processes (like user tasks, for example)
    terminateChildrenProcesses(0, [&waitCondition]() { waitCondition.notifyAll(); });

    // wait for termination to finish
    // either it is finished or we proceed in 30 sec in anyway
    waitCondition.waitUntil(std::chrono::system_clock::now() + std::chrono::seconds(30));

    if (m_intercomChannel)
        m_intercomChannel->stop();

    // Stop Resource monitor
    m_resourceMonitorTimer->cancel();

    // stop channel
    stop();
}

void CCommanderChannel::send_cmdUPDATE_KEY(const SSenderInfo& _sender,
                                           SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment)
{
    LOG(debug) << "Received a key update notifications: " << *_attachment;

    try
    {
        string propertyName(_attachment->m_propertyName);
        uint64_t taskID(_attachment->m_senderTaskID);

        // Store the local pointer to the topology in order to keep it in memory.
        // If the global m_topo goes out of scope, for instance, during the topology update, old topology will still be
        // in memory.
        CTopoCore::Ptr_t topo{ nullptr };
        {
            lock_guard<mutex> lock(m_topoMutex);
            topo = m_topo;
        }

        CTopoTask::Ptr_t task;
        {
            lock_guard<mutex> lock(m_topoMutex);
            task = topo->getRuntimeTaskById(taskID).m_task;
        }
        auto property = task->getProperty(propertyName);
        // Property doesn't exists for task
        if (property == nullptr)
        {
            stringstream ss;
            ss << "Can't propagate property <" << propertyName << "> that doesn't exist for task <" << task->getName()
               << ">";
            m_intercomChannel->pushMsg<cmdSIMPLE_MSG>(
                SSimpleMsgCmd(ss.str(), error, cmdUPDATE_KEY), _sender.m_ID, _sender.m_ID);
            return;
        }
        // Cant' propagate property with read access type
        if (property->getAccessType() == CTopoProperty::EAccessType::READ)
        {
            stringstream ss;
            ss << "Can't propagate property <" << property->getName() << "> which has a READ access type for task <"
               << task->getName() << ">";
            m_intercomChannel->pushMsg<cmdSIMPLE_MSG>(
                SSimpleMsgCmd(ss.str(), error, cmdUPDATE_KEY), _sender.m_ID, _sender.m_ID);
            return;
        }
        // Can't send property with a collection scope if a task is outside a collection
        if ((property->getScopeType() == CTopoProperty::EScopeType::COLLECTION) &&
            (task->getParent()->getType() != CTopoBase::EType::COLLECTION))
        {
            stringstream ss;
            ss << "Can't propagate property <" << property->getName()
               << "> which has a COLLECTION scope type but task <" << task->getName() << "> is not in any collection";
            m_intercomChannel->pushMsg<cmdSIMPLE_MSG>(
                SSimpleMsgCmd(ss.str(), error, cmdUPDATE_KEY), _sender.m_ID, _sender.m_ID);
            return;
        }

        STopoRuntimeTask::FilterIteratorPair_t taskIt;
        {
            lock_guard<mutex> lock(m_topoMutex);
            taskIt = topo->getRuntimeTaskIteratorForPropertyName(propertyName, taskID);
        }

        for (auto it = taskIt.first; it != taskIt.second; ++it)
        {
            uint64_t receiverTaskID(it->first);

            // Dont't send message to itself
            if (taskID == receiverTaskID)
                continue;

            CTopoTask::Ptr_t task;
            {
                lock_guard<mutex> lock(m_topoMutex);
                task = topo->getRuntimeTaskById(receiverTaskID).m_task;
            }

            auto property = task->getProperty(propertyName);
            if (property != nullptr && (property->getAccessType() == CTopoProperty::EAccessType::READ ||
                                        property->getAccessType() == CTopoProperty::EAccessType::READWRITE))
            {
                SUpdateKeyCmd cmd;
                cmd.m_propertyName = propertyName;
                cmd.m_value = _attachment->m_value;
                cmd.m_receiverTaskID = receiverTaskID;
                cmd.m_senderTaskID = taskID;

                bool localPush(true);
                uint64_t slotID(0);
                {
                    lock_guard<mutex> lock(m_taskIDToSlotIDMapMutex);
                    auto it = m_taskIDToSlotIDMap.find(receiverTaskID);
                    localPush = (it != m_taskIDToSlotIDMap.end());
                    slotID = it->second;
                }

                if (localPush)
                {
                    LOG(debug) << "Push update key via shared memory: cmd=<" << cmd << ">; slotID=" << slotID;
                    m_intercomChannel->pushMsg<cmdUPDATE_KEY>(cmd, slotID, slotID);
                }
                else
                {
                    LOG(debug) << "Push update key via network channel: <" << cmd
                               << ">; protocolHeaderID=" << _sender.m_ID;
                    this->pushMsg<cmdUPDATE_KEY>(cmd, _sender.m_ID);
                }

                LOG(debug) << "Property update from agent channel: <" << cmd << ">";
            }
        }
    }
    catch (exception& _e)
    {
        LOG(error) << "Failed to update key: " << _e.what();
    }
}

bool CCommanderChannel::on_cmdUSER_TASK_DONE(SCommandAttachmentImpl<cmdUSER_TASK_DONE>::ptr_t _attachment,
                                             SSenderInfo& /*_sender*/)
{
    LOG(debug) << "Received user task done: " << *_attachment;

    // Forward message to user task
    // WORKAROUND: to prevent locking the container on msg push, we create a tmp container with slot IDs
    vector<slotId_t> slotsTmp;
    {
        lock_guard<mutex> lock(m_mutexSlots);
        transform(m_slots.begin(),
                  m_slots.end(),
                  back_inserter(slotsTmp),
                  boost::bind(&SSlotInfo::container_t::value_type::first, boost::placeholders::_1));
    }

    for (const auto& i : slotsTmp)
    {
        m_intercomChannel->pushMsg<cmdUSER_TASK_DONE>(*_attachment, i, i);
    }

    return true;
}

bool CCommanderChannel::isLowDiskSpace(uintmax_t* _available)
{
    try
    {
        fs::space_info diskSpaceInfo = fs::space(fs::path(CUserDefaults::getDDSPath()));

        if (_available != nullptr)
            *_available = diskSpaceInfo.available;

        // In MB
        unsigned long nLimit = stoul(CUserDefaults::instance().getValueForKey("agent.disk_space_threshold"));
        // Ignore the filter when threshold = 0
        if (nLimit == 0)
            return false;

        return ((diskSpaceInfo.available / g_bytesInMB) < nLimit);
    }
    catch (exception& _e)
    {
        LOG(error) << "Failed to get available disk space: " << _e.what();
    }
    return false;
}
