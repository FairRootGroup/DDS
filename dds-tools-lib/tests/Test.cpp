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

using namespace std;
using namespace dds;
using namespace dds::tools_api;

BOOST_AUTO_TEST_SUITE(test_dds_tools)

BOOST_AUTO_TEST_CASE(test_dds_tools_session)
{
    CSession session;
    boost::uuids::uuid sid = session.create();
    BOOST_CHECK(!sid.is_nil());
    BOOST_CHECK(session.IsRunning());

    CSession sessionAttach;
    sessionAttach.attach(session.getSessionID());
    BOOST_CHECK(!sessionAttach.getSessionID().is_nil());
    BOOST_CHECK(sessionAttach.IsRunning());

    session.shutdown();
    BOOST_CHECK(session.getSessionID().is_nil());
    BOOST_CHECK(!session.IsRunning());
    BOOST_CHECK(!sessionAttach.IsRunning());
}

BOOST_AUTO_TEST_SUITE_END()
