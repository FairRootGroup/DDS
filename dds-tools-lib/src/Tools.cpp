// Copyright 2019 GSI, Inc. All rights reserved.
//
//
//

#include "Tools.h"
// STD
#include <sstream>
// BOOST
#include <boost/any.hpp>
#include <boost/filesystem.hpp>
#include <boost/process.hpp>
#include <boost/regex.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
// MiscCommon
#include "Process.h"
// DDS
#include "DDSIntercomGuard.h"
#include "ToolsProtocol.h"
#include "UserDefaults.h"
#include "dds_intercom.h"

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
// CSession::SImpl
///////////////////////////////////
struct CSession::SImpl
{
    SImpl()
        : m_sid(boost::uuids::nil_uuid())
        , m_service()
        , m_customCmd(m_service)
    {
    }

    ~SImpl()
    {
    }

    // Disable copy constructors and assignment operators
    SImpl(const SImpl&) = delete;
    SImpl(SImpl&&) = delete;
    SImpl& operator=(const SImpl&) = delete;
    SImpl& operator=(SImpl&&) = delete;

    boost::uuids::uuid m_sid;                      ///< Session ID.
    dds::intercom_api::CIntercomService m_service; ///< Intercom service.
    dds::intercom_api::CCustomCmd m_customCmd;     ///< Custom commands API. Used for communication with commander.
    requests_t m_requests;                         ///< Array of requests.
};

///////////////////////////////////
// CSession
///////////////////////////////////
CSession::CSession()
    : m_impl(make_shared<SImpl>())
{
}

CSession::~CSession()
{
    if (m_impl->m_sid.is_nil())
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
            m_impl->m_sid = boost::uuids::string_generator()(results[1].str());
        }
    }

    // Reinit UserDefaults and Log with new session ID
    CUserDefaults::instance().reinit(boost::uuids::string_generator()(boost::uuids::to_string(m_impl->m_sid)),
                                     CUserDefaults::instance().currentUDFile());
    Logger::instance().reinit();

    // Subscribe to custom commands after the DDS session has started
    subscribe();

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

    m_impl->m_sid = _sid;

    // Reinit UserDefaults and Log with new session ID
    CUserDefaults::instance().reinit(boost::uuids::string_generator()(boost::uuids::to_string(m_impl->m_sid)),
                                     CUserDefaults::instance().currentUDFile());

    // Subscribe to custom commands after the DDS session has been attached
    subscribe();
}

void CSession::shutdown()
{
    // stop active session
    string sOut;
    string sErr;
    int nExitCode(0);
    stringstream ssCmd;
    ssCmd << boost::process::search_path("dds-session").string() << " stop " << boost::uuids::to_string(m_impl->m_sid);
    execute(ssCmd.str(), std::chrono::seconds(g_WAIT_PROCESS_SEC), &sOut, &sErr, &nExitCode);

    if (nExitCode == 0)
        m_impl->m_sid = boost::uuids::nil_uuid();

    unsubscribe();
}

boost::uuids::uuid CSession::getSessionID() const
{
    return m_impl->m_sid;
}

void CSession::blockCurrentThread()
{
    if (m_impl->m_sid.is_nil())
        throw runtime_error("ToolsAPI: First create or attache to a DDS session.");

    size_t num_requests = m_impl->m_requests.size();

    // We wait only if _block is true and we have subscribers
    if (num_requests > 0)
    {
        internal_api::CDDSIntercomGuard::instance().waitCondition();
    }
}

void CSession::subscribe()
{
    m_impl->m_customCmd.subscribe([this](const string& _command, const string& _condition, uint64_t _senderId) {
        // TODO: FIXME: temporary solution for Tools API and custom command living in the same process
        try
        {
            istringstream ss(_command);
            notify(ss);
        }
        catch (exception& error)
        {
        }
    });

    m_impl->m_service.start();
}

