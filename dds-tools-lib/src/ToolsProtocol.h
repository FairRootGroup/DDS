// Copyright 2019 GSI, Inc. All rights reserved.
//
//
//

#ifndef DDS_TOOLSPROTOCOL_H
#define DDS_TOOLSPROTOCOL_H

// STD
#include <bitset>
#include <chrono>
#include <ostream>
#include <string>
#include <vector>
// BOOST
#include <boost/property_tree/ptree.hpp>
// DDS
#include "Intercom.h"
#include "ToolsProtocolCore.h"

namespace dds
{
    namespace tools_api
    {
        /// \brief Structure holds information of a done response.
        DDS_TOOLS_DECLARE_DATA_CLASS(SBaseResponseData, SDoneResponseData, "done")

        /// \brief Structure holds information of a message response.
        struct SMessageResponseData : SBaseResponseData<SMessageResponseData>
        {
            SMessageResponseData();
            SMessageResponseData(const boost::property_tree::ptree& _pt);

            std::string m_msg;                                                                  ///< Message text.
            dds::intercom_api::EMsgSeverity m_severity = dds::intercom_api::EMsgSeverity::info; ///< Message severity.

          private:
            friend SBaseData<SMessageResponseData>;
            friend SBaseResponseData<SMessageResponseData>;
            void _fromPT(const boost::property_tree::ptree& _pt);
            void _toPT(boost::property_tree::ptree& _pt) const;
            static constexpr const char* _protocolTag = "message";

          public:
            /// \brief Equality operator.
            bool operator==(const SMessageResponseData& _val) const;
            /// \brief Ostream operator.
            friend std::ostream& operator<<(std::ostream& _os, const SMessageResponseData& _data);
        };

        /// \brief Structure holds information of a progress response.
        struct SProgressResponseData : SBaseResponseData<SProgressResponseData>
        {
            SProgressResponseData();
            SProgressResponseData(const boost::property_tree::ptree& _pt);
            SProgressResponseData(
                uint16_t _srcCmd, uint32_t _completed, uint32_t _total, uint32_t _errors, uint32_t _time = 0);

            uint32_t m_completed = 0;
            uint32_t m_total = 0;
            uint32_t m_errors = 0;
            uint32_t m_time = 0;
            uint16_t m_srcCommand = 0; ///< Reserved for internal use

          private:
            friend SBaseData<SProgressResponseData>;
            friend SBaseResponseData<SProgressResponseData>;
            void _fromPT(const boost::property_tree::ptree& _pt);
            void _toPT(boost::property_tree::ptree& _pt) const;
            static constexpr const char* _protocolTag = "progress";

          public:
            /// \brief Equality operator.
            bool operator==(const SProgressResponseData& _val) const;
            /// \brief Ostream operator.
            friend std::ostream& operator<<(std::ostream& _os, const SProgressResponseData& _data);
        };

        /// \brief Structure holds information of a submit request.
        struct SSubmitRequestData : SBaseRequestData<SSubmitRequestData>
        {
            /// \brief Additional flags of the SSubmitRequestData
            /// Uee SSubmitRequestData::setFlag to set flags.
            enum class ESubmitRequestFlags : uint32_t
            {
                enable_overbooking,
                //----------
                size_value
            };

            using flagContainer_t = std::bitset<(uint32_t)ESubmitRequestFlags::size_value>;

            SSubmitRequestData();
            SSubmitRequestData(const boost::property_tree::ptree& _pt);

            void setFlag(const ESubmitRequestFlags& _flag, bool _value);
            static void setFlag(flagContainer_t* _flagContainer, const ESubmitRequestFlags& _flag, bool _value);
            bool isFlagEnabled(const ESubmitRequestFlags& _flag) const;
            static bool isFlagEnabled(const uint32_t& _flagContainer, const ESubmitRequestFlags& _flag);

            std::string m_rms;            ///< RMS.
            uint32_t m_instances = 0;     ///< A number of instances.
            uint32_t m_minInstances = 0;  ///< A minimum number of instances.
            uint32_t m_slots = 0;         ///< Number of task slots.
            uint32_t m_flags = 0;         ///< Additional flags, see SSubmitRequestData::ESubmitRequestFlags
            std::string m_config;         ///< A path to the RMS job configuration file.
            std::string m_pluginPath;     ///< Optional. A plug-in's directory search path
            std::string m_groupName;      ///<  A group name of agents.
            std::string m_submissionTag;  ///< A Submission Tag
            std::string m_envCfgFilePath; ///< A path to a user environment script. Will be executed once per agent
                                          ///< (valid for all task slots of the agent)
            std::string m_inlineConfig;   ///< Content of this buffer will be added to the RMS job configuration file.

