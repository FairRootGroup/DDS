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
// MiscCommon
#include "TimeMeasure.h"

using namespace dds;
using namespace dds::topology_api;
using namespace MiscCommon;

BOOST_AUTO_TEST_SUITE(test_dds_topology_performance)

BOOST_AUTO_TEST_CASE(test_dds_topology_performance_1)
{
    CTopology topology;

    auto execTime = STimeMeasure<>::execution([&topology]()
                                              {
                                                  for (size_t i = 0; i < 3; i++)
                                                  {
                                                      topology.init("topology_test_8.xml");
                                                  }
                                              });

    std::cout << "test_dds_topology_performance_1 execution time: " << execTime << " msec\n";
}

BOOST_AUTO_TEST_SUITE_END()