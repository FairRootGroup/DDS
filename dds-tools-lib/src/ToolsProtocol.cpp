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

///////////////////////////////////
// SDone
///////////////////////////////////
void dds::tools_api::SDoneResponseData::_toPT(boost::property_tree::ptree& _pt) const
{
}

void dds::tools_api::SDoneResponseData::_fromPT(const boost::property_tree::ptree& _pt)
{
}

///////////////////////////////////
// SProgress
///////////////////////////////////
void dds::tools_api::SProgressResponseData::_toPT(boost::property_tree::ptree& _pt) const
{
    _pt.put<uint32_t>("completed", m_completed);
    _pt.put<uint32_t>("total", m_total);
    _pt.put<uint32_t>("errors", m_errors);
    _pt.put<uint32_t>("time", m_time);
    _pt.put<uint32_t>("srcCommand", m_srcCommand);
}

void dds::tools_api::SProgressResponseData::_fromPT(const boost::property_tree::ptree& _pt)
{
    m_completed = _pt.get<uint32_t>("completed", 0);
    m_total = _pt.get<uint32_t>("total", 0);
    m_errors = _pt.get<uint32_t>("errors", 0);
    m_time = _pt.get<uint32_t>("time", 0);
    m_srcCommand = _pt.get<uint32_t>("srcCommand", 0);
}

///////////////////////////////////
// SMessage
///////////////////////////////////

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

void dds::tools_api::SMessageResponseData::_toPT(boost::property_tree::ptree& _pt) const
{
    _pt.put<string>("msg", m_msg);
    _pt.put<string>("severity", SeverityToTag(m_severity));
}

void dds::tools_api::SMessageResponseData::_fromPT(const boost::property_tree::ptree& _pt)
{
    m_severity = TagToSeverity(_pt.get<string>("severity", "info"));
    m_msg = _pt.get<string>("msg", "");
}

///////////////////////////////////
// SGetLog
///////////////////////////////////
void dds::tools_api::SGetLogRequestData::_fromPT(const boost::property_tree::ptree& _pt)
{
}
void dds::tools_api::SGetLogRequestData::_toPT(boost::property_tree::ptree& _pt) const
{
}

///////////////////////////////////
// SSubmit
///////////////////////////////////
void dds::tools_api::SSubmitRequestData::_toPT(boost::property_tree::ptree& _pt) const
{
    _pt.put<int>("instances", m_instances);
    _pt.put<string>("config", m_config);
    _pt.put<string>("rms", m_rms);
    _pt.put<string>("pluginPath", m_pluginPath);
}

void dds::tools_api::SSubmitRequestData::_fromPT(const boost::property_tree::ptree& _pt)
{
    m_instances = _pt.get<int>("instances", 0);
    m_config = _pt.get<string>("config", "");
    m_rms = _pt.get<string>("rms", "");
    m_pluginPath = _pt.get<string>("pluginPath", "");
}

///////////////////////////////////
// STopology
///////////////////////////////////
void dds::tools_api::STopologyRequestData::_toPT(boost::property_tree::ptree& _pt) const
{
    _pt.put<uint8_t>("updateType", static_cast<uint8_t>(m_updateType));
    _pt.put<string>("topologyFile", m_topologyFile);
    _pt.put<bool>("disableValidation", m_disableValidation);
}

void dds::tools_api::STopologyRequestData::_fromPT(const boost::property_tree::ptree& _pt)
{
    m_updateType = static_cast<EUpdateType>(_pt.get<uint8_t>("updateType", 0));
    m_topologyFile = _pt.get<string>("topologyFile", "");
    m_disableValidation = _pt.get<bool>("disableValidation", false);
}

///////////////////////////////////
// SCommanderInfo
///////////////////////////////////
void dds::tools_api::SCommanderInfoResponseData::_toPT(boost::property_tree::ptree& _pt) const
{
    _pt.put<pid_t>("pid", m_pid);
    _pt.put<uint32_t>("idleAgentsCount", m_idleAgentsCount);
    _pt.put<std::string>("activeTopologyName", m_activeTopologyName);
}

void dds::tools_api::SCommanderInfoResponseData::_fromPT(const boost::property_tree::ptree& _pt)
{
    m_pid = _pt.get<pid_t>("pid", 0);
    m_idleAgentsCount = _pt.get<uint32_t>("idleAgentsCount", 0);
    m_activeTopologyName = _pt.get<std::string>("activeTopologyName", std::string());
}

void dds::tools_api::SCommanderInfoRequestData::_fromPT(const boost::property_tree::ptree& _pt)
{
}
void dds::tools_api::SCommanderInfoRequestData::_toPT(boost::property_tree::ptree& _pt) const
{
}

///////////////////////////////////
// SAgentInfo
///////////////////////////////////
void dds::tools_api::SAgentInfoResponseData::_toPT(boost::property_tree::ptree& _pt) const
{
    _pt.put<uint32_t>("activeAgentsCount", m_activeAgentsCount);
    _pt.put<uint32_t>("index", m_index);
    _pt.put<string>("agentInfo", m_agentInfo);
}

void dds::tools_api::SAgentInfoResponseData::_fromPT(const boost::property_tree::ptree& _pt)
{
    m_activeAgentsCount = _pt.get<uint32_t>("activeAgentsCount", 0);
    m_index = _pt.get<uint32_t>("index", 0);
    m_agentInfo = _pt.get<string>("agentInfo", "");
}

void dds::tools_api::SAgentInfoRequestData::_fromPT(const boost::property_tree::ptree& _pt)
{
}
void dds::tools_api::SAgentInfoRequestData::_toPT(boost::property_tree::ptree& _pt) const
{
}
