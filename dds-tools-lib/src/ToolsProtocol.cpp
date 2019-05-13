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
std::string dds::tools_api::SDone::_toJSON() const
{
    ptree pt;

    pt.put<uint64_t>("dds.tools-api.done.requestID", m_requestID);

    stringstream json;
    write_json(json, pt);

    return json.str();
}

void dds::tools_api::SDone::_fromJSON(const std::string& _json)
{
    ptree pt;
    istringstream stream(_json);
    read_json(stream, pt);
    _fromPT(pt);
}

void dds::tools_api::SDone::_fromPT(const boost::property_tree::ptree& _pt)
{
    const ptree& pt = _pt.get_child("dds.tools-api");
    m_requestID = pt.get<uint64_t>("done.requestID", 0);
}

bool dds::tools_api::SDone::operator==(const SDone& val) const
{
    return (m_requestID == val.m_requestID);
}

///////////////////////////////////
// SProgress
///////////////////////////////////
std::string dds::tools_api::SProgress::_toJSON() const
{
    ptree pt;

    pt.put<uint32_t>("dds.tools-api.progress.completed", m_completed);
    pt.put<uint32_t>("dds.tools-api.progress.total", m_total);
    pt.put<uint32_t>("dds.tools-api.progress.errors", m_errors);
    pt.put<uint32_t>("dds.tools-api.progress.time", m_time);
    pt.put<uint32_t>("dds.tools-api.progress.srcCommand", m_srcCommand);
    pt.put<uint32_t>("dds.tools-api.progress.requestID", m_requestID);

    stringstream json;
    write_json(json, pt);

    return json.str();
}

void dds::tools_api::SProgress::_fromJSON(const std::string& _json)
{
    ptree pt;
    istringstream stream(_json);
    read_json(stream, pt);
    _fromPT(pt);
}

void dds::tools_api::SProgress::_fromPT(const boost::property_tree::ptree& _pt)
{
    const ptree& pt = _pt.get_child("dds.tools-api");
    m_completed = pt.get<uint32_t>("progress.completed", 0);
    m_total = pt.get<uint32_t>("progress.total", 0);
    m_errors = pt.get<uint32_t>("progress.errors", 0);
    m_time = pt.get<uint32_t>("progress.time", 0);
    m_srcCommand = pt.get<uint32_t>("progress.srcCommand", 0);
    m_requestID = pt.get<uint32_t>("progress.requestID", 0);
}

