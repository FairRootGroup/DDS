// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// BOOST: tests
// Defines test_main function to link with actual unit test code.
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>
// DDS
#include "Topology.h"
#include "SSHScheduler.h"
#include "AgentChannel.h"
#include "HostInfoCmd.h"
// MiscCommon
#include "TimeMeasure.h"
// BOOST
#include "boost/asio.hpp"
// STD
#include <sstream>

using namespace std;
using namespace dds;
using namespace MiscCommon;

BOOST_AUTO_TEST_SUITE(test_dds_scheduler_performance)

BOOST_AUTO_TEST_CASE(test_dds_scheduler_performance_1)
{
    Logger::instance().init(); // Initialize log
    CUserDefaults::instance(); // Initialize user defaults

    boost::asio::io_service io_service;

    CTopology topology;
    topology.init("topology_scheduler_test_1.xml");

    CAgentChannel::connectionPtrVector_t agents;
    size_t nofTasks = 3;
    size_t nofAgentsPerTask = 1000;

    agents.reserve(nofTasks * nofAgentsPerTask);

    for (size_t i = 0; i < nofTasks; ++i)
        for (size_t j = 0; j < nofAgentsPerTask; ++j)
        {
            CAgentChannel::connectionPtr_t agent = CAgentChannel::makeNew(io_service);

            stringstream ss;
            ss << "host" << (i + 1) << "_" << j % 100;

            SHostInfoCmd hostInfo;
            hostInfo.m_host = ss.str();

            agent->setRemoteHostInfo(hostInfo);

            agents.push_back(agent);
        }

    CAgentChannel::weakConnectionPtrVector_t weakAgents;
    weakAgents.reserve(agents.size());
    for (auto& v : agents)
    {
        weakAgents.push_back(v);
    }

    CSSHScheduler scheduler;

    auto execTime = STimeMeasure<std::chrono::microseconds>::execution([&scheduler, &topology, &weakAgents]()
                                                                       {
                                                                           scheduler.makeSchedule(topology, weakAgents);
                                                                       });
    double execTimeSeconds = execTime * 1e-6;

    BOOST_CHECK(execTimeSeconds < 3.0);

    std::cout << "SSH scheduler execution time: " << execTimeSeconds << " s\n";

    // Test failure
    // Change host info to non existing pattern
    for (auto agent : agents)
    {
        SHostInfoCmd hostInfo;
        hostInfo.m_host = "nohost";
        agent->setRemoteHostInfo(hostInfo);
    }
    auto execFailTime = STimeMeasure<std::chrono::microseconds>::execution(
        [&scheduler, &topology, &weakAgents]()
        {
            BOOST_CHECK_THROW(scheduler.makeSchedule(topology, weakAgents), runtime_error);
        });
    double execFailTimeSeconds = execFailTime * 1e-6;

    BOOST_CHECK(execFailTimeSeconds < 0.1);
    std::cout << "SSH scheduler fail execution time: " << execFailTimeSeconds << " s\n";
}

BOOST_AUTO_TEST_CASE(test_dds_scheduler_1)
{
    Logger::instance().init(); // Initialize log
    CUserDefaults::instance(); // Initialize user defaults

    boost::asio::io_service io_service;

    CTopology topology;
    topology.init("topology_scheduler_test_2.xml");

    CAgentChannel::connectionPtrVector_t agents;
    size_t n = 3;
    size_t nofAgentsPerGroup = 7;

    agents.reserve(n * nofAgentsPerGroup);

    for (size_t i = 0; i < n; ++i)
        for (size_t j = 0; j < nofAgentsPerGroup; ++j)
        {
            CAgentChannel::connectionPtr_t agent = CAgentChannel::makeNew(io_service);

            stringstream ss;
            if (j != 6)
            {
                size_t h = (j < 4) ? j : 3;
                ss << "host" << (h + 1) << "_" << i;
            }
            else
            {
                ss << "noname_host";
            }

            SHostInfoCmd hostInfo;
            hostInfo.m_host = ss.str();

            agent->setRemoteHostInfo(hostInfo);

            agents.push_back(agent);

            std::cout << ss.str() << std::endl;
        }

    CAgentChannel::weakConnectionPtrVector_t weakAgents;
    weakAgents.reserve(agents.size());
    for (auto& v : agents)
    {
        weakAgents.push_back(v);
    }

    CSSHScheduler scheduler;
    scheduler.makeSchedule(topology, weakAgents);
    scheduler.printSchedule();
}

BOOST_AUTO_TEST_SUITE_END()
