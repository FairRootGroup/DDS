// Copyright 2019 GSI, Inc. All rights reserved.
//
//
//

#include "Tools.h"
// STD
#include <chrono>
#include <sstream>
// BOOST
#include <boost/any.hpp>
#include <boost/filesystem.hpp>
#include <boost/process.hpp>
#include <boost/regex.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
// MiscCommon
#include "ConditionEvent.h"
#include "Process.h"
// DDS
#include "Intercom.h"
#include "IntercomServiceCore.h"
#include "ToolsProtocol.h"
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
// CSession::SImpl
///////////////////////////////////
struct CSession::SImpl
{
    SImpl()
        : m_sid(boost::uuids::nil_uuid())
        , m_service(nullptr)
        , m_customCmd(nullptr)
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

    boost::uuids::uuid m_sid;                                       ///< Session ID.
    std::shared_ptr<dds::intercom_api::CIntercomService> m_service; ///< Intercom service.
    std::shared_ptr<dds::intercom_api::CCustomCmd>
        m_customCmd;       ///< Custom commands API. Used for communication with commander.
    requests_t m_requests; ///< Array of requests.
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
    lock_guard<mutex> lock(m_mtxRequests);
    m_impl->m_requests.clear();
}

boost::uuids::uuid CSession::create()
{
    if (!isDDSAvailable())
        throw runtime_error(
            "ToolsAPI: Missing DDS environment. Make sure to init DDS env using DDS_env.sh before using this API");

    if (!m_impl->m_sid.is_nil())
        throw runtime_error("ToolsAPI: DDS session is already running.");

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
    if (m_impl->m_sid.is_nil())
        throw runtime_error("Failed to parse DDS session ID from: " + sOut + "; error: " + sErr);

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

    if (!m_impl->m_sid.is_nil())
        throw runtime_error("ToolsAPI: DDS session is already running.");

    m_impl->m_sid = _sid;

    // Reinit UserDefaults and Log with new session ID
    CUserDefaults::instance().reinit(boost::uuids::string_generator()(boost::uuids::to_string(m_impl->m_sid)),
                                     CUserDefaults::instance().currentUDFile());
    Logger::instance().reinit();

    // Subscribe to custom commands after the DDS session has been attached
    subscribe();
}

void CSession::shutdown()
{
    if (!isDDSAvailable())
        throw runtime_error(
            "ToolsAPI: Missing DDS environment. Make sure to init DDS env using DDS_env.sh before using this API");

    if (m_impl->m_sid.is_nil())
        throw runtime_error("ToolsAPI: DDS session is not running.");

    // stop active session
    string sOut;
    string sErr;
    int nExitCode(0);
    stringstream ssCmd;
    ssCmd << boost::process::search_path("dds-session").string() << " stop " << boost::uuids::to_string(m_impl->m_sid);
    execute(ssCmd.str(), std::chrono::seconds(g_WAIT_PROCESS_SEC), &sOut, &sErr, &nExitCode);

    m_impl->m_sid = boost::uuids::nil_uuid();
    m_impl->m_customCmd.reset();
    m_impl->m_service.reset();
    m_impl->m_requests.clear();

    if (nExitCode != 0)
        throw runtime_error("ToolsAPI: Failed to stop DDS session. Exit code: " + to_string(nExitCode));
}

void CSession::detach()
{
    if (m_impl->m_sid.is_nil())
        throw runtime_error("ToolsAPI: DDS session is not running.");

    m_impl->m_sid = boost::uuids::nil_uuid();
    m_impl->m_customCmd.reset();
    m_impl->m_service.reset();
    m_impl->m_requests.clear();
}

boost::uuids::uuid CSession::getSessionID() const
{
    return m_impl->m_sid;
}

string CSession::getDefaultSessionIDString()
{
    return CUserDefaults::instance().getDefaultSID();
}

boost::uuids::uuid CSession::getDefaultSessionID()
{
    try
    {
        const string sid = getDefaultSessionIDString();
        if (sid.empty())
            return boost::uuids::nil_uuid();
        else
            return boost::uuids::string_generator()(sid);
    }
    catch (...)
    {
        return boost::uuids::nil_uuid();
    }
}