          private:
            friend SBaseData<SSubmitRequestData>;
            void _fromPT(const boost::property_tree::ptree& _pt);
            void _toPT(boost::property_tree::ptree& _pt) const;
            static constexpr const char* _protocolTag = "submit";

          public:
            /// \brief Equality operator.
            bool operator==(const SSubmitRequestData& _val) const;
            /// \brief Ostream operator.
            friend std::ostream& operator<<(std::ostream& _os, const SSubmitRequestData& _data);

          private:
            flagContainer_t m_flagContainer;
        };

        /// \brief Structure holds information of a submit response.
        struct SSubmitResponseData : SBaseResponseData<SSubmitResponseData>
        {
            SSubmitResponseData();
            SSubmitResponseData(const boost::property_tree::ptree& _pt);

            std::vector<std::string> m_jobIDs; ///< RMS Job IDs (can be multiple for job arrays)
            uint32_t m_allocNodes{ 0 };        ///< Number of allocated nodes
            uint32_t m_state{ 0 };             ///< Job state (1=RUNNING, 2=COMPLETED)
            bool m_jobInfoAvailable{ false };  ///< Flag indicating if job info was successfully retrieved

          private:
            friend SBaseData<SSubmitResponseData>;
            friend SBaseResponseData<SSubmitResponseData>;
            void _fromPT(const boost::property_tree::ptree& _pt);
            void _toPT(boost::property_tree::ptree& _pt) const;
            static constexpr const char* _protocolTag = "submit";

          public:
            /// \brief Equality operator.
            bool operator==(const SSubmitResponseData& _val) const;
            /// \brief Ostream operator.
            friend std::ostream& operator<<(std::ostream& _os, const SSubmitResponseData& _data);
        };

        /// \brief Request class of submit.
        using SSubmitRequest = SBaseRequestImpl<SSubmitRequestData, SSubmitResponseData>;

        /// \brief Structure holds information of topology request.
        struct STopologyRequestData : SBaseRequestData<STopologyRequestData>
        {
            STopologyRequestData();
            STopologyRequestData(const boost::property_tree::ptree& _pt);

            enum class EUpdateType : uint8_t
            {
                UPDATE = 0,
                ACTIVATE,
                STOP
            };
            EUpdateType m_updateType = EUpdateType::UPDATE; ///< Topology update type: Update, Activate, Stop
            std::string m_topologyFile;                     ///< A topology file to process
            bool m_disableValidation = false; ///< A flag to disable topology validation before processing it.

          private:
            friend SBaseData<STopologyRequestData>;
            void _fromPT(const boost::property_tree::ptree& _pt);
            void _toPT(boost::property_tree::ptree& _pt) const;
            static constexpr const char* _protocolTag = "topology";

          public:
            /// \brief Equality operator.
            bool operator==(const STopologyRequestData& _val) const;
            /// \brief Ostream operator.
            friend std::ostream& operator<<(std::ostream& _os, const STopologyRequestData& _data);
        };

        /// \brief Structure holds information of topology response - activated and stopped tasks.
        struct STopologyResponseData : SBaseResponseData<STopologyResponseData>
        {
            STopologyResponseData();
            STopologyResponseData(const boost::property_tree::ptree& _pt);

            bool m_activated{ true };     ///< True if task was activated, otherwise it's stopped
            uint64_t m_agentID{ 0 };      ///< Agent ID
            uint64_t m_slotID{ 0 };       ///< Slot ID
            uint64_t m_taskID{ 0 };       ///< Task ID, 0 if not assigned
            uint64_t m_collectionID{ 0 }; ///< Collection ID, 0 if not in a collection
            std::string m_path;           ///< Path in the topology
            std::string m_host;           ///< Hostname
            std::string m_wrkDir;         ///< Wrk directory

          private:
            friend SBaseData<STopologyResponseData>;
            friend SBaseResponseData<STopologyResponseData>;
            void _fromPT(const boost::property_tree::ptree& _pt);
            void _toPT(boost::property_tree::ptree& _pt) const;
            static constexpr const char* _protocolTag = "topology";

