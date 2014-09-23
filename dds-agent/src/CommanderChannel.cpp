// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "version.h"
#include "CommanderChannel.h"
#include "BOOST_FILESYSTEM.h"
// MiscCommon
#include "FindCfgFile.h"
// BOOST
#include <boost/crc.hpp>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-register"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#pragma clang diagnostic pop

using namespace MiscCommon;
using namespace dds;
using namespace std;
namespace fs = boost::filesystem;

CCommanderChannel::CCommanderChannel(boost::asio::io_service& _service)
    : CConnectionImpl<CCommanderChannel>(_service)
    , m_isHandShakeOK(false)
{
}

bool CCommanderChannel::on_cmdREPLY_HANDSHAKE_OK(SCommandAttachmentImpl<cmdREPLY_HANDSHAKE_OK>::ptr_t _attachment)
{
    m_isHandShakeOK = true;

    return true;
}

bool CCommanderChannel::on_cmdSIMPLE_MSG(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment)
{
    switch (_attachment->m_srcCommand)
    {
        case cmdTRANSPORT_TEST:
        {
            pushMsg<cmdSIMPLE_MSG>(*_attachment);
            return true;
        }

        default:
            LOG(debug) << "Received command cmdSIMPLE_MSG does not have a listener";
            return true;
    }

    return true;
}

bool CCommanderChannel::on_cmdGET_HOST_INFO(SCommandAttachmentImpl<cmdGET_HOST_INFO>::ptr_t _attachment)
{
    pid_t pid = getpid();

    SHostInfoCmd cmd;
    get_cuser_name(&cmd.m_username);
    get_hostname(&cmd.m_host);
    cmd.m_version = PROJECT_VERSION_STRING;
    cmd.m_DDSPath = CUserDefaults::getDDSPath();
    cmd.m_agentPort = 0;
    cmd.m_agentPid = pid;
    cmd.m_timeStamp = 0;

    // CProtocolMessage::protocolMessagePtr_t msg = make_shared<CProtocolMessage>();
    // msg->encodeWithAttachment<cmdREPLY_HOST_INFO>(cmd);
    // pushMsg(msg);

    pushMsg<cmdREPLY_HOST_INFO>(cmd);
    // pushMsg<cmdSIMPLE_MSG>(cmd);

    return true;
}

bool CCommanderChannel::on_cmdDISCONNECT(SCommandAttachmentImpl<cmdDISCONNECT>::ptr_t _attachment)
{
    LOG(info) << "The Agent [" << m_id << "] disconnected... Bye";
    stop();

    return true;
}

bool CCommanderChannel::on_cmdSHUTDOWN(SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment)
{
    deleteAgentUUIDFile();
    LOG(info) << "The Agent [" << m_id << "] exited.";
    stop();

    // return false to let connection manager to catch this message as weel
    return false;
}

bool CCommanderChannel::on_cmdBINARY_ATTACHMENT_RECEIVED(
    SCommandAttachmentImpl<cmdBINARY_ATTACHMENT_RECEIVED>::ptr_t _attachment)
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

        default:
            LOG(debug) << "Received command cmdBINARY_ATTACHMENT_RECEIVED does not have a listener";
            return true;
    }

    return true;
}

bool CCommanderChannel::on_cmdGET_UUID(SCommandAttachmentImpl<cmdGET_UUID>::ptr_t _attachment)
{
    // If file exist return uuid from file.
    // If file does not exist than return uuid_nil.

    const string sAgentUUIDFile(CUserDefaults::instance().getAgentUUIDFile());
    if (MiscCommon::file_exists(sAgentUUIDFile))
    {
        readAgentUUIDFile();
    }
    else
    {
        m_id = boost::uuids::nil_uuid();
    }

    SUUIDCmd msg_cmd;
    msg_cmd.m_id = m_id;
    pushMsg<cmdREPLY_UUID>(msg_cmd);

    return true;
}

bool CCommanderChannel::on_cmdSET_UUID(SCommandAttachmentImpl<cmdSET_UUID>::ptr_t _attachment)
{
    LOG(info) << "cmdSET_UUID attachment [" << *_attachment << "] from " << remoteEndIDString();

    m_id = _attachment->m_id;

    createAgentUUIDFile();

    return true;
}

