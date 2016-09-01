// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef DDSINTERCOMGUARD_H
#define DDSINTERCOMGUARD_H
// DDS
#include "AgentConnectionManager.h"
#include "SMAgentChannel.h"
#include "dds_intercom_error_codes.h"
// STD
#include <string>
#include <vector>
// BOOST
#include <boost/signals2/signal.hpp>

namespace dds
{
    namespace internal_api
    {
        // Key-value types
        /// \typedef Update key callback function
        typedef boost::signals2::signal<void(
            const std::string& /*_propertyID*/, const std::string& /*_key*/, const std::string& /*_value*/)>
            keyValueSignal_t;
        /// \typedef Delete key callback function
        typedef boost::signals2::signal<void(const std::string& /*_propertyID*/, const std::string& /*_key*/)>
            keyValueDeleteSignal_t;

        // Custom command typeы
        /// \typedef Custom command cllback function
        typedef boost::signals2::signal<void(
            const std::string& /*_command*/, const std::string& /*_condition*/, uint64_t /*_senderID*/)>
            customCmdSignal_t;
        typedef boost::signals2::signal<void(const std::string&)> customCmdReplySignal_t;

        typedef boost::signals2::connection connection_t;

        class CDDSIntercomGuard
        {
            // key -> SUpdateKeyCmd command
            typedef std::map<std::string, std::map<std::string, protocol_api::SUpdateKeyCmd>> updateKeyCache_t;

          private:
            CDDSIntercomGuard();
            ~CDDSIntercomGuard();

          public:
            static CDDSIntercomGuard& instance();

            connection_t connectError(intercom_api::errorSignal_t::slot_function_type _subscriber);
            connection_t connectCustomCmd(customCmdSignal_t::slot_function_type _subscriber);
            connection_t connectCustomCmdReply(customCmdReplySignal_t::slot_function_type _subscriber);
            connection_t connectKeyValue(keyValueSignal_t::slot_function_type _subscriber);
            connection_t connectKeyValueDelete(keyValueDeleteSignal_t::slot_function_type _subscriber);
            void disconnectCustomCmd();
            void disconnectKeyValue();

            // Messages from shared memory
            bool on_cmdUPDATE_KEY_SM(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdUPDATE_KEY>::ptr_t _attachment);
            bool on_cmdDELETE_KEY_SM(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdDELETE_KEY>::ptr_t _attachment);
            bool on_cmdCUSTOM_CMD_SM(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdCUSTOM_CMD>::ptr_t _attachment);
            bool on_cmdSIMPLE_MSG_SM(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdSIMPLE_MSG>::ptr_t _attachment);

            void sendCustomCmd(const std::string& _command, const std::string& _condition);
            void putValue(const std::string& _key, const std::string& _value);

            // Remove shared memory
            static void clean();

            void waitCondition();
            void stopCondition();

            void start();

            void initAgentConnection();

          public:
            // Signals for subscriptions
            intercom_api::errorSignal_t m_errorSignal;
            keyValueSignal_t m_keyValueUpdateSignal;
            keyValueDeleteSignal_t m_keyValueDeleteSignal;
            customCmdSignal_t m_customCmdSignal;
            customCmdReplySignal_t m_customCmdReplySignal;

          private:
            CAgentConnectionManager::ptr_t m_agentConnectionMng;
            std::mutex m_initAgentConnectionMutex;

            updateKeyCache_t m_updateKeyCache; ///< Local cache of Update Key command
            std::mutex m_updateKeyCacheMutex;  ///< Mutex for local cache

            CSMAgentChannel::connectionPtr_t m_SMChannel; ///< Shared memory channel for comunication with DDS agent

            bool m_useSMTransport;
            bool m_started;
        };
    }
}

#endif
