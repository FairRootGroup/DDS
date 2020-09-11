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

void createDDS(CSession& _session)
{
    boost::uuids::uuid sid;
    BOOST_CHECK_NO_THROW(sid = _session.create());
    BOOST_CHECK(!sid.is_nil());
    // BOOST_CHECK(CSession::getDefaultSessionID() == sid);
    BOOST_CHECK(_session.IsRunning());
    BOOST_CHECK_THROW(_session.create(), runtime_error);
    BOOST_CHECK_THROW(_session.attach(sid), runtime_error);

    CSession sessionAttach;
    BOOST_CHECK_NO_THROW(sessionAttach.attach(_session.getSessionID()));
    BOOST_CHECK(!sessionAttach.getSessionID().is_nil());
    BOOST_CHECK(sessionAttach.IsRunning());

    BOOST_CHECK_NO_THROW(_session.shutdown());
    BOOST_CHECK(_session.getSessionID().is_nil());
    BOOST_CHECK(!_session.IsRunning());
    BOOST_CHECK_THROW(_session.shutdown(), runtime_error);

    BOOST_CHECK(!sessionAttach.IsRunning());
    BOOST_CHECK(!sessionAttach.getSessionID().is_nil());
    BOOST_CHECK_NO_THROW(sessionAttach.detach());
    BOOST_CHECK(sessionAttach.getSessionID().is_nil());
}

BOOST_AUTO_TEST_CASE(test_dds_tools_session_create_mult)
{
    // Start and stop DDS session multiple times.
    // Each time create new DDSSession instance.
    for (size_t i = 0; i < kDDSNumTestIterations; i++)
    {
        CSession session;
        createDDS(session);
    }
}

BOOST_AUTO_TEST_CASE(test_dds_tools_session_create_single)
{
    // Start and stop DDS session multiple times.
    // Common DDSSession instance.
    CSession session;
    for (size_t i = 0; i < kDDSNumTestIterations; i++)
    {
        createDDS(session);
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

void runDDS(CSession& _session)
{
    namespace fs = boost::filesystem;
    fs::path topoPath(fs::canonical(fs::path("property_test.xml")));
    fs::path upTopoPath(fs::canonical(fs::path("property_test_up.xml")));
    fs::path downTopoPath(topoPath);

    boost::uuids::uuid sid = _session.create();
    BOOST_CHECK(!sid.is_nil());
    BOOST_CHECK(_session.IsRunning());

    CTopology topo(topoPath.string());
    auto numAgents = topo.getRequiredNofAgents(10);
    makeRequests(_session, topoPath, STopologyRequest::request_t::EUpdateType::ACTIVATE, numAgents, numAgents);

    CTopology upTopo(upTopoPath.string());
    auto upNumAgents = upTopo.getRequiredNofAgents(10);
    makeRequests(_session, upTopoPath, STopologyRequest::request_t::EUpdateType::UPDATE, numAgents, upNumAgents);

    makeRequests(
        _session, downTopoPath, STopologyRequest::request_t::EUpdateType::UPDATE, make_pair(0, 0), upNumAgents);

    _session.shutdown();
    BOOST_CHECK(_session.getSessionID().is_nil());
    BOOST_CHECK(!_session.IsRunning());
}

BOOST_AUTO_TEST_CASE(test_dds_tools_session_run_mult)
{
    // Start and stop DDS session multiple times.
    // Each time create new DDSSession instance.
    for (size_t i = 0; i < kDDSNumTestIterations; i++)
    {
        CSession session;
        runDDS(session);
    }
}

BOOST_AUTO_TEST_CASE(test_dds_tools_session_run_single)
{
    // Start and stop DDS session multiple times.
    // Common DDSSession instance.
    CSession session;
    for (size_t i = 0; i < kDDSNumTestIterations; i++)
    {
        runDDS(session);
    }
}

void runDDSInf(CSession& _session)
{
    const std::chrono::seconds timeout(30);
    const std::chrono::seconds sleepTime(3);

    namespace fs = boost::filesystem;
    fs::path topoPath(fs::canonical(fs::path("property_test_inf.xml")));
    fs::path upTopoPath(fs::canonical(fs::path("property_test_inf_up.xml")));
    fs::path downTopoPath(topoPath);

    boost::uuids::uuid sid = _session.create();
    BOOST_CHECK(!sid.is_nil());
    BOOST_CHECK(_session.IsRunning());

    const size_t numSlots(20);

    // Submit enough agent for the upscaled topology
    SSubmitRequest::request_t submitInfo;
    submitInfo.m_rms = "localhost";
    submitInfo.m_slots = numSlots;
    BOOST_CHECK_NO_THROW(_session.syncSendRequest<SSubmitRequest>(submitInfo, timeout, &std::cout));

    checkIdleAgents(_session, numSlots);

    for (size_t i = 0; i < kDDSNumTestIterations; i++)
    {
        // Activate default topology
        STopologyRequest::request_t topoInfo;
        topoInfo.m_topologyFile = topoPath.string();
        topoInfo.m_updateType = STopologyRequest::request_t::EUpdateType::ACTIVATE;
        BOOST_CHECK_NO_THROW(_session.syncSendRequest<STopologyRequest>(topoInfo, timeout, &std::cout));

        std::this_thread::sleep_for(sleepTime);

        // Upscale topology
        STopologyRequest::request_t upTopoInfo;
        upTopoInfo.m_topologyFile = upTopoPath.string();
        upTopoInfo.m_updateType = STopologyRequest::request_t::EUpdateType::UPDATE;
        BOOST_CHECK_NO_THROW(_session.syncSendRequest<STopologyRequest>(upTopoInfo, timeout, &std::cout));

        std::this_thread::sleep_for(sleepTime);

        // Downscale topology
        STopologyRequest::request_t downTopoInfo;
        downTopoInfo.m_topologyFile = downTopoPath.string();
        downTopoInfo.m_updateType = STopologyRequest::request_t::EUpdateType::UPDATE;
        BOOST_CHECK_NO_THROW(_session.syncSendRequest<STopologyRequest>(downTopoInfo, timeout, &std::cout));

        std::this_thread::sleep_for(sleepTime);

        // Stop topology
        STopologyRequest::request_t stopTopoInfo;
        stopTopoInfo.m_updateType = STopologyRequest::request_t::EUpdateType::STOP;
        BOOST_CHECK_NO_THROW(_session.syncSendRequest<STopologyRequest>(stopTopoInfo, timeout, &std::cout));

        checkIdleAgents(_session, numSlots);
    }

    _session.shutdown();
    BOOST_CHECK(_session.getSessionID().is_nil());
    BOOST_CHECK(!_session.IsRunning());
}

BOOST_AUTO_TEST_CASE(test_dds_tools_session_run_mult_inf)
{
    // Start and stop DDS session multiple times.
    // Each time create new DDSSession instance.
    for (size_t i = 0; i < kDDSNumTestIterations; i++)
    {
        CSession session;
        runDDSInf(session);
    }
}

BOOST_AUTO_TEST_CASE(test_dds_tools_session_run_single_inf)
{
    // Start and stop DDS session multiple times.
    // Common DDSSession instance.
    CSession session;
    for (size_t i = 0; i < kDDSNumTestIterations; i++)
    {
        runDDSInf(session);
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
