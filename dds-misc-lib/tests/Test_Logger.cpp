// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// BOOST: tests
// Defines test_main function to link with actual unit test code.
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

// DDS
//#include "Logger.h"

using namespace std;
// using namespace MiscCommon;

BOOST_AUTO_TEST_SUITE(test_dds_topology)

BOOST_AUTO_TEST_CASE(test_dds_topology_1)
{
    /*   CTopology topology;
       topology.init("topology_test_1.xml");

       TopoElementPtr_t e1 = topology.getTopoElementByIndex(CIndex("main/task1"));
       BOOST_CHECK(e1->getPath() == "main/task1");

       TopoElementPtr_t e2 = topology.getTopoElementByIndex(CIndex("main/collection1"));
       BOOST_CHECK(e2->getPath() == "main/collection1");

       TopoElementPtr_t e3 = topology.getTopoElementByIndex(CIndex("main/group1"));
       BOOST_CHECK(e3->getPath() == "main/group1");

       TopoElementPtr_t e4 = topology.getTopoElementByIndex(CIndex("main/group1/collection1"));
       BOOST_CHECK(e4->getPath() == "main/group1/collection1");

       TopoElementPtr_t e5 = topology.getTopoElementByIndex(CIndex("main/group2/collection2/task5"));
       BOOST_CHECK(e5->getPath() == "main/group2/collection2/task5");

       BOOST_CHECK_THROW(topology.getTopoElementByIndex(CIndex("wrong_path")), runtime_error);
       */
}

BOOST_AUTO_TEST_SUITE_END()