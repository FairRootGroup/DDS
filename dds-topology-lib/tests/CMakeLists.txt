# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project(dds_topology_lib-tests)

#=============================================================================

add_executable(dds_topology_lib-tests Test.cpp)
add_executable(dds_topology_lib-performance-tests TestPerformance.cpp)

target_link_libraries(dds_topology_lib-tests
  PUBLIC
  dds_topology_lib
	dds_user_defaults_lib
	Boost::unit_test_framework
	Boost::filesystem
)

target_link_libraries(dds_topology_lib-performance-tests
  PUBLIC
  dds_topology_lib
	dds_user_defaults_lib
	Boost::unit_test_framework
	Boost::filesystem
)

install(TARGETS
  dds_topology_lib-tests
  dds_topology_lib-performance-tests

  RUNTIME DESTINATION tests
)

#=============================================================================

add_executable(task-test_index task-test_index.cpp)

target_link_libraries(task-test_index
  dds_intercom_lib
)

target_include_directories(task-test_index
PRIVATE
     $<TARGET_PROPERTY:dds_intercom_lib,INTERFACE_INCLUDE_DIRECTORIES>
)

install(TARGETS task-test_index
  RUNTIME DESTINATION tests
)

#=============================================================================

install(FILES 
  topology_test_1.xml
  topology_test_2.xml
  topology_test_3.xml
  topology_test_4.xml
  topology_test_5.xml
  topology_test_6.xml
  topology_test_7.xml
  topology_test_8.xml
  topology_test_diff_1.xml
  topology_test_diff_2.xml
  topology_test_1_iterators_1.txt
  topology_test_1_iterators_2.txt
  topology_test_1_iterators_3.txt
  topology_test_1_iterators_4.txt
  topology_test_1_iterators_5.txt
  topology_test_1_maps_2.txt
  topology_test_1_maps_3.txt
  topology_test_7_maps_2.txt
  topology_test_7_maps_3.txt
  topology_test_diff.txt
  topology_test_property_1.xml
  topology_test_property_2.xml
  topology_test_creator_1.xml

  DESTINATION tests
)
