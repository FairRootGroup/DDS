// Copyright 2014 GSI, Inc. All rights reserved.
//
//
// DDS
#include "dds_intercom.h"
// BOOST
#include <boost/property_tree/json_parser.hpp>
// STD
#include <iostream>
#include <sstream>

using namespace dds;
using namespace std;
using namespace boost::property_tree;

const std::string g_sRmsAgentSign = "rms_agent_sign";

std::string MsgSeverityToTag(EMsgSeverity _severity)
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

EMsgSeverity TagToMsgSeverity(const std::string& _tag)
{
    if (_tag == "info")
        return EMsgSeverity::info;
    else if (_tag == "error")
        return EMsgSeverity::error;
    else
        throw runtime_error("Message severity for tag " + _tag + " does not exist.");
}

CRMSPluginProtocol::CRMSPluginProtocol()
{
    m_customCmd.subscribe([this](const string& _command, const string& _condition, uint64_t _senderId) {
        istringstream ss(_command);
        parse(ss);
    });
}

CRMSPluginProtocol::~CRMSPluginProtocol()
{
    unsubscribe();
}

void CRMSPluginProtocol::subscribeSubmit(signalSubmit_t::slot_function_type _subscriber)
{
    m_signalSubmit.connect(_subscriber);
}

void CRMSPluginProtocol::subscribeMessage(signalMessage_t::slot_function_type _subscriber)
{
    m_signalMessage.connect(_subscriber);
}

void CRMSPluginProtocol::subscribeRequirement(signalRequirement_t::slot_function_type _subscriber)
{
    m_signalRequirement.connect(_subscriber);
}

void CRMSPluginProtocol::unsubscribe()
{
    m_signalSubmit.disconnect_all_slots();
    m_signalMessage.disconnect_all_slots();
    m_signalRequirement.disconnect_all_slots();
}

void CRMSPluginProtocol::send(EMsgSeverity _severity, const std::string& _msg)
{
    // Form JSON message
    stringstream json;
    json << "{" << endl
         << "\"message\":" << endl
         << "{" << endl
         << "\"message\": \"" << _msg << "\"," << endl
         << "\"severity\": \"" << MsgSeverityToTag(_severity) << "\"" << endl
         << "}" << endl
         << "}" << endl;
    m_customCmd.send(_msg, g_sRmsAgentSign);
}

void CRMSPluginProtocol::parse(std::istream& _stream)
{
    ptree pt;

    try
    {
        read_json(_stream, pt);

        for (const auto& child : pt)
        {
            const string& tag = child.first;
            const ptree& childpt = child.second;
            if (tag == "submit")
            {
                SSubmit submit;
                submit.m_nInstances = childpt.get<int>("nInstances", 0);
                submit.m_cfgFilePath = childpt.get<string>("cfgFilePath", "");
                m_signalSubmit(submit);
            }
            else if (tag == "message")
            {
                SMessage message;
                message.m_msgSeverity = TagToMsgSeverity(childpt.get<string>("severity", "info"));
                message.m_msg = childpt.get<string>("message", "");
                m_signalMessage(message);
            }
            else if (tag == "requirement")
            {
                SRequirement requirement;
                requirement.m_hostName = childpt.get<string>("hostName", "");
                m_signalRequirement(requirement);
            }
        }
    }
    catch (exception& error)
    {
        string msg("Can't parse input message: ");
        msg += error.what();
        send(EMsgSeverity::error, msg);
    }
}
