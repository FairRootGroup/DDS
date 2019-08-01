// Copyright 2019 GSI, Inc. All rights reserved.
//
//
//

// BOOST: tests
// Defines test_main function to link with actual unit test code.
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
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

BOOST_AUTO_TEST_CASE(test_dds_tools_session_create)
{
    // TODO: FIXME: Restarting the session in the same process is not supported yet. Uncomment when the feature is
    // available.
    //    CSession session;
    //    boost::uuids::uuid sid = session.create();
    //    BOOST_CHECK(!sid.is_nil());
    //    BOOST_CHECK(session.IsRunning());
    //
    //    CSession sessionAttach;
    //    sessionAttach.attach(session.getSessionID());
    //    BOOST_CHECK(!sessionAttach.getSessionID().is_nil());
    //    BOOST_CHECK(sessionAttach.IsRunning());
    //
    //    session.shutdown();
    //    BOOST_CHECK(session.getSessionID().is_nil());
    //    BOOST_CHECK(!session.IsRunning());
    //    BOOST_CHECK(!sessionAttach.IsRunning());
}

void checkIdleAgents(CSession& _session, size_t _numAgents)
{
    const std::chrono::seconds timeout(10);
    const std::chrono::milliseconds requestInterval(500);
    const size_t maxRequests(10);

    BOOST_CHECK_NO_THROW(_session.waitForNumAgents<CSession::EAgentState::idle>(
        _numAgents, timeout, requestInterval, maxRequests, &std::cout));

    SAgentCountRequest::response_t agentCountInfo;
    BOOST_CHECK_NO_THROW(_session.syncSendRequest<SAgentCountRequest>(
        SAgentCountRequest::request_t(), agentCountInfo, timeout, &std::cout));
    BOOST_CHECK_EQUAL(agentCountInfo.m_activeAgentsCount, _numAgents);
    BOOST_CHECK_EQUAL(agentCountInfo.m_idleAgentsCount, _numAgents);
    BOOST_CHECK_EQUAL(agentCountInfo.m_executingAgentsCount, 0);
}

BOOST_AUTO_TEST_CASE(test_dds_tools_session_1)
{
    const string topoFile("property_test.xml");
    const std::chrono::seconds timeout(20);

    CSession session;
    boost::uuids::uuid sid = session.create();
    BOOST_CHECK(!sid.is_nil());
    BOOST_CHECK(session.IsRunning());

    CTopology topo(topoFile);
    size_t numAgents = topo.getRequiredNofAgents();

    SSubmitRequest::request_t submitInfo;
    submitInfo.m_rms = "localhost";
    submitInfo.m_instances = numAgents;
    BOOST_CHECK_NO_THROW(session.syncSendRequest<SSubmitRequest>(submitInfo, timeout, &std::cout));

    checkIdleAgents(session, numAgents);

    STopologyRequest::request_t topoInfo;
    topoInfo.m_topologyFile = topoFile;
    topoInfo.m_updateType = STopologyRequest::request_t::EUpdateType::ACTIVATE;
    BOOST_CHECK_NO_THROW(session.syncSendRequest<STopologyRequest>(topoInfo, timeout, &std::cout));

    checkIdleAgents(session, numAgents);

    SAgentInfoRequest::responseVector_t agentInfo;
    BOOST_CHECK_NO_THROW(
        session.syncSendRequest<SAgentInfoRequest>(SAgentInfoRequest::request_t(), agentInfo, timeout, &std::cout));
    BOOST_CHECK_EQUAL(agentInfo.size(), numAgents);

    SCommanderInfoRequest::response_t commanderInfo;
    BOOST_CHECK_NO_THROW(session.syncSendRequest<SCommanderInfoRequest>(
        SCommanderInfoRequest::request_t(), commanderInfo, timeout, &std::cout));

    BOOST_CHECK_NO_THROW(session.syncSendRequest<SGetLogRequest>(SGetLogRequest::request_t(), timeout, &std::cout));

    session.shutdown();
    BOOST_CHECK(session.getSessionID().is_nil());
    BOOST_CHECK(!session.IsRunning());
}

BOOST_AUTO_TEST_SUITE_END()
