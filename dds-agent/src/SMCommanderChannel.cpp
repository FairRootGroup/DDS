// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "SMCommanderChannel.h"
#include "BOOST_FILESYSTEM.h"
#include "ProtocolDef.h"
#include "dds_env_prop.h"
#include "version.h"
// MiscCommon
#include "Process.h"
// BOOST
#include <boost/crc.hpp>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-register"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#pragma clang diagnostic pop
#include <boost/algorithm/string/replace.hpp>
#include <boost/process.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>

using namespace MiscCommon;
using namespace dds;
using namespace dds::user_defaults_api;
using namespace dds::protocol_api;
using namespace std;
namespace fs = boost::filesystem;
namespace bp = boost::process;

CSMCommanderChannel::CSMCommanderChannel(boost::asio::io_service& _service,
                                         const string& _inputName,
                                         const string& _outputName,
                                         uint64_t _protocolHeaderID,
                                         EMQOpenType _inputOpenType,
                                         EMQOpenType _outputOpenType)
    : CBaseSMChannelImpl<CSMCommanderChannel>(
          _service, _inputName, _outputName, _protocolHeaderID, _inputOpenType, _outputOpenType)
    , m_id()
    , m_taskID(0)
    , m_taskIndex(0)
    , m_collectionIndex(numeric_limits<uint32_t>::max())
    , m_taskPath()
    , m_groupName()
    , m_collectionName()
    , m_taskName()
    , m_activateMutex()
{
    registerHandler<EChannelEvents::OnSMStart>([this](const SSenderInfo& _sender) {
        pushMsg<cmdLOBBY_MEMBER_INFO>(
            SSimpleMsgCmd(getInputName(), info, cmdLOBBY_MEMBER_INFO), _sender.m_ID, EOutputID::Leader);
    });
}

CSMCommanderChannel::~CSMCommanderChannel()
{
    removeMessageQueue();
}

bool CSMCommanderChannel::on_cmdLOBBY_MEMBER_INFO_OK(
    protocol_api::SCommandAttachmentImpl<protocol_api::cmdLOBBY_MEMBER_INFO_OK>::ptr_t _attachment,
    protocol_api::SSenderInfo& _sender)
{
    LOG(info) << "Received confirmation from lobby leader";
    LOG(info) << "Sending handshake to commander with PHID: " << this->m_protocolHeaderID;

    // Prepare a hand shake message
    SVersionCmd cmd;
    cmd.m_channelType = EChannelType::AGENT;
    cmd.m_sSID = CUserDefaults::instance().getSID();
    cmd.m_version = DDS_PROTOCOL_VERSION;
    pushMsg<cmdLOBBY_MEMBER_HANDSHAKE>(cmd, this->m_protocolHeaderID);

    return true;
}

bool CSMCommanderChannel::on_cmdLOBBY_MEMBER_INFO_ERR(
    protocol_api::SCommandAttachmentImpl<protocol_api::cmdLOBBY_MEMBER_INFO_ERR>::ptr_t _attachment,
    protocol_api::SSenderInfo& _sender)
{
    LOG(error) << "Received error from lobby leader: " << _attachment->m_sMsg;
    return true;
}

bool CSMCommanderChannel::on_cmdREPLY_LOBBY_MEMBER_HANDSHAKE_OK(
    protocol_api::SCommandAttachmentImpl<protocol_api::cmdREPLY_LOBBY_MEMBER_HANDSHAKE_OK>::ptr_t _attachment,
    protocol_api::SSenderInfo& _sender)
{
    LOG(info) << "SM: Handshake successfull. PHID: " << this->m_protocolHeaderID;
    return true;
}

bool CSMCommanderChannel::on_cmdREPLY_LOBBY_MEMBER_HANDSHAKE_ERR(
    protocol_api::SCommandAttachmentImpl<protocol_api::cmdREPLY_LOBBY_MEMBER_HANDSHAKE_ERR>::ptr_t _attachment,
    protocol_api::SSenderInfo& _sender)
{
    LOG(fatal) << "SM: Handshake failed. PHID: " << this->m_protocolHeaderID;
    return false;
}

bool CSMCommanderChannel::on_cmdSIMPLE_MSG(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment,
                                           SSenderInfo& _sender)
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

bool CSMCommanderChannel::on_cmdGET_HOST_INFO(SCommandAttachmentImpl<cmdGET_HOST_INFO>::ptr_t _attachment,
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

bool CSMCommanderChannel::on_cmdSHUTDOWN(SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment, SSenderInfo& _sender)
{
    deleteAgentIDFile();
    LOG(info) << "The SM Agent [" << m_id << "] received cmdSHUTDOWN.";
    // return false to let connection manager to catch this message as well
    return false;
}

bool CSMCommanderChannel::on_cmdBINARY_ATTACHMENT_RECEIVED(
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
        }
        default:
            LOG(debug) << "Received command cmdBINARY_ATTACHMENT_RECEIVED does not have a listener";
            return true;
    }

    return true;
}

