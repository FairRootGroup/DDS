// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "ConnectionManager.h"
#include "Topology.h"
// BOOST
#include <boost/filesystem.hpp>
#include <boost/crc.hpp>

using namespace dds;
using namespace std;
using namespace MiscCommon;
namespace fs = boost::filesystem;

CConnectionManager::CConnectionManager(const SOptions_t& _options,
                                       boost::asio::io_service& _io_service,
                                       boost::asio::ip::tcp::endpoint& _endpoint)
    : CConnectionManagerImpl<CAgentChannel, CConnectionManager>(_options, _io_service, _endpoint)
{
}

CConnectionManager::~CConnectionManager()
{
}

void CConnectionManager::newClientCreated(CAgentChannel::connectionPtr_t _newClient)
{
    // Subscribe on protocol messages
    _newClient->registerMessageHandler(
        cmdGET_LOG,
        [this](CProtocolMessage::protocolMessagePtr_t _msg, CAgentChannel* _channel) -> bool
        {
            return this->on_cmdGET_LOG(_msg, useRawPtr(_channel));
        });

    _newClient->registerMessageHandler(
        cmdBINARY_ATTACHMENT_LOG,
        [this](CProtocolMessage::protocolMessagePtr_t _msg, CAgentChannel* _channel) -> bool
        {
            return this->on_cmdBINARY_ATTACHMENT_LOG(_msg, useRawPtr(_channel));
        });

    _newClient->registerMessageHandler(
        cmdGET_LOG_ERROR,
        [this](CProtocolMessage::protocolMessagePtr_t _msg, CAgentChannel* _channel) -> bool
        {
            return this->on_cmdGET_LOG_ERROR(_msg, useRawPtr(_channel));
        });

    _newClient->registerMessageHandler(
        cmdGET_AGENTS_INFO,
        [this](CProtocolMessage::protocolMessagePtr_t _msg, CAgentChannel* _channel) -> bool
        {
            return this->agentsInfoHandler(_msg, useRawPtr(_channel));
        });

    _newClient->registerMessageHandler(
        cmdSUBMIT,
        [this](CProtocolMessage::protocolMessagePtr_t _msg, CAgentChannel* _channel) -> bool
        {
            return this->on_cmdSUBMIT(_msg, useRawPtr(_channel));
        });

    _newClient->registerMessageHandler(
        cmdSUBMIT_START,
        [this](CProtocolMessage::protocolMessagePtr_t _msg, CAgentChannel* _channel) -> bool
        {
            return this->on_cmdSUBMIT_START(_msg, useRawPtr(_channel));
        });

    _newClient->registerMessageHandler(
        cmdSTART_DOWNLOAD_TEST,
        [this](CProtocolMessage::protocolMessagePtr_t _msg, CAgentChannel* _channel) -> bool
        {
            return this->on_cmdSTART_DOWNLOAD_TEST(_msg, useRawPtr(_channel));
        });

    _newClient->registerMessageHandler(
        cmdDOWNLOAD_TEST_STAT,
        [this](CProtocolMessage::protocolMessagePtr_t _msg, CAgentChannel* _channel) -> bool
        {
            return this->on_cmdDOWNLOAD_TEST_STAT(_msg, useRawPtr(_channel));
        });

    _newClient->registerMessageHandler(
        cmdDOWNLOAD_TEST_ERROR,
        [this](CProtocolMessage::protocolMessagePtr_t _msg, CAgentChannel* _channel) -> bool
        {
            return this->on_cmdDOWNLOAD_TEST_ERROR(_msg, useRawPtr(_channel));
        });

    _newClient->registerMessageHandler(
        cmdSIMPLE_MSG,
        [this](CProtocolMessage::protocolMessagePtr_t _msg, CAgentChannel* _channel) -> bool
        {
            return this->on_cmdSIMPLE_MSG(_msg, useRawPtr(_channel));
        });
}

