# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project(dds_protocol_lib-tests)

##################################################################
# ProtocolMessage-tests
##################################################################

set(test dds_protocol_lib-ProtocolMessage-tests)

set(CLNT_SOURCE_FILES
  Test_ProtocolMessage.cpp
  TestCmd.cpp
)

set(CLNT_HEADER_FILES
	TestCmd.h
)

add_executable(${test} ${CLNT_SOURCE_FILES} ${CLNT_HEADER_FILES})

target_link_libraries(${test}
  PUBLIC
	dds_protocol_lib
  Boost::boost
  Boost::system
  Boost::unit_test_framework
)

install(TARGETS ${test} DESTINATION tests)
