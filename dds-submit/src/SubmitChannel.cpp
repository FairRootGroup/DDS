// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "SubmitChannel.h"
// BOOST
#include <boost/filesystem/operations.hpp>

using namespace MiscCommon;
using namespace dds;
using namespace dds::submit_cmd;
using namespace dds::protocol_api;
using namespace std;

CSubmitChannel::CSubmitChannel(boost::asio::io_service& _service)
    : CClientChannelImpl<CSubmitChannel>(_service, EChannelType::UI)
{
    subscribeOnEvent(EChannelEvents::OnRemoteEndDissconnected, [](CSubmitChannel* _channel) {
        LOG(MiscCommon::log_stderr) << "Server has closed the connection.";
    });

    subscribeOnEvent(EChannelEvents::OnHandshakeOK, [this](CSubmitChannel* _channel) {
        if (m_sRMS != "localhost")
        {
            // Create the command's attachment
            SSubmitCmd cmd;
            cmd.m_sRMSType = m_sRMS;
            cmd.m_sCfgFile = m_sCfgFile;
            cmd.m_nNumberOfAgents = m_number;
            pushMsg<cmdSUBMIT>(cmd);
        }
        else // localhost RMS requires special treatment
        {
            // Create temporary ssh configuration
            const char* tmpdir = std::getenv("TMPDIR");
            if (nullptr == tmpdir)
            {
                LOG(MiscCommon::log_stderr) << "$TMPDIR is not defined. To be able to use \'localhost\' please "
                                               "define location of the temporary directory using TMPDIR "
                                               "environment variable.";
                return;
            }
            LOG(MiscCommon::log_stdout_clean) << "Using \'" << tmpdir << "\' to spawn agents.";
            boost::filesystem::path tmpfileName(tmpdir);
            tmpfileName /= "dds_ssh.cfg";
            ofstream f(tmpfileName.string());

            string userName;
            MiscCommon::get_cuser_name(&userName);

            boost::filesystem::path workingDirectoryPath(tmpdir);
            workingDirectoryPath /= "dds-agents";
            if (!boost::filesystem::exists(workingDirectoryPath) &&
                !boost::filesystem::create_directory(workingDirectoryPath))
            {
                LOG(MiscCommon::log_stderr) << "Can't create working directory: " << workingDirectoryPath.string();
                return;
            }

            stringstream ss;
            ss << "wn, " << userName << "@localhost, ," << workingDirectoryPath.string() << ", "
               << (m_number ? m_number : std::thread::hardware_concurrency());

            LOG(MiscCommon::log_stdout_clean) << "SSH plug-in config:\n" << ss.str();

            f << ss.str();

            f.close();

            // Create the command's attachment
            SSubmitCmd cmd;
            cmd.m_sRMSType = m_sRMS;
            cmd.m_sCfgFile = tmpfileName.string();
            pushMsg<cmdSUBMIT>(cmd);
        }
    });

    subscribeOnEvent(EChannelEvents::OnConnected, [](CSubmitChannel* _channel) {
        LOG(MiscCommon::log_stdout) << "Connection established.";
        LOG(MiscCommon::log_stdout) << "Requesting server to process job submission...";
    });

    subscribeOnEvent(EChannelEvents::OnFailedToConnect,
                     [](CSubmitChannel* _channel) { LOG(MiscCommon::log_stdout) << "Failed to connect."; });
}

void CSubmitChannel::setCfgFile(const string& _val)
{
    m_sCfgFile = _val;
}

void CSubmitChannel::setRMSType(const string& _val)
{
    m_sRMS = _val;
}

void CSubmitChannel::setNumber(const size_t _val)
{
    m_number = _val;
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
