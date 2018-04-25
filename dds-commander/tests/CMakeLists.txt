# Copyright 2014 GSI, Inc. All rights reserved.
#
#
#
project( dds-commander-tests )

include_directories(${dds-commander_SOURCE_DIR}/src ${Boost_INCLUDE_DIRS})
#=============================================================================


add_executable(dds-scheduler-tests
TestScheduler.cpp
${dds-commander_SOURCE_DIR}/src/AgentChannel.cpp
${dds-commander_SOURCE_DIR}/src/ChannelId.cpp
${dds-commander_SOURCE_DIR}/src/SSHScheduler.cpp)

target_link_libraries (
	dds-scheduler-tests
    dds_protocol_lib
    dds_topology_lib
	dds_user_defaults_lib
	${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    ${Boost_LOG_LIBRARY}
    ${Boost_LOG_SETUP_LIBRARY}
    ${Boost_THREAD_LIBRARY}
	${Boost_FILESYSTEM_LIBRARY}
)

install(TARGETS dds-scheduler-tests DESTINATION tests)

target_include_directories(dds-scheduler-tests
	PRIVATE
		$<TARGET_PROPERTY:dds_user_defaults_lib,INTERFACE_INCLUDE_DIRECTORIES>
    INTERFACE
        ${dds_topology_lib_SOURCE_DIR}/src
    INTERFACE
        ${dds_protocol_lib_SOURCE_DIR}/src
)

if( BUILD_TESTS )
  install(FILES ${dds-commander-tests_SOURCE_DIR}/topology_scheduler_test_1.xml DESTINATION tests)
  install(FILES ${dds-commander-tests_SOURCE_DIR}/topology_scheduler_test_2.xml DESTINATION tests)
endif( BUILD_TESTS )
