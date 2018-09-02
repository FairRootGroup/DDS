# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project(dds-tutorial2)

##################################################################
# task-custom-cmd
##################################################################

set(task task-custom-cmd)

add_executable(${task} task-custom-cmd.cpp)

target_link_libraries(${task}
  PUBLIC
	dds_intercom_lib
  Boost::boost
	Boost::program_options
)

install(TARGETS ${task}
  RUNTIME DESTINATION tutorials/tutorial2
)

##################################################################
# ui-custom-cmd
##################################################################

set(task ui-custom-cmd)

add_executable(${task} ui-custom-cmd.cpp)

target_link_libraries(${task}
  PUBLIC
	dds_intercom_lib
  Boost::boost
	Boost::program_options
)

install(TARGETS ${task}
  RUNTIME DESTINATION tutorials/tutorial2
)

##################################################################

install(FILES 
  tutorial2_topo.xml
  tutorial2_hosts.cfg

  DESTINATION tutorials/tutorial2
)
