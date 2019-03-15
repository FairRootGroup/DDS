// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
// DDS
#include "DDSIntercomGuard.h"
#include "BOOST_FILESYSTEM.h"
#include "UserDefaults.h"
#include "dds_env_prop.h"

// BOOST
#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/property_tree/ptree.hpp>

// silence "Unused typedef" warning using clang 3.7+ and boost < 1.59
#if BOOST_VERSION < 105900
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-local-typedef"
#endif
#include <boost/property_tree/ini_parser.hpp>
#if BOOST_VERSION < 105900
#pragma clang diagnostic pop
#endif

using namespace std;
using namespace dds;
using namespace dds::user_defaults_api;
using namespace dds::internal_api;
using namespace dds::protocol_api;
using namespace MiscCommon;
namespace ip = boost::interprocess;

CDDSIntercomGuard::CDDSIntercomGuard()
    : m_useSMTransport(true)
    , m_started(false)
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

CDDSIntercomGuard::~CDDSIntercomGuard()
{
}

CDDSIntercomGuard& CDDSIntercomGuard::instance()
{
    static CDDSIntercomGuard instance;
    return instance;
}

void CDDSIntercomGuard::start(const std::string& _sessionID)
{
    // Choose transport type.
    // If we connect to agent, i.e. agent config file exists than we use shared memory transport.
    // If we connect to commander, i.e. commander config file exists we use asio transport.
    m_useSMTransport = CUserDefaults::instance().isAgentInstance();

    if (m_useSMTransport)
    {
        LOG(info) << "CCDDSIntercomGuard: using shared memory for transport";

        // Shared memory channel for communication with DDS Agent
        std::string inputName = CUserDefaults::instance().getSMInputName();
        std::string outputName = CUserDefaults::instance().getSMOutputName();
        // For the user task shared memory names are opposite: input shared memory name has output name and output
        // shared
        // memory name has input name.
        m_SMChannel = CSMAgentChannel::makeNew(m_io_service, outputName, inputName, 0);

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

        // Subscribe for cmdDELETE_KEY from SM channel
        m_SMChannel->registerHandler<cmdDELETE_KEY>(
            [this](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdDELETE_KEY>::ptr_t _attachment) {
                this->on_cmdDELETE_KEY(_sender, _attachment);
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
            m_started = false;
            LOG(info) << "CCDDSIntercomGuard: Failed to initialize";
            return;
        }
    }
    else
    {
        LOG(info) << "CCDDSIntercomGuard: using TCP for transport";

        // For plugins current SID is already set
        bool defaultExists = _sessionID.empty() && !CUserDefaults::instance().getCurrentSID().empty();
        if (!defaultExists)
        {
            try
            {
                // Try to convert sessionID string to boost::uuid
                const boost::uuids::uuid& sid = boost::uuids::string_generator()(_sessionID);
                // Reinit UserDefaults and Log with new session ID
                CUserDefaults::instance().reinit(sid, CUserDefaults::instance().currentUDFile());
                Logger::instance().reinit();
            }
            catch (std::exception& _e)
            {
                LOG(error) << "Invalid DDS session ID: " << _sessionID;
                return;
            }
        }

        {
            lock_guard<std::mutex> lock(m_initAgentConnectionMutex);

            LOG(info) << "CCDDSIntercomGuard: is going to init agent connection";

            if (!m_agentConnectionMng)
            {
                LOG(info) << "CCDDSIntercomGuard: start init agent connection";

                m_agentConnectionMng.reset();
                m_agentConnectionMng = make_shared<CAgentConnectionManager>(m_io_service);
                m_agentConnectionMng->start();
                if (!m_agentConnectionMng->started())
                {
                    m_started = false;
                    LOG(info) << "CCDDSIntercomGuard: Failed to initialize";
                    return;
                }
            }
        }
    }

    // Don't block main thread, start transport service on a thread-pool
    const int nConcurrentThreads(3);
    LOG(MiscCommon::info) << "Starting DDS transport engine using " << nConcurrentThreads << " concurrent threads.";
    for (int x = 0; x < nConcurrentThreads; ++x)
    {
        m_workerThreads.create_thread(boost::bind(&boost::asio::io_service::run, &(m_io_service)));
    }

    m_started = true;
}

void CDDSIntercomGuard::stop()
{
    if (!m_started)
        return;

    m_started = false;

    disconnectCustomCmd();
    disconnectKeyValue();

    if (m_SMChannel != nullptr)
    {
        m_SMChannel->stop();
    }

    m_io_service.stop();
    m_workerThreads.join_all();
}

connection_t CDDSIntercomGuard::connectError(intercom_api::errorSignal_t::slot_function_type _subscriber)
{
    return m_errorSignal.connect(_subscriber);
}

connection_t CDDSIntercomGuard::connectCustomCmd(customCmdSignal_t::slot_function_type _subscriber)
{
    return m_customCmdSignal.connect(_subscriber);
}

connection_t CDDSIntercomGuard::connectCustomCmdReply(customCmdReplySignal_t::slot_function_type _subscriber)
{
    return m_customCmdReplySignal.connect(_subscriber);
}

connection_t CDDSIntercomGuard::connectKeyValue(keyValueSignal_t::slot_function_type _subscriber)
{
    return m_keyValueUpdateSignal.connect(_subscriber);
}

connection_t CDDSIntercomGuard::connectKeyValueDelete(keyValueTaskDoneSignal_t::slot_function_type _subscriber)
{
    return m_keyValueTaskDoneSignal.connect(_subscriber);
}

void CDDSIntercomGuard::disconnectCustomCmd()
{
    // TODO: Thread safe disconnect?
    // disconnect custom command signals
    m_customCmdSignal.disconnect_all_slots();
    m_customCmdReplySignal.disconnect_all_slots();
    m_errorSignal.disconnect_all_slots();
}

void CDDSIntercomGuard::disconnectKeyValue()
{
    // TODO: Thread safe disconnect?
    // disconnect key-value signals
    m_keyValueUpdateSignal.disconnect_all_slots();
    m_errorSignal.disconnect_all_slots();
}

// Messages from shared memory
void CDDSIntercomGuard::on_cmdUPDATE_KEY_SM(
    const protocol_api::SSenderInfo& _sender,
    protocol_api::SCommandAttachmentImpl<protocol_api::cmdUPDATE_KEY>::ptr_t _attachment)
{
    execUserSignal(
        m_keyValueUpdateSignal, _attachment->m_propertyName, _attachment->m_value, _attachment->m_senderTaskID);
}

void CDDSIntercomGuard::on_cmdUSER_TASK_DONE_SM(
    const protocol_api::SSenderInfo& _sender,
    protocol_api::SCommandAttachmentImpl<protocol_api::cmdUSER_TASK_DONE>::ptr_t _attachment)
{
    execUserSignal(m_keyValueTaskDoneSignal, _attachment->m_taskID, _attachment->m_exitCode);
}

void CDDSIntercomGuard::on_cmdDELETE_KEY(
    const protocol_api::SSenderInfo& _sender,
    protocol_api::SCommandAttachmentImpl<protocol_api::cmdDELETE_KEY>::ptr_t _attachment)
{
    LOG(info) << "Received cmdDELETE_KEY: " << *_attachment;
}

void CDDSIntercomGuard::on_cmdCUSTOM_CMD_SM(
    const protocol_api::SSenderInfo& _sender,
    protocol_api::SCommandAttachmentImpl<protocol_api::cmdCUSTOM_CMD>::ptr_t _attachment)
{
    execUserSignal(m_customCmdSignal, _attachment->m_sCmd, _attachment->m_sCondition, _attachment->m_senderId);
}

void CDDSIntercomGuard::on_cmdSIMPLE_MSG_SM(
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
            return;
    }
}
//