          public:
            /// \brief Equality operator.
            bool operator==(const STopologyResponseData& _val) const;
            /// \brief Ostream operator.
            friend std::ostream& operator<<(std::ostream& _os, const STopologyResponseData& _data);
        };

        /// \brief Request class of topology.
        using STopologyRequest = SBaseRequestImpl<STopologyRequestData, STopologyResponseData>;

        /// \brief Structure holds information of a getlog request.
        DDS_TOOLS_DECLARE_DATA_CLASS(SBaseRequestData, SGetLogRequestData, "getlog")

        /// \brief Request class of getlog.
        using SGetLogRequest = SBaseRequestImpl<SGetLogRequestData, SEmptyResponseData>;

        /// \brief Structure holds information of commanderInfo response.
        struct SCommanderInfoResponseData : SBaseResponseData<SCommanderInfoResponseData>
        {
            SCommanderInfoResponseData();
            SCommanderInfoResponseData(const boost::property_tree::ptree& _pt);

            pid_t m_pid = 0;                  ///< PID of the commander
            std::string m_activeTopologyName; ///< Name of active topology, empty if none is active
            std::string m_activeTopologyPath; ///< Filepath of active topology, empty if none is active

          private:
            friend SBaseData<SCommanderInfoResponseData>;
            friend SBaseResponseData<SCommanderInfoResponseData>;
            void _fromPT(const boost::property_tree::ptree& _pt);
            void _toPT(boost::property_tree::ptree& _pt) const;
            static constexpr const char* _protocolTag = "commanderInfo";

          public:
            /// \brief Equality operator.
            bool operator==(const SCommanderInfoResponseData& _val) const;
            /// \brief Ostream operator.
            friend std::ostream& operator<<(std::ostream& _os, const SCommanderInfoResponseData& _data);
        };

        /// \brief Structure holds information of a commanderInfo request.
        DDS_TOOLS_DECLARE_DATA_CLASS(SBaseRequestData, SCommanderInfoRequestData, "commanderInfo")

        /// \brief Request class of commanderInfo.
        using SCommanderInfoRequest = SBaseRequestImpl<SCommanderInfoRequestData, SCommanderInfoResponseData>;

        /// \brief Structure holds information of agentInfo response.
        struct SAgentInfoResponseData : SBaseResponseData<SAgentInfoResponseData>
        {
            SAgentInfoResponseData();
            SAgentInfoResponseData(const boost::property_tree::ptree& _pt);

            uint32_t m_index{ 0 };                                                   ///< Index of the current agent
            uint64_t m_agentID{ 0 };                                                 ///< Agent ID
            std::chrono::milliseconds m_startUpTime{ std::chrono::milliseconds(0) }; ///< Agent's startup time
            std::string m_username;                                                  ///< Username
            std::string m_host;                                                      ///< Hostname
            std::string m_DDSPath;                                                   ///< DDS path
            std::string m_groupName;                                                 ///< Agent group name
            uint32_t m_agentPid{ 0 };                                                ///< Agent's process ID
            uint32_t m_nSlots{ 0 };                                                  ///< Number of task slots
            uint32_t m_nIdleSlots{ 0 };                                              ///< Number of idle slots
            uint32_t m_nExecutingSlots{ 0 };                                         ///< Number of executing slots

          private:
            friend SBaseData<SAgentInfoResponseData>;
            friend SBaseResponseData<SAgentInfoResponseData>;
            void _fromPT(const boost::property_tree::ptree& _pt);
            void _toPT(boost::property_tree::ptree& _pt) const;
            static constexpr const char* _protocolTag = "agentInfo";

          public:
            /// \brief Equality operator.
            bool operator==(const SAgentInfoResponseData& _val) const;
            /// \brief Ostream operator.
            friend std::ostream& operator<<(std::ostream& _os, const SAgentInfoResponseData& _data);
        };

        /// \brief Structure holds information of agentInfo request.
        DDS_TOOLS_DECLARE_DATA_CLASS(SBaseRequestData, SAgentInfoRequestData, "agentInfo")

        /// \brief Request class of agentInfo.
        using SAgentInfoRequest = SBaseRequestImpl<SAgentInfoRequestData, SAgentInfoResponseData>;

