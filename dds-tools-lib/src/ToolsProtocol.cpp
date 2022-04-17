// Copyright 2019 GSI, Inc. All rights reserved.
//
//
//

#include "ToolsProtocol.h"

using namespace std;
using namespace dds;
using namespace dds::tools_api;
using namespace dds::intercom_api;
using namespace boost::property_tree;

// this declaration is important to help older compilers to eat this static constexpr
constexpr const char* SGetLogRequestData::_protocolTag;
constexpr const char* SAgentInfoRequestData::_protocolTag;
constexpr const char* SSlotInfoRequestData::_protocolTag;
constexpr const char* SAgentCountRequestData::_protocolTag;
constexpr const char* SCommanderInfoRequestData::_protocolTag;
constexpr const char* SDoneResponseData::_protocolTag;

///////////////////////////////////
// SProgressResponseData
///////////////////////////////////

// this declaration is important to help older compilers to eat this static constexpr
constexpr const char* SProgressResponseData::_protocolTag;

SProgressResponseData::SProgressResponseData()
{
}

SProgressResponseData::SProgressResponseData(const boost::property_tree::ptree& _pt)
{
    fromPT(_pt);
}

SProgressResponseData::SProgressResponseData(
    uint16_t _srcCmd, uint32_t _completed, uint32_t _total, uint32_t _errors, uint32_t _time)
    : m_completed(_completed)
    , m_total(_total)
    , m_errors(_errors)
    , m_time(_time)
    , m_srcCommand(_srcCmd)
{
}

void SProgressResponseData::_toPT(boost::property_tree::ptree& _pt) const
{
    _pt.put<uint32_t>("completed", m_completed);
    _pt.put<uint32_t>("total", m_total);
    _pt.put<uint32_t>("errors", m_errors);
    _pt.put<uint32_t>("time", m_time);
    _pt.put<uint32_t>("srcCommand", m_srcCommand);
}

void SProgressResponseData::_fromPT(const boost::property_tree::ptree& _pt)
{
    m_completed = _pt.get<uint32_t>("completed", 0);
    m_total = _pt.get<uint32_t>("total", 0);
    m_errors = _pt.get<uint32_t>("errors", 0);
    m_time = _pt.get<uint32_t>("time", 0);
    m_srcCommand = _pt.get<uint32_t>("srcCommand", 0);
}

bool SProgressResponseData::operator==(const SProgressResponseData& _val) const
{
    return (SBaseData::operator==(_val) && m_completed == _val.m_completed && m_total == _val.m_total &&
            m_errors == _val.m_errors && m_time == _val.m_time);
}

// We need to put function implementation in the same "dds::tools_api" namespace as a friend function declaration.
// Such declaration "std::ostream& dds::tools_api::operator<<(std::ostream& _os, const ***& _data)" doesn't help.
// In order to silent GCC warning "*** has not been declared within 'dds::tools_api'"
namespace dds
{
    namespace tools_api
    {
        std::ostream& operator<<(std::ostream& _os, const SProgressResponseData& _data)
        {
            return _os << _data.defaultToString() << "; completed: " << _data.m_completed
                       << "; total: " << _data.m_total << "; errors: " << _data.m_errors << "; time: " << _data.m_time
                       << "; srcCommand: " << _data.m_srcCommand;
        }
    } // namespace tools_api
} // namespace dds

///////////////////////////////////
// SMessageResponseData
///////////////////////////////////

// this declaration is important to help older compilers to eat this static constexpr
constexpr const char* SMessageResponseData::_protocolTag;

string SeverityToTag(EMsgSeverity _severity)
{
    switch (_severity)
    {
        case EMsgSeverity::info:
            return "info";
        case EMsgSeverity::error:
            return "error";
        default:
            throw runtime_error("Message severity not found.");
    }
}

EMsgSeverity TagToSeverity(const std::string& _tag)
{
    if (_tag == "info")
        return EMsgSeverity::info;
    else if (_tag == "error")
        return EMsgSeverity::error;
    else
        throw runtime_error("Message severity for tag " + _tag + " does not exist.");
}

