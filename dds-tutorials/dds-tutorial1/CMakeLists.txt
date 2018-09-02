# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project(dds-tutorial1)

##################################################################
# task-type-one
##################################################################

set(task task-type-one)

add_executable(${task} task-type-one.cpp)

target_link_libraries(${task}
  PUBLIC
	dds_intercom_lib
  Boost::boost
	Boost::program_options
)

install(TARGETS ${task}
  RUNTIME DESTINATION tutorials/tutorial1
)

##################################################################
# task-type-two
##################################################################

set(task task-type-two)

add_executable(${task} task-type-two.cpp)

target_link_libraries(${task}
  PUBLIC
	dds_intercom_lib
  Boost::boost
	Boost::program_options
)

install(TARGETS ${task}
  RUNTIME DESTINATION tutorials/tutorial1
)

##################################################################

install(FILES 
  tutorial1_topo.xml
  tutorial1_hosts.cfg

  DESTINATION tutorials/tutorial1
)
