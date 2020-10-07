// Copyright 2019 GSI, Inc. All rights reserved.
//
//
//

// BOOST: tests
// Defines test_main function to link with actual unit test code.
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/filesystem.hpp>
#include <boost/test/output_test_stream.hpp>
#include <boost/test/unit_test.hpp>
// DDS
#include "Tools.h"
#include "Topology.h"
// STD
#include <thread>

using namespace std;
using namespace dds;
using namespace dds::tools_api;
using namespace dds::topology_api;

BOOST_AUTO_TEST_SUITE(test_dds_tools_session)

const size_t kDDSNumTestIterations = 3;
const size_t kDDSNumParallelSessions = 1;

void createDDS(vector<CSession>& _sessions)
{
    for (auto& session : _sessions)
    {
        boost::uuids::uuid sid;
        BOOST_CHECK_NO_THROW(sid = session.create());
        BOOST_CHECK(!sid.is_nil());
        // BOOST_CHECK(CSession::getDefaultSessionID() == sid);
        BOOST_CHECK(session.IsRunning());
        BOOST_CHECK_THROW(session.create(), runtime_error);
        BOOST_CHECK_THROW(session.attach(sid), runtime_error);
    }

    vector<CSession> attachedSessions(_sessions.size());
    size_t index{ 0 };
    for (auto& session : _sessions)
    {
        auto& attachedSession = attachedSessions[index];
        BOOST_CHECK_NO_THROW(attachedSession.attach(session.getSessionID()));
        BOOST_CHECK(!attachedSession.getSessionID().is_nil());
        BOOST_CHECK(attachedSession.IsRunning());
        index++;
    }

    for (auto& session : _sessions)
    {
        BOOST_CHECK_NO_THROW(session.shutdown());
        BOOST_CHECK(session.getSessionID().is_nil());
        BOOST_CHECK(!session.IsRunning());
        BOOST_CHECK_THROW(session.shutdown(), runtime_error);
    }

    for (auto& attachedSession : attachedSessions)
    {
        BOOST_CHECK(!attachedSession.IsRunning());
        BOOST_CHECK(!attachedSession.getSessionID().is_nil());
        BOOST_CHECK_NO_THROW(attachedSession.detach());
        BOOST_CHECK(attachedSession.getSessionID().is_nil());
    }
}

BOOST_AUTO_TEST_CASE(test_dds_tools_session_create_mult)
{
    // Start and stop DDS sessions multiple times.
    // Each time create new DDSSession instance.
    for (size_t i = 0; i < kDDSNumTestIterations; i++)
    {
        vector<CSession> sessions(kDDSNumParallelSessions);
        createDDS(sessions);
    }
}

BOOST_AUTO_TEST_CASE(test_dds_tools_session_create_single)
{
    // Start and stop DDS sessions multiple times.
    // Common DDSSession instance.
    vector<CSession> sessions(kDDSNumParallelSessions);
    for (size_t i = 0; i < kDDSNumTestIterations; i++)
    {
        createDDS(sessions);
    }
}

void checkIdleAgents(CSession& _session, size_t _numAgents)
{
    const std::chrono::seconds timeout(30);
    const std::chrono::milliseconds requestInterval(500);

    BOOST_CHECK_NO_THROW(
        _session.waitForNumAgents<CSession::EAgentState::idle>(_numAgents, timeout, requestInterval, &std::cout));

    BOOST_CHECK_THROW(_session.waitForNumAgents<CSession::EAgentState::idle>(
                          _numAgents + 1, std::chrono::seconds(2), requestInterval, &std::cout),
                      std::runtime_error);

    SAgentCountRequest::response_t agentCountInfo;
    BOOST_CHECK_NO_THROW(_session.syncSendRequest<SAgentCountRequest>(
        SAgentCountRequest::request_t(), agentCountInfo, timeout, &std::cout));
    BOOST_CHECK_EQUAL(agentCountInfo.m_activeSlotsCount, _numAgents);
    BOOST_CHECK_EQUAL(agentCountInfo.m_idleSlotsCount, _numAgents);
    BOOST_CHECK_EQUAL(agentCountInfo.m_executingSlotsCount, 0);
}