void CSession::unsubscribe()
{
    m_impl->m_customCmd.unsubscribe();

    m_impl->m_requests.clear();
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
            const requestID_t requestID = child.second.get<requestID_t>("requestID");

            auto it = m_impl->m_requests.find(requestID);
            if (it == m_impl->m_requests.end())
                continue;

            if (it->second.type() == typeid(SSubmitRequest::ptr_t))
            {
                processRequest<SSubmitRequest>(it->second, child, nullptr);
            }
            else if (it->second.type() == typeid(STopologyRequest::ptr_t))
            {
                processRequest<STopologyRequest>(it->second, child, nullptr);
            }
            else if (it->second.type() == typeid(SGetLogRequest::ptr_t))
            {
                processRequest<SGetLogRequest>(it->second, child, nullptr);
            }
            else if (it->second.type() == typeid(SCommanderInfoRequest::ptr_t))
            {
                processRequest<SCommanderInfoRequest>(
                    it->second, child, [&child](SCommanderInfoRequest::ptr_t _request) {
                        SCommanderInfoResponseData data;
                        data.fromPT(child.second);
                        _request->execResponseCallback(data);
                    });
            }
            else if (it->second.type() == typeid(SAgentInfoRequest::ptr_t))
            {
                processRequest<SAgentInfoRequest>(it->second, child, [&child](SAgentInfoRequest::ptr_t _request) {
                    SAgentInfoResponseData data;
                    data.fromPT(child.second);
                    _request->execResponseCallback(data);
                });
            }
            else if (it->second.type() == typeid(SAgentCountRequest::ptr_t))
            {
                processRequest<SAgentCountRequest>(it->second, child, [&child](SAgentCountRequest::ptr_t _request) {
                    SAgentCountResponseData data;
                    data.fromPT(child.second);
                    _request->execResponseCallback(data);
                });
            }
        }
    }
    catch (exception& error)
    {
        throw runtime_error("ToolsAPI: Can't parse input message: " + string(error.what()));
    }
}

template <class T>
void CSession::processRequest(requests_t::mapped_type _request,
                              const boost::property_tree::ptree::value_type& _child,
                              std::function<void(typename T::ptr_t)> _processResponseCallback)
{
    const std::string& tag = _child.first;

    auto request = boost::any_cast<typename T::ptr_t>(_request);
    try
    {
        if (tag == "done")
        {
            request->execDoneCallback();
        }
        else if (tag == "message")
        {
            SMessageResponseData msg;
            msg.fromPT(_child.second);
            request->execMessageCallback(msg);
        }
        else if (tag == "progress")
        {
            SProgressResponseData progress;
            progress.fromPT(_child.second);
            request->execProgressCallback(progress);
        }
        else
        {
            if (_processResponseCallback)
                _processResponseCallback(request);
        }
    }
    catch (const std::exception& _e)
    {
        throw std::runtime_error("DDS tools API: User's callback exception: " + std::string(_e.what()));
    }
}

template <class T>
void CSession::sendRequest(typename T::ptr_t _request)
{
    if (!_request)
        throw std::runtime_error("sendRequest: argument can't be NULL");

    requestID_t reqID = _request->getRequest().m_requestID;
    m_impl->m_requests.insert(std::make_pair(reqID, _request));

    m_impl->m_customCmd.send(_request->getRequest().toJSON(), dds::intercom_api::g_sToolsAPISign);
}

template void CSession::sendRequest<SSubmitRequest>(SSubmitRequest::ptr_t);
template void CSession::sendRequest<STopologyRequest>(STopologyRequest::ptr_t);
template void CSession::sendRequest<SGetLogRequest>(SGetLogRequest::ptr_t);
template void CSession::sendRequest<SCommanderInfoRequest>(SCommanderInfoRequest::ptr_t);
template void CSession::sendRequest<SAgentInfoRequest>(SAgentInfoRequest::ptr_t);
template void CSession::sendRequest<SAgentCountRequest>(SAgentCountRequest::ptr_t);