bool dds::tools_api::SProgress::operator==(const SProgress& val) const
{
    return (m_requestID == val.m_requestID);
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

string dds::tools_api::SMessage::_toJSON() const
{
    ptree pt;

    pt.put<string>("dds.tools-api.message.msg", m_msg);
    pt.put<string>("dds.tools-api.message.severity", SeverityToTag(m_severity));
    pt.put<uint64_t>("dds.tools-api.message.requestID", m_requestID);

    stringstream json;
    write_json(json, pt);

    return json.str();
}

void dds::tools_api::SMessage::_fromJSON(const std::string& _json)
{
    ptree pt;
    istringstream stream(_json);
    read_json(stream, pt);
    _fromPT(pt);
}

void dds::tools_api::SMessage::_fromPT(const boost::property_tree::ptree& _pt)
{
    const ptree& pt = _pt.get_child("dds.tools-api");
    m_severity = TagToSeverity(pt.get<string>("message.severity", "info"));
    m_msg = pt.get<string>("message.msg", "");
    m_requestID = pt.get<uint64_t>("message.requestID", 0);
}

bool dds::tools_api::SMessage::operator==(const SMessage& val) const
{
    return (m_severity == val.m_severity) && (m_msg == val.m_msg);
}

///////////////////////////////////
// SSubmit
///////////////////////////////////
std::string dds::tools_api::SSubmit::_toJSON() const
{
    ptree pt;

    pt.put<int>("dds.tools-api.submit.instances", m_instances);
    pt.put<string>("dds.tools-api.submit.config", m_config);
    pt.put<string>("dds.tools-api.submit.rms", m_rms);
    pt.put<string>("dds.tools-api.submit.pluginPath", m_pluginPath);
    pt.put<uint64_t>("dds.tools-api.submit.requestID", m_requestID);

    stringstream json;
    write_json(json, pt);

    return json.str();
}

void dds::tools_api::SSubmit::_fromJSON(const std::string& _json)
{
    ptree pt;
    istringstream stream(_json);
    read_json(stream, pt);
    _fromPT(pt);
}

void dds::tools_api::SSubmit::_fromPT(const boost::property_tree::ptree& _pt)
{
    const ptree& pt = _pt.get_child("dds.tools-api");
    m_instances = pt.get<int>("submit.instances", 0);
    m_config = pt.get<string>("submit.config", "");
    m_rms = pt.get<string>("submit.rms", "");
    m_pluginPath = pt.get<string>("submit.pluginPath", "");
    m_requestID = pt.get<uint64_t>("submit.requestID", 0);
}

bool dds::tools_api::SSubmit::operator==(const SSubmit& val) const
{
    return (m_instances == val.m_instances) && (m_config == val.m_config) && (m_rms == val.m_rms);
}

///////////////////////////////////
// STopology
///////////////////////////////////
std::string dds::tools_api::STopology::_toJSON() const
{
    ptree pt;

    pt.put<uint8_t>("dds.tools-api.topology.updateType", static_cast<uint8_t>(m_updateType));
    pt.put<string>("dds.tools-api.topology.topologyFile", m_topologyFile);
    pt.put<bool>("dds.tools-api.topology.disableValidation", m_disableValidation);
    pt.put<uint64_t>("dds.tools-api.topology.requestID", m_requestID);
    stringstream json;
    write_json(json, pt);

    return json.str();
}

void dds::tools_api::STopology::_fromJSON(const std::string& _json)
{
    ptree pt;
    istringstream stream(_json);
    read_json(stream, pt);
    _fromPT(pt);
}

void dds::tools_api::STopology::_fromPT(const boost::property_tree::ptree& _pt)
{
    const ptree& pt = _pt.get_child("dds.tools-api");
    m_updateType = static_cast<EUpdateType>(pt.get<uint8_t>("topology.updateType", 0));
    m_topologyFile = pt.get<string>("topology.topologyFile", "");
    m_disableValidation = pt.get<bool>("topology.disableValidation", false);
    m_requestID = pt.get<uint64_t>("topology.requestID", 0);
}

bool dds::tools_api::STopology::operator==(const STopology& val) const
{
    return (m_updateType == val.m_updateType) && (m_topologyFile == val.m_topologyFile) &&
           (m_disableValidation == val.m_disableValidation);
}

///////////////////////////////////
// SGetLog
///////////////////////////////////
std::string dds::tools_api::SGetLog::_toJSON() const
{
    ptree pt;

    pt.put<uint64_t>("dds.tools-api.getlog.requestID", m_requestID);

    stringstream json;
    write_json(json, pt);

    return json.str();
}

void dds::tools_api::SGetLog::_fromJSON(const std::string& _json)
{
    ptree pt;
    istringstream stream(_json);
    read_json(stream, pt);
    _fromPT(pt);
}

void dds::tools_api::SGetLog::_fromPT(const boost::property_tree::ptree& _pt)
{
    const ptree& pt = _pt.get_child("dds.tools-api");
    m_requestID = pt.get<uint64_t>("getlog.requestID", 0);
}

bool dds::tools_api::SGetLog::operator==(const SGetLog& val) const
{
    return (m_requestID == val.m_requestID);
}

///////////////////////////////////
// SCommanderInfo
///////////////////////////////////
std::string dds::tools_api::SCommanderInfo::_toJSON() const
{
    ptree pt;

    pt.put<pid_t>("dds.tools-api.commanderInfo.pid", m_pid);
    pt.put<uint32_t>("dds.tools-api.commanderInfo.idleAgentsCount", m_idleAgentsCount);
    pt.put<uint64_t>("dds.tools-api.commanderInfo.requestID", m_requestID);

    stringstream json;
    write_json(json, pt);

    return json.str();
}

void dds::tools_api::SCommanderInfo::_fromJSON(const std::string& _json)
{
    ptree pt;
    istringstream stream(_json);
    read_json(stream, pt);
    _fromPT(pt);
}

void dds::tools_api::SCommanderInfo::_fromPT(const boost::property_tree::ptree& _pt)
{
    const ptree& pt = _pt.get_child("dds.tools-api");
    m_pid = pt.get<pid_t>("commanderInfo.pid", 0);
    m_idleAgentsCount = pt.get<uint32_t>("commanderInfo.config", 0);
    m_requestID = pt.get<uint64_t>("commanderInfo.requestID", 0);
}

bool dds::tools_api::SCommanderInfo::operator==(const SCommanderInfo& val) const
{
    return (m_pid == val.m_pid) && (m_idleAgentsCount == val.m_idleAgentsCount) && (m_requestID == val.m_requestID);
}

///////////////////////////////////
// SAgentInfo
///////////////////////////////////
std::string dds::tools_api::SAgentInfo::_toJSON() const
{
    ptree pt;

    pt.put<uint32_t>("dds.tools-api.agentInfo.activeAgentsCount", m_activeAgentsCount);
    pt.put<uint32_t>("dds.tools-api.agentInfo.index", m_index);
    pt.put<string>("dds.tools-api.agentInfo.agentInfo", m_agentInfo);
    pt.put<uint64_t>("dds.tools-api.agentInfo.requestID", m_requestID);

    stringstream json;
    write_json(json, pt);

    return json.str();
}

void dds::tools_api::SAgentInfo::_fromJSON(const std::string& _json)
{
    ptree pt;
    istringstream stream(_json);
    read_json(stream, pt);
    _fromPT(pt);
}

void dds::tools_api::SAgentInfo::_fromPT(const boost::property_tree::ptree& _pt)
{
    const ptree& pt = _pt.get_child("dds.tools-api");
    m_activeAgentsCount = pt.get<uint32_t>("agentInfo.activeAgentsCount", 0);
    m_index = pt.get<uint32_t>("agentInfo.index", 0);
    m_agentInfo = pt.get<string>("agentInfo.agentInfo", "");
    m_requestID = pt.get<uint64_t>("agentInfo.requestID", 0);
}

bool dds::tools_api::SAgentInfo::operator==(const SAgentInfo& val) const
{
    return (m_activeAgentsCount == val.m_activeAgentsCount) && (m_index == val.m_index) &&
           (m_agentInfo == val.m_agentInfo) && (m_requestID == val.m_requestID);
}