SMessageResponseData::SMessageResponseData()
{
}

SMessageResponseData::SMessageResponseData(const boost::property_tree::ptree& _pt)
{
    fromPT(_pt);
}

void SMessageResponseData::_toPT(boost::property_tree::ptree& _pt) const
{
    _pt.put<string>("msg", m_msg);
    _pt.put<string>("severity", SeverityToTag(m_severity));
}

void SMessageResponseData::_fromPT(const boost::property_tree::ptree& _pt)
{
    m_severity = TagToSeverity(_pt.get<string>("severity", "info"));
    m_msg = _pt.get<string>("msg", "");
}

bool SMessageResponseData::operator==(const SMessageResponseData& _val) const
{
    return (SBaseData::operator==(_val) && m_msg == _val.m_msg && m_severity == _val.m_severity);
}

// We need to put function implementation in the same "dds::tools_api" namespace as a friend function declaration.
// Such declaration "std::ostream& dds::tools_api::operator<<(std::ostream& _os, const ***& _data)" doesn't help.
// In order to silent GCC warning "*** has not been declared within 'dds::tools_api'"
namespace dds
{
    namespace tools_api
    {
        std::ostream& operator<<(std::ostream& _os, const SMessageResponseData& _data)
        {
            return _os << _data.defaultToString() << "; severity: " << _data.m_severity << "; msg: " << _data.m_msg;
        }
    } // namespace tools_api
} // namespace dds

///////////////////////////////////
// SSubmitRequestData
///////////////////////////////////

// this declaration is important to help older compilers to eat this static constexpr
constexpr const char* SSubmitRequestData::_protocolTag;

SSubmitRequestData::SSubmitRequestData()
{
}

SSubmitRequestData::SSubmitRequestData(const boost::property_tree::ptree& _pt)
{
    fromPT(_pt);
}

void SSubmitRequestData::_toPT(boost::property_tree::ptree& _pt) const
{
    _pt.put<int>("instances", m_instances);
    _pt.put<int>("slots", m_slots);
    _pt.put<string>("config", m_config);
    _pt.put<string>("rms", m_rms);
    _pt.put<string>("pluginPath", m_pluginPath);
    _pt.put<string>("groupName", m_groupName);
}

void SSubmitRequestData::_fromPT(const boost::property_tree::ptree& _pt)
{
    m_instances = _pt.get<int>("instances", 0);
    m_slots = _pt.get<int>("slots", 0);
    m_config = _pt.get<string>("config", "");
    m_rms = _pt.get<string>("rms", "");
    m_pluginPath = _pt.get<string>("pluginPath", "");
    m_groupName = _pt.get<string>("groupName", "");
}

bool SSubmitRequestData::operator==(const SSubmitRequestData& _val) const
{
    return (SBaseData::operator==(_val) && m_rms == _val.m_rms && m_instances == _val.m_instances &&
            m_slots == _val.m_slots && m_config == _val.m_config && m_pluginPath == _val.m_pluginPath &&
            m_groupName == _val.m_groupName);
}

// We need to put function implementation in the same "dds::tools_api" namespace as a friend function declaration.
// Such declaration "std::ostream& dds::tools_api::operator<<(std::ostream& _os, const ***& _data)" doesn't help.
// In order to silent GCC warning "*** has not been declared within 'dds::tools_api'"
namespace dds
{
    namespace tools_api
    {
        std::ostream& operator<<(std::ostream& _os, const SSubmitRequestData& _data)
        {
            return _os << _data.defaultToString() << "; instances: " << _data.m_instances
                       << "; slots: " << _data.m_slots << "; config: " << _data.m_config << "; rms: " << _data.m_rms
                       << "; pluginPath: " << _data.m_pluginPath << "; groupName: " << _data.m_groupName;
        }
    } // namespace tools_api
} // namespace dds

///////////////////////////////////
// STopologyRequestData
///////////////////////////////////

// this declaration is important to help older compilers to eat this static constexpr
constexpr const char* STopologyRequestData::_protocolTag;

STopologyRequestData::STopologyRequestData()
{
}