bool CCommanderChannel::on_cmdGET_LOG(SCommandAttachmentImpl<cmdGET_LOG>::ptr_t _attachment)
{
    try
    {
        string logDir(CUserDefaults::getDDSPath());

        string hostname;
        get_hostname(&hostname);

        std::time_t now = chrono::system_clock::to_time_t(chrono::system_clock::now());
        struct std::tm* ptm = std::localtime(&now);
        char buffer[20];
        std::strftime(buffer, 20, "%Y-%m-%d-%H-%M-%S", ptm);

        stringstream ss;
        // We do not use put_time for the moment as gcc4.9 does not support it.
        // ss << std::put_time(ptm, "%Y-%m-%d-%H-%M-%S") << "_" << hostname << "_" << m_id;
        ss << buffer << "_" << hostname << "_" << m_id;

        string archiveName(ss.str());

        fs::path archiveDir(logDir + archiveName);
        if (!fs::exists(archiveDir) && !fs::create_directory(archiveDir))
        {
            string msg("Could not create directory: " + archiveDir.string());
            pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(msg, MiscCommon::error, cmdGET_LOG));
            return true;
        }

        vector<fs::path> logFiles;
        BOOSTHelper::get_files_by_extension(logDir, ".log", logFiles);

        for (const auto& v : logFiles)
        {
            fs::path dest(archiveDir.string() + "/" + v.filename().string());
            fs::copy_file(v, dest);
        }

        CFindCfgFile<string> cfg;
        cfg.SetOrder("/usr/bin/tar")("/usr/local/bin/tar")("/opt/local/bin/tar")("/bin/tar");
        string tarPath;
        cfg.GetCfg(&tarPath);

        string archiveDirName = logDir + archiveName;
        string archiveFileName = archiveDirName + ".tar.gz";
        stringstream ssCmd;
        ssCmd << tarPath << " czf " << archiveFileName << " -C" << logDir << " " << archiveName;
        string output;
        do_execv(ssCmd.str(), 60, &output);

        MiscCommon::BYTEVector_t data;

        ifstream f(archiveFileName.c_str());
        if (!f.is_open() || !f.good())
        {
            string msg("Could not open archive with log files: " + archiveFileName);
            pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(msg, MiscCommon::error, cmdGET_LOG));
            return true;
        }
        f.seekg(0, ios::end);
        data.reserve(f.tellg());
        f.seekg(0, ios::beg);
        data.assign((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

        string fileName(archiveName);
        fileName += ".tar.gz";

        pushBinaryAttachmentCmd(data, fileName, cmdGET_LOG);

        f.close();
        fs::remove(archiveFileName);
        fs::remove_all(archiveDirName);
    }
    catch (exception& e)
    {
        LOG(error) << e.what();
        pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(e.what(), MiscCommon::error, cmdGET_LOG));
    }

    return true;
}

void CCommanderChannel::readAgentUUIDFile()
{
    const string sAgentUUIDFile(CUserDefaults::getAgentUUIDFile());
    LOG(MiscCommon::info) << "Reading an agent UUID file: " << sAgentUUIDFile;
    ifstream f(sAgentUUIDFile.c_str());
    if (!f.is_open() || !f.good())
    {
        string msg("Could not open an agent UUID file: ");
        msg += sAgentUUIDFile;
        throw runtime_error(msg);
    }
    f >> m_id;
}

void CCommanderChannel::createAgentUUIDFile() const
{
    const string sAgentUUIDFile(CUserDefaults::getAgentUUIDFile());
    LOG(MiscCommon::info) << "Creating an agent UUID file: " << sAgentUUIDFile;
    ofstream f(sAgentUUIDFile.c_str());
    if (!f.is_open() || !f.good())
    {
        string msg("Could not open an agent UUID file: ");
        msg += sAgentUUIDFile;
        throw runtime_error(msg);
    }

    f << m_id;
}

void CCommanderChannel::deleteAgentUUIDFile() const
{
    const string sAgentUUIDFile(CUserDefaults::getAgentUUIDFile());
    if (sAgentUUIDFile.empty())
        return;

    // TODO: check error code
    unlink(sAgentUUIDFile.c_str());
}

void CCommanderChannel::onRemoteEndDissconnected()
{
    sendYourself<cmdSHUTDOWN>();
}

bool CCommanderChannel::on_cmdASSIGN_USER_TASK(SCommandAttachmentImpl<cmdASSIGN_USER_TASK>::ptr_t _attachment)
{
    LOG(MiscCommon::info) << "Recieved a user task assigment. " << *_attachment;
    m_sUsrExe = _attachment->m_sExeFile;
    return true;
}

bool CCommanderChannel::on_cmdACTIVATE_AGENT(SCommandAttachmentImpl<cmdACTIVATE_AGENT>::ptr_t _attachment)
{
    string sUsrExe(m_sUsrExe);
    smart_path(&sUsrExe);
    StringVector_t params;
    string output;
    pid_t pidUsrTask(0);

    try
    {
        LOG(MiscCommon::info) << "Executing user task: " << sUsrExe;
        pidUsrTask = do_execv(sUsrExe, 0, &output);
    }
    catch (exception& e)
    {
        LOG(error) << e.what();

        // Send response back to server
        SSimpleMsgCmd cmd;
        cmd.m_sMsg = e.what();
        cmd.m_msgSeverity = MiscCommon::error;
        cmd.m_srcCommand = cmdACTIVATE_AGENT;
        pushMsg<cmdSIMPLE_MSG>(cmd);
    }

    stringstream ss;
    ss << "User task (pid:" << pidUsrTask << ") is activated.";
    LOG(MiscCommon::info) << ss.str();

    m_onNewUserTaskCallback(pidUsrTask);

    // Send response back to server
    SSimpleMsgCmd cmd;
    cmd.m_sMsg = ss.str();
    cmd.m_msgSeverity = MiscCommon::info;
    cmd.m_srcCommand = cmdACTIVATE_AGENT;
    pushMsg<cmdSIMPLE_MSG>(cmd);

    return true;
}
