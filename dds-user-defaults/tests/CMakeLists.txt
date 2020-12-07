# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project(dds-user-defaults-tests)

#=============================================================================
set(test dds-user-defaults-tests)

add_executable(${test} test.cpp)

target_link_libraries(${test}
  PUBLIC
  dds_user_defaults_lib
  Boost::boost
  Boost::system
  Boost::unit_test_framework
)

install(TARGETS ${test} DESTINATION "${PROJECT_INSTALL_TESTS}")
