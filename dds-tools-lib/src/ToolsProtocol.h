// Copyright 2019 GSI, Inc. All rights reserved.
//
//
//

#ifndef DDS_TOOLSPROTOCOL_H
#define DDS_TOOLSPROTOCOL_H

// STD
#include <string>
// BOOST
#include <boost/property_tree/json_parser.hpp>
// DDS
#include "dds_intercom.h"

/*
{
    "dds": {
        "tools-api": {
            "message": {
                "msg": "string",
                "severity": 123,
                "requestID": 123
            },
            "done": {
                "requestID": 123
            },
            "progress":{
                "completed": 123,
                "total": 123,
                "errors": 123,
                "time": 123,
                "srcCommand": 123
            },
            "submit": {
                "rms": "string",
                "instances": 123,
                "config": "string",
                "pluginPath": "string",
                "requestID": 123
            },
            "topology": {
                "updateType": 123,
                "topologyFile": "string",
                "disableValidation": false,
            },
            "getlog": {
            },
            "commanderInfo":
            {
                "pid":123,
                "idleAgentsCount": 123
            },
            "agentInfo":
            {
                "activeAgentsCount": 123,
                "index": 123,
                "agentInfo": "string"
            },

        }
    }
}
*/

namespace dds
{
    namespace tools_api
    {
        template <class T>
        struct SBaseMessageImpl
        {
            uint64_t m_requestID = 0;

            void setRequestID(const uint64_t _requestID)
            {
                m_requestID = _requestID;
            }

            /// \brief Converts structure to JSON.
            /// \return String with JSON.
            std::string toJSON() const
            {
                return static_cast<const T*>(this)->_toJSON();
            }

            /// \brief Init structure from JSON.
            /// \param[in] _json JSON string with structure details.
            void fromJSON(const std::string& _json)
            {
                return static_cast<T*>(this)->_fromJSON(_json);
            }

            /// \brief Init structure from boost's property tree.
            /// \param[in] _pt Property tree with structure details.
            void fromPT(const boost::property_tree::ptree& _pt)
            {
                return static_cast<T*>(this)->_fromPT(_pt);
            }

            /// \brief Equality operator.
            bool operator==(const T& _val) const
            {
                return static_cast<T*>(this)->operator==(_val);
            }
        };

        /// \brief Structure holds information of a done notification. It indicates that a request with a corresponding
        /// requestID is finished and there will be no other responses from DDS in this regards.
        struct SDone : public SBaseMessageImpl<SDone>
        {
          private:
            friend SBaseMessageImpl<SDone>;

            /// \brief Converts structure to JSON.
            /// \return String with JSON.
            std::string _toJSON() const;

            /// \brief Init structure from JSON.
            /// \param[in] _json JSON string with structure details.
            void _fromJSON(const std::string& _json);

            /// \brief Init structure from boost's property tree.
            /// \param[in] _pt Property tree with structure details.
            void _fromPT(const boost::property_tree::ptree& _pt);

            /// \brief Equality operator.
            bool operator==(const SDone& _val) const;
        };

        /// \brief Structure holds information of a progress notification.
        struct SProgress : public SBaseMessageImpl<SProgress>
        {
            uint32_t m_completed = 0;
            uint32_t m_total = 0;
            uint32_t m_errors = 0;
            uint32_t m_time = 0;
            uint16_t m_srcCommand = 0; ///< Reserved for internal use

            SProgress()
                : SBaseMessageImpl<SProgress>()
            {
            }
            SProgress(uint16_t _srcCmd, uint32_t _completed, uint32_t _total, uint32_t _errors, uint32_t _time = 0)
                : SBaseMessageImpl<SProgress>()
            {
                m_srcCommand = _srcCmd;
                m_completed = _completed;
                m_total = _total;
                m_errors = _errors;
                m_time = _time;
            }

          private:
            friend SBaseMessageImpl<SProgress>;

            /// \brief Converts structure to JSON.
            /// \return String with JSON.
            std::string _toJSON() const;

            /// \brief Init structure from JSON.
            /// \param[in] _json JSON string with structure details.
            void _fromJSON(const std::string& _json);

            /// \brief Init structure from boost's property tree.
            /// \param[in] _pt Property tree with structure details.
            void _fromPT(const boost::property_tree::ptree& _pt);

            /// \brief Equality operator.
            bool operator==(const SProgress& _val) const;
        };

        /// \brief Structure holds information of a message notification.
        struct SMessage : public SBaseMessageImpl<SMessage>
        {
            std::string m_msg;                                                                  ///< Message text.
            dds::intercom_api::EMsgSeverity m_severity = dds::intercom_api::EMsgSeverity::info; ///< Message severity.

          private:
            friend SBaseMessageImpl<SMessage>;

            /// \brief Converts structure to JSON.
            /// \return String with JSON.
            std::string _toJSON() const;