bool CSMCommanderChannel::on_cmdGET_ID(SCommandAttachmentImpl<cmdGET_ID>::ptr_t _attachment, SSenderInfo& _sender)
{
    LOG(info) << "cmdGET_ID received from DDS Server";

    // If file exist return id from file.
    // If file does not exist than return uuid_nil.

    const string sAgentIDFile(CUserDefaults::instance().getAgentIDFile());
    if (file_exists(sAgentIDFile))
    {
        readAgentIDFile();
    }
    else
    {
        m_id = 0;
    }

    SIDCmd msg_cmd;
    msg_cmd.m_id = m_id;
    pushMsg<cmdREPLY_ID>(msg_cmd);

    return true;
}

bool CSMCommanderChannel::on_cmdSET_ID(SCommandAttachmentImpl<cmdSET_ID>::ptr_t _attachment, SSenderInfo& _sender)
{
    LOG(info) << "cmdSET_ID attachment [" << *_attachment << "] from DDS Server";

    m_id = _attachment->m_id;

    createAgentIDFile();

    return true;
}

bool CSMCommanderChannel::on_cmdGET_LOG(SCommandAttachmentImpl<cmdGET_LOG>::ptr_t _attachment, SSenderInfo& _sender)
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
        pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(e.what(), error, cmdGET_LOG));
    }

    return true;
}