        /// \brief Structure holds information of slot info response.
        struct SSlotInfoResponseData : SBaseResponseData<SSlotInfoResponseData>
        {
            SSlotInfoResponseData();
            SSlotInfoResponseData(const boost::property_tree::ptree& _pt);

            uint32_t m_index{ 0 };   ///< Index of the current slot
            uint64_t m_agentID{ 0 }; ///< Agent ID
            uint64_t m_slotID{ 0 };  ///< Agent ID
            uint64_t m_taskID{ 0 };  ///< Task ID, 0 if not assigned
            uint32_t m_state{ 0 };   ///< Slot state
            std::string m_host;      ///< Hostname
            std::string m_wrkDir;    ///< Wrk directory

          private:
            friend SBaseData<SSlotInfoResponseData>;
            friend SBaseResponseData<SSlotInfoResponseData>;
            void _fromPT(const boost::property_tree::ptree& _pt);
            void _toPT(boost::property_tree::ptree& _pt) const;
            static constexpr const char* _protocolTag = "slotInfo";

          public:
            /// \brief Equality operator.
            bool operator==(const SSlotInfoResponseData& _val) const;
            /// \brief Ostream operator.
            friend std::ostream& operator<<(std::ostream& _os, const SSlotInfoResponseData& _data);
        };

        /// \brief Structure holds information of slottInfo request.
        DDS_TOOLS_DECLARE_DATA_CLASS(SBaseRequestData, SSlotInfoRequestData, "slotInfo")

        /// \brief Request class of agentInfo.
        using SSlotInfoRequest = SBaseRequestImpl<SSlotInfoRequestData, SSlotInfoResponseData>;

        /// \brief Structure holds information of agentCount response.
        struct SAgentCountResponseData : SBaseResponseData<SAgentCountResponseData>
        {
            SAgentCountResponseData();
            SAgentCountResponseData(const boost::property_tree::ptree& _pt);

            uint32_t m_activeSlotsCount = 0;    ///< The number of online slots
            uint32_t m_idleSlotsCount = 0;      ///< The count of idle slots
            uint32_t m_executingSlotsCount = 0; ///< The count of executing slots

          private:
            friend SBaseData<SAgentCountResponseData>;
            friend SBaseResponseData<SAgentCountResponseData>;
            void _fromPT(const boost::property_tree::ptree& _pt);
            void _toPT(boost::property_tree::ptree& _pt) const;
            static constexpr const char* _protocolTag = "agentCount";

          public:
            /// \brief Equality operator.
            bool operator==(const SAgentCountResponseData& _val) const;
            /// \brief Ostream operator.
            friend std::ostream& operator<<(std::ostream& _os, const SAgentCountResponseData& _data);
        };

        /// \brief Structure holds information of agentCount response.
        DDS_TOOLS_DECLARE_DATA_CLASS(SBaseRequestData, SAgentCountRequestData, "agentCount")

        /// \brief Request class of agentCount.
        using SAgentCountRequest = SBaseRequestImpl<SAgentCountRequestData, SAgentCountResponseData>;

        /// \brief Structure holds information of onTaskDone response.
        struct SOnTaskDoneResponseData : SBaseResponseData<SOnTaskDoneResponseData>
        {
            SOnTaskDoneResponseData();
            SOnTaskDoneResponseData(const boost::property_tree::ptree& _pt);

            uint64_t m_taskID{ 0 };   ///< Task ID
            uint32_t m_exitCode{ 0 }; ///< Exit code
            uint32_t m_signal{ 0 };   ///< A signal number if the process is killed by/stopped by a signal
            std::string m_host;       ///< Hostname
            std::string m_wrkDir;     ///< Working directory
            std::string m_taskPath;   ///< Task path in the topology

          private:
            friend SBaseData<SOnTaskDoneResponseData>;
            friend SBaseResponseData<SOnTaskDoneResponseData>;
            void _fromPT(const boost::property_tree::ptree& _pt);
            void _toPT(boost::property_tree::ptree& _pt) const;
            static constexpr const char* _protocolTag = "onTaskDone";

          public:
            /// \brief Equality operator.
            bool operator==(const SOnTaskDoneResponseData& _val) const;
            /// \brief Ostream operator.
            friend std::ostream& operator<<(std::ostream& _os, const SOnTaskDoneResponseData& _data);
        };

