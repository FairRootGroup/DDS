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

install(TARGETS ${test} DESTINATION "${PROJECT_INSTALL_TESTS}")

##################################################################
# task-test_key_value
##################################################################

add_executable(task-test_key_value task-test_key_value.cpp)
target_link_libraries(task-test_key_value
  PUBLIC
  dds_intercom_lib
  dds_topology_lib
  Boost::boost
)
install(TARGETS task-test_key_value DESTINATION "${PROJECT_INSTALL_TESTS}")

##################################################################
# task-test_custom_cmd
##################################################################

add_executable(task-test_custom_cmd task-test_custom_cmd.cpp)
target_link_libraries(task-test_custom_cmd
  PUBLIC
  dds_intercom_lib
  dds_topology_lib
  Boost::boost
)
install(TARGETS task-test_custom_cmd DESTINATION "${PROJECT_INSTALL_TESTS}")
