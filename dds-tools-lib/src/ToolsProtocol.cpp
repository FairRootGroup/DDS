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

std::ostream& dds::tools_api::operator<<(std::ostream& _os, const SProgressResponseData& _data)
{
    return _os << _data.defaultToString() << "; completed: " << _data.m_completed << "; total: " << _data.m_total
               << "; errors: " << _data.m_errors << "; time: " << _data.m_time
               << "; srcCommand: " << _data.m_srcCommand;
}

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

std::ostream& dds::tools_api::operator<<(std::ostream& _os, const SMessageResponseData& _data)
{
    return _os << _data.defaultToString() << "; severity: " << _data.m_severity << "; msg: " << _data.m_msg;
}

///////////////////////////////////
// SSubmitRequestData
///////////////////////////////////

// this declaration is important to help older compilers to eat this static constexpr
constexpr const char* SSubmitRequestData::_protocolTag;

void SSubmitRequestData::_toPT(boost::property_tree::ptree& _pt) const
{
    _pt.put<int>("instances", m_instances);
    _pt.put<int>("slots", m_slots);
    _pt.put<string>("config", m_config);
    _pt.put<string>("rms", m_rms);
    _pt.put<string>("pluginPath", m_pluginPath);
}

void SSubmitRequestData::_fromPT(const boost::property_tree::ptree& _pt)
{
    m_instances = _pt.get<int>("instances", 0);
    m_slots = _pt.get<int>("slots", 0);
    m_config = _pt.get<string>("config", "");
    m_rms = _pt.get<string>("rms", "");
    m_pluginPath = _pt.get<string>("pluginPath", "");
}

bool SSubmitRequestData::operator==(const SSubmitRequestData& _val) const
{
    return (SBaseData::operator==(_val) && m_rms == _val.m_rms && m_instances == _val.m_instances &&
            m_slots == _val.m_slots && m_config == _val.m_config && m_pluginPath == _val.m_pluginPath);
}

std::ostream& dds::tools_api::operator<<(std::ostream& _os, const SSubmitRequestData& _data)
{
    return _os << _data.defaultToString() << "; instances: " << _data.m_instances << "; slots: " << _data.m_slots
               << "; config: " << _data.m_config << "; rms: " << _data.m_rms << "; pluginPath: " << _data.m_pluginPath;
}

///////////////////////////////////
// STopologyRequestData
///////////////////////////////////

// this declaration is important to help older compilers to eat this static constexpr
constexpr const char* STopologyRequestData::_protocolTag;

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

std::ostream& dds::tools_api::operator<<(std::ostream& _os, const STopologyRequestData& _data)
{
    return _os << _data.defaultToString() << "; updateType: " << static_cast<uint8_t>(_data.m_updateType)
               << "; topologyFile: " << _data.m_topologyFile << "; disableValidation: " << _data.m_disableValidation;
}

///////////////////////////////////
// SCommanderInfoResponseData
///////////////////////////////////

// this declaration is important to help older compilers to eat this static constexpr
constexpr const char* SCommanderInfoResponseData::_protocolTag;

void SCommanderInfoResponseData::_toPT(boost::property_tree::ptree& _pt) const
{
    _pt.put<pid_t>("pid", m_pid);
    _pt.put<std::string>("activeTopologyName", m_activeTopologyName);
}

void SCommanderInfoResponseData::_fromPT(const boost::property_tree::ptree& _pt)
{
    m_pid = _pt.get<pid_t>("pid", 0);
    m_activeTopologyName = _pt.get<std::string>("activeTopologyName", std::string());
}

bool SCommanderInfoResponseData::operator==(const SCommanderInfoResponseData& _val) const
{
    return (SBaseData::operator==(_val) && m_pid == _val.m_pid && m_activeTopologyName == _val.m_activeTopologyName);
}

std::ostream& dds::tools_api::operator<<(std::ostream& _os, const SCommanderInfoResponseData& _data)
{
    return _os << _data.defaultToString() << "; pid: " << _data.m_pid
               << "; activeTopologyName: " << _data.m_activeTopologyName;
}

///////////////////////////////////
// SAgentInfoResponseData
///////////////////////////////////

// this declaration is important to help older compilers to eat this static constexpr
constexpr const char* SAgentInfoResponseData::_protocolTag;

void SAgentInfoResponseData::_toPT(boost::property_tree::ptree& _pt) const
{
    _pt.put<uint32_t>("index", m_index);
    _pt.put<uint64_t>("agentID", m_agentID);
    _pt.put<size_t>("startUpTime", m_startUpTime.count());
    _pt.put<string>("username", m_username);
    _pt.put<string>("host", m_host);
    _pt.put<string>("DDSPath", m_DDSPath);
    _pt.put<uint32_t>("agentPid", m_agentPid);
    _pt.put<uint32_t>("nSlots", m_nSlots);
}

void SAgentInfoResponseData::_fromPT(const boost::property_tree::ptree& _pt)
{
    m_index = _pt.get<uint32_t>("index", 0);
    m_agentID = _pt.get<uint64_t>("agentID", 0);
    m_startUpTime = std::chrono::milliseconds(_pt.get<size_t>("startUpTime", 0));
    m_username = _pt.get<string>("username", "");
    m_host = _pt.get<string>("host", "");
    m_DDSPath = _pt.get<string>("DDSPath", "");
    m_agentPid = _pt.get<uint32_t>("agentPid", 0);
    m_nSlots = _pt.get<uint32_t>("nSlots", 0);
}

bool SAgentInfoResponseData::operator==(const SAgentInfoResponseData& _val) const
{
    return (SBaseData::operator==(_val) && m_index == _val.m_index && m_agentID == _val.m_agentID &&
            m_startUpTime == _val.m_startUpTime && m_username == _val.m_username && m_host == _val.m_host &&
            m_DDSPath == _val.m_DDSPath && m_agentPid == _val.m_agentPid && m_nSlots == _val.m_nSlots);
}

std::ostream& dds::tools_api::operator<<(std::ostream& _os, const SAgentInfoResponseData& _data)
{
    return _os << _data.defaultToString() << "; index: " << _data.m_index << "; agentID: " << _data.m_agentID
               << "; startUpTime: " << _data.m_startUpTime.count() << "; username: " << _data.m_username
               << "; host: " << _data.m_host << "; DDSPath: " << _data.m_DDSPath << "; agentPid: " << _data.m_agentPid
               << "; nSlots: " << _data.m_nSlots;
}

///////////////////////////////////
// SAgentCountResponseData
///////////////////////////////////

// this declaration is important to help older compilers to eat this static constexpr
constexpr const char* SAgentCountResponseData::_protocolTag;

void SAgentCountResponseData::_toPT(boost::property_tree::ptree& _pt) const
{
    _pt.put<uint32_t>("activeAgentsCount", m_activeAgentsCount);
    _pt.put<uint32_t>("idleAgentsCount", m_idleAgentsCount);
    _pt.put<uint32_t>("executingAgentsCount", m_executingAgentsCount);
}

void SAgentCountResponseData::_fromPT(const boost::property_tree::ptree& _pt)
{
    m_activeAgentsCount = _pt.get<uint32_t>("activeAgentsCount", 0);
    m_idleAgentsCount = _pt.get<uint32_t>("idleAgentsCount", 0);
    m_executingAgentsCount = _pt.get<uint32_t>("executingAgentsCount", 0);
}

bool SAgentCountResponseData::operator==(const SAgentCountResponseData& _val) const
{
    return (SBaseData::operator==(_val) && m_activeAgentsCount == _val.m_activeAgentsCount &&
            m_idleAgentsCount == _val.m_idleAgentsCount && m_executingAgentsCount == _val.m_executingAgentsCount);
}

std::ostream& dds::tools_api::operator<<(std::ostream& _os, const SAgentCountResponseData& _data)
{
    return _os << _data.defaultToString() << "; activeAgentsCount: " << _data.m_activeAgentsCount
               << "; idleAgentsCount: " << _data.m_idleAgentsCount
               << "; executingAgentsCount: " << _data.m_executingAgentsCount;
}
