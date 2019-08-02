// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
// DDS
#include "IntercomServiceCore.h"
#include "UserDefaults.h"
#include "dds_env_prop.h"
// BOOST
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>

using namespace std;
using namespace dds;
using namespace dds::user_defaults_api;
using namespace dds::internal_api;
using namespace dds::protocol_api;
using namespace MiscCommon;

CIntercomServiceCore::CIntercomServiceCore()
    : m_started(false)
{
    try
    {
        CUserDefaults::instance(); // Initialize user defaults
        Logger::instance().init(); // Initialize log
    }
    catch (const std::exception& _e)
    {
        cerr << "DDSIntercomGuard: Error: " << _e.what();
    }
}

CIntercomServiceCore::~CIntercomServiceCore()
{
}

void CIntercomServiceCore::start(const std::string& _sessionID)
{
    if (m_started)
        return;

    m_started = true;

    try
    {
        // Choose transport type.
        // If we connect to agent, i.e. agent config file exists than we use shared memory transport.
        // If we connect to commander, i.e. commander config file exists we use asio transport.
        bool useSMTransport = CUserDefaults::instance().isAgentInstance();

        if (useSMTransport)
        {
            LOG(info) << "CCDDSIntercomGuard: using shared memory for transport";
            setupSMChannel();
        }
        else
        {
            LOG(info) << "CCDDSIntercomGuard: using TCP for transport";
            setupChannel(_sessionID);
        }
    }
    catch (exception& e)
    {
        m_started = false;
        string errorMsg("CCDDSIntercomGuard: error in the transport service: ");
        errorMsg += e.what();
        LOG(error) << errorMsg;
        execUserSignal(m_errorSignal, intercom_api::EErrorCode::TransportServiceFailed, errorMsg);
        return;
    }

    // Don't block main thread, start transport service on a thread-pool
    const int nConcurrentThreads(3);
    LOG(MiscCommon::info) << "Starting DDS transport engine using " << nConcurrentThreads << " concurrent threads.";
    for (int x = 0; x < nConcurrentThreads; ++x)
    {
        m_workerThreads.create_thread(boost::bind(&boost::asio::io_context::run, &(m_io_context)));
    }
}

void CIntercomServiceCore::setupSMChannel()
{
    // Shared memory channel for communication with DDS Agent
    std::string inputName = CUserDefaults::instance().getSMInputName();
    std::string outputName = CUserDefaults::instance().getSMOutputName();
    // For the user task shared memory names are opposite: input shared memory name has output name and output
    // shared
    // memory name has input name.
    m_SMChannel = CSMAgentChannel::makeNew(m_io_context, outputName, inputName, 0);

    // Subscribe for cmdUPDATE_KEY from SM channel
    m_SMChannel->registerHandler<cmdUPDATE_KEY>(
        [this](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment) {
            this->on_cmdUPDATE_KEY_SM(_sender, _attachment);
        });

    // Subscribe for cmdUSER_TASK_DONE from SM channel
    m_SMChannel->registerHandler<cmdUSER_TASK_DONE>(
        [this](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdUSER_TASK_DONE>::ptr_t _attachment) {
            this->on_cmdUSER_TASK_DONE_SM(_sender, _attachment);
        });

    // Subscribe for cmdCUSTOM_CMD from SM channel
    m_SMChannel->registerHandler<cmdCUSTOM_CMD>(
        [this](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdCUSTOM_CMD>::ptr_t _attachment) {
            this->on_cmdCUSTOM_CMD_SM(_sender, _attachment);
        });
    //

    // Subscribe for cmdSIMPLE_MSG from SM channel
    m_SMChannel->registerHandler<cmdSIMPLE_MSG>(
        [this](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment) {
            this->on_cmdSIMPLE_MSG_SM(_sender, _attachment);
        });
    //

    m_SMChannel->start();
    if (!m_SMChannel->started())
    {
        throw runtime_error("Failed to initialize SM channel");
    }
}