bool CConnectionManager::on_cmdGET_LOG(CProtocolMessage::protocolMessagePtr_t _msg,
                                       CAgentChannel::weakConnectionPtr_t _channel)
{
    std::lock_guard<std::mutex> lock(m_getLog.m_mutexStart);
    try
    {

        if (!m_getLog.m_channel.expired())
        {
            SSimpleMsgCmd cmd;
            cmd.m_sMsg = "Can not process the request. dds-getlog already in progress.";
            CProtocolMessage::protocolMessagePtr_t msg = make_shared<CProtocolMessage>();
            msg->encodeWithAttachment<cmdGET_LOG_FATAL>(cmd);
            auto p = _channel.lock();
            p->pushMsg(msg);
            return true;
        }
        m_getLog.m_channel = _channel;
        auto p = m_getLog.m_channel.lock();
        m_getLog.zeroCounters();

        // Create directory to store logs
        const string sLogStorageDir(CUserDefaults::instance().getAgentLogStorageDir());
        fs::path dir(sLogStorageDir);
        if (!fs::exists(dir) && !fs::create_directory(dir))
        {
            SSimpleMsgCmd cmd;
            cmd.m_sMsg = "Could not create directory " + sLogStorageDir + " to save log files.";
            CProtocolMessage::protocolMessagePtr_t pm = make_shared<CProtocolMessage>();
            pm->encodeWithAttachment<cmdGET_LOG_FATAL>(cmd);
            p->pushMsg(pm);

            m_getLog.m_channel.reset();

            return true;
        }

        CAgentChannel::weakConnectionPtrVector_t channels(
            getChannels([](CAgentChannel::connectionPtr_t _v)
                        {
                            return (_v->getType() == EAgentChannelType::AGENT && _v->started());
                        }));

        m_getLog.m_nofRequests = channels.size();

        // Send messages to all agents
        for (const auto& v : channels)
        {
            if (v.expired())
                continue;
            auto ptr = v.lock();

            CProtocolMessage::protocolMessagePtr_t msg = make_shared<CProtocolMessage>();
            msg->encode<cmdGET_LOG>();
            ptr->pushMsg(msg);
        }

        if (m_getLog.m_nofRequests == 0)
        {
            SSimpleMsgCmd cmd;
            cmd.m_sMsg = "There are no connecting agents.";
            CProtocolMessage::protocolMessagePtr_t pm = make_shared<CProtocolMessage>();
            pm->encodeWithAttachment<cmdGET_LOG_FATAL>(cmd);
            p->pushMsg(pm);
        }
    }
    catch (bad_weak_ptr& e)
    {
        // TODO: Do we need to log something here?
    }
    return true;
}

bool CConnectionManager::on_cmdBINARY_ATTACHMENT_LOG(CProtocolMessage::protocolMessagePtr_t _msg,
                                                     CAgentChannel::weakConnectionPtr_t _channel)
{
    try
    {
        SBinaryAttachmentCmd recieved_cmd;
        recieved_cmd.convertFromData(_msg->bodyToContainer());

        std::lock_guard<std::mutex> lock(m_getLog.m_mutexReceive);

        m_getLog.m_nofReceived++;
        stringstream ss;
        auto ptrChannel = _channel.lock();
        ss << m_getLog.nofReceived() << "/" << m_getLog.m_nofRequests << " [" << ptrChannel->getId() << "] -> "
           << recieved_cmd.m_fileName;

        SSimpleMsgCmd cmd;
        cmd.m_sMsg = ss.str();

        CProtocolMessage::protocolMessagePtr_t msg = make_shared<CProtocolMessage>();
        msg->encodeWithAttachment<cmdLOG_RECIEVED>(cmd);
        auto p = m_getLog.m_channel.lock();
        p->syncPushMsg(msg);

        checkAllLogsReceived();
    }
    catch (bad_weak_ptr& e)
    {
        // TODO: Do we need to log something here?
    }

    return true;
}

