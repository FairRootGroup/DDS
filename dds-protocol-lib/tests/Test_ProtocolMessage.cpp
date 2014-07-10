// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
// Unit tests
//
// BOOST: tests
// Defines test_main function to link with actual unit test code.
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

// DDS
#include "ProtocolMessage.h"

using boost::unit_test::test_suite;

// STD
#include <string>

// Our
#include "MiscUtils.h"

using namespace MiscCommon;
using namespace std;

BOOST_AUTO_TEST_SUITE(Test_ProtocolMessage);

//----------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(Test_ProtocolMessage_CASE1)
{
}

BOOST_AUTO_TEST_SUITE_END();
