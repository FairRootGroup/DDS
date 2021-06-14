// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef DDS_INTERCOM_H_
#define DDS_INTERCOM_H_
// DDS
#include "IntercomErrorCodes.h"
// STD
#include <ostream>
#include <string>
// BOOST
#include <boost/property_tree/json_parser.hpp>
#include <boost/signals2/signal.hpp>

namespace dds
{
    namespace internal_api
    {
        class CIntercomServiceCore;
    }

    /// \brief DDS intercom API
    ///
    /// \detail
    /// Internally CKeyValue and CCustomCmd use shared memory transport to talk to DDS agent.
    /// CCustomCmd can also connect to DDS commander. In case of a problem it tries to reconnect for 2
    /// minutes and if not connected send an error with EErrorCode::ConnectionFailed error code. If none of the DDS
    /// services are running a different error is sent - EErrorCode::TransportServiceFailed. In the error message a
    /// detailed information is provided about what caused the error.
    /// In order to get an error messages from DDS one should subscribe using
    /// CIntercomService::subscribeOnError(errorSignal_t::slot_function_type _subscriber) function.
    ///
    namespace intercom_api
    {
        ///////////////////////////////////
        // DDS intercom service
        ///////////////////////////////////
        class CIntercomService
        {
          public:
            /// \typedef Task done callback function
            typedef boost::signals2::signal<void(uint64_t /*_taskID*/, uint32_t /*_exitCode*/)> taskDoneSignal_t;

          public:
            CIntercomService();
            ~CIntercomService();

            /// \brief Subscribe on error messages from DDS intercom service
            void subscribeOnError(errorSignal_t::slot_function_type _subscriber);
            /// \brief Subscribe on Task Done events
            void subscribeOnTaskDone(taskDoneSignal_t::slot_function_type _subscriber);
            /// \brief Start DDS service, i.e. receiving and sending messages.
            void start(const std::string& _sessionID = "");

            // FIXME: workaround for old functionality. This functions will be removed in the future releases.
            void waitCondition();
            void stopCondition();

          private:
            friend class CKeyValue;
            friend class CCustomCmd;
            std::shared_ptr<internal_api::CIntercomServiceCore> m_impl;
        };

        ///////////////////////////////////
        // DDS key-value
        ///////////////////////////////////
        class CKeyValue
        {
          public:
            /// \typedef Update key callback function
            typedef boost::signals2::signal<void(
                const std::string& /*_propertyName*/, const std::string& /*_value*/, uint64_t /*_senderTaskID*/)>
                signal_t;

          public:
            CKeyValue(CIntercomService& _service);
            ~CKeyValue();

          public:
            void putValue(const std::string& _key, const std::string& _value);
            void subscribe(signal_t::slot_function_type _subscriber);
            void unsubscribe();

          private:
            CIntercomService& m_service; ///< Reference to intercom service.
        };

        ///////////////////////////////////
        // DDS custom commands
        ///////////////////////////////////
        class CCustomCmd
        {
          public:
            /// \typedef Custom command cllback function
            typedef boost::signals2::signal<void(
                const std::string& /*_command*/, const std::string& /*_condition*/, uint64_t /*_senderID*/)>
                signal_t;
            typedef boost::signals2::signal<void(const std::string&)> replySignal_t;

          public:
            CCustomCmd(CIntercomService& _service);
            ~CCustomCmd();

          public:
            void send(const std::string& _command, const std::string& _condition);
            void subscribe(signal_t::slot_function_type _subscriber);
            void subscribeOnReply(replySignal_t::slot_function_type _subscriber);
            void unsubscribe();

          private:
            CIntercomService& m_service; ///< Reference to intercom service.
        };

        ///////////////////////////////////
        // DDS RMS plugins
        ///////////////////////////////////

        /// Sign that is send to commander if RMS plug-in is connecting to it.
        const std::string g_sRmsAgentSign = "rms_agent_sign";
        /// Sign that is send to commander if Tools-API is connecting to it.
        const std::string g_sToolsAPISign = "tools_api_sign";

        /// \brief Enumeration with message severity.
        enum class EMsgSeverity
        {
            info, ///< Information messages.
            error ///< Error messages.
        };

        /// \brief Support ostreaming EMsgSeverity
        inline std::ostream& operator<<(std::ostream& _os, EMsgSeverity _severity)
        {
            switch (_severity)
            {
                case EMsgSeverity::info:
                    return _os << "info";
                case EMsgSeverity::error:
                default:
                    return _os << "error";
            }
        }

        /// \brief Structure holds information of submit notification.
        struct SSubmit
        {
            /// \brief Default constructor.
            SSubmit();

            /// \brief Converts structure to JSON.
            /// \return String with JSON.
            std::string toJSON();

            /// \brief Init structure from JSON.
            /// \param[in] _json JSON string with structure details.
            void fromJSON(const std::string& _json);

            /// \brief Init structure from boost's property tree.
            /// \param[in] _pt Property tree with structure details.
            void fromPT(const boost::property_tree::ptree& _pt);

            /// \brief Equality operator.
            bool operator==(const SSubmit& _val) const;

