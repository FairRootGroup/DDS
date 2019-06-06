// Copyright 2019 GSI, Inc. All rights reserved.
//
//
//

#include "Tools.h"
// STD
#include <sstream>
// BOOST
#include <boost/process.hpp>
#include <boost/regex.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
// MiscCommon
#include "Process.h"
// DDS
#include "DDSIntercomGuard.h"
#include "UserDefaults.h"

using namespace std;
using namespace dds;
using namespace dds::tools_api;
using namespace dds::user_defaults_api;
using namespace MiscCommon;
using namespace dds::intercom_api;
using namespace boost::property_tree;
namespace fs = boost::filesystem;

const size_t g_WAIT_PROCESS_SEC = 30;

///////////////////////////////////
// CSession
///////////////////////////////////
CSession::CSession()
    : m_sid(boost::uuids::nil_uuid())
    , m_service()
    , m_customCmd(m_service)
{
    m_customCmd.subscribe([this](const string& _command, const string& _condition, uint64_t _senderId) {
        istringstream ss(_command);
        notify(ss);
    });

    m_service.start();
}

CSession::~CSession()
{
    if (m_sid.is_nil())
        return;

    unsubscribe();

    stop();
}

void CSession::stop()
{
    unsubscribe();

    // Stop intercome
    internal_api::CDDSIntercomGuard::instance().stopCondition();
}

boost::uuids::uuid CSession::create()
{
    if (!isDDSAvailable())
        throw runtime_error(
            "ToolsAPI: Missing DDS environment. Make sure to init DDS env using DDS_env.sh before using this API");

    // Call "dds-session start" to fireup a new session
    // Get new session ID
    string sOut;
    string sErr;
    int nExitCode(0);

    stringstream ssCmd;
    ssCmd << boost::process::search_path("dds-session").string() << " start";
    execute(ssCmd.str(), std::chrono::seconds(g_WAIT_PROCESS_SEC), &sOut, &sErr, &nExitCode);

    if (nExitCode != 0 || !sErr.empty())
    {
        stringstream ss;
        if (nExitCode != 0)
            ss << "Failed to start a new session. Error code " << nExitCode << "; ";
        if (!sErr.empty())
            ss << "error: " << sErr;

        throw runtime_error(ss.str());
    }

    // Parse the output to get the session ID
    const boost::regex expression(
        "DDS session ID: ([[:xdigit:]]{8}-[[:xdigit:]]{4}-[[:xdigit:]]{4}-[[:xdigit:]]{4}-[[:xdigit:]]{12}).*");
    boost::smatch results;
    if (boost::regex_match(sOut, results, expression))
    {
        if (results.size() >= 1)
        {
            m_sid = boost::uuids::string_generator()(results[1].str());
        }
    }

    // Reinit UserDefaults and Log with new session ID
    CUserDefaults::instance().reinit(boost::uuids::string_generator()(boost::uuids::to_string(m_sid)),
                                     CUserDefaults::instance().currentUDFile());
    // Logger::instance().reinit();

    return getSessionID();
}

void CSession::attach(const std::string& _sid)
{
    attach(boost::uuids::string_generator()(_sid));
}

void CSession::attach(const boost::uuids::uuid& _sid)
{
    if (!isDDSAvailable())
        throw runtime_error(
            "ToolsAPI: Missing DDS environment. Make sure to init DDS env using DDS_env.sh before using this API");

    m_sid = _sid;

    // Reinit UserDefaults and Log with new session ID
    CUserDefaults::instance().reinit(boost::uuids::string_generator()(boost::uuids::to_string(m_sid)),
                                     CUserDefaults::instance().currentUDFile());
}

void CSession::shutdown()
{
    // stop active session
    string sOut;
    string sErr;
    int nExitCode(0);
    stringstream ssCmd;
    ssCmd << boost::process::search_path("dds-session").string() << " stop " << boost::uuids::to_string(m_sid);
    execute(ssCmd.str(), std::chrono::seconds(g_WAIT_PROCESS_SEC), &sOut, &sErr, &nExitCode);

    if (nExitCode == 0)
        m_sid = boost::uuids::nil_uuid();
}

boost::uuids::uuid CSession::getSessionID() const
{
    return m_sid;
}

void CSession::blockCurrentThread()
{
    if (m_sid.is_nil())
        throw runtime_error("ToolsAPI: First create or attache to a DDS session.");

    size_t num_slots = m_signalMessage.num_slots();

    // We wait only if _block is true and we have subscribers
    if (num_slots > 0)
    {
        internal_api::CDDSIntercomGuard::instance().waitCondition();
    }
}

void CSession::unsubscribe()
{
    m_customCmd.unsubscribe();

    m_signalMessage.disconnect_all_slots();
    m_signalDone.disconnect_all_slots();
    m_signalProgress.disconnect_all_slots();
    m_signalSubmit.disconnect_all_slots();
    m_signalTopology.disconnect_all_slots();
    m_signalGetLog.disconnect_all_slots();
    m_signalCommanderInfo.disconnect_all_slots();
    m_signalAgentInfo.disconnect_all_slots();
}

bool CSession::isDDSAvailable() const
{
    char* pchDDSLocation;
    pchDDSLocation = getenv("DDS_LOCATION");
    if (pchDDSLocation == NULL || strlen(pchDDSLocation) == 0)
        return false;

    return (fs::is_directory(pchDDSLocation));
}

bool CSession::IsRunning() const
{
    return CUserDefaults::instance().IsSessionRunning();
}

void CSession::notify(std::istream& _stream)
{
    ptree pt;

    try
    {
        read_json(_stream, pt);

        const ptree& childPT = pt.get_child("dds.tools-api");

        for (const auto& child : childPT)
        {
            const string& tag = child.first;
            if (tag == "message")
            {
                SMessage message;
                message.fromPT(pt);
                m_signalMessage(message);
            }
            else if (tag == "done")
            {
                SDone done;
                done.fromPT(pt);
                m_signalDone(done);
            }
            else if (tag == "progress")
            {
                SProgress progress;
                progress.fromPT(pt);
                m_signalProgress(progress);
            }
            else if (tag == "submit")
            {
                SSubmit submit;
                submit.fromPT(pt);
                m_signalSubmit(submit);
            }
            else if (tag == "topology")
            {
                STopology topo;
                topo.fromPT(pt);
                m_signalTopology(topo);
            }
            else if (tag == "getlog")
            {
                SGetLog getlog;
                getlog.fromPT(pt);
                m_signalGetLog(getlog);
            }
            else if (tag == "commanderInfo")
            {
                SCommanderInfo commanderInfo;
                commanderInfo.fromPT(pt);
                m_signalCommanderInfo(commanderInfo);
            }
            else if (tag == "agentInfo")
            {
                SAgentInfo agentInfo;
                agentInfo.fromPT(pt);
                m_signalAgentInfo(agentInfo);
            }
        }
    }
    catch (exception& error)
    {
        SMessage msg;
        msg.m_msg = "ToolsAPI: Can't parse input message: ";
        msg.m_msg += error.what();
        msg.m_severity = EMsgSeverity::error;
        sendRequest(msg);
    }
}
