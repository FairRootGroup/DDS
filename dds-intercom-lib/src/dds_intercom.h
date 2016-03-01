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

    const std::string g_sRmsAgentSign = "rms_agent_sign";

    enum class EMsgSeverity
    {
        info,
        error
    };

    struct SSubmit
    {
        SSubmit();

        std::string toJSON();
        void fromJSON(const std::string& _json);
        void fromPT(const boost::property_tree::ptree& _pt);
        bool operator==(const SSubmit& _val) const;

        uint32_t m_nInstances;
        std::string m_cfgFilePath;
        std::string m_id;
    };

    struct SMessage
    {
        SMessage();

        std::string toJSON();
        void fromJSON(const std::string& _json);
        void fromPT(const boost::property_tree::ptree& _pt);
        bool operator==(const SMessage& _val) const;

        EMsgSeverity m_msgSeverity;
        std::string m_msg;
        std::string m_id;
    };

    struct SRequirement
    {
        SRequirement();

        std::string toJSON();
        void fromJSON(const std::string& _json);
        void fromPT(const boost::property_tree::ptree& _pt);
        bool operator==(const SRequirement& _val) const;

        std::string m_hostName;
        std::string m_id;
    };

    struct SInit
    {
        SInit();

        std::string toJSON();
        void fromJSON(const std::string& _json);
        void fromPT(const boost::property_tree::ptree& _pt);
        bool operator==(const SInit& _val) const;

        std::string m_id;
    };

    class CRMSPluginProtocol
    {
      public:
        typedef boost::signals2::signal<void(const SSubmit&)> signalSubmit_t;
        typedef boost::signals2::signal<void(const SMessage&)> signalMessage_t;
        typedef boost::signals2::signal<void(const SRequirement&)> signalRequirement_t;

      public:
        CRMSPluginProtocol(const std::string& _id);
        ~CRMSPluginProtocol();

      public:
        void onSubmit(signalSubmit_t::slot_function_type _subscriber);
        void onMessage(signalMessage_t::slot_function_type _subscriber);
        void onRequirement(signalRequirement_t::slot_function_type _subscriber);

        void sendInit();
        void sendMessage(EMsgSeverity _severity, const std::string& _msg);

        void notify(std::istream& _stream);

      private:
        void unsubscribe();

        signalSubmit_t m_signalSubmit;
        signalMessage_t m_signalMessage;
        signalRequirement_t m_signalRequirement;

        std::string m_id;

        CCustomCmd m_customCmd;
    };
}

#endif /* DDS_INTERCOM_H_ */