void CSession::blockCurrentThread()
{
    if (m_impl->m_sid.is_nil() || m_impl->m_service == nullptr)
        throw runtime_error("ToolsAPI: First create or attach to a DDS session.");

    if (m_impl->m_requests.size() > 0)
    {
        m_impl->m_service->waitCondition();
    }
}

void CSession::unblockCurrentThread()
{
    if (m_impl->m_sid.is_nil() || m_impl->m_service == nullptr)
        throw runtime_error("ToolsAPI: First create or attach to a DDS session.");

    m_impl->m_service->stopCondition();
}

void CSession::subscribe()
{
    m_impl->m_service = make_shared<dds::intercom_api::CIntercomService>();
    m_impl->m_customCmd = make_shared<dds::intercom_api::CCustomCmd>(*m_impl->m_service);

    m_impl->m_customCmd->subscribe([this](const string& _command, const string& _condition, uint64_t _senderId) {
        istringstream ss(_command);
        notify(ss);
    });

    m_impl->m_service->start();
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

            // WARNING: The lock will prevent this function to be executed in multiple threads.
            //           By the current design all user requests are processed sequentially (one thread).
            // WARNING: A deadlock can occur if a user calls CSession destructor in one of the requests' callback
            lock_guard<mutex> lock(m_mtxRequests);
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
    if (_request == nullptr)
        throw std::runtime_error("sendRequest: argument can't be NULL");

    if (m_impl->m_customCmd == nullptr)
        throw std::runtime_error("sendRequest: custom commands service is not running");

    requestID_t reqID = _request->getRequest().m_requestID;
    m_impl->m_requests.insert(std::make_pair(reqID, _request));

    m_impl->m_customCmd->send(_request->getRequest().toJSON(), dds::intercom_api::g_sToolsAPISign);
}

template void CSession::sendRequest<SSubmitRequest>(SSubmitRequest::ptr_t);
template void CSession::sendRequest<STopologyRequest>(STopologyRequest::ptr_t);
template void CSession::sendRequest<SGetLogRequest>(SGetLogRequest::ptr_t);
template void CSession::sendRequest<SCommanderInfoRequest>(SCommanderInfoRequest::ptr_t);
template void CSession::sendRequest<SAgentInfoRequest>(SAgentInfoRequest::ptr_t);
template void CSession::sendRequest<SAgentCountRequest>(SAgentCountRequest::ptr_t);

template <class Request_t>
void CSession::syncSendRequest(const typename Request_t::request_t& _requestData,
                               const std::chrono::seconds& _timeout,
                               std::ostream* _out)
{
    typename Request_t::responseVector_t responseVector;
    syncSendRequest<Request_t>(_requestData, responseVector, _timeout, _out);
}

template void CSession::syncSendRequest<SSubmitRequest>(const SSubmitRequest::request_t&,
                                                        const std::chrono::seconds&,
                                                        ostream*);
template void CSession::syncSendRequest<STopologyRequest>(const STopologyRequest::request_t&,
                                                          const std::chrono::seconds&,
                                                          ostream*);
template void CSession::syncSendRequest<SGetLogRequest>(const SGetLogRequest::request_t&,
                                                        const std::chrono::seconds&,
                                                        ostream*);

template <class Request_t>
void CSession::syncSendRequest(const typename Request_t::request_t& _requestData,
                               typename Request_t::response_t& _responseData,
                               const std::chrono::seconds& _timeout,
                               std::ostream* _out)
{
    typename Request_t::responseVector_t responseVector;
    syncSendRequest<Request_t>(_requestData, responseVector, _timeout, _out);
    if (responseVector.empty())
        throw runtime_error("Request failed: empty vector of response data");
    _responseData = responseVector.front();
}

template void CSession::syncSendRequest<SCommanderInfoRequest>(const SCommanderInfoRequest::request_t&,
                                                               SCommanderInfoRequest::response_t&,
                                                               const std::chrono::seconds&,
                                                               ostream*);
template void CSession::syncSendRequest<SAgentCountRequest>(const SAgentCountRequest::request_t&,
                                                            SAgentCountRequest::response_t&,
                                                            const std::chrono::seconds&,
                                                            ostream*);

