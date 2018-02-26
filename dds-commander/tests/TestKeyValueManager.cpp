// Copyright 2014 GSI, Inc. All rights reserved.
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
#include "KeyValueManager.h"
#include "Logger.h"
#include "Topology.h"
#include "UpdateKeyCmd.h"
#include "UserDefaults.h"

using namespace std;
using namespace dds;
using namespace dds::topology_api;
using namespace dds::commander_cmd;
using namespace dds::protocol_api;
using namespace MiscCommon;
using namespace user_defaults_api;

BOOST_AUTO_TEST_SUITE(test_dds_key_value_manager)

BOOST_AUTO_TEST_CASE(test_dds_key_value_manager_1)
{
    CUserDefaults::instance(); // Initialize user defaults
    Logger::instance().init(); // Initialize log

    CTopology topology;
    topology.init("key_value_manager_test_1.xml");

    CKeyValueManager kvm;
    kvm.initWithTopology(topology);

    // Test key value update
    int valueCounter = 0;
    CTopology::TaskInfoIteratorPair_t tasks = topology.getTaskInfoIterator();
    for (auto it = tasks.first; it != tasks.second; it++)
    {
        uint64_t id = it->first;
        const STaskInfo& taskInfo = it->second;

        auto props = taskInfo.m_task->getProperties();
        for (auto v : props)
        {
            stringstream ss;
            ss << v->getId() << "." << id;

            SUpdateKeyCmd cmd;
            cmd.m_sKey = ss.str();
            cmd.m_sValue = to_string(valueCounter);

            SUpdateKeyCmd serverCmd;

            kvm.updateKeyValue(cmd, serverCmd);

            valueCounter++;
        }
    }

    // Test key value delete by deleting only the first task
    uint64_t id = tasks.first->first;
    kvm.deleteKeyValue(id);

    boost::test_tools::output_test_stream output1("key_value_manager_test_result_1.txt", true);
    output1 << kvm;
    BOOST_CHECK(output1.match_pattern());
}

BOOST_AUTO_TEST_SUITE_END()