            /// \brief Init structure from JSON.
            /// \param[in] _json JSON string with structure details.
            void _fromJSON(const std::string& _json);

            /// \brief Init structure from boost's property tree.
            /// \param[in] _pt Property tree with structure details.
            void _fromPT(const boost::property_tree::ptree& _pt);

            /// \brief Equality operator.
            bool operator==(const SMessage& _val) const;
        };

        /// \brief Structure holds information of a submit notification.
        struct SSubmit : public SBaseMessageImpl<SSubmit>
        {
            std::string m_rms;        ///< RMS.
            uint32_t m_instances = 0; ///< Number of instances.
            std::string m_config;     ///< Path to the configuration file.
            std::string m_pluginPath; ///< Optional. A plug-in's directory search path

          private:
            friend SBaseMessageImpl<SSubmit>;

            /// \brief Converts structure to JSON.
            /// \return String with JSON.
            std::string _toJSON() const;

            /// \brief Init structure from JSON.
            /// \param[in] _json JSON string with structure details.
            void _fromJSON(const std::string& _json);

            /// \brief Init structure from boost's property tree.
            /// \param[in] _pt Property tree with structure details.
            void _fromPT(const boost::property_tree::ptree& _pt);

            /// \brief Equality operator.
            bool operator==(const SSubmit& _val) const;
        };

        /// \brief Structure holds information of topology notifications.
        struct STopology : public SBaseMessageImpl<STopology>
        {
            enum class EUpdateType : uint8_t
            {
                UPDATE = 0,
                ACTIVATE,
                STOP
            };
            EUpdateType m_updateType;         ///< Topology update type: Update, Activate, Stop
            std::string m_topologyFile;       ///< A topology file to process
            bool m_disableValidation = false; ///< A flag to disiable topology validation before processing it.
          private:
            friend SBaseMessageImpl<STopology>;

            /// \brief Converts structure to JSON.
            /// \return String with JSON.
            std::string _toJSON() const;

            /// \brief Init structure from JSON.
            /// \param[in] _json JSON string with structure details.
            void _fromJSON(const std::string& _json);

            /// \brief Init structure from boost's property tree.
            /// \param[in] _pt Property tree with structure details.
            void _fromPT(const boost::property_tree::ptree& _pt);

            /// \brief Equality operator.
            bool operator==(const STopology& _val) const;
        };

        /// \brief Structure holds information of a getlog notification.
        struct SGetLog : public SBaseMessageImpl<SGetLog>
        {
          private:
            friend SBaseMessageImpl<SGetLog>;

            /// \brief Converts structure to JSON.
            /// \return String with JSON.
            std::string _toJSON() const;

            /// \brief Init structure from JSON.
            /// \param[in] _json JSON string with structure details.
            void _fromJSON(const std::string& _json);

            /// \brief Init structure from boost's property tree.
            /// \param[in] _pt Property tree with structure details.
            void _fromPT(const boost::property_tree::ptree& _pt);

            /// \brief Equality operator.
            bool operator==(const SGetLog& _val) const;
        };

        /// \brief Structure holds information of a commanderInfo notification.
        struct SCommanderInfo : public SBaseMessageImpl<SCommanderInfo>
        {
            pid_t m_pid = 0;                ///< PID of the commander
            uint32_t m_idleAgentsCount = 0; ///< The count of idle agents

          private:
            friend SBaseMessageImpl<SCommanderInfo>;

            /// \brief Converts structure to JSON.
            /// \return String with JSON.
            std::string _toJSON() const;

            /// \brief Init structure from JSON.
            /// \param[in] _json JSON string with structure details.
            void _fromJSON(const std::string& _json);

            /// \brief Init structure from boost's property tree.
            /// \param[in] _pt Property tree with structure details.
            void _fromPT(const boost::property_tree::ptree& _pt);

            /// \brief Equality operator.
            bool operator==(const SCommanderInfo& _val) const;
        };

        /// \brief Structure holds information of a agentInfo notification.
        struct SAgentInfo : public SBaseMessageImpl<SAgentInfo>
        {
            uint32_t m_activeAgentsCount = 0; /// the number of online agents
            uint32_t m_index = 0;             /// index of the current agent
            std::string m_agentInfo;          /// info on the current agent

          private:
            friend SBaseMessageImpl<SAgentInfo>;

            /// \brief Converts structure to JSON.
            /// \return String with JSON.
            std::string _toJSON() const;

            /// \brief Init structure from JSON.
            /// \param[in] _json JSON string with structure details.
            void _fromJSON(const std::string& _json);

            /// \brief Init structure from boost's property tree.
            /// \param[in] _pt Property tree with structure details.
            void _fromPT(const boost::property_tree::ptree& _pt);

            /// \brief Equality operator.
            bool operator==(const SAgentInfo& _val) const;
        };
    } // namespace tools_api
} // namespace dds

#endif /* DDS_TOOLSPROTOCOL_H */
