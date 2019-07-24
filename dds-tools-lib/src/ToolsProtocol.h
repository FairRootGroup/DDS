// Copyright 2019 GSI, Inc. All rights reserved.
//
//
//

#ifndef DDS_TOOLSPROTOCOL_H
#define DDS_TOOLSPROTOCOL_H

// STD
#include <ostream>
#include <string>
// BOOST
#include <boost/property_tree/json_parser.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
// DDS
#include "ToolsProtocolCore.h"

namespace dds
{
    namespace tools_api
    {
        /// \brief Structure holds information of a done notification.
        struct SDoneResponseData : SBaseResponseData<SDoneResponseData>
        {
          private:
            friend SBaseData<SDoneResponseData>;
            friend SBaseResponseData<SDoneResponseData>;
            void _fromPT(const boost::property_tree::ptree& _pt);
            void _toPT(boost::property_tree::ptree& _pt) const;
            static std::string _protocolTag()
            {
                return "done";
            }

          public:
            /// \brief Equality operator.
            bool operator==(const SDoneResponseData& _val) const
            {
                return SBaseData::operator==(_val);
            }
        };

        /// \brief Structure holds information of a message notification.
        struct SMessageResponseData : SBaseResponseData<SMessageResponseData>
        {
            std::string m_msg;                                                                  ///< Message text.
            dds::intercom_api::EMsgSeverity m_severity = dds::intercom_api::EMsgSeverity::info; ///< Message severity.

          private:
            friend SBaseData<SMessageResponseData>;
            friend SBaseResponseData<SMessageResponseData>;
            void _fromPT(const boost::property_tree::ptree& _pt);
            void _toPT(boost::property_tree::ptree& _pt) const;
            static std::string _protocolTag()
            {
                return "message";
            }

          public:
            /// \brief Equality operator.
            bool operator==(const SMessageResponseData& _val) const
            {
                return (SBaseData::operator==(_val) && m_msg == _val.m_msg && m_severity == _val.m_severity);
            }

            /// \brief Support ostreaming SMessageResponseData
            friend std::ostream& operator<<(std::ostream& _os, SMessageResponseData _m)
            {
                return _os << "<" << _m.m_severity << "> " << _m.m_msg;
            }
        };

        /// \brief Structure holds information of a progress notification.
        struct SProgressResponseData : SBaseResponseData<SProgressResponseData>
        {
            uint32_t m_completed = 0;
            uint32_t m_total = 0;
            uint32_t m_errors = 0;
            uint32_t m_time = 0;
            uint16_t m_srcCommand = 0; ///< Reserved for internal use

            SProgressResponseData()
            {
            }
            SProgressResponseData(
                uint16_t _srcCmd, uint32_t _completed, uint32_t _total, uint32_t _errors, uint32_t _time = 0)
            {
                m_srcCommand = _srcCmd;
                m_completed = _completed;
                m_total = _total;
                m_errors = _errors;
                m_time = _time;
            }

          private:
            friend SBaseData<SProgressResponseData>;
            friend SBaseResponseData<SProgressResponseData>;
            void _fromPT(const boost::property_tree::ptree& _pt);
            void _toPT(boost::property_tree::ptree& _pt) const;
            static std::string _protocolTag()
            {
                return "progress";
            }

          public:
            /// \brief Equality operator.
            bool operator==(const SProgressResponseData& _val) const
            {
                return (SBaseData::operator==(_val) && m_completed == _val.m_completed && m_total == _val.m_total &&
                        m_errors == _val.m_errors && m_time == _val.m_time);
            }
        };

        struct SSubmitRequestData : SBaseRequestData<SSubmitRequestData>
        {
            std::string m_rms;        ///< RMS.
            uint32_t m_instances = 0; ///< Number of instances.
            std::string m_config;     ///< Path to the configuration file.
            std::string m_pluginPath; ///< Optional. A plug-in's directory search path

          private:
            friend SBaseData<SSubmitRequestData>;
            void _fromPT(const boost::property_tree::ptree& _pt);
            void _toPT(boost::property_tree::ptree& _pt) const;
            static std::string _protocolTag()
            {
                return "submit";
            }

          public:
            /// \brief Equality operator.
            bool operator==(const SSubmitRequestData& _val) const
            {
                return (SBaseData::operator==(_val) && m_rms == _val.m_rms && m_instances == _val.m_instances &&
                        m_config == _val.m_config && m_pluginPath == _val.m_pluginPath);
            }
        };

        /// \brief Structure holds information of a submit notification.
        using SSubmitRequest = SBaseRequestImpl<SSubmitRequestData, SEmptyResponseData>;

        struct STopologyRequestData : SBaseResponseData<STopologyRequestData>
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
            friend SBaseData<STopologyRequestData>;
            void _fromPT(const boost::property_tree::ptree& _pt);
            void _toPT(boost::property_tree::ptree& _pt) const;
            static std::string _protocolTag()
            {
                return "topology";
            }

