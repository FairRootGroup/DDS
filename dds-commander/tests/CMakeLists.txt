# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project(dds-commander-tests)

#=============================================================================

set(test dds-scheduler-tests)

add_executable(${test}
  TestScheduler.cpp
  ${dds-commander_SOURCE_DIR}/src/AgentChannel.cpp
  ${dds-commander_SOURCE_DIR}/src/ChannelId.cpp
  ${dds-commander_SOURCE_DIR}/src/Scheduler.cpp
)

target_link_libraries(${test}
  PUBLIC
  dds_protocol_lib
  dds_topology_lib
  dds_user_defaults_lib
  dds_tools_lib
  Boost::boost
  Boost::unit_test_framework
  Boost::log
  Boost::log_setup
  Boost::thread
  Boost::filesystem
)

target_include_directories(${test}
  PUBLIC
  $<BUILD_INTERFACE:${dds-commander_SOURCE_DIR}/src>
)

install(TARGETS ${test}
  RUNTIME DESTINATION tests
)

if(BUILD_TESTS)
  install(FILES
    topology_scheduler_test_1.xml
    topology_scheduler_test_2.xml
    
    DESTINATION tests
  )
endif()