void CSMCommanderChannel::readAgentIDFile()
{
    const string sAgentIDFile(CUserDefaults::getAgentIDFile());
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

void CSMCommanderChannel::createAgentIDFile() const
{
    const string sAgentIDFile(CUserDefaults::getAgentIDFile());
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

void CSMCommanderChannel::deleteAgentIDFile() const
{
    const string sAgentIDFile(CUserDefaults::getAgentIDFile());
    if (sAgentIDFile.empty())
        return;

    // TODO: check error code
    unlink(sAgentIDFile.c_str());
}

bool CSMCommanderChannel::on_cmdASSIGN_USER_TASK(SCommandAttachmentImpl<cmdASSIGN_USER_TASK>::ptr_t _attachment,
                                                 SSenderInfo& _sender)
{
    // Mutex is used to garantee that cmdASSIGN_USER_TASK and cmdACTIVATE_AGENT are not executed at the same time.
    // Note that mutex doesn't garantee that cmdASSIGN_USER_TASK is executed before cmdACTIVATE_AGENT this can be
    // implemented later using condition_variable.
    lock_guard<mutex> lock(m_activateMutex);

    LOG(info) << "Received a user task assignment. " << *_attachment;
    m_sUsrExe = _attachment->m_sExeFile;
    m_taskID = _attachment->m_taskID;
    m_taskIndex = _attachment->m_taskIndex;
    m_collectionIndex = _attachment->m_collectionIndex;
    m_taskPath = _attachment->m_taskPath;
    m_groupName = _attachment->m_groupName;
    m_collectionName = _attachment->m_collectionName;
    m_taskName = _attachment->m_taskName;

    // Replace all %taskIndex% and %collectionIndex% in executable path with their values.
    boost::algorithm::replace_all(m_sUsrExe, "%taskIndex%", to_string(m_taskIndex));
    if (m_collectionIndex != numeric_limits<uint32_t>::max())
        boost::algorithm::replace_all(m_sUsrExe, "%collectionIndex%", to_string(m_collectionIndex));

    return true;
}

bool CSMCommanderChannel::on_cmdACTIVATE_AGENT(SCommandAttachmentImpl<cmdACTIVATE_AGENT>::ptr_t _attachment,
                                               SSenderInfo& _sender)
{
    // See comment in on_cmdASSIGN_USER_TASK for details.
    lock_guard<mutex> lock(m_activateMutex);

    string sUsrExe(m_sUsrExe);
    smart_path(&sUsrExe);

    if (sUsrExe.empty())
    {
        LOG(info) << "Received activation command. Ignoring the command, since no task is assigned.";
        // Send response back to server
        pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd("No task is assigned. Activation is ignored.", info, cmdACTIVATE_AGENT));
        return true;
    }

    StringVector_t params;
    pid_t pidUsrTask(0);

    try
    {
        // set task's environment
        LOG(info) << "Setting up task's environment: "
                  << "DDS_TASK_ID:" << m_taskID << " DDS_TASK_INDEX:" << m_taskIndex
                  << " DDS_COLLECTION_INDEX:" << m_collectionIndex << " DDS_TASK_PATH:" << m_taskPath
                  << " DDS_GROUP_NAME:" << m_groupName << " DDS_COLLECTION_NAME:" << m_collectionName
                  << " DDS_TASK_NAME:" << m_taskName
                  << " DDS_SESSION_ID: " << dds::env_prop<dds::EEnvProp::dds_session_id>();
        if (::setenv("DDS_TASK_ID", to_string(m_taskID).c_str(), 1) == -1)
            throw MiscCommon::system_error("Failed to set up $DDS_TASK_ID");
        if (::setenv("DDS_TASK_INDEX", to_string(m_taskIndex).c_str(), 1) == -1)
            throw MiscCommon::system_error("Failed to set up $DDS_TASK_INDEX");
        if (m_collectionIndex != numeric_limits<uint32_t>::max())
            if (::setenv("DDS_COLLECTION_INDEX", to_string(m_collectionIndex).c_str(), 1) == -1)
                throw MiscCommon::system_error("Failed to set up $DDS_COLLECTION_INDEX");
        if (::setenv("DDS_TASK_PATH", m_taskPath.c_str(), 1) == -1)
            throw MiscCommon::system_error("Failed to set up $DDS_TASK_PATH");
        if (::setenv("DDS_GROUP_NAME", m_groupName.c_str(), 1) == -1)
            throw MiscCommon::system_error("Failed to set up $DDS_GROUP_NAME");
        if (::setenv("DDS_COLLECTION_NAME", m_collectionName.c_str(), 1) == -1)
            throw MiscCommon::system_error("Failed to set up $DDS_COLLECTION_NAME");
        if (::setenv("DDS_TASK_NAME", m_taskName.c_str(), 1) == -1)
            throw MiscCommon::system_error("Failed to set up $DDS_TASK_NAME");

        // execute the task
        LOG(info) << "Executing user task: " << sUsrExe;

        // Task output files: <user_task_name>_<datetime>_<task_id>_<out/err>.log
        stringstream ssTaskOutput;
        ssTaskOutput << CUserDefaults::getDDSPath() << m_taskName;

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
        ssTaskOutput << "_" << m_taskID;

        string sTaskStdOut(ssTaskOutput.str() + "_out.log");
        string sTaskStdErr(ssTaskOutput.str() + "_err.log");

        pidUsrTask = execute(sUsrExe, sTaskStdOut, sTaskStdErr);
    }
    catch (exception& _e)
    {
        LOG(error) << _e.what();
        // Send response back to server
        pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(_e.what(), error, cmdACTIVATE_AGENT));
        return true;
    }

    stringstream ss;
    ss << "User task (pid:" << pidUsrTask << ") is activated.";
    LOG(info) << ss.str();

    dispatchHandlers<>(EChannelEvents::OnNewUserTask, _sender, pidUsrTask);

    // Send response back to server
    pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(ss.str(), info, cmdACTIVATE_AGENT));

    return true;
}

bool CSMCommanderChannel::on_cmdSTOP_USER_TASK(SCommandAttachmentImpl<cmdSTOP_USER_TASK>::ptr_t _attachment,
                                               SSenderInfo& _sender)
{
    if (m_taskID == 0)
    {
        // No running tasks, nothing to stop
        // Send response back to server
        pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd("No tasks is running. Nothing to stop.", info, cmdSTOP_USER_TASK));
        return true;
    }

    // Let the parent should terminate user tasks
    return false;
}

bool CSMCommanderChannel::on_cmdUPDATE_KEY(SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment,
                                           SSenderInfo& _sender)
{
    LOG(debug) << "Received a key update notifications: " << *_attachment;
    return false;
}

bool CSMCommanderChannel::on_cmdUPDATE_KEY_ERROR(SCommandAttachmentImpl<cmdUPDATE_KEY_ERROR>::ptr_t _attachment,
                                                 SSenderInfo& _sender)
{
    LOG(debug) << "Received a key update error notifications: " << *_attachment;
    return false;
}

bool CSMCommanderChannel::on_cmdDELETE_KEY(SCommandAttachmentImpl<cmdDELETE_KEY>::ptr_t _attachment,
                                           SSenderInfo& _sender)
{
    LOG(debug) << "Received a key delete notifications: " << *_attachment;
    return false;
}

bool CSMCommanderChannel::on_cmdCUSTOM_CMD(SCommandAttachmentImpl<cmdCUSTOM_CMD>::ptr_t _attachment,
                                           SSenderInfo& _sender)
{
    LOG(debug) << "Received custom command: " << *_attachment;
    return false;
}
