// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "CommanderChannel.h"
#include "UserDefaults.h"
#include "version.h"
#include "BOOST_FILESYSTEM.h"
// MiscCommon
#include "FindCfgFile.h"
// BOOST
#include <boost/crc.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

using namespace MiscCommon;
using namespace dds;
using namespace std;
namespace fs = boost::filesystem;

CCommanderChannel::CCommanderChannel(boost::asio::io_service& _service)
    : CConnectionImpl<CCommanderChannel>(_service)
    , m_isHandShakeOK(false)
{
}

void CCommanderChannel::onHeaderRead()
{
    m_headerReadTime = std::chrono::steady_clock::now();
}

bool CCommanderChannel::on_cmdREPLY_HANDSHAKE_OK(const CProtocolMessage& _msg)
{
    m_isHandShakeOK = true;

    return true;
}

bool CCommanderChannel::on_cmdSIMPLE_MSG(const CProtocolMessage& _msg)
{
    return true;
}

bool CCommanderChannel::on_cmdGET_HOST_INFO(const CProtocolMessage& _msg)
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

    CProtocolMessage msg;
    msg.encodeWithAttachment<cmdREPLY_HOST_INFO>(cmd);
    pushMsg(msg);

    return true;
}

bool CCommanderChannel::on_cmdDISCONNECT(const CProtocolMessage& _msg)
{
    stop();
    LOG(info) << "The Agent [" << m_id << "] disconnected...Bye";

    return true;
}

bool CCommanderChannel::on_cmdSHUTDOWN(const CProtocolMessage& _msg)
{
    stop();
    deleteAgentUUIDFile();
    LOG(info) << "The Agent [" << m_id << "] exited.";
    exit(EXIT_SUCCESS);

    return true;
}

bool CCommanderChannel::on_cmdBINARY_ATTACHMENT(const CProtocolMessage& _msg)
{
    SBinaryAttachmentCmd cmd;
    cmd.convertFromData(_msg.bodyToContainer());

    chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    chrono::milliseconds downloadTime = chrono::duration_cast<chrono::milliseconds>(now - m_headerReadTime);

    // Calculate CRC32 of the recieved file data
    boost::crc_32_type crc32;
    crc32.process_bytes(&cmd.m_fileData[0], cmd.m_fileData.size());

    if (crc32.checksum() == cmd.m_crc32)
    {
        // Do something if file is correctly downloaded
    }

    // Form reply command
    SBinaryDownloadStatCmd reply_cmd;
    reply_cmd.m_recievedCrc32 = crc32.checksum();
    reply_cmd.m_recievedFileSize = cmd.m_fileData.size();
    reply_cmd.m_downloadTime = downloadTime.count();

    CProtocolMessage msg;
    msg.encodeWithAttachment<cmdBINARY_DOWNLOAD_STAT>(reply_cmd);
    pushMsg(msg);

    return true;
}

bool CCommanderChannel::on_cmdGET_UUID(const CProtocolMessage& _msg)
{
    LOG(info) << "Recieved a cmdGET_UUID command from: " << socket().remote_endpoint().address().to_string();

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
    CProtocolMessage msg;
    msg.encodeWithAttachment<cmdREPLY_UUID>(msg_cmd);
    pushMsg(msg);

    return true;
}

bool CCommanderChannel::on_cmdSET_UUID(const CProtocolMessage& _msg)
{
    SUUIDCmd cmd;
    cmd.convertFromData(_msg.bodyToContainer());

    LOG(info) << "Recieved a cmdSET_UUID [" << cmd
              << "] command from: " << socket().remote_endpoint().address().to_string();

    m_id = cmd.m_id;

    createAgentUUIDFile();

    return true;
}

bool CCommanderChannel::on_cmdGET_LOG(const CProtocolMessage& _msg)
{
    LOG(info) << "Recieved a cmdGET_LOG command from: " << socket().remote_endpoint().address().to_string();

    try
    {
        string logDir(CUserDefaults::getDDSPath());

        string hostname;
        get_hostname(&hostname);

        std::time_t now = chrono::system_clock::to_time_t(chrono::system_clock::now());
        struct std::tm* ptm = std::localtime(&now);

        stringstream ss;
        ss << std::put_time(ptm, "%Y-%m-%d-%H-%M-%S") << "_" << hostname << "_" << m_id;

        string archiveName(ss.str());

        fs::path archiveDir(logDir + archiveName);
        if (!fs::exists(archiveDir) && !fs::create_directory(archiveDir))
        {
            string msg("Could not create directory: " + archiveDir.string());
            sendGetLogError(msg);
            return true;
        }

        vector<fs::path> logFiles;
        BOOSTHelper::get_files_by_extension(logDir, ".log", logFiles);

        for (const auto& v : logFiles)
        {
            fs::path dest(archiveDir.string() + "/" + v.filename().string());
            fs::copy_file(v, dest, fs::copy_option::overwrite_if_exists);
        }

        CFindCfgFile<string> cfg;
        cfg.SetOrder("/usr/bin/tar")("/usr/local/bin/tar")("/opt/local/bin/tar");
        string tarPath;
        cfg.GetCfg(&tarPath);

        string archiveDirName = logDir + archiveName;
        string archiveFileName = archiveDirName + ".tar.gz";
        StringVector_t params{ "czf", archiveFileName, "-C", logDir, archiveName };
        string output;

        do_execv(tarPath, params, 60, &output);

        SBinaryAttachmentCmd cmd;

        ifstream f(archiveFileName.c_str());
        if (!f.is_open() || !f.good())
        {
            string msg("Could not open archive with log files: " + archiveFileName);
            sendGetLogError(msg);
            return true;
        }
        f.seekg(0, ios::end);
        cmd.m_fileData.reserve(f.tellg());
        f.seekg(0, ios::beg);
        cmd.m_fileData.assign((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

        boost::crc_32_type crc;
        crc.process_bytes(&cmd.m_fileData[0], cmd.m_fileData.size());

        cmd.m_crc32 = crc.checksum();
        cmd.m_fileName = archiveName + ".tar.gz";
        cmd.m_fileSize = cmd.m_fileData.size();

        f.close();
        fs::remove(archiveFileName);
        fs::remove_all(archiveDirName);

        CProtocolMessage msg;
        msg.encodeWithAttachment<cmdBINARY_ATTACHMENT_LOG>(cmd);
        pushMsg(msg);
    }
    catch (exception& e)
    {
        LOG(info) << e.what();
        sendGetLogError(e.what());
    }

    return true;
}

void CCommanderChannel::sendGetLogError(const string& _msg)
{
    SSimpleMsgCmd cmd;
    cmd.m_sMsg = _msg;
    CProtocolMessage pm;
    pm.encodeWithAttachment<cmdGET_LOG_ERROR>(cmd);
    pushMsg(pm);
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
    stop();
    deleteAgentUUIDFile();
    LOG(info) << "The Agent [" << m_id << "] exited.";
    exit(EXIT_SUCCESS);
}