void makeRequests(CSession& _session,
                  const boost::filesystem::path& _topoPath,
                  STopologyRequest::request_t::EUpdateType _updateType,
                  const pair<size_t, size_t>& _submitAgents,
                  const pair<size_t, size_t>& _totalAgents)
{
    const std::chrono::seconds timeout(30);

    if (_submitAgents.second > 0)
    {
        SSubmitRequest::request_t submitInfo;
        submitInfo.m_rms = "localhost";
        submitInfo.m_instances = _submitAgents.first;
        submitInfo.m_slots = _submitAgents.second;
        BOOST_CHECK_NO_THROW(_session.syncSendRequest<SSubmitRequest>(submitInfo, timeout, &std::cout));
    }

    size_t numSlots = _totalAgents.first * _totalAgents.second;
    checkIdleAgents(_session, numSlots);

    STopologyRequest::request_t topoInfo;
    topoInfo.m_topologyFile = _topoPath.string();
    topoInfo.m_updateType = _updateType;
    BOOST_CHECK_NO_THROW(_session.syncSendRequest<STopologyRequest>(topoInfo, timeout, &std::cout));

    checkIdleAgents(_session, numSlots);

    SAgentInfoRequest::responseVector_t agentInfo;
    BOOST_CHECK_NO_THROW(
        _session.syncSendRequest<SAgentInfoRequest>(SAgentInfoRequest::request_t(), agentInfo, timeout, &std::cout));
    BOOST_CHECK_EQUAL(agentInfo.size(), _totalAgents.first);

    SCommanderInfoRequest::response_t commanderInfo;
    BOOST_CHECK_NO_THROW(_session.syncSendRequest<SCommanderInfoRequest>(
        SCommanderInfoRequest::request_t(), commanderInfo, timeout, &std::cout));

    BOOST_CHECK_NO_THROW(_session.syncSendRequest<SGetLogRequest>(SGetLogRequest::request_t(), timeout, &std::cout));
}

void runDDS(vector<CSession>& _sessions)
{
    namespace fs = boost::filesystem;
    fs::path topoPath(fs::canonical(fs::path("property_test.xml")));
    fs::path upTopoPath(fs::canonical(fs::path("property_test_up.xml")));
    fs::path downTopoPath(topoPath);

    for (auto& session : _sessions)
    {
        boost::uuids::uuid sid = session.create();
        BOOST_CHECK(!sid.is_nil());
        BOOST_CHECK(session.IsRunning());
    }

    CTopology topo(topoPath.string());
    auto numAgents = topo.getRequiredNofAgents(10);
    for (auto& session : _sessions)
    {
        makeRequests(session, topoPath, STopologyRequest::request_t::EUpdateType::ACTIVATE, numAgents, numAgents);
    }

    CTopology upTopo(upTopoPath.string());
    auto upNumAgents = upTopo.getRequiredNofAgents(10);
    for (auto& session : _sessions)
    {
        makeRequests(session, upTopoPath, STopologyRequest::request_t::EUpdateType::UPDATE, numAgents, upNumAgents);
    }

    for (auto& session : _sessions)
    {
        makeRequests(
            session, downTopoPath, STopologyRequest::request_t::EUpdateType::UPDATE, make_pair(0, 0), upNumAgents);
    }

    for (auto& session : _sessions)
    {
        session.shutdown();
        BOOST_CHECK(session.getSessionID().is_nil());
        BOOST_CHECK(!session.IsRunning());
    }
}

BOOST_AUTO_TEST_CASE(test_dds_tools_session_run_mult)
{
    // Start and stop DDS session multiple times.
    // Each time create new DDSSession instance.
    for (size_t i = 0; i < kDDSNumTestIterations; i++)
    {
        vector<CSession> sessions(kDDSNumParallelSessions);
        runDDS(sessions);
    }
}

BOOST_AUTO_TEST_CASE(test_dds_tools_session_run_single)
{
    // Start and stop DDS session multiple times.
    // Common DDSSession instance.
    vector<CSession> sessions(kDDSNumParallelSessions);
    for (size_t i = 0; i < kDDSNumTestIterations; i++)
    {
        runDDS(sessions);
    }
}

