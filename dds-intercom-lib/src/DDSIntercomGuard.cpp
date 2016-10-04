// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
// DDS
#include "DDSIntercomGuard.h"
#include "BOOST_FILESYSTEM.h"
#include "UserDefaults.h"

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
    Logger::instance().init(); // Initialize log
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
        m_SMChannel = CSMAgentChannel::makeNew(outputName, inputName);

        // Subscribe for cmdUPDATE_KEY from SM channel
        std::function<bool(SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment)> fUPDATE_KEY_SM =
            [this](SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment) -> bool {
            return this->on_cmdUPDATE_KEY_SM(_attachment);
        };
        m_SMChannel->registerMessageHandler<cmdUPDATE_KEY>(fUPDATE_KEY_SM);

        // Subscribe for cmdDELETE_KEY from SM channel
        std::function<bool(SCommandAttachmentImpl<cmdDELETE_KEY>::ptr_t _attachment)> fDELETE_KEY_SM =
            [this](SCommandAttachmentImpl<cmdDELETE_KEY>::ptr_t _attachment) -> bool {
            return this->on_cmdDELETE_KEY_SM(_attachment);
        };
        m_SMChannel->registerMessageHandler<cmdDELETE_KEY>(fDELETE_KEY_SM);

        // Subscribe for cmdCUSTOM_CMD from SM channel
        std::function<bool(SCommandAttachmentImpl<cmdCUSTOM_CMD>::ptr_t _attachment)> fCUSTOM_CMD_SM =
            [this](SCommandAttachmentImpl<cmdCUSTOM_CMD>::ptr_t _attachment) -> bool {
            return this->on_cmdCUSTOM_CMD_SM(_attachment);
        };
        m_SMChannel->registerMessageHandler<cmdCUSTOM_CMD>(fCUSTOM_CMD_SM);
        //

        // Subscribe for cmdSIMPLE_MSG from SM channel
        std::function<bool(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment)> fSIMPLE_MSG_SM =
            [this](SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment) -> bool {
            return this->on_cmdSIMPLE_MSG_SM(_attachment);
        };
        m_SMChannel->registerMessageHandler<cmdSIMPLE_MSG>(fSIMPLE_MSG_SM);
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
                m_agentConnectionMng = make_shared<CAgentConnectionManager>();
                m_agentConnectionMng->start();
            }
        }
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

// Messages from shared memory
bool CDDSIntercomGuard::on_cmdUPDATE_KEY_SM(
    protocol_api::SCommandAttachmentImpl<protocol_api::cmdUPDATE_KEY>::ptr_t _attachment)
{
    string propertyID = _attachment->getPropertyID();
    {
        std::lock_guard<std::mutex> lock(m_updateKeyCacheMutex);
        // TODO: do we need to store copy? or we can store shared ptr?
        m_updateKeyCache[propertyID][_attachment->m_sKey] = *_attachment;
    }

    m_keyValueUpdateSignal(propertyID, _attachment->m_sKey, _attachment->m_sValue);

    return true;
}

bool CDDSIntercomGuard::on_cmdDELETE_KEY_SM(
    protocol_api::SCommandAttachmentImpl<protocol_api::cmdDELETE_KEY>::ptr_t _attachment)
{
    string propertyID = _attachment->getPropertyID();
    {
        std::lock_guard<std::mutex> lock(m_updateKeyCacheMutex);
        auto iter = m_updateKeyCache.find(propertyID);
        if (iter != m_updateKeyCache.end())
        {
            iter->second.erase(_attachment->m_sKey);
        }
    }

    m_keyValueDeleteSignal(propertyID, _attachment->m_sKey);

    return true;
}

bool CDDSIntercomGuard::on_cmdCUSTOM_CMD_SM(
    protocol_api::SCommandAttachmentImpl<protocol_api::cmdCUSTOM_CMD>::ptr_t _attachment)
{
    m_customCmdSignal(_attachment->m_sCmd, _attachment->m_sCondition, _attachment->m_senderId);

    return true;
}

bool CDDSIntercomGuard::on_cmdSIMPLE_MSG_SM(
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
            return true;
    }

    return true;
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

        m_agentConnectionMng->sendCustomCmd(cmd);
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

        LOG(info) << "CCDDSIntercomGuard::sendCmd: sending custom command: " << _command;

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

    LOG(debug) << "CCDDSIntercomGuard: key=" << _key << " value=" << _value;

    SUpdateKeyCmd cmd;
    cmd.m_sKey = _key;
    cmd.m_sValue = _value;

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