void CIntercomServiceCore::setupChannel(const std::string& _sessionID)
{
    // For plugins current SID is already set
    bool defaultExists = _sessionID.empty() && !CUserDefaults::instance().getCurrentSID().empty();
    if (!defaultExists)
    {
        // Try to convert sessionID string to boost::uuid
        const boost::uuids::uuid& sid = boost::uuids::string_generator()(_sessionID);
        // Reinit UserDefaults and Log with new session ID
        CUserDefaults::instance().reinit(sid, CUserDefaults::instance().currentUDFile());
        Logger::instance().reinit();
    }

    LOG(info) << "CCDDSIntercomGuard: is going to init agent connection";

    // Read server info file
    const string sCommanderCfg(CUserDefaults::instance().getServerInfoFileLocation());
    string sHost;
    string sPort;
    EChannelType channelType(EChannelType::UNKNOWN);
    if (fs::exists(sCommanderCfg))
    {
        LOG(info) << "Reading server info from: " << sCommanderCfg;
        boost::property_tree::ptree pt;
        boost::property_tree::ini_parser::read_ini(sCommanderCfg, pt);
        sHost = pt.get<string>("ui.host");
        sPort = pt.get<string>("ui.port");
        channelType = EChannelType::UI;
    }
    else
    {
        throw runtime_error("Cannot find DDS commander info file.");
    }

    LOG(info) << "Contacting DDS commander on " << sHost << ":" << sPort;

    // Resolve endpoint iterator from host and port
    boost::asio::ip::tcp::resolver resolver(m_io_context);
    boost::asio::ip::tcp::resolver::query query(sHost, sPort);
    boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

    // Create new communication channel and push handshake message
    m_channel = CAgentChannel::makeNew(m_io_context, 0);
    m_channel->setChannelType(channelType);

    // Subscribe for cmdSHUTDOWN from TCP channel
    m_channel->registerHandler<cmdSHUTDOWN>(
        [this](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment) { stop(); });
    //

    // Subscribe for cmdCUSTOM_CMD from TCP channel
    m_channel->registerHandler<cmdCUSTOM_CMD>(
        [this](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdCUSTOM_CMD>::ptr_t _attachment) {
            this->on_cmdCUSTOM_CMD_SM(_sender, _attachment);
        });
    //

    // Subscribe for cmdSIMPLE_MSG from TCP channel
    m_channel->registerHandler<cmdSIMPLE_MSG>(
        [this](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment) {
            this->on_cmdSIMPLE_MSG_SM(_sender, _attachment);
        });
    //

    m_channel->registerHandler<EChannelEvents::OnRemoteEndDissconnected>([this](const SSenderInfo& _sender) {
        execUserSignal(m_errorSignal, intercom_api::EErrorCode::RemoteEndDisconnected, "Remote end disconnected");
        stopCondition();
    });

    m_channel->registerHandler<protocol_api::EChannelEvents::OnFailedToConnect>([this](const SSenderInfo& _sender) {
        m_channel->reconnectAgentWithErrorHandler([this](const string& _errorMsg) {
            execUserSignal(m_errorSignal, intercom_api::EErrorCode::ConnectionFailed, _errorMsg);
            stopCondition();
        });
    });

    m_channel->connect(endpoint_iterator);
}

void CIntercomServiceCore::stop()
{
    if (!m_started)
        return;

    m_started = false;

    disconnectCustomCmd();
    disconnectKeyValue();

    if (m_channel != nullptr)
    {
        m_channel->stop();
    }

    if (m_SMChannel != nullptr)
    {
        m_SMChannel->stop();
    }

    m_io_context.stop();
    m_workerThreads.join_all();
}

connection_t CIntercomServiceCore::connectError(intercom_api::errorSignal_t::slot_function_type _subscriber)
{
    return m_errorSignal.connect(_subscriber);
}

connection_t CIntercomServiceCore::connectCustomCmd(customCmdSignal_t::slot_function_type _subscriber)
{
    return m_customCmdSignal.connect(_subscriber);
}

connection_t CIntercomServiceCore::connectCustomCmdReply(customCmdReplySignal_t::slot_function_type _subscriber)
{
    return m_customCmdReplySignal.connect(_subscriber);
}

connection_t CIntercomServiceCore::connectKeyValue(keyValueSignal_t::slot_function_type _subscriber)
{
    return m_keyValueUpdateSignal.connect(_subscriber);
}

connection_t CIntercomServiceCore::connectKeyValueTaskDone(keyValueTaskDoneSignal_t::slot_function_type _subscriber)
{
    return m_keyValueTaskDoneSignal.connect(_subscriber);
}

void CIntercomServiceCore::disconnectCustomCmd()
{
    // TODO: Thread safe disconnect?
    // disconnect custom command signals
    m_customCmdSignal.disconnect_all_slots();
    m_customCmdReplySignal.disconnect_all_slots();
    m_errorSignal.disconnect_all_slots();
}

void CIntercomServiceCore::disconnectKeyValue()
{
    // TODO: Thread safe disconnect?
    // disconnect key-value signals
    m_keyValueUpdateSignal.disconnect_all_slots();
    m_errorSignal.disconnect_all_slots();
}

// Messages from TCP and shared memory
void CIntercomServiceCore::on_cmdUPDATE_KEY_SM(
    const protocol_api::SSenderInfo& _sender,
    protocol_api::SCommandAttachmentImpl<protocol_api::cmdUPDATE_KEY>::ptr_t _attachment)
{
    execUserSignal(
        m_keyValueUpdateSignal, _attachment->m_propertyName, _attachment->m_value, _attachment->m_senderTaskID);
}

void CIntercomServiceCore::on_cmdUSER_TASK_DONE_SM(
    const protocol_api::SSenderInfo& _sender,
    protocol_api::SCommandAttachmentImpl<protocol_api::cmdUSER_TASK_DONE>::ptr_t _attachment)
{
    execUserSignal(m_keyValueTaskDoneSignal, _attachment->m_taskID, _attachment->m_exitCode);
}