STopologyRequestData::STopologyRequestData(const boost::property_tree::ptree& _pt)
{
    fromPT(_pt);
}

void STopologyRequestData::_toPT(boost::property_tree::ptree& _pt) const
{
    _pt.put<uint8_t>("updateType", static_cast<uint8_t>(m_updateType));
    _pt.put<string>("topologyFile", m_topologyFile);
    _pt.put<bool>("disableValidation", m_disableValidation);
}

void STopologyRequestData::_fromPT(const boost::property_tree::ptree& _pt)
{
    m_updateType = static_cast<EUpdateType>(_pt.get<uint8_t>("updateType", 0));
    m_topologyFile = _pt.get<string>("topologyFile", "");
    m_disableValidation = _pt.get<bool>("disableValidation", false);
}

bool STopologyRequestData::operator==(const STopologyRequestData& _val) const
{
    return (SBaseData::operator==(_val) && m_updateType == _val.m_updateType && m_topologyFile == _val.m_topologyFile &&
            m_disableValidation == _val.m_disableValidation);
}

// We need to put function implementation in the same "dds::tools_api" namespace as a friend function declaration.
// Such declaration "std::ostream& dds::tools_api::operator<<(std::ostream& _os, const ***& _data)" doesn't help.
// In order to silent GCC warning "*** has not been declared within 'dds::tools_api'"
namespace dds
{
    namespace tools_api
    {
        std::ostream& operator<<(std::ostream& _os, const STopologyRequestData& _data)
        {
            return _os << _data.defaultToString() << "; updateType: " << static_cast<uint8_t>(_data.m_updateType)
                       << "; topologyFile: " << _data.m_topologyFile
                       << "; disableValidation: " << _data.m_disableValidation;
        }
    } // namespace tools_api
} // namespace dds

///////////////////////////////////
// STopologyResponseData
///////////////////////////////////

// this declaration is important to help older compilers to eat this static constexpr
constexpr const char* STopologyResponseData::_protocolTag;

STopologyResponseData::STopologyResponseData()
{
}

STopologyResponseData::STopologyResponseData(const boost::property_tree::ptree& _pt)
{
    fromPT(_pt);
}

void STopologyResponseData::_toPT(boost::property_tree::ptree& _pt) const
{
    _pt.put<bool>("activated", m_activated);
    _pt.put<uint64_t>("agentID", m_agentID);
    _pt.put<uint64_t>("slotID", m_slotID);
    _pt.put<uint64_t>("taskID", m_taskID);
    _pt.put<uint64_t>("collectionID", m_collectionID);
    _pt.put<string>("path", m_path);
    _pt.put<string>("host", m_host);
    _pt.put<string>("wrkDir", m_wrkDir);
}

void STopologyResponseData::_fromPT(const boost::property_tree::ptree& _pt)
{
    m_activated = _pt.get<bool>("activated", true);
    m_agentID = _pt.get<uint64_t>("agentID", 0);
    m_slotID = _pt.get<uint64_t>("slotID", 0);
    m_taskID = _pt.get<uint64_t>("taskID", 0);
    m_collectionID = _pt.get<uint64_t>("collectionID", 0);
    m_path = _pt.get<string>("path", "");
    m_host = _pt.get<string>("host", "");
    m_wrkDir = _pt.get<string>("wrkDir", "");
}

bool STopologyResponseData::operator==(const STopologyResponseData& _val) const
{
    return (SBaseData::operator==(_val) && m_activated == _val.m_activated && m_agentID == _val.m_agentID &&
            m_slotID == _val.m_slotID && m_taskID == _val.m_taskID && m_collectionID == _val.m_collectionID &&
            m_path == _val.m_path && m_host == _val.m_host && m_wrkDir == _val.m_wrkDir);
}

