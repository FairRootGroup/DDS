# Copyright 2014 GSI, Inc. All rights reserved.
#
#
#
project( dds-topology-lib-tests )

include_directories(${dds-topology-lib_SOURCE_DIR}/src ${Boost_INCLUDE_DIRS})
#=============================================================================


add_executable(dds-topology-lib-tests Test.cpp )
add_executable(dds-topology-lib-performance-tests TestPerformance.cpp )

target_link_libraries (
	dds-topology-lib-tests
    dds-topology-lib
	dds-user-defaults-lib
	${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
	${Boost_FILESYSTEM_LIBRARY}
)

target_link_libraries (
    dds-topology-lib-performance-tests
    dds-topology-lib
    dds-user-defaults-lib
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY}
)

install(TARGETS dds-topology-lib-tests DESTINATION tests)
install(TARGETS dds-topology-lib-performance-tests DESTINATION tests)

target_include_directories(dds-topology-lib-tests
	PRIVATE
		$<TARGET_PROPERTY:dds-user-defaults-lib,INTERFACE_INCLUDE_DIRECTORIES>
)

# Copy schema file and test XML files to the build directory.
#add_custom_target(copy_to_build ALL
#            COMMAND ${CMAKE_COMMAND} -E copy ${TOPOLOGY_SCHEMA_PATH} "${dds-topology-lib-tests_BINARY_DIR}"
#            COMMAND ${CMAKE_COMMAND} -E copy ${dds-topology-lib-tests_SOURCE_DIR}/topology_test_1.xml "${dds-topology-lib-tests_BINARY_DIR}"
#            COMMAND ${CMAKE_COMMAND} -E copy ${dds-topology-lib-tests_SOURCE_DIR}/topology_test_2.xml "${dds-topology-lib-tests_BINARY_DIR}"
#            COMMAND ${CMAKE_COMMAND} -E copy ${dds-topology-lib-tests_SOURCE_DIR}/topology_test_3.xml "${dds-topology-lib-tests_BINARY_DIR}"
#            COMMAND ${CMAKE_COMMAND} -E copy ${dds-topology-lib-tests_SOURCE_DIR}/topology_test_4.xml "${dds-topology-lib-tests_BINARY_DIR}"
#            COMMAND ${CMAKE_COMMAND} -E copy ${dds-topology-lib-tests_SOURCE_DIR}/topology_test_5.xml "${dds-topology-lib-tests_BINARY_DIR}"
#			COMMAND ${CMAKE_COMMAND} -E copy ${dds-topology-lib-tests_SOURCE_DIR}/topology_test_6.xml "${dds-topology-lib-tests_BINARY_DIR}"
#            COMMENT "Copying XML Schema file and test topology XML to build directory."
#            VERBATIM)
            
            
if( BUILD_TESTS )
  install(PROGRAMS ${dds-topology-lib-tests_SOURCE_DIR}/topology_test_1.xml DESTINATION tests)
  install(PROGRAMS ${dds-topology-lib-tests_SOURCE_DIR}/topology_test_2.xml DESTINATION tests)
  install(PROGRAMS ${dds-topology-lib-tests_SOURCE_DIR}/topology_test_3.xml DESTINATION tests)
  install(PROGRAMS ${dds-topology-lib-tests_SOURCE_DIR}/topology_test_4.xml DESTINATION tests)
  install(PROGRAMS ${dds-topology-lib-tests_SOURCE_DIR}/topology_test_5.xml DESTINATION tests)
  install(PROGRAMS ${dds-topology-lib-tests_SOURCE_DIR}/topology_test_6.xml DESTINATION tests)
  install(PROGRAMS ${dds-topology-lib-tests_SOURCE_DIR}/topology_test_7.xml DESTINATION tests)
  install(PROGRAMS ${dds-topology-lib-tests_SOURCE_DIR}/topology_test_8.xml DESTINATION tests)
  install(PROGRAMS ${dds-topology-lib-tests_SOURCE_DIR}/topology_test_1_iterators_1.txt DESTINATION tests)
  install(PROGRAMS ${dds-topology-lib-tests_SOURCE_DIR}/topology_test_1_iterators_2.txt DESTINATION tests)
  install(PROGRAMS ${dds-topology-lib-tests_SOURCE_DIR}/topology_test_1_iterators_3.txt DESTINATION tests)
  install(PROGRAMS ${dds-topology-lib-tests_SOURCE_DIR}/topology_test_1_iterators_4.txt DESTINATION tests)
  install(PROGRAMS ${dds-topology-lib-tests_SOURCE_DIR}/topology_test_1_maps_1.txt DESTINATION tests)
  install(PROGRAMS ${dds-topology-lib-tests_SOURCE_DIR}/topology_test_1_maps_2.txt DESTINATION tests)
  install(PROGRAMS ${dds-topology-lib-tests_SOURCE_DIR}/topology_test_1_maps_3.txt DESTINATION tests)
  install(PROGRAMS ${dds-topology-lib-tests_SOURCE_DIR}/topology_test_1_maps_4.txt DESTINATION tests)
  install(PROGRAMS ${dds-topology-lib-tests_SOURCE_DIR}/topology_test_1_maps_5.txt DESTINATION tests)
  install(PROGRAMS ${dds-topology-lib-tests_SOURCE_DIR}/topology_test_7_maps_1.txt DESTINATION tests)
  install(PROGRAMS ${dds-topology-lib-tests_SOURCE_DIR}/topology_test_7_maps_2.txt DESTINATION tests)
  install(PROGRAMS ${dds-topology-lib-tests_SOURCE_DIR}/topology_test_7_maps_3.txt DESTINATION tests)
  install(PROGRAMS ${dds-topology-lib-tests_SOURCE_DIR}/topology_test_7_maps_4.txt DESTINATION tests)
  install(PROGRAMS ${dds-topology-lib-tests_SOURCE_DIR}/topology_test_7_maps_5.txt DESTINATION tests)
endif( BUILD_TESTS )