void CDDSIntercomGuard::sendCustomCmd(const std::string& _command, const std::string& _condition)
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
    if (!m_useSMTransport)
    {
        if (m_agentConnectionMng == nullptr)
        {
            stringstream ss;
            ss << "CCDDSIntercomGuard: Agent connection channel is not running. Failed to send custom command: "
               << _command;
            LOG(error) << ss.str();
            execUserSignal(m_errorSignal, intercom_api::EErrorCode::SendCustomCmdFailed, ss.str());
            return;
        }

        LOG(info) << "CCDDSIntercomGuard::sendCmd: sending custom command: " << _command;
        SCustomCmdCmd cmd;
        cmd.m_sCmd = _command;
        cmd.m_sCondition = _condition;

        m_agentConnectionMng->sendCustomCmd(cmd, 0);
    }
    else
    {
        if (m_SMChannel == nullptr)
        {
            stringstream ss;
            ss << "CCDDSIntercomGuard: Shared memory channel is not running. Failed to send custom command: "
               << _command;
            LOG(error) << ss.str();
            execUserSignal(m_errorSignal, intercom_api::EErrorCode::SendCustomCmdFailed, ss.str());
            return;
        }

        SCustomCmdCmd cmd;
        cmd.m_sCmd = _command;
        cmd.m_sCondition = _condition;

        m_SMChannel->pushMsg<cmdCUSTOM_CMD>(cmd);
    }
}

void CDDSIntercomGuard::putValue(const std::string& _key, const std::string& _value)
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

void CDDSIntercomGuard::clean()
{
    std::string inputName = CUserDefaults::instance().getSMInputName();
    std::string outputName = CUserDefaults::instance().getSMOutputName();
    const bool inputRemoved = boost::interprocess::message_queue::remove(inputName.c_str());
    const bool outputRemoved = boost::interprocess::message_queue::remove(outputName.c_str());
    LOG(MiscCommon::info) << "Message queue " << inputName << " remove status: " << inputRemoved;
    LOG(MiscCommon::info) << "Message queue " << outputName << " remove status: " << outputRemoved;
}

void CDDSIntercomGuard::waitCondition()
{
    if (!m_started || !m_agentConnectionMng)
    {
        LOG(error) << "CCDDSIntercomGuard::waitCondition: Agent connection channel is not running.";
        return;
    }

    LOG(info) << "CCDDSIntercomGuard::waitCondition: stop thread and wait for notification.";
    return m_agentConnectionMng->waitCondition();
}

void CDDSIntercomGuard::stopCondition()
{
    if (!m_agentConnectionMng)
    {
        LOG(error) << "CCDDSIntercomGuard::stopCondition: Agent connection channel is not running.";
        return;
    }

    LOG(info) << "CCDDSIntercomGuard::stopCondition: notification received, continue execution.";
    return m_agentConnectionMng->stopCondition();
}