void runDDSInf(vector<CSession>& _sessions)
{
    const std::chrono::seconds timeout(30);
    const std::chrono::seconds sleepTime(3);

    namespace fs = boost::filesystem;
    fs::path topoPath(fs::canonical(fs::path("property_test_inf.xml")));
    fs::path upTopoPath(fs::canonical(fs::path("property_test_inf_up.xml")));
    fs::path downTopoPath(topoPath);

    for (auto& session : _sessions)
    {
        boost::uuids::uuid sid = session.create();
        BOOST_CHECK(!sid.is_nil());
        BOOST_CHECK(session.IsRunning());
    }

    const size_t numSlots(20);

    // Submit enough agent for the upscaled topology
    SSubmitRequest::request_t submitInfo;
    submitInfo.m_rms = "localhost";
    submitInfo.m_slots = numSlots;
    for (auto& session : _sessions)
    {
        BOOST_CHECK_NO_THROW(session.syncSendRequest<SSubmitRequest>(submitInfo, timeout, &std::cout));
    }

    for (auto& session : _sessions)
    {
        checkIdleAgents(session, numSlots);
    }

    for (size_t i = 0; i < kDDSNumTestIterations; i++)
    {
        // Activate default topology
        STopologyRequest::request_t topoInfo;
        topoInfo.m_topologyFile = topoPath.string();
        topoInfo.m_updateType = STopologyRequest::request_t::EUpdateType::ACTIVATE;
        for (auto& session : _sessions)
        {
            BOOST_CHECK_NO_THROW(session.syncSendRequest<STopologyRequest>(topoInfo, timeout, &std::cout));
        }
        std::this_thread::sleep_for(sleepTime);

        // Upscale topology
        STopologyRequest::request_t upTopoInfo;
        upTopoInfo.m_topologyFile = upTopoPath.string();
        upTopoInfo.m_updateType = STopologyRequest::request_t::EUpdateType::UPDATE;
        for (auto& session : _sessions)
        {
            BOOST_CHECK_NO_THROW(session.syncSendRequest<STopologyRequest>(upTopoInfo, timeout, &std::cout));
        }
        std::this_thread::sleep_for(sleepTime);

        // Downscale topology
        STopologyRequest::request_t downTopoInfo;
        downTopoInfo.m_topologyFile = downTopoPath.string();
        downTopoInfo.m_updateType = STopologyRequest::request_t::EUpdateType::UPDATE;
        for (auto& session : _sessions)
        {
            BOOST_CHECK_NO_THROW(session.syncSendRequest<STopologyRequest>(downTopoInfo, timeout, &std::cout));
        }
        std::this_thread::sleep_for(sleepTime);

        // Stop topology
        STopologyRequest::request_t stopTopoInfo;
        stopTopoInfo.m_updateType = STopologyRequest::request_t::EUpdateType::STOP;
        for (auto& session : _sessions)
        {
            BOOST_CHECK_NO_THROW(session.syncSendRequest<STopologyRequest>(stopTopoInfo, timeout, &std::cout));
        }

        for (auto& session : _sessions)
        {
            checkIdleAgents(session, numSlots);
        }
    }

    for (auto& session : _sessions)
    {
        session.shutdown();
        BOOST_CHECK(session.getSessionID().is_nil());
        BOOST_CHECK(!session.IsRunning());
    }
}

BOOST_AUTO_TEST_CASE(test_dds_tools_session_run_mult_inf)
{
    // Start and stop DDS session multiple times.
    // Each time create new DDSSession instance.
    for (size_t i = 0; i < kDDSNumTestIterations; i++)
    {
        vector<CSession> sessions(kDDSNumParallelSessions);
        runDDSInf(sessions);
    }
}

BOOST_AUTO_TEST_CASE(test_dds_tools_session_run_single_inf)
{
    // Start and stop DDS session multiple times.
    // Common DDSSession instance.
    vector<CSession> sessions(kDDSNumParallelSessions);
    for (size_t i = 0; i < kDDSNumTestIterations; i++)
    {
        runDDSInf(sessions);
    }
}

BOOST_AUTO_TEST_CASE(test_dds_tools_session_user_defaults_with_session)
{
    // Start and stop DDS session multiple times.
    // Common DDSSession instance.
    CSession session;
    boost::uuids::uuid sid = session.create();
    BOOST_CHECK(!sid.is_nil());
    BOOST_CHECK(!session.userDefaultsGetValueForKey("server.work_dir").empty());
    BOOST_CHECK(session.userDefaultsGetValueForKey("bad_key").empty());
    BOOST_CHECK(!session.userDefaultsGetValueForKey("server.log_dir")
                     .empty()); // expected value: $HOME/.DDS/log/sessions/b383d852-19a7-4ac5-9cbe-dc00d686d36f
    session.shutdown();
}

BOOST_AUTO_TEST_CASE(test_dds_tools_session_user_defaults_without_session)
{
    // Start and stop DDS session multiple times.
    // Common DDSSession instance.
    CSession session;
    BOOST_CHECK(!session.userDefaultsGetValueForKey("server.work_dir").empty());
    BOOST_CHECK(!session.userDefaultsGetValueForKey("server.log_dir").empty()); // expected value: $HOME/.DDS/log
}

BOOST_AUTO_TEST_SUITE_END()