          public:
            /// \brief Equality operator.
            bool operator==(const STopologyRequestData& _val) const
            {
                return (SBaseData::operator==(_val) && m_updateType == _val.m_updateType &&
                        m_topologyFile == _val.m_topologyFile && m_disableValidation == _val.m_disableValidation);
            }
        };

        /// \brief Structure holds information of topology notifications.
        using STopologyRequest = SBaseRequestImpl<STopologyRequestData, SEmptyResponseData>;

        struct SGetLogRequestData : SBaseRequestData<SGetLogRequestData>
        {
          private:
            friend SBaseData<SGetLogRequestData>;
            friend SBaseRequestData<SGetLogRequestData>;
            void _fromPT(const boost::property_tree::ptree& _pt);
            void _toPT(boost::property_tree::ptree& _pt) const;
            static std::string _protocolTag()
            {
                return "getlog";
            }

          public:
            /// \brief Equality operator.
            bool operator==(const SGetLogRequestData& _val) const
            {
                return SBaseData::operator==(_val);
            }
        };

        /// \brief Structure holds information of a getlog notification.
        using SGetLogRequest = SBaseRequestImpl<SGetLogRequestData, SEmptyResponseData>;

        struct SCommanderInfoResponseData : SBaseResponseData<SCommanderInfoResponseData>
        {
            pid_t m_pid = 0;                  ///< PID of the commander
            std::string m_activeTopologyName; ///< Name of active topology, empty if none is active

          private:
            friend SBaseData<SCommanderInfoResponseData>;
            friend SBaseResponseData<SCommanderInfoResponseData>;
            void _fromPT(const boost::property_tree::ptree& _pt);
            void _toPT(boost::property_tree::ptree& _pt) const;
            static std::string _protocolTag()
            {
                return "commanderInfo";
            }

          public:
            /// \brief Equality operator.
            bool operator==(const SCommanderInfoResponseData& _val) const
            {
                return (SBaseData::operator==(_val) && m_pid == _val.m_pid &&
                        m_activeTopologyName == _val.m_activeTopologyName);
            }
        };

        struct SCommanderInfoRequestData : SBaseRequestData<SCommanderInfoRequestData>
        {
          private:
            friend SBaseData<SCommanderInfoRequestData>;
            friend SBaseRequestData<SCommanderInfoRequestData>;
            void _fromPT(const boost::property_tree::ptree& _pt);
            void _toPT(boost::property_tree::ptree& _pt) const;
            static std::string _protocolTag()
            {
                return "commanderInfo";
            }

          public:
            /// \brief Equality operator.
            bool operator==(const SCommanderInfoRequestData& _val) const
            {
                return SBaseData::operator==(_val);
            }
        };

        /// \brief Structure holds information of a commanderInfo notification.
        using SCommanderInfoRequest = SBaseRequestImpl<SCommanderInfoRequestData, SCommanderInfoResponseData>;

        struct SAgentInfoResponseData : SBaseResponseData<SAgentInfoResponseData>
        {
            uint32_t m_activeAgentsCount = 0;    ///< the number of online agents
            uint32_t m_idleAgentsCount = 0;      ///< The count of idle agents
            uint32_t m_executingAgentsCount = 0; ///< The count of executing agents
            uint32_t m_index = 0;                ///< index of the current agent
            std::string m_agentInfo;             ///< info on the current agent

          private:
            friend SBaseData<SAgentInfoResponseData>;
            friend SBaseResponseData<SAgentInfoResponseData>;
            void _fromPT(const boost::property_tree::ptree& _pt);
            void _toPT(boost::property_tree::ptree& _pt) const;
            static std::string _protocolTag()
            {
                return "agentInfo";
            }

          public:
            /// \brief Equality operator.
            bool operator==(const SAgentInfoResponseData& _val) const
            {
                return (SBaseData::operator==(_val) && m_activeAgentsCount == _val.m_activeAgentsCount &&
                        m_idleAgentsCount == _val.m_idleAgentsCount &&
                        m_executingAgentsCount == _val.m_executingAgentsCount && m_index == _val.m_index &&
                        m_agentInfo == _val.m_agentInfo);
            }
        };

        struct SAgentInfoRequestData : SBaseRequestData<SAgentInfoRequestData>
        {
            bool m_countersOnly = false; ///< Send number of active, idle and executing agents
          private:
            friend SBaseData<SAgentInfoRequestData>;
            friend SBaseRequestData<SAgentInfoRequestData>;
            void _fromPT(const boost::property_tree::ptree& _pt);
            void _toPT(boost::property_tree::ptree& _pt) const;
            static std::string _protocolTag()
            {
                return "agentInfo";
            }

          public:
            /// \brief Equality operator.
            bool operator==(const SAgentInfoRequestData& _val) const
            {
                return SBaseData::operator==(_val) && m_countersOnly == _val.m_countersOnly;
            }
        };

        /// \brief Structure holds information of a agentInfo notification.
        using SAgentInfoRequest = SBaseRequestImpl<SAgentInfoRequestData, SAgentInfoResponseData>;
    } // namespace tools_api
} // namespace dds

#endif /* DDS_TOOLSPROTOCOL_H */
