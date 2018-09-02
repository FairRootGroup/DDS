# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project(dds-submit-ssh-tests)

#=============================================================================

set(test dds-submit-ssh_test_threadpool)

add_executable(${test} test_threadpool.cpp)

target_link_libraries(${test}
  PUBLIC
  MiscCommon
  Boost::system
  Boost::thread
)

target_include_directories(${test}
  PUBLIC
  $<BUILD_INTERFACE:${dds-submit-ssh_SOURCE_DIR}/src>
  $<BUILD_INTERFACE:${dds-submit-ssh_BINARY_DIR}/src>
)

install(TARGETS ${test} DESTINATION tests)