template <class Request_t>
void CSession::syncSendRequest(const typename Request_t::request_t& _requestData,
                               typename Request_t::responseVector_t& _responseDataVector,
                               const std::chrono::seconds& _timeout,
                               std::ostream* _out)
{
    // TODO: FIXME: Dublicate output to DDS log?

    if (getSessionID().is_nil() || !IsRunning())
        throw runtime_error("Failed to send request: DDS session is not running");

    _responseDataVector.clear();

    typename Request_t::ptr_t requestPtr = Request_t::makeRequest(_requestData);

    requestPtr->setResponseCallback(
        [&_responseDataVector](const typename Request_t::response_t& _data) { _responseDataVector.push_back(_data); });

    //    requestPtr->setProgressCallback([](const SProgressResponseData&) {
    //        // No progress reporting for sync version
    //    });

    requestPtr->setMessageCallback([&_out](const SMessageResponseData& _message) {
        if (_message.m_severity == dds::intercom_api::EMsgSeverity::error)
        {
            throw runtime_error("Failed to submit agents: server reports error: " + _message.m_msg);
        }
        else
        {
            if (_out != nullptr)
                *_out << "Server reports: " << _message.m_msg << endl;
        }
    });

    CConditionEvent cv;

    requestPtr->setDoneCallback([&cv]() { cv.notifyAll(); });

    sendRequest<Request_t>(requestPtr);

    if (_timeout.count() == 0)
    {
        cv.wait();
    }
    else
    {
        if (!cv.waitFor(_timeout))
        {
            throw runtime_error("Timed out waiting for request");
        }
    }

    if (_out != nullptr)
        *_out << "Request finished successfully" << endl;
}

template void CSession::syncSendRequest<SAgentInfoRequest>(const SAgentInfoRequest::request_t&,
                                                           SAgentInfoRequest::responseVector_t&,
                                                           const std::chrono::seconds&,
                                                           ostream*);

template <CSession::EAgentState _state>
void CSession::waitForNumAgents(size_t _numAgents,
                                const std::chrono::seconds& _timeout,
                                const std::chrono::milliseconds& _requestInterval,
                                ostream* _out)
{
    using Time_t = std::chrono::seconds;
    std::chrono::system_clock::time_point start{ std::chrono::system_clock::now() };

    auto remained{ _timeout };
    while (true)
    {
        SAgentCountRequest::response_t response;
        syncSendRequest<SAgentCountRequest>(SAgentCountRequest::request_t(), response, remained, _out);

        // Check if we have the required number of agents
        if ((_state == CSession::EAgentState::active && (response.m_activeSlotsCount < _numAgents)) ||
            (_state == CSession::EAgentState::idle && (response.m_idleSlotsCount < _numAgents)) ||
            (_state == CSession::EAgentState::executing && (response.m_executingSlotsCount < _numAgents)))
        {
            remained = std::chrono::duration_cast<Time_t>(_timeout - (std::chrono::system_clock::now() - start) -
                                                          _requestInterval);
            if (_timeout.count() != 0 && remained.count() <= 0)
            {
                throw runtime_error("Failed to wait for the required number of agents: exceed timeout");
            }
            this_thread::sleep_for(_requestInterval);
        }
        else
        {
            return;
        }
    }
}

template void CSession::waitForNumAgents<CSession::EAgentState::active>(size_t,
                                                                        const std::chrono::seconds&,
                                                                        const std::chrono::milliseconds&,
                                                                        ostream*);
template void CSession::waitForNumAgents<CSession::EAgentState::idle>(size_t,
                                                                      const std::chrono::seconds&,
                                                                      const std::chrono::milliseconds&,
                                                                      ostream*);
template void CSession::waitForNumAgents<CSession::EAgentState::executing>(size_t,
                                                                           const std::chrono::seconds&,
                                                                           const std::chrono::milliseconds&,
                                                                           ostream*);

std::string CSession::userDefaultsGetValueForKey(const std::string& _key) const noexcept
{
    return CUserDefaults::instance().getValueForKey(_key);
}
