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

void CDDSIntercomGuard::start()
{
    // Choose transport type.
    // If we connect to agent, i.e. agent config file exists than we use shared memory transport.
    // If we connect to commander, i.e. commander config file exists we use asio transport.
    const string sAgentIDFile(CUserDefaults::instance().getAgentIDFile());
    m_useSMTransport = fs::exists(sAgentIDFile);

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

        // Subscribe for cmdUPDATE_KEY_ERROR from SM channel
        m_SMChannel->registerHandler<cmdUPDATE_KEY_ERROR>(
            [this](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdUPDATE_KEY_ERROR>::ptr_t _attachment) {
                this->on_cmdUPDATE_KEY_ERROR_SM(_sender, _attachment);
            });

        // Subscribe for cmdDELETE_KEY from SM channel
        m_SMChannel->registerHandler<cmdDELETE_KEY>(
            [this](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdDELETE_KEY>::ptr_t _attachment) {
                this->on_cmdDELETE_KEY_SM(_sender, _attachment);
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
    }
    else
    {
        LOG(info) << "CCDDSIntercomGuard: using asio for transport";
        {
            lock_guard<std::mutex> lock(m_initAgentConnectionMutex);

            LOG(info) << "CCDDSIntercomGuard: is going to init agent connection";

            if (!m_agentConnectionMng)
            {
                LOG(info) << "CCDDSIntercomGuard: start init agent connection";

                m_agentConnectionMng.reset();
                m_agentConnectionMng = make_shared<CAgentConnectionManager>(m_io_service);
                m_agentConnectionMng->start();
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

connection_t CDDSIntercomGuard::connectKeyValueDelete(keyValueDeleteSignal_t::slot_function_type _subscriber)
{
    return m_keyValueDeleteSignal.connect(_subscriber);
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

bool CDDSIntercomGuard::updateCacheIfNeeded(const SUpdateKeyCmd& _cmd)
{
    bool isVersionOK(true);
    {
        std::lock_guard<std::mutex> lock(m_updateKeyCacheMutex);

        // Check if version in the attachment is more than the version in the cache.
        // If this is true than we update cache and call user's callback, otherwise not.
        auto it = m_updateKeyCache.find(_cmd.m_sKey);
        if (it != m_updateKeyCache.end())
        {
            isVersionOK = (it->second < _cmd.m_version);
        }

        if (isVersionOK)
        {
            // Version is correct - update local key-value cache
            m_updateKeyCache[_cmd.m_sKey] = _cmd.m_version;
        }
    }
    return isVersionOK;
}

// Messages from shared memory
void CDDSIntercomGuard::on_cmdUPDATE_KEY_SM(
    const protocol_api::SSenderInfo& _sender,
    protocol_api::SCommandAttachmentImpl<protocol_api::cmdUPDATE_KEY>::ptr_t _attachment)
{
    string propertyID(_attachment->getPropertyID());
    bool isVersionOK = updateCacheIfNeeded(*_attachment);

    // Version is correct - call user's callback
    if (isVersionOK)
    {
        m_keyValueUpdateSignal(propertyID, _attachment->m_sKey, _attachment->m_sValue);
    }
    else
    {
        LOG(warning) << "Cache not updated. Version mismatch for key " << _attachment->m_sKey
                     << "; update version: " << _attachment->m_version;
    }
}

void CDDSIntercomGuard::on_cmdUPDATE_KEY_ERROR_SM(
    const protocol_api::SSenderInfo& _sender,
    protocol_api::SCommandAttachmentImpl<protocol_api::cmdUPDATE_KEY_ERROR>::ptr_t _attachment)
{
    if (_attachment->m_errorCode == intercom_api::EErrorCode::KeyValueNotFound)
    {
        LOG(warning) << "Key-value not found on the DDS commander: " << _attachment->m_userCmd;
    }
    else if (_attachment->m_errorCode == intercom_api::EErrorCode::KeyValueVersionMismatch)
    {
        string propertyID(_attachment->m_serverCmd.getPropertyID());
        bool isVersionOK = updateCacheIfNeeded(_attachment->m_serverCmd);

        if (!isVersionOK)
        {
            LOG(warning) << "Cache not updated. Attachment: " << *_attachment;
        }

        LOG(warning) << "Key-value update error: propertyID: " << propertyID
                     << "; ServerCmd: " << _attachment->m_serverCmd << "; UserCmd: " << _attachment->m_userCmd
                     << "; errorCode: " << _attachment->m_errorCode;

        // In case of error we force the key update with a current value stored in a cache
        string value("");
        {
            std::lock_guard<std::mutex> lock(m_putValueCacheMutex);
            value = m_putValueCache[propertyID];
        }
        putValue(propertyID, value);
    }
}

void CDDSIntercomGuard::on_cmdDELETE_KEY_SM(
    const protocol_api::SSenderInfo& _sender,
    protocol_api::SCommandAttachmentImpl<protocol_api::cmdDELETE_KEY>::ptr_t _attachment)
{
    string propertyID = _attachment->getPropertyID();
    {
        std::lock_guard<std::mutex> lock(m_updateKeyCacheMutex);
        m_updateKeyCache.erase(_attachment->m_sKey);
    }

    m_keyValueDeleteSignal(propertyID, _attachment->m_sKey);
}

void CDDSIntercomGuard::on_cmdCUSTOM_CMD_SM(
    const protocol_api::SSenderInfo& _sender,
    protocol_api::SCommandAttachmentImpl<protocol_api::cmdCUSTOM_CMD>::ptr_t _attachment)
{
    m_customCmdSignal(_attachment->m_sCmd, _attachment->m_sCondition, _attachment->m_senderId);
}

void CDDSIntercomGuard::on_cmdSIMPLE_MSG_SM(
    const protocol_api::SSenderInfo& _sender,
    protocol_api::SCommandAttachmentImpl<protocol_api::cmdSIMPLE_MSG>::ptr_t _attachment)
{
    switch (_attachment->m_srcCommand)
    {
        case cmdCUSTOM_CMD:
            LOG(static_cast<ELogSeverityLevel>(_attachment->m_msgSeverity)) << _attachment->m_sMsg;
            m_customCmdReplySignal(_attachment->m_sMsg);
            break;

        case cmdUPDATE_KEY:
            LOG(static_cast<ELogSeverityLevel>(_attachment->m_msgSeverity)) << _attachment->m_sMsg;
            if (_attachment->m_msgSeverity == MiscCommon::error)
            {
                m_errorSignal(intercom_api::EErrorCode::UpdateKeyValueFailed, _attachment->m_sMsg);
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
        m_errorSignal(intercom_api::EErrorCode::SendCustomCmdFailed, ss.str());
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
            m_errorSignal(intercom_api::EErrorCode::SendCustomCmdFailed, ss.str());
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
            m_errorSignal(intercom_api::EErrorCode::SendCustomCmdFailed, ss.str());
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
        m_errorSignal(intercom_api::EErrorCode::SendKeyValueFailed, ss.str());
        return;
    }
    if (m_SMChannel == nullptr)
    {
        stringstream ss;
        ss << "CCDDSIntercomGuard: Shared memory channel is not running. Failed to put the key-value <" << _key
           << " --> " << _value;
        LOG(error) << ss.str();
        m_errorSignal(intercom_api::EErrorCode::SendKeyValueFailed, ss.str());
        return;
    }

    LOG(debug) << "CCDDSIntercomGuard putValue: key=" << _key << " value=" << _value;

    SUpdateKeyCmd cmd;
    cmd.setKey(_key, env_prop<task_id>());
    cmd.m_sValue = _value;

    {
        std::lock_guard<std::mutex> lock(m_updateKeyCacheMutex);

        // Get current version from cache if it exists
        auto it = m_updateKeyCache.find(cmd.m_sKey);
        if (it != m_updateKeyCache.end())
        {
            cmd.m_version = it->second;
        }
    }

    {
        std::lock_guard<std::mutex> lock(m_putValueCacheMutex);
        m_putValueCache[_key] = _value;
    }

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
    if (!m_agentConnectionMng)
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