// We need to put function implementation in the same "dds::tools_api" namespace as a friend function declaration.
// Such declaration "std::ostream& dds::tools_api::operator<<(std::ostream& _os, const ***& _data)" doesn't help.
// In order to silent GCC warning "*** has not been declared within 'dds::tools_api'"
namespace dds
{
    namespace tools_api
    {
        std::ostream& operator<<(std::ostream& _os, const STopologyResponseData& _data)
        {
            return _os << _data.defaultToString() << "; activated: " << _data.m_activated
                       << "; agentID: " << _data.m_agentID << "; slotID: " << _data.m_slotID
                       << "; taskID: " << _data.m_taskID << "; collectionID: " << _data.m_collectionID
                       << "; path: " << quoted(_data.m_path) << "; host: " << _data.m_host
                       << "; wrkDir: " << quoted(_data.m_wrkDir);
        }
    } // namespace tools_api
} // namespace dds

///////////////////////////////////
// SCommanderInfoResponseData
///////////////////////////////////

// this declaration is important to help older compilers to eat this static constexpr
constexpr const char* SCommanderInfoResponseData::_protocolTag;

SCommanderInfoResponseData::SCommanderInfoResponseData()
{
}

SCommanderInfoResponseData::SCommanderInfoResponseData(const boost::property_tree::ptree& _pt)
{
    fromPT(_pt);
}

void SCommanderInfoResponseData::_toPT(boost::property_tree::ptree& _pt) const
{
    _pt.put<pid_t>("pid", m_pid);
    _pt.put<std::string>("activeTopologyName", m_activeTopologyName);
    _pt.put<std::string>("activeTopologyPath", m_activeTopologyPath);
}

void SCommanderInfoResponseData::_fromPT(const boost::property_tree::ptree& _pt)
{
    m_pid = _pt.get<pid_t>("pid", 0);
    m_activeTopologyName = _pt.get<std::string>("activeTopologyName", std::string());
    m_activeTopologyPath = _pt.get<std::string>("activeTopologyPath", std::string());
}

bool SCommanderInfoResponseData::operator==(const SCommanderInfoResponseData& _val) const
{
    return (SBaseData::operator==(_val) && m_pid == _val.m_pid && m_activeTopologyName == _val.m_activeTopologyName &&
            m_activeTopologyPath == _val.m_activeTopologyPath);
}

// We need to put function implementation in the same "dds::tools_api" namespace as a friend function declaration.
// Such declaration "std::ostream& dds::tools_api::operator<<(std::ostream& _os, const ***& _data)" doesn't help.
// In order to silent GCC warning "*** has not been declared within 'dds::tools_api'"
namespace dds
{
    namespace tools_api
    {
        std::ostream& operator<<(std::ostream& _os, const SCommanderInfoResponseData& _data)
        {
            return _os << _data.defaultToString() << "; pid: " << _data.m_pid
                       << "; activeTopologyName: " << _data.m_activeTopologyName
                       << "; activeTopologyPath: " << _data.m_activeTopologyPath;
        }
    } // namespace tools_api
} // namespace dds

///////////////////////////////////
// SAgentInfoResponseData
///////////////////////////////////

// this declaration is important to help older compilers to eat this static constexpr
constexpr const char* SAgentInfoResponseData::_protocolTag;

SAgentInfoResponseData::SAgentInfoResponseData()
{
}

SAgentInfoResponseData::SAgentInfoResponseData(const boost::property_tree::ptree& _pt)
{
    fromPT(_pt);
}

void SAgentInfoResponseData::_toPT(boost::property_tree::ptree& _pt) const
{
    _pt.put<uint32_t>("index", m_index);
    _pt.put<uint64_t>("agentID", m_agentID);
    _pt.put<size_t>("startUpTime", m_startUpTime.count());
    _pt.put<string>("username", m_username);
    _pt.put<string>("host", m_host);
    _pt.put<string>("DDSPath", m_DDSPath);
    _pt.put<string>("groupName", m_groupName);
    _pt.put<uint32_t>("agentPid", m_agentPid);
    _pt.put<uint32_t>("slots", m_nSlots);
    _pt.put<uint32_t>("idleSlots", m_nIdleSlots);
    _pt.put<uint32_t>("executingSlots", m_nExecutingSlots);
}