bool CConnectionManager::on_cmdGET_LOG_ERROR(CProtocolMessage::protocolMessagePtr_t _msg,
                                             CAgentChannel::weakConnectionPtr_t _channel)
{
    try
    {
        SSimpleMsgCmd recieved_cmd;
        recieved_cmd.convertFromData(_msg->bodyToContainer());

        std::lock_guard<std::mutex> lock(m_getLog.m_mutexReceive);

        m_getLog.m_nofReceivedErrors++;
        stringstream ss;
        auto ptrChannel = _channel.lock();
        ss << m_getLog.nofReceived() << "/" << m_getLog.m_nofRequests << " Error from agent [" << ptrChannel->getId()
           << "]: " << recieved_cmd.m_sMsg;

        SSimpleMsgCmd cmd;
        cmd.m_sMsg = ss.str();

        CProtocolMessage::protocolMessagePtr_t msg = make_shared<CProtocolMessage>();
        msg->encodeWithAttachment<cmdGET_LOG_ERROR>(cmd);
        auto p = m_getLog.m_channel.lock();
        p->syncPushMsg(msg);

        checkAllLogsReceived();
    }
    catch (bad_weak_ptr& e)
    {
        // TODO: Do we need to log something here?
    }

    return true;
}

void CConnectionManager::checkAllLogsReceived()
{
    if (m_getLog.allReceived())
    {
        stringstream ss;
        ss << "recieved: " << m_getLog.nofReceived() << ", total: " << m_getLog.m_nofRequests
           << ", errors: " << m_getLog.m_nofReceivedErrors;

        SSimpleMsgCmd cmd;
        cmd.m_sMsg = ss.str();

        CProtocolMessage::protocolMessagePtr_t msg = make_shared<CProtocolMessage>();
        msg->encodeWithAttachment<cmdALL_LOGS_RECIEVED>(cmd);
        try
        {
            auto p = m_getLog.m_channel.lock();
            p->pushMsg(msg);

            m_getLog.m_channel.reset();
        }
        catch (bad_weak_ptr& e)
        {
            // TODO: Do we need to log something here?
        }
    }
}

bool CConnectionManager::on_cmdSUBMIT(CProtocolMessage::protocolMessagePtr_t _msg,
                                      CAgentChannel::weakConnectionPtr_t _channel)
{
    try
    {
        SSubmitCmd cmd;
        cmd.convertFromData(_msg->bodyToContainer());

        auto p = _channel.lock();

        if (cmd.m_nRMSTypeCode == SSubmitCmd::SSH)
        {
            LOG(info) << "SSH RMS is defined by: [" << cmd.m_sSSHCfgFile << "]";

            // TODO: Job submission should be moved from here to a thread
            // Resolve topology
            CTopology topology;
            m_sCurrentTopoFile = cmd.m_sTopoFile;
            topology.init(m_sCurrentTopoFile);
            // TODO: Compare number of job slots in the ssh (in case of ssh) config file to what topo wants from us.

            // Submitting the job
            string outPut;
            string sCommand("$DDS_LOCATION/bin/dds-ssh");
            smart_path(&sCommand);
            StringVector_t params;
            const size_t nCmdTimeout = 35; // in sec.
            params.push_back("-c" + cmd.m_sSSHCfgFile);
            params.push_back("submit");
            try
            {
                do_execv(sCommand, params, nCmdTimeout, &outPut);

                SSimpleMsgCmd msg_cmd;
                msg_cmd.m_sMsg = "Dummy job info, JOBIds";
                CProtocolMessage::protocolMessagePtr_t msg = make_shared<CProtocolMessage>();
                msg->encodeWithAttachment<cmdREPLY_SUBMIT_OK>(msg_cmd);
                p->pushMsg(msg);
            }
            catch (exception& e)
            {
                string sMsg("Failed to process the task: ");
                sMsg += cmd.m_sTopoFile;
                throw runtime_error(sMsg);
            }
            if (!outPut.empty())
            {
                ostringstream ss;
                ss << "Cmnd Output: " << outPut;
                LOG(info) << ss.str();
                SSimpleMsgCmd msg_cmd;
                msg_cmd.m_sMsg = ss.str();
                CProtocolMessage::protocolMessagePtr_t msg = make_shared<CProtocolMessage>();
                msg->encodeWithAttachment<cmdSIMPLE_MSG>(msg_cmd);
                p->pushMsg(msg);
            }
        }
    }
    catch (bad_weak_ptr& e)
    {
        // TODO: Do we need to log something here?
    }
    catch (exception& e)
    {
        SSimpleMsgCmd msg_cmd;
        msg_cmd.m_sMsg = e.what();
        CProtocolMessage::protocolMessagePtr_t msg = make_shared<CProtocolMessage>();
        msg->encodeWithAttachment<cmdREPLY_ERR_SUBMIT>(msg_cmd);
        if (!_channel.expired())
        {
            auto p = _channel.lock();
            p->pushMsg(msg);
        }
    }

    return true;
}