void CIntercomServiceCore::on_cmdCUSTOM_CMD_SM(
    const protocol_api::SSenderInfo& _sender,
    protocol_api::SCommandAttachmentImpl<protocol_api::cmdCUSTOM_CMD>::ptr_t _attachment)
{
    LOG(info) << "Received custom command: " << *_attachment;
    execUserSignal(m_customCmdSignal, _attachment->m_sCmd, _attachment->m_sCondition, _attachment->m_senderId);
}

void CIntercomServiceCore::on_cmdSIMPLE_MSG_SM(
    const protocol_api::SSenderInfo& _sender,
    protocol_api::SCommandAttachmentImpl<protocol_api::cmdSIMPLE_MSG>::ptr_t _attachment)
{
    switch (_attachment->m_srcCommand)
    {
        case cmdCUSTOM_CMD:
            LOG(static_cast<ELogSeverityLevel>(_attachment->m_msgSeverity)) << _attachment->m_sMsg;
            execUserSignal(m_customCmdReplySignal, _attachment->m_sMsg);
            break;

        case cmdUPDATE_KEY:
            LOG(static_cast<ELogSeverityLevel>(_attachment->m_msgSeverity)) << _attachment->m_sMsg;
            if (_attachment->m_msgSeverity == MiscCommon::error)
            {
                execUserSignal(m_errorSignal, intercom_api::EErrorCode::UpdateKeyValueFailed, _attachment->m_sMsg);
            }
            break;

        default:
            LOG(debug) << "Received command cmdSIMPLE_MSG does not have a listener";
    }
}
//

void CIntercomServiceCore::sendCustomCmd(const std::string& _command, const std::string& _condition)
{
    if (!m_started)
    {
        stringstream ss;
        ss << "CCDDSIntercomGuard: Failed to send custom command <" << _command
           << "> because service was not started. Call start() before sending custom commands.";
        LOG(error) << ss.str();
        execUserSignal(m_errorSignal, intercom_api::EErrorCode::SendCustomCmdFailed, ss.str());
        return;
    }
    if (m_channel == nullptr && m_SMChannel == nullptr)
    {
        stringstream ss;
        ss << "Neither TCP nor SM channel is running. Failed to send custom command: " << _command;
        LOG(error) << ss.str();
        execUserSignal(m_errorSignal, intercom_api::EErrorCode::SendCustomCmdFailed, ss.str());
        return;
    }

    if (m_channel != nullptr)
    {
        SCustomCmdCmd cmd;
        cmd.m_sCmd = _command;
        cmd.m_sCondition = _condition;
        m_channel->pushMsg<cmdCUSTOM_CMD>(cmd, 0);
    }
    else if (m_SMChannel != nullptr)
    {
        SCustomCmdCmd cmd;
        cmd.m_sCmd = _command;
        cmd.m_sCondition = _condition;
        m_SMChannel->pushMsg<cmdCUSTOM_CMD>(cmd);
    }
}

void CIntercomServiceCore::putValue(const std::string& _key, const std::string& _value)
{
    if (!m_started)
    {
        stringstream ss;
        ss << "CCDDSIntercomGuard: Failed to put the key-value <" << _key << " --> " << _value
           << "> because service was not started. Call start() before putting key-value.";
        LOG(error) << ss.str();
        execUserSignal(m_errorSignal, intercom_api::EErrorCode::SendKeyValueFailed, ss.str());
        return;
    }
    if (m_SMChannel == nullptr)
    {
        stringstream ss;
        ss << "CCDDSIntercomGuard: Shared memory channel is not running. Failed to put the key-value <" << _key
           << " --> " << _value;
        LOG(error) << ss.str();
        execUserSignal(m_errorSignal, intercom_api::EErrorCode::SendKeyValueFailed, ss.str());
        return;
    }

    LOG(debug) << "CCDDSIntercomGuard putValue: key=" << _key << " value=" << _value;

    SUpdateKeyCmd cmd;
    cmd.m_propertyName = _key;
    cmd.m_senderTaskID = env_prop<task_id>();
    cmd.m_value = _value;
    m_SMChannel->pushMsg<cmdUPDATE_KEY>(cmd);
}

void CIntercomServiceCore::clean()
{
    std::string inputName = CUserDefaults::instance().getSMInputName();
    std::string outputName = CUserDefaults::instance().getSMOutputName();
    const bool inputRemoved = boost::interprocess::message_queue::remove(inputName.c_str());
    const bool outputRemoved = boost::interprocess::message_queue::remove(outputName.c_str());
    LOG(MiscCommon::info) << "Message queue " << inputName << " remove status: " << inputRemoved;
    LOG(MiscCommon::info) << "Message queue " << outputName << " remove status: " << outputRemoved;
}

void CIntercomServiceCore::waitCondition()
{
    if (!m_started || m_channel == nullptr)
    {
        LOG(error) << "waitCondition: Agent connection channel is not running.";
        return;
    }
    unique_lock<mutex> lock(m_waitMutex);
    m_waitCondition.wait_until(lock, std::chrono::system_clock::now() + std::chrono::minutes(10));
}

void CIntercomServiceCore::stopCondition()
{
    m_waitCondition.notify_all();
}
