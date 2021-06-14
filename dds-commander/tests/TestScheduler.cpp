// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// BOOST: tests
// Defines test_main function to link with actual unit test code.
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/tools/output_test_stream.hpp>
#include <boost/test/unit_test.hpp>
// DDS
#include "AgentChannel.h"
#include "ConnectionManager.h"
#include "HostInfoCmd.h"
#include "Scheduler.h"
#include "TopoCore.h"
// MiscCommon
#include "TimeMeasure.h"
// BOOST
#include <boost/asio.hpp>
// STD
#include <sstream>

using namespace std;
using namespace dds;
using namespace dds::commander_cmd;
using namespace dds::topology_api;
using namespace dds::user_defaults_api;
using namespace dds::protocol_api;
using namespace MiscCommon;

BOOST_AUTO_TEST_SUITE(test_dds_scheduler_performance)

void make_agent(boost::asio::io_context& _io_context,
                CConnectionManager::channelInfo_t::container_t& _agents,
                const string& _hostName,
                const string& _workerId,
                uint64_t _protocolHeaderID,
                size_t _numSlotsPerAgent)
{
    CAgentChannel::connectionPtr_t agent{ CAgentChannel::makeNew(_io_context, _protocolHeaderID) };
    SAgentInfo& info{ agent->getAgentInfo() };
    info.m_remoteHostInfo.m_host = _hostName;
    info.m_remoteHostInfo.m_workerId = _workerId;

    // Add slots to agent
    for (size_t is = 0; is < _numSlotsPerAgent; ++is)
    {
        SSlotInfo sinfo;
        sinfo.m_state = EAgentState::idle;
        sinfo.m_id = 100000 * _protocolHeaderID + is;
        agent->getAgentInfo().addSlot(sinfo);
        _agents.push_back(CConnectionManager::channelInfo_t(agent, sinfo.m_id, true));
    }
}

BOOST_AUTO_TEST_CASE(test_dds_scheduler_performance_1)
{
    boost::asio::io_context io_context;

    CTopoCore topology;
    topology.init("topology_scheduler_test_1.xml");

    CConnectionManager::channelInfo_t::container_t agents;
    const size_t numTasks{ 3 };
    const size_t numAgents{ 30 };
    const size_t numSlotsPerAgent{ 100 };

    for (size_t ia = 0; ia < numAgents; ++ia)
    {
        const string host{ "host" + to_string((ia % numTasks) + 1) + "_" + to_string(ia) };
        const uint64_t agentID{ ia + 1 };
        make_agent(io_context, agents, host, "", agentID, numSlotsPerAgent);
    }

    using weak_t = CConnectionManager::weakChannelInfo_t;
    weak_t::container_t weakAgents;
    std::transform(
        agents.begin(), agents.end(), std::back_inserter(weakAgents), [](const auto& _v) -> auto {
            return weak_t(_v.m_channel, _v.m_protocolHeaderID, _v.m_isSlot);
        });

    CScheduler scheduler;

    auto execTime = STimeMeasure<std::chrono::microseconds>::execution(
        [&scheduler, &topology, &weakAgents]() { scheduler.makeSchedule(topology, weakAgents); });
    double execTimeSeconds = execTime * 1e-6;

    BOOST_CHECK(execTimeSeconds < 3.0);

    std::cout << "Scheduler execution time: " << execTimeSeconds << " s\n";

    // Test failure
    // Change host info to non existing pattern
    for (auto agent : agents)
    {
        agent.m_channel->getAgentInfo().m_remoteHostInfo.m_host = "nohost";
    }
    auto execFailTime = STimeMeasure<std::chrono::microseconds>::execution(
        [&scheduler, &topology, &weakAgents]()
        { BOOST_CHECK_THROW(scheduler.makeSchedule(topology, weakAgents), runtime_error); });
    double execFailTimeSeconds = execFailTime * 1e-6;

    BOOST_CHECK(execFailTimeSeconds < 0.1);
    std::cout << "Scheduler fail execution time: " << execFailTimeSeconds << " s\n";
}