        /// \brief Structure holds information of onTaskDone response.
        DDS_TOOLS_DECLARE_DATA_CLASS(SBaseRequestData, SOnTaskDoneRequestData, "onTaskDone")

        /// \brief Request class of onTaskDone.
        using SOnTaskDoneRequest = SBaseRequestImpl<SOnTaskDoneRequestData, SOnTaskDoneResponseData>;

        /// \brief Structure holds information of agentCommand request.
        struct SAgentCommandRequestData : SBaseRequestData<SAgentCommandRequestData>
        {

            SAgentCommandRequestData();
            SAgentCommandRequestData(const boost::property_tree::ptree& _pt);

            enum class EAgentCommandType : uint8_t
            {
                shutDownByID = 0, ///<  m_arg1 should be set to a desired agent ID to shutdown
                shutDownBySlotID, ///<  m_arg1 should be set to a desired slot ID. The corresponding agent holding this
                                  ///<  slot will be shutdown.
                // stopTaskByTaskID,
                // stopTaskByAgentID,
                // restartTaskByTaskID,
                // restartTaskByAgentID
            };

            EAgentCommandType m_commandType = EAgentCommandType::shutDownByID;
            uint64_t m_arg1{ 0 }; ///< argument #1 - numeric. The usage depends on the command.
            std::string m_arg2;   ///< argument #1 - string.  The usage depends on the command.

          private:
            friend SBaseData<SAgentCommandRequestData>;
            void _fromPT(const boost::property_tree::ptree& _pt);
            void _toPT(boost::property_tree::ptree& _pt) const;
            static constexpr const char* _protocolTag = "agentCommand";

          public:
            /// \brief Equality operator.
            bool operator==(const SAgentCommandRequestData& _val) const;
            /// \brief Ostream operator.
            friend std::ostream& operator<<(std::ostream& _os, const SAgentCommandRequestData& _data);
        };

        /// \brief Request class of submit.
        using SAgentCommandRequest = SBaseRequestImpl<SAgentCommandRequestData, SEmptyResponseData>;

        /// \brief Structure holds information of RMS job info request.
        struct SRMSJobInfoRequestData : SBaseRequestData<SRMSJobInfoRequestData>
        {
            SRMSJobInfoRequestData();
            SRMSJobInfoRequestData(const boost::property_tree::ptree& _pt);

            std::string m_submissionID; ///< Submission ID.

          private:
            friend SBaseData<SRMSJobInfoRequestData>;
            void _fromPT(const boost::property_tree::ptree& _pt);
            void _toPT(boost::property_tree::ptree& _pt) const;
            static constexpr const char* _protocolTag = "rmsJobInfo";

          public:
            /// \brief Equality operator.
            bool operator==(const SRMSJobInfoRequestData& _val) const;
            /// \brief Ostream operator.
            friend std::ostream& operator<<(std::ostream& _os, const SRMSJobInfoRequestData& _data);
        };

        /// \brief Structure holds information of RMS job info response.
        struct SRMSJobInfoResponseData : SBaseResponseData<SRMSJobInfoResponseData>
        {
            SRMSJobInfoResponseData();
            SRMSJobInfoResponseData(const boost::property_tree::ptree& _pt);

            uint32_t m_allocNodes; ///< Allocated nodes.
            uint32_t m_state;      ///< Job state.
            std::string m_jobName; ///< Job name.

          private:
            friend SBaseData<SRMSJobInfoResponseData>;
            friend SBaseResponseData<SRMSJobInfoResponseData>;
            void _fromPT(const boost::property_tree::ptree& _pt);
            void _toPT(boost::property_tree::ptree& _pt) const;
            static constexpr const char* _protocolTag = "rmsJobInfo";

          public:
            /// \brief Equality operator.
            bool operator==(const SRMSJobInfoResponseData& _val) const;
            /// \brief Ostream operator.
            friend std::ostream& operator<<(std::ostream& _os, const SRMSJobInfoResponseData& _data);
        };

        /// \brief Request class of RMS job info.
        using SRMSJobInfoRequest = SBaseRequestImpl<SRMSJobInfoRequestData, SRMSJobInfoResponseData>;

    } // namespace tools_api
} // namespace dds

#endif /* DDS_TOOLSPROTOCOL_H */
