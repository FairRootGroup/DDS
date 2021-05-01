// Copyright 2014 GSI, Inc. All rights reserved.
//
//
// DDS
#include "Intercom.h"
#include "IntercomServiceCore.h"
// BOOST
#include <boost/property_tree/json_parser.hpp>
// STD
#include <iostream>
#include <sstream>

using namespace dds;
using namespace dds::intercom_api;
using namespace std;
using namespace boost::property_tree;

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

SSubmit::SSubmit()
    : m_nInstances(0)
    , m_cfgFilePath()
    , m_id()
{
}

///////////////////////////////////
// SSubmit
///////////////////////////////////

std::string SSubmit::toJSON()
{
    ptree pt;

    pt.put<string>("dds.plug-in.id", m_id);
    pt.put<int>("dds.plug-in.submit.nInstances", m_nInstances);
    pt.put<int>("dds.plug-in.submit.slots", m_slots);
    pt.put<string>("dds.plug-in.submit.cfgFilePath", m_cfgFilePath);
    pt.put<string>("dds.plug-in.submit.wrkPackagePath", m_wrkPackagePath);

    stringstream json;
    write_json(json, pt);

    return json.str();
}

void SSubmit::fromJSON(const std::string& _json)
{
    ptree pt;
    istringstream stream(_json);
    read_json(stream, pt);
    fromPT(pt);
}

void SSubmit::fromPT(const boost::property_tree::ptree& _pt)
{
    const ptree& pt = _pt.get_child("dds.plug-in");
    m_nInstances = pt.get<int>("submit.nInstances", 0);
    m_slots = pt.get<int>("submit.slots", 0);
    m_cfgFilePath = pt.get<string>("submit.cfgFilePath", "");
    m_wrkPackagePath = pt.get<string>("submit.wrkPackagePath", "");
    m_id = pt.get<string>("id");
}

bool SSubmit::operator==(const SSubmit& val) const
{
    return (m_id == val.m_id) && (m_nInstances == val.m_nInstances) && (m_slots == val.m_slots) &&
           (m_cfgFilePath == val.m_cfgFilePath) && (m_wrkPackagePath == val.m_wrkPackagePath);
}

///////////////////////////////////
// SMessage
///////////////////////////////////

SMessage::SMessage()
    : m_msgSeverity()
    , m_msg()
    , m_id()
{
}

std::string SMessage::toJSON()
{
    ptree pt;

    pt.put<string>("dds.plug-in.id", m_id);
    pt.put<string>("dds.plug-in.message.msg", m_msg);
    pt.put<string>("dds.plug-in.message.msgSeverity", MsgSeverityToTag(m_msgSeverity));

    stringstream json;
    write_json(json, pt);

    return json.str();
}

void SMessage::fromJSON(const std::string& _json)
{
    ptree pt;
    istringstream stream(_json);
    read_json(stream, pt);
    fromPT(pt);
}

void SMessage::fromPT(const boost::property_tree::ptree& _pt)
{
    const ptree& pt = _pt.get_child("dds.plug-in");
    m_msgSeverity = TagToMsgSeverity(pt.get<string>("message.msgSeverity", "info"));
    m_msg = pt.get<string>("message.msg", "");
    m_id = pt.get<string>("id");
}

bool SMessage::operator==(const SMessage& val) const
{
    return (m_id == val.m_id) && (m_msgSeverity == val.m_msgSeverity) && (m_msg == val.m_msg);
}

///////////////////////////////////
// SInit
///////////////////////////////////

SInit::SInit()
{
}

std::string SInit::toJSON()
{
    ptree pt;

    pt.put<string>("dds.plug-in.id", m_id);
    pt.put<string>("dds.plug-in.init", "");

    stringstream json;
    write_json(json, pt);

    return json.str();
}

void SInit::fromJSON(const std::string& _json)
{
    ptree pt;
    istringstream stream(_json);
    read_json(stream, pt);
    fromPT(pt);
}

void SInit::fromPT(const boost::property_tree::ptree& _pt)
{
    const ptree& pt = _pt.get_child("dds.plug-in");
    m_id = pt.get<string>("id");
}

bool SInit::operator==(const SInit& val) const
{
    return (m_id == val.m_id);
}

///////////////////////////////////
// CRMSPluginProtocol
///////////////////////////////////

CRMSPluginProtocol::CRMSPluginProtocol(const std::string& _id)
    : m_signalSubmit()
    , m_signalMessage()
    , m_id(_id)
    , m_service()
    , m_customCmd(m_service)
{
    m_customCmd.subscribe([this](const string& _command, const string& /*_condition*/, uint64_t /*_senderId*/) {
        istringstream ss(_command);
        notify(ss);
    });

    m_service.start();
}

CRMSPluginProtocol::~CRMSPluginProtocol()
{
    unsubscribe();
}

void CRMSPluginProtocol::onSubmit(signalSubmit_t::slot_function_type _subscriber)
{
    m_signalSubmit.connect(_subscriber);
}

void CRMSPluginProtocol::onMessage(signalMessage_t::slot_function_type _subscriber)
{
    m_signalMessage.connect(_subscriber);
}

void CRMSPluginProtocol::unsubscribe()
{
    m_signalSubmit.disconnect_all_slots();
    m_signalMessage.disconnect_all_slots();
}

void CRMSPluginProtocol::start(bool _block)
{
    SInit init;
    init.m_id = m_id;
    m_customCmd.send(init.toJSON(), g_sRmsAgentSign);

    size_t num_slots = m_signalSubmit.num_slots() + m_signalMessage.num_slots();

    // We wait only if _block is true and we have subscribers
    if (_block && num_slots > 0)
    {
        m_service.waitCondition();
    }
}

void CRMSPluginProtocol::stop()
{
    m_service.stopCondition();
}

void CRMSPluginProtocol::sendMessage(EMsgSeverity _severity, const std::string& _msg)
{
    SMessage message;
    message.m_id = m_id;
    message.m_msg = _msg;
    message.m_msgSeverity = _severity;
    m_customCmd.send(message.toJSON(), g_sRmsAgentSign);
}

void CRMSPluginProtocol::notify(std::istream& _stream)
{
    ptree pt;

    try
    {
        read_json(_stream, pt);

        const ptree& childPT = pt.get_child("dds.plug-in");

        for (const auto& child : childPT)
        {
            const string& tag = child.first;
            if (tag == "submit")
            {
                SSubmit submit;
                submit.fromPT(pt);
                m_signalSubmit(submit);
            }
            else if (tag == "message")
            {
                SMessage message;
                message.fromPT(pt);
                m_signalMessage(message);
            }
        }
    }
    catch (exception& error)
    {
        string msg("Can't parse input message: ");
        msg += error.what();
        sendMessage(EMsgSeverity::error, msg);
    }
}
