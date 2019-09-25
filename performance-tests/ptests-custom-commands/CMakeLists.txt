# Copyright 2019 GSI, Inc. All rights reserved.
#
#
project(ptests-custom-commands)

##################################################################
# task
##################################################################

set(task task-ptests-custom-commands)

add_executable(${task} task.cpp)

target_link_libraries(${task}
  PUBLIC
	dds_intercom_lib
    Boost::boost
	Boost::program_options
)

install(TARGETS ${task}
  RUNTIME DESTINATION tests/performance-tests/ptests-custom-commands
)

##################################################################
# ui
##################################################################

set(task ui-ptests-custom-commands)

add_executable(${task} ui.cpp)

target_link_libraries(${task}
  PUBLIC
	dds_intercom_lib
    Boost::boost
	Boost::program_options
)

install(TARGETS ${task}
  RUNTIME DESTINATION tests/performance-tests/ptests-custom-commands
)

##################################################################

install(FILES 
  ptests-custom-commands_topo.xml
  
  DESTINATION tests/performance-tests/ptests-custom-commands
)
