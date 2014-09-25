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

// BOOST
#include <boost/test/unit_test.hpp>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-register"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#pragma clang diagnostic pop

using boost::unit_test::test_suite;
using namespace std;

BOOST_AUTO_TEST_SUITE(Test_KeyValue);

//----------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(Test_KeyValue_T1)
{
}

BOOST_AUTO_TEST_SUITE_END();