bool CConnectionManager::on_cmdSUBMIT_START(CProtocolMessage::protocolMessagePtr_t _msg,
                                            CAgentChannel::weakConnectionPtr_t _channel)
{
    // Start distirbuiting user tasks between agents
    // TODO: We might need to create a thread here to avoid blocking a thread of the transport
    CTopology topology;
    topology.init(m_sCurrentTopoFile);

    // remember the UI channel, which requested to submit the job
    m_chSubmitUI = _channel;

    try
    {
        // Start distirbuiting user tasks between agents
        // TODO: We might need to create a thread here to avoid blocking a thread of the transport
        CTopology topology;
        topology.init(m_sCurrentTopoFile);

        // Send binaries of user jobs to all active agents.
        // Send activate signal to all agents. This will trigger start of user jobs on the agents.
        CAgentChannel::weakConnectionPtrVector_t channels(
            getChannels([](CAgentChannel::connectionPtr_t _v)
                        {
                            return (_v->getType() == EAgentChannelType::AGENT && _v->started());
                        }));

        for (const auto& v : channels)
        {
            if (v.expired())
                continue;
            auto ptr = v.lock();

            // Assgin user's tasks to agents
            SAssignUserTaskCmd msg_cmd;
            msg_cmd.m_sExeFile = "/Users/anar/Test.sh";
            CProtocolMessage::protocolMessagePtr_t msg = make_shared<CProtocolMessage>();
            msg->encodeWithAttachment<cmdASSIGN_USER_TASK>(msg_cmd);
            ptr->pushMsg(msg);

            // Active agents.
            ptr->pushMsg<cmdACTIVATE_AGENT>();
        }
    }
    catch (bad_weak_ptr& e)
    {
        // TODO: Do we need to log something here?
    }
    return true;
}

bool CConnectionManager::agentsInfoHandler(CProtocolMessage::protocolMessagePtr_t _msg,
                                           CAgentChannel::weakConnectionPtr_t _channel)
{
    try
    {
        SAgentsInfoCmd cmd;
        stringstream ss;

        CAgentChannel::weakConnectionPtrVector_t channels(
            getChannels([](CAgentChannel::connectionPtr_t _v)
                        {
                            return (_v->getType() == EAgentChannelType::AGENT && _v->started());
                        }));

        for (const auto& v : channels)
        {
            if (v.expired())
                continue;
            auto ptr = v.lock();

            ++cmd.m_nActiveAgents;
            ss << ptr->getId() << " " << ptr->getRemoteHostInfo().m_username << "@" << ptr->getRemoteHostInfo().m_host
               << ":" << ptr->getRemoteHostInfo().m_DDSPath << " (pid:" << ptr->getRemoteHostInfo().m_agentPid << ")\n";
        }
        cmd.m_sListOfAgents = ss.str();

        CProtocolMessage::protocolMessagePtr_t msg = make_shared<CProtocolMessage>();
        msg->encodeWithAttachment<cmdREPLY_AGENTS_INFO>(cmd);
        if (!_channel.expired())
        {
            auto p = _channel.lock();
            p->pushMsg(msg);
        }
    }
    catch (bad_weak_ptr& e)
    {
        // TODO: Do we need to log something here?
    }

    return true;
}

bool CConnectionManager::on_cmdSTART_DOWNLOAD_TEST(CProtocolMessage::protocolMessagePtr_t _msg,
                                                   CAgentChannel::weakConnectionPtr_t _channel)
{
    try
    {
        std::lock_guard<std::mutex> lock(m_downloadTest.m_mutexStart);

        if (!m_downloadTest.m_channel.expired())
        {
            SSimpleMsgCmd cmd;
            cmd.m_sMsg = "Can not process the request. dds-test already in progress.";
            CProtocolMessage::protocolMessagePtr_t msg = make_shared<CProtocolMessage>();
            msg->encodeWithAttachment<cmdDOWNLOAD_TEST_FATAL>(cmd);
            auto p = _channel.lock();
            p->pushMsg(msg);
            return true;
        }
        m_downloadTest.m_channel = _channel;
        m_downloadTest.zeroCounters();

        CAgentChannel::weakConnectionPtrVector_t channels(
            getChannels([](CAgentChannel::connectionPtr_t _v)
                        {
                            return (_v->getType() == EAgentChannelType::AGENT && _v->started());
                        }));

        m_downloadTest.m_nofRequests = 2 * channels.size();

        // Send messages to aganets
        for (const auto& v : channels)
        {
            if (v.expired())
                continue;
            auto ptr = v.lock();

            sendTestBinaryAttachment(1000, ptr);
            sendTestBinaryAttachment(10000, ptr);
            sendTestBinaryAttachment(1000, ptr);
            sendTestBinaryAttachment(100000, ptr);
            sendTestBinaryAttachment(1000, ptr);
            sendTestBinaryAttachment(1000000, ptr);
            sendTestBinaryAttachment(1000, ptr);
            sendTestBinaryAttachment(10000000, ptr);
            sendTestBinaryAttachment(1000, ptr);
        }

        if (m_downloadTest.m_nofRequests == 0)
        {
            SSimpleMsgCmd cmd;
            cmd.m_sMsg = "There are no active agents.";
            CProtocolMessage::protocolMessagePtr_t pm = make_shared<CProtocolMessage>();
            pm->encodeWithAttachment<cmdDOWNLOAD_TEST_FATAL>(cmd);
            if (!m_downloadTest.m_channel.expired())
            {
                auto p = m_downloadTest.m_channel.lock();
                p->pushMsg(pm);
            }
        }
    }
    catch (bad_weak_ptr& e)
    {
        // TODO: Do we need to log something here?
    }

    return true;
}

void CConnectionManager::sendTestBinaryAttachment(size_t _binarySize, CAgentChannel::connectionPtr_t _channel)
{
    SBinaryAttachmentCmd cmd;

    for (size_t i = 0; i < _binarySize; ++i)
    {
        // char c = rand() % 256;
        char c = 1;
        cmd.m_fileData.push_back(c);
    }

    // Calculate CRC32 of the test file data
    boost::crc_32_type crc;
    crc.process_bytes(&cmd.m_fileData[0], cmd.m_fileData.size());

    cmd.m_crc32 = crc.checksum();
    cmd.m_fileName = "test_data_" + std::to_string(_binarySize) + ".bin";
    cmd.m_fileSize = cmd.m_fileData.size();

    CProtocolMessage::protocolMessagePtr_t msg = make_shared<CProtocolMessage>();
    msg->encodeWithAttachment<cmdDOWNLOAD_TEST>(cmd);
    _channel->pushMsg(msg);
}

bool CConnectionManager::on_cmdDOWNLOAD_TEST_STAT(CProtocolMessage::protocolMessagePtr_t _msg,
                                                  CAgentChannel::weakConnectionPtr_t _channel)
{
    SBinaryDownloadStatCmd recieved_cmd;
    recieved_cmd.convertFromData(_msg->bodyToContainer());

    {
        std::lock_guard<std::mutex> lock(m_downloadTest.m_mutexReceive);

        ++m_downloadTest.m_nofReceived;
        stringstream ss;
        auto p = _channel.lock();
        float downloadTime = 0.000001 * recieved_cmd.m_downloadTime; // micros->s
        float speed = (downloadTime != 0.) ? 0.001 * recieved_cmd.m_recievedFileSize / downloadTime : 0;
        ss << m_downloadTest.nofReceived() << "/" << m_downloadTest.m_nofRequests << " [" << p->getId()
           << "]: " << recieved_cmd.m_recievedFileSize << " bytes in " << downloadTime << " s (" << speed << " KB/s)";

        m_downloadTestStat.m_totalReceived += recieved_cmd.m_recievedFileSize;
        m_downloadTestStat.m_totalTime += recieved_cmd.m_downloadTime;

        SSimpleMsgCmd cmd;
        cmd.m_sMsg = ss.str();
        CProtocolMessage::protocolMessagePtr_t msg = make_shared<CProtocolMessage>();
        msg->encodeWithAttachment<cmdDOWNLOAD_TEST_RECIEVED>(cmd);
        if (!m_downloadTest.m_channel.expired())
        {
            auto pDownloadUI = m_downloadTest.m_channel.lock();
            pDownloadUI->pushMsg(msg);
        }

        checkAllDownloadTestsReceived();
    }

    return true;
}

