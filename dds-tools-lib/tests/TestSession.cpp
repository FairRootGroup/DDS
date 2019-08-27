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

using namespace std;
using namespace dds;
using namespace dds::tools_api;
using namespace dds::topology_api;

BOOST_AUTO_TEST_SUITE(test_dds_tools_session)

void createDDS(CSession& _session)
{
    boost::uuids::uuid sid;
    BOOST_CHECK_NO_THROW(sid = _session.create());
    BOOST_CHECK(!sid.is_nil());
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
}

BOOST_AUTO_TEST_CASE(test_dds_tools_session_create_mult)
{
    // Start and stop DDS session multiple times.
    // Each time create new DDSSession instance.
    for (size_t i = 0; i < 5; i++)
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
    for (size_t i = 0; i < 5; i++)
    {
        createDDS(session);
    }
}

void checkIdleAgents(CSession& _session, size_t _numAgents)
{
    const std::chrono::seconds timeout(30);
    const std::chrono::milliseconds requestInterval(500);
    const size_t maxRequests(60);

    BOOST_CHECK_NO_THROW(_session.waitForNumAgents<CSession::EAgentState::idle>(
        _numAgents, timeout, requestInterval, maxRequests, &std::cout));

    SAgentCountRequest::response_t agentCountInfo;
    BOOST_CHECK_NO_THROW(_session.syncSendRequest<SAgentCountRequest>(
        SAgentCountRequest::request_t(), agentCountInfo, timeout, &std::cout));
    BOOST_CHECK_EQUAL(agentCountInfo.m_activeAgentsCount, _numAgents);
    BOOST_CHECK_EQUAL(agentCountInfo.m_idleAgentsCount, _numAgents);
    BOOST_CHECK_EQUAL(agentCountInfo.m_executingAgentsCount, 0);
}

void runDDS(CSession& _session)
{
    const string topoFile("property_test.xml");
    const std::chrono::seconds timeout(30);
    boost::filesystem::path topoPath(boost::filesystem::current_path());
    topoPath.append(topoFile);

    boost::uuids::uuid sid = _session.create();
    BOOST_CHECK(!sid.is_nil());
    BOOST_CHECK(_session.IsRunning());

    CTopology topo(topoPath.string());
    size_t numAgents = topo.getRequiredNofAgents();

    SSubmitRequest::request_t submitInfo;
    submitInfo.m_rms = "localhost";
    submitInfo.m_instances = numAgents;
    BOOST_CHECK_NO_THROW(_session.syncSendRequest<SSubmitRequest>(submitInfo, timeout, &std::cout));

    checkIdleAgents(_session, numAgents);

    STopologyRequest::request_t topoInfo;
    topoInfo.m_topologyFile = topoPath.string();
    topoInfo.m_updateType = STopologyRequest::request_t::EUpdateType::ACTIVATE;
    BOOST_CHECK_NO_THROW(_session.syncSendRequest<STopologyRequest>(topoInfo, timeout, &std::cout));

    checkIdleAgents(_session, numAgents);

    SAgentInfoRequest::responseVector_t agentInfo;
    BOOST_CHECK_NO_THROW(
        _session.syncSendRequest<SAgentInfoRequest>(SAgentInfoRequest::request_t(), agentInfo, timeout, &std::cout));
    BOOST_CHECK_EQUAL(agentInfo.size(), numAgents);

    SCommanderInfoRequest::response_t commanderInfo;
    BOOST_CHECK_NO_THROW(_session.syncSendRequest<SCommanderInfoRequest>(
        SCommanderInfoRequest::request_t(), commanderInfo, timeout, &std::cout));

    BOOST_CHECK_NO_THROW(_session.syncSendRequest<SGetLogRequest>(SGetLogRequest::request_t(), timeout, &std::cout));

    _session.shutdown();
    BOOST_CHECK(_session.getSessionID().is_nil());
    BOOST_CHECK(!_session.IsRunning());
}

BOOST_AUTO_TEST_CASE(test_dds_tools_session_run_mult)
{
    // Start and stop DDS session multiple times.
    // Each time create new DDSSession instance.
    for (size_t i = 0; i < 5; i++)
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
    for (size_t i = 0; i < 5; i++)
    {
        runDDS(session);
    }
}

BOOST_AUTO_TEST_SUITE_END()