BOOST_AUTO_TEST_CASE(test_dds_scheduler_1)
{
    boost::asio::io_context io_context;

    CTopoCore topology;
    topology.init("topology_scheduler_test_2.xml");

    const size_t numSlotsPerAgent{ 3 };

    CConnectionManager::channelInfo_t::container_t agents;
    uint64_t counter{ 1 };

    // Requirement type "hostname"
    make_agent(io_context, agents, "host1_0", "wn1", counter++, numSlotsPerAgent);
    make_agent(io_context, agents, "host2_0", "wn2", counter++, numSlotsPerAgent);
    make_agent(io_context, agents, "host3_0", "wn3", counter++, numSlotsPerAgent);
    make_agent(io_context, agents, "host4_0", "wn4", counter++, numSlotsPerAgent);
    make_agent(io_context, agents, "host4_0", "wn4", counter++, numSlotsPerAgent);
    make_agent(io_context, agents, "host4_0", "wn4", counter++, numSlotsPerAgent);

    // Requirement type "wnname"
    make_agent(io_context, agents, "host5_0", "wn5", counter++, numSlotsPerAgent);
    make_agent(io_context, agents, "host6_0", "wn6", counter++, numSlotsPerAgent);

    // No Requirement
    make_agent(io_context, agents, "noname_host", "noname_wn", counter++, numSlotsPerAgent);

    using weak_t = CConnectionManager::weakChannelInfo_t;
    weak_t::container_t weakAgents;
    std::transform(
        agents.begin(), agents.end(), std::back_inserter(weakAgents), [](const auto& _v) -> auto {
            return weak_t(_v.m_channel, _v.m_protocolHeaderID, _v.m_isSlot);
        });

    CScheduler scheduler;
    BOOST_CHECK_NO_THROW(scheduler.makeSchedule(topology, weakAgents));
    cout << scheduler.toString();
}

BOOST_AUTO_TEST_CASE(test_dds_scheduler_2)
{
    boost::asio::io_context io_context;

    CConnectionManager::channelInfo_t::container_t agents;

    const size_t numSlotsPerAgent{ 11 };
    make_agent(io_context, agents, "host.com", "wn", 100, numSlotsPerAgent);

    using weak_t = CConnectionManager::weakChannelInfo_t;
    weak_t::container_t weakAgents;
    std::transform(
        agents.begin(), agents.end(), std::back_inserter(weakAgents), [](const auto& _v) -> auto {
            return weak_t(_v.m_channel, _v.m_protocolHeaderID, _v.m_isSlot);
        });

    CTopoCore topo;
    topo.init("topology_test_diff_1.xml");

    CTopoCore newTopo;
    newTopo.init("topology_test_diff_2.xml");

    CTopoCore::IdSet_t removedTasks;
    CTopoCore::IdSet_t removedCollections;
    CTopoCore::IdSet_t addedTasks;
    CTopoCore::IdSet_t addedCollections;

    topo.getDifference(newTopo, removedTasks, removedCollections, addedTasks, addedCollections);

    BOOST_CHECK(removedTasks.size() == 9);
    BOOST_CHECK(removedCollections.size() == 4);
    BOOST_CHECK(addedTasks.size() == 11);
    BOOST_CHECK(addedCollections.size() == 5);

    CScheduler scheduler;
    BOOST_CHECK_NO_THROW(scheduler.makeSchedule(newTopo, weakAgents, addedTasks, addedCollections));
    BOOST_CHECK(scheduler.getSchedule().size() == numSlotsPerAgent);
    cout << scheduler.toString();
}

BOOST_AUTO_TEST_CASE(test_dds_scheduler_host_pattern_matches)
{
    BOOST_CHECK(CScheduler::hostPatternMatches(".+.gsi.de", "dds.gsi.de") == true);
    BOOST_CHECK(CScheduler::hostPatternMatches(".+.gsi.de", "gsi.de") == false);
    BOOST_CHECK(CScheduler::hostPatternMatches(".+.gsi.de", "google.com") == false);
}

BOOST_AUTO_TEST_SUITE_END()
