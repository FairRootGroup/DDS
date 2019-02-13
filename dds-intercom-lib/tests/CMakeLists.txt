# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project(dds_intercom_lib-tests)

##################################################################
# Unit Test
##################################################################

set(test ${PROJECT_NAME})

add_executable(${test} Test.cpp)

target_link_libraries(${test}
  PUBLIC
  dds_intercom_lib
  Boost::boost
  Boost::system
  Boost::unit_test_framework
  Boost::program_options
)

install(TARGETS ${test} DESTINATION tests)

##################################################################
# task-test_key_value
##################################################################

set(test task-test_key_value)

add_executable(${test} task-test_key_value.cpp)

target_link_libraries(${test}
  PUBLIC
  dds_intercom_lib
  Boost::boost
)

install(TARGETS ${test} DESTINATION tests)

##################################################################

install(FILES
  topo_test_key-value-1.xml
  topo_test_key-value-2.xml

  DESTINATION tests
)