void SAgentInfoResponseData::_fromPT(const boost::property_tree::ptree& _pt)
{
    m_index = _pt.get<uint32_t>("index", 0);
    m_agentID = _pt.get<uint64_t>("agentID", 0);
    m_startUpTime = std::chrono::milliseconds(_pt.get<size_t>("startUpTime", 0));
    m_username = _pt.get<string>("username", "");
    m_host = _pt.get<string>("host", "");
    m_DDSPath = _pt.get<string>("DDSPath", "");
    m_groupName = _pt.get<string>("groupName", "");
    m_agentPid = _pt.get<uint32_t>("agentPid", 0);
    m_nSlots = _pt.get<uint32_t>("slots", 0);
    m_nIdleSlots = _pt.get<uint32_t>("idleSlots", 0);
    m_nExecutingSlots = _pt.get<uint32_t>("executingSlots", 0);
}

bool SAgentInfoResponseData::operator==(const SAgentInfoResponseData& _val) const
{
    return (SBaseData::operator==(_val) && m_index == _val.m_index && m_agentID == _val.m_agentID &&
            m_startUpTime == _val.m_startUpTime && m_username == _val.m_username && m_host == _val.m_host &&
            m_DDSPath == _val.m_DDSPath && m_groupName == _val.m_groupName && m_agentPid == _val.m_agentPid &&
            m_nSlots == _val.m_nSlots && m_nIdleSlots == _val.m_nIdleSlots &&
            m_nExecutingSlots == _val.m_nExecutingSlots);
}

// We need to put function implementation in the same "dds::tools_api" namespace as a friend function declaration.
// Such declaration "std::ostream& dds::tools_api::operator<<(std::ostream& _os, const ***& _data)" doesn't help.
// In order to silent GCC warning "*** has not been declared within 'dds::tools_api'"
namespace dds
{
    namespace tools_api
    {
        std::ostream& operator<<(std::ostream& _os, const SAgentInfoResponseData& _data)
        {
            return _os << _data.defaultToString() << "; index: " << _data.m_index << "; agentID: " << _data.m_agentID
                       << "; groupName: " << _data.m_groupName << "; startUpTime: " << _data.m_startUpTime.count()
                       << "; username: " << _data.m_username << "; host: " << _data.m_host
                       << "; DDSPath: " << _data.m_DDSPath << "; agentPid: " << _data.m_agentPid
                       << "; nSlots: " << _data.m_nSlots << "; nIdleSlots: " << _data.m_nIdleSlots
                       << "; nExecutingSlots: " << _data.m_nExecutingSlots;
        }
    } // namespace tools_api
} // namespace dds

///////////////////////////////////
// SSlotInfoResponseData
///////////////////////////////////

// this declaration is important to help older compilers to eat this static constexpr
constexpr const char* SSlotInfoResponseData::_protocolTag;

SSlotInfoResponseData::SSlotInfoResponseData()
{
}

SSlotInfoResponseData::SSlotInfoResponseData(const boost::property_tree::ptree& _pt)
{
    fromPT(_pt);
}

void SSlotInfoResponseData::_toPT(boost::property_tree::ptree& _pt) const
{
    _pt.put<uint32_t>("index", m_index);
    _pt.put<uint64_t>("agentID", m_agentID);
    _pt.put<uint64_t>("slotID", m_slotID);
    _pt.put<uint64_t>("taskID", m_taskID);
    _pt.put<uint32_t>("state", m_state);
    _pt.put<string>("host", m_host);
    _pt.put<string>("wrkDir", m_wrkDir);
}

void SSlotInfoResponseData::_fromPT(const boost::property_tree::ptree& _pt)
{
    m_index = _pt.get<uint32_t>("index", 0);
    m_agentID = _pt.get<uint64_t>("agentID", 0);
    m_slotID = _pt.get<uint64_t>("slotID", 0);
    m_taskID = _pt.get<uint64_t>("taskID", 0);
    m_state = _pt.get<uint32_t>("state", 0);
    m_host = _pt.get<string>("host", "");
    m_wrkDir = _pt.get<string>("wrkDir", "");
}