            uint32_t m_nInstances;        ///< A number of instances.
            uint32_t m_slots;             ///< A number of task slots.
            std::string m_cfgFilePath;    ///< Path to the configuration file.
            std::string m_id;             ///< ID for communication with DDS commander.
            std::string m_wrkPackagePath; ///< A full path of the agent worker package, which needs to be deployed.
        };

        /// \brief Structure holds information of message notification.
        struct SMessage
        {
            /// \brief Default constructor.
            SMessage();

            /// \brief Converts structure to JSON.
            /// \return String with JSON.
            std::string toJSON();

            /// \brief Init structure from JSON.
            /// \param[in] _json JSON string with structure details.
            void fromJSON(const std::string& _json);

            /// \brief Init structure from boost's property tree.
            /// \param[in] _pt Property tree with structure details.
            void fromPT(const boost::property_tree::ptree& _pt);

            /// \brief Equality operator.
            bool operator==(const SMessage& _val) const;

            EMsgSeverity m_msgSeverity; ///< Message severity.
            std::string m_msg;          ///< Message text.
            std::string m_id;           ///< ID for communication with DDS commander.
        };

        /// \brief Structure holds information of init notification.
        struct SInit
        {
            /// \brief Default constructor.
            SInit();

            /// \brief Converts structure to JSON.
            /// \return String with JSON.
            std::string toJSON();

            /// \brief Init structure from JSON.
            /// \param[in] _json JSON string with structure details.
            void fromJSON(const std::string& _json);

            /// \brief Init structure from boost's property tree.
            /// \param[in] _pt Property tree with structure details.
            void fromPT(const boost::property_tree::ptree& _pt);

            /// \brief Equality operator.
            bool operator==(const SInit& _val) const;

            std::string m_id; ///< ID for communication with DDS commander.
        };

        /// \brief Class implements basic API for DDS RMS plug-ins.
        ///
        /// Example Usage:
        /// \code
        ///
        /// try {
        ///    CRMSPluginProtocol prot("plug-in-id");
        ///
        ///    prot.onSubmit([](const SSubmit& _submit) {
        ///        // Implement submit related functionality here.
        ///
        ///        // After submit has completed call stop() function.
        ///        prot.stop();
        ///    });
        ///
        ///    prot.onMessage([](const SMessage& _message) {
        ///        // Message from commander received.
        ///        // Implement related functionality here.
        ///    });
        ///
        ///    // Let DDS commander know that we are online and wait for notifications from commander
        ///    prot.start();
        /// } catch (exception& _e) {
        ///    // Report error to DDS commander
        ///    proto.sendMessage(dds::EMsgSeverity::error, e.what());
        /// }
        ///
        /// \endcode
        class CRMSPluginProtocol
        {
          public:
            /// \brief Callback function for submit notifications.
            typedef boost::signals2::signal<void(const SSubmit&)> signalSubmit_t;

            /// \brief Callback function for message notifications.
            typedef boost::signals2::signal<void(const SMessage&)> signalMessage_t;

          public:
            /// \brief Constructor with ID.
            /// \param[in] _id DDS commander provides an ID which has to be used for the communication.
            CRMSPluginProtocol(const std::string& _id);

            /// \brief Destructor.
            ~CRMSPluginProtocol();

          public:
            /// \brief Subscribe for submit notifications.
            /// \param[in] _subscriber Callback function that is called when notification arrives.
            void onSubmit(signalSubmit_t::slot_function_type _subscriber);

            /// \brief Subscribe for message notifications.
            /// \param[in] _subscriber Callback function that is called when notification arrives.
            void onMessage(signalMessage_t::slot_function_type _subscriber);

            /// \brief Send message to DDS commander.
            /// \param[in] _severity Message severity.
            /// \param[in] _msg Message text.
            void sendMessage(EMsgSeverity _severity, const std::string& _msg);

            /// \brief Send initial request to the commander and start listening for notifications.
            /// \brief param[in] _block If true than we stop the main thread.
            ///
            /// If _block is true function stops the thread and waits until one of the conditions is applied:
            /// 1. 10 minutes timeout;
            /// 2. Failed connection to DDS commander or disconnection from DDS commander;
            /// 3. Explicit call of stop() function.
            ///
            /// \note If there are no subscribers function doesn't wait.
            void start(bool _block = true);

            /// \brief Stop waiting.
            void stop();

          protected:
            /// \brief Parse the input stream and notify subscribers.
            /// \param[in] _stream Stream with JSON data from DDS commander.
            void notify(std::istream& _stream);

          private:
            /// \brief Unsubscribe all user callbacks.
            void unsubscribe();

            signalSubmit_t m_signalSubmit;   ///< Submit signal.
            signalMessage_t m_signalMessage; ///< Message signal.

            std::string m_id; ///< ID for communication with DDS commander (provided via constructor).

            CIntercomService m_service; ///< Intercom service.
            CCustomCmd m_customCmd;     ///< Custom commands API which is used for communication with DDS commander.
        };
    } // namespace intercom_api
} // namespace dds

#endif /* DDS_INTERCOM_H_ */