bool CConnectionManager::on_cmdDOWNLOAD_TEST_ERROR(CProtocolMessage::protocolMessagePtr_t _msg,
                                                   CAgentChannel::weakConnectionPtr_t _channel)
{
    SSimpleMsgCmd recieved_cmd;
    recieved_cmd.convertFromData(_msg->bodyToContainer());

    {
        std::lock_guard<std::mutex> lock(m_downloadTest.m_mutexReceive);

        m_downloadTest.m_nofReceivedErrors++;
        stringstream ss;
        auto p = _channel.lock();
        ss << m_downloadTest.nofReceived() << "/" << m_downloadTest.m_nofRequests << " Error from agent [" << p->getId()
           << "]: " << recieved_cmd.m_sMsg;

        SSimpleMsgCmd cmd;
        cmd.m_sMsg = ss.str();

        CProtocolMessage::protocolMessagePtr_t msg = make_shared<CProtocolMessage>();
        msg->encodeWithAttachment<cmdDOWNLOAD_TEST_ERROR>(cmd);
        if (!m_downloadTest.m_channel.expired())
        {
            auto pDownloadUI = m_downloadTest.m_channel.lock();
            pDownloadUI->syncPushMsg(msg);
        }

        checkAllDownloadTestsReceived();
    }

    return true;
}

void CConnectionManager::checkAllDownloadTestsReceived()
{
    if (m_downloadTest.allReceived())
    {
        stringstream ss;
        ss << "recieved: " << m_downloadTest.nofReceived() << ", total: " << m_downloadTest.m_nofRequests
           << ", errors: " << m_downloadTest.m_nofReceivedErrors << " | ";

        float downloadTime = 0.000001 * m_downloadTestStat.m_totalTime; // micros->s
        float speed = (downloadTime != 0.) ? 0.001 * m_downloadTestStat.m_totalReceived / downloadTime : 0;
        ss << "download " << m_downloadTestStat.m_totalReceived << " bytes in " << downloadTime << " s (" << speed
           << " KB/s)";

        SSimpleMsgCmd cmd;
        cmd.m_sMsg = ss.str();

        CProtocolMessage::protocolMessagePtr_t msg = make_shared<CProtocolMessage>();
        msg->encodeWithAttachment<cmdALL_DOWNLOAD_TESTS_RECIEVED>(cmd);
        if (!m_downloadTest.m_channel.expired())
        {
            auto pDownloadUI = m_downloadTest.m_channel.lock();
            pDownloadUI->pushMsg(msg);

            m_downloadTest.m_channel.reset();
        }
    }
}

bool CConnectionManager::on_cmdSIMPLE_MSG(CProtocolMessage::protocolMessagePtr_t _msg,
                                          CAgentChannel::weakConnectionPtr_t _channel)
{
    SSimpleMsgCmd cmd;
    cmd.convertFromData(_msg->bodyToContainer());

    switch (cmd.m_srcCommand)
    {
        case cmdACTIVATE_AGENT:
            if (!m_chSubmitUI.expired())
            {
                auto p = m_chSubmitUI.lock();
                p->pushMsg(_msg);
                // close connection
                // p->pushMsg<cmdSHUTDOWN>();
                m_chSubmitUI.reset();
            }
            return true; // let others to process this message
    }
    return false;
}