bool SSlotInfoResponseData::operator==(const SSlotInfoResponseData& _val) const
{
    return (SBaseData::operator==(_val) && m_index == _val.m_index && m_agentID == _val.m_agentID &&
            m_slotID == _val.m_slotID && m_taskID == _val.m_taskID && m_state == _val.m_state &&
            m_host == _val.m_host && m_wrkDir == _val.m_wrkDir);
}

// We need to put function implementation in the same "dds::tools_api" namespace as a friend function declaration.
// Such declaration "std::ostream& dds::tools_api::operator<<(std::ostream& _os, const ***& _data)" doesn't help.
// In order to silent GCC warning "*** has not been declared within 'dds::tools_api'"
namespace dds
{
    namespace tools_api
    {
        std::ostream& operator<<(std::ostream& _os, const SSlotInfoResponseData& _data)
        {
            return _os << _data.defaultToString() << "; index: " << _data.m_index << "; agentID: " << _data.m_agentID
                       << "; slotID: " << _data.m_slotID << "; taskID: " << _data.m_taskID
                       << "; state: " << _data.m_state << "; host: " << _data.m_host
                       << "; wrkDir: " << quoted(_data.m_wrkDir);
        }
    } // namespace tools_api
} // namespace dds

///////////////////////////////////
// SAgentCountResponseData
///////////////////////////////////

// this declaration is important to help older compilers to eat this static constexpr
constexpr const char* SAgentCountResponseData::_protocolTag;

SAgentCountResponseData::SAgentCountResponseData()
{
}

SAgentCountResponseData::SAgentCountResponseData(const boost::property_tree::ptree& _pt)
{
    fromPT(_pt);
}

void SAgentCountResponseData::_toPT(boost::property_tree::ptree& _pt) const
{
    _pt.put<uint32_t>("activeSlotsCount", m_activeSlotsCount);
    _pt.put<uint32_t>("idleSlotsCount", m_idleSlotsCount);
    _pt.put<uint32_t>("executingSlotsCount", m_executingSlotsCount);
}

void SAgentCountResponseData::_fromPT(const boost::property_tree::ptree& _pt)
{
    m_activeSlotsCount = _pt.get<uint32_t>("activeSlotsCount", 0);
    m_idleSlotsCount = _pt.get<uint32_t>("idleSlotsCount", 0);
    m_executingSlotsCount = _pt.get<uint32_t>("executingSlotsCount", 0);
}

bool SAgentCountResponseData::operator==(const SAgentCountResponseData& _val) const
{
    return (SBaseData::operator==(_val) && m_activeSlotsCount == _val.m_activeSlotsCount &&
            m_idleSlotsCount == _val.m_idleSlotsCount && m_executingSlotsCount == _val.m_executingSlotsCount);
}

// We need to put function implementation in the same "dds::tools_api" namespace as a friend function declaration.
// Such declaration "std::ostream& dds::tools_api::operator<<(std::ostream& _os, const ***& _data)" doesn't help.
// In order to silent GCC warning "*** has not been declared within 'dds::tools_api'"
namespace dds
{
    namespace tools_api
    {
        std::ostream& operator<<(std::ostream& _os, const SAgentCountResponseData& _data)
        {
            return _os << _data.defaultToString() << "; activeSlotsCount: " << _data.m_activeSlotsCount
                       << "; idleSlotsCount: " << _data.m_idleSlotsCount
                       << "; executingSlotsCount: " << _data.m_executingSlotsCount;
        }
    } // namespace tools_api
} // namespace dds

///////////////////////////////////
// SOnTaskDoneResponseData
///////////////////////////////////

// this declaration is important to help older compilers to eat this static constexpr
constexpr const char* SOnTaskDoneResponseData::_protocolTag;

SOnTaskDoneResponseData::SOnTaskDoneResponseData()
{
}

SOnTaskDoneResponseData::SOnTaskDoneResponseData(const boost::property_tree::ptree& _pt)
{
    fromPT(_pt);
}

