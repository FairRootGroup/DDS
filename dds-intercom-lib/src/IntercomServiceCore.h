// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef DDS_INTERCOM_SERVICE_CORE_H
#define DDS_INTERCOM_SERVICE_CORE_H
// DDS
#include "AgentChannel.h"
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
            const std::string& /*_propertyName*/, const std::string& /*_value*/, uint64_t /*_senderTaskID*/)>
            keyValueSignal_t;
        /// \typedef Task Done callback function
        typedef boost::signals2::signal<void(uint64_t /*_taskID*/, uint32_t /*_exitCode*/)> keyValueTaskDoneSignal_t;

        // Custom command types
        /// \typedef Custom command callback function
        typedef boost::signals2::signal<void(
            const std::string& /*_command*/, const std::string& /*_condition*/, uint64_t /*_senderID*/)>
            customCmdSignal_t;
        typedef boost::signals2::signal<void(const std::string&)> customCmdReplySignal_t;

        typedef boost::signals2::connection connection_t;

        class CIntercomServiceCore
        {
          public:
            using ptr_t = std::shared_ptr<CIntercomServiceCore>;

          public:
            CIntercomServiceCore();
            ~CIntercomServiceCore();

            // Disable copy constructors and assignment operators
            CIntercomServiceCore(const CIntercomServiceCore&) = delete;
            CIntercomServiceCore(CIntercomServiceCore&&) = delete;
            CIntercomServiceCore& operator=(const CIntercomServiceCore&) = delete;
            CIntercomServiceCore& operator=(CIntercomServiceCore&&) = delete;

            connection_t connectError(intercom_api::errorSignal_t::slot_function_type _subscriber);
            connection_t connectCustomCmd(customCmdSignal_t::slot_function_type _subscriber);
            connection_t connectCustomCmdReply(customCmdReplySignal_t::slot_function_type _subscriber);
            connection_t connectKeyValue(keyValueSignal_t::slot_function_type _subscriber);
            connection_t connectKeyValueTaskDone(keyValueTaskDoneSignal_t::slot_function_type _subscriber);
            void disconnectCustomCmd();
            void disconnectKeyValue();

            void sendCustomCmd(const std::string& _command, const std::string& _condition);
            void putValue(const std::string& _key, const std::string& _value);

            // Remove shared memory
            static void clean();

            void waitCondition();
            void stopCondition();

            void start(const std::string& _sessionID);
            void stop();

          private:
            // Messages from TCP and shared memory channels
            void on_cmdUPDATE_KEY_SM(
                const protocol_api::SSenderInfo& _sender,
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdUPDATE_KEY>::ptr_t _attachment);
            void on_cmdUSER_TASK_DONE_SM(
                const protocol_api::SSenderInfo& _sender,
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdUSER_TASK_DONE>::ptr_t _attachment);
            void on_cmdCUSTOM_CMD_SM(
                const protocol_api::SSenderInfo& _sender,
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdCUSTOM_CMD>::ptr_t _attachment);
            void on_cmdSIMPLE_MSG_SM(
                const protocol_api::SSenderInfo& _sender,
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdSIMPLE_MSG>::ptr_t _attachment);

            void setupSMChannel();
            void setupChannel(const std::string& _sessionID);

            template <class Signal_t, typename... Args>
            void execUserSignal(Signal_t& _signal, Args&&... args)
            {
                try
                {
                    _signal(args...);
                }
                catch (std::exception& _e)
                {
                    std::string msg("Exception in user code: ");
                    msg += _e.what();
                    if (!std::is_same<Signal_t, intercom_api::errorSignal_t>::value)
                    {
                        execUserSignal(m_errorSignal, intercom_api::EErrorCode::UserCodeException, msg);
                    }
                    LOG(MiscCommon::error) << msg;
                }
            }

          private:
            // Signals
            intercom_api::errorSignal_t m_errorSignal;         ///< Error signal
            keyValueSignal_t m_keyValueUpdateSignal;           ///< Key-value update signal
            keyValueTaskDoneSignal_t m_keyValueTaskDoneSignal; ///< Task done signal
            customCmdSignal_t m_customCmdSignal;               ///< Custom command signal
            customCmdReplySignal_t m_customCmdReplySignal;     ///< Reply on custom command signal

            boost::asio::io_context m_io_context;         ///> boost::asio IO context
            boost::thread_group m_workerThreads;          ///> Thread container
            CAgentChannel::connectionPtr_t m_channel;     ///< TCP channel for communication with DDS commander
            CSMAgentChannel::connectionPtr_t m_SMChannel; ///< Shared memory channel for comunication with DDS agent
            std::atomic<bool> m_started;                  ///< True if started, False otherwise

            /// Condition variable used to stop the current thread.
            /// Execution continues in three cases:
            /// 1) 10 minutes timeout
            /// 2) Failed connection or disconnection
            /// 3) Explicit call to stopCondition
            std::mutex m_waitMutex;
            std::condition_variable m_waitCondition;
        };
    } // namespace internal_api
} // namespace dds

#endif
