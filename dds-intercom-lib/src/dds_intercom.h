// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef DDS_INTERCOM_H_
#define DDS_INTERCOM_H_
// STD
#include <string>
// BOOST
#include <boost/property_tree/json_parser.hpp>
#include <boost/signals2/signal.hpp>

namespace dds
{
    namespace intercom_api
    {
        ///////////////////////////////////
        // DDS key-value
        ///////////////////////////////////
        class CKeyValue
        {
          public:
            typedef std::map<std::string, std::string> valuesMap_t;
            typedef boost::signals2::signal<void(const std::string&, const std::string&)> signal_t;
            typedef boost::signals2::signal<void(const std::string&)> errorSignal_t;
            typedef boost::signals2::connection connection_t;

          public:
            ~CKeyValue();

          public:
            int putValue(const std::string& _key, const std::string& _value);
            void getValues(const std::string& _key, valuesMap_t* _values);
            void subscribe(signal_t::slot_function_type _subscriber);
            void subscribeError(errorSignal_t::slot_function_type _subscriber);
            void unsubscribe();
        };

        ///////////////////////////////////
        // DDS custom commands
        ///////////////////////////////////
        class CCustomCmd
        {
          public:
            typedef boost::signals2::signal<void(const std::string&, const std::string&, uint64_t)> signal_t;
            typedef boost::signals2::signal<void(const std::string&)> replySignal_t;
            typedef boost::signals2::connection connection_t;

          public:
            ~CCustomCmd();

          public:
            int send(const std::string& _command, const std::string& _condition);
            void subscribe(signal_t::slot_function_type _subscriber);
            void subscribeReply(replySignal_t::slot_function_type _subscriber);
            void unsubscribe();
        };

        ///////////////////////////////////
        // DDS RMS plugins
        ///////////////////////////////////

        /// Sign that is send to commander if RMS plug-in is connecting to it.
        const std::string g_sRmsAgentSign = "rms_agent_sign";

        /// \brief Enumeration with message severity.
        enum class EMsgSeverity
        {
            info, ///< Information messages.
            error ///< Error messages.
        };

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

            uint32_t m_nInstances;          ///< Number of instances.
            std::string m_cfgFilePath;      ///< Path to the configuration file.
            std::string m_id;               ///< ID for communication with DDS commander.
            std::string m_wrkPackagePath;   ///< A full path of the agent worker package, which needs to be deployed.
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

        /// \brief Structure holds information of requirement notification.
        struct SRequirement
        {
            /// \brief Default constructor.
            SRequirement();

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
            bool operator==(const SRequirement& _val) const;

            std::string m_hostName; ///< Host name pattern.
            std::string m_id;       ///< ID for communication with DDS commander.
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
        ///    prot.onRequirement([](const SRequirement& _requirement) {
        ///        // Implement functionality related to requirements here.
        ///    });
        ///
        ///    // This must be the first call to let DDS commander know that we are online.
        ///    prot.sendInit();
        ///
        ///    // Stop here and wait for notifications from commander.
        ///    prot.wait();
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

            /// \brief Callback function for requirements notifications.
            typedef boost::signals2::signal<void(const SRequirement&)> signalRequirement_t;

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

            /// \brief Subscribe for requirement notifications.
            /// \param[in] _subscriber Callback function that is called when notification arrives.
            void onRequirement(signalRequirement_t::slot_function_type _subscriber);

            /// \brief Send initial request to the commander.
            void sendInit();

            /// \brief Send message to DDS commander.
            /// \param[in] _severity Message severity.
            /// \param[in] _msg Message text.
            void sendMessage(EMsgSeverity _severity, const std::string& _msg);

            /// \brief Stop the thread and wait until one of the conditions is applied.
            /// 1. 10 minutes timeout;
            /// 2. Failed connection to DDS commander or disconnection from DDS commander;
            /// 3. Explicit call of stop() function.
            void wait();

            /// \brief Stop waiting.
            void stop();

          protected:
            /// \brief Parse the input stream and notify subscribers.
            /// \param[in] _stream Stream with JSON data from DDS commander.
            void notify(std::istream& _stream);

          private:
            /// \brief Unsubscribe all user callbacks.
            void unsubscribe();

            signalSubmit_t m_signalSubmit;           ///< Submit signal.
            signalMessage_t m_signalMessage;         ///< Message signal.
            signalRequirement_t m_signalRequirement; ///< Requirement signal.

            std::string m_id; ///< ID for communication with DDS commander (provided via constructor).

            CCustomCmd m_customCmd; ///< Custom commands API which is used for communication with DDS commander.
        };
    }
}

#endif /* DDS_INTERCOM_H_ */