void SOnTaskDoneResponseData::_toPT(boost::property_tree::ptree& _pt) const
{
    _pt.put<uint64_t>("taskID", m_taskID);
    _pt.put<uint32_t>("exitCode", m_exitCode);
    _pt.put<uint32_t>("signal", m_signal);
    _pt.put<string>("host", m_host);
    _pt.put<string>("wrkDir", m_wrkDir);
    _pt.put<string>("taskPath", m_taskPath);
}

void SOnTaskDoneResponseData::_fromPT(const boost::property_tree::ptree& _pt)
{
    m_taskID = _pt.get<uint64_t>("taskID", 0);
    m_exitCode = _pt.get<uint32_t>("exitCode", 0);
    m_signal = _pt.get<uint32_t>("signal", 0);
    m_host = _pt.get<string>("host", string());
    m_wrkDir = _pt.get<string>("wrkDir", string());
    m_taskPath = _pt.get<string>("taskPath", string());
}

bool SOnTaskDoneResponseData::operator==(const SOnTaskDoneResponseData& _val) const
{
    return (SBaseData::operator==(_val) && m_taskID == _val.m_taskID && m_exitCode == _val.m_exitCode &&
            m_signal == _val.m_signal && m_host == _val.m_host && m_wrkDir == _val.m_wrkDir &&
            m_taskPath == _val.m_taskPath);
}

// We need to put function implementation in the same "dds::tools_api" namespace as a friend function declaration.
// Such declaration "std::ostream& dds::tools_api::operator<<(std::ostream& _os, const ***& _data)" doesn't help.
// In order to silent GCC warning "*** has not been declared within 'dds::tools_api'"
namespace dds
{
    namespace tools_api
    {
        std::ostream& operator<<(std::ostream& _os, const SOnTaskDoneResponseData& _data)
        {
            return _os << _data.defaultToString() << "; taskID: " << _data.m_taskID
                       << "; exitCode: " << _data.m_exitCode << "; signal: " << _data.m_signal
                       << "; host: " << _data.m_host << "; wrkDir: " << quoted(_data.m_wrkDir)
                       << "; taskPath: " << _data.m_taskPath;
        }
    } // namespace tools_api
} // namespace dds

///////////////////////////////////
// SAgentCommandRequestData
///////////////////////////////////

// this declaration is important to help older compilers to eat this static constexpr
constexpr const char* SAgentCommandRequestData::_protocolTag;

SAgentCommandRequestData::SAgentCommandRequestData()
{
}

SAgentCommandRequestData::SAgentCommandRequestData(const boost::property_tree::ptree& _pt)
{
    fromPT(_pt);
}

void SAgentCommandRequestData::_toPT(boost::property_tree::ptree& _pt) const
{
    _pt.put<uint8_t>("commandType", static_cast<uint8_t>(m_commandType));
    _pt.put<uint64_t>("arg1", m_arg1);
    _pt.put<string>("arg2", m_arg2);
}

void SAgentCommandRequestData::_fromPT(const boost::property_tree::ptree& _pt)
{
    m_commandType = static_cast<EAgentCommandType>(_pt.get<uint8_t>("commandType", 0));
    m_arg1 = _pt.get<uint64_t>("arg1", 0);
    m_arg2 = _pt.get<string>("arg2", string());
}

bool SAgentCommandRequestData::operator==(const SAgentCommandRequestData& _val) const
{
    return (SBaseData::operator==(_val) && m_commandType == _val.m_commandType && m_arg1 == _val.m_arg1 &&
            m_arg2 == _val.m_arg2);
}

// We need to put function implementation in the same "dds::tools_api" namespace as a friend function declaration.
// Such declaration "std::ostream& dds::tools_api::operator<<(std::ostream& _os, const ***& _data)" doesn't help.
// In order to silent GCC warning "*** has not been declared within 'dds::tools_api'"
namespace dds
{
    namespace tools_api
    {
        std::ostream& operator<<(std::ostream& _os, const SAgentCommandRequestData& _data)
        {
            return _os << _data.defaultToString() << "; commandType: " << static_cast<uint8_t>(_data.m_commandType)
                       << "; arg1: " << _data.m_arg1 << "; arg2: " << _data.m_arg2;
        }
    } // namespace tools_api
} // namespace dds
