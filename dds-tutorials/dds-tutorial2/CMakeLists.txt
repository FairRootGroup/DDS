# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project( dds-tutorial2 )


include_directories(
	${Boost_INCLUDE_DIRS}
)

##################################################################
# task-custom-cmd
##################################################################

set( TUTSRV_SOURCE_FILES
	./task-custom-cmd.cpp
)

add_executable(task-custom-cmd ${TUTSRV_SOURCE_FILES})

target_link_libraries (
	task-custom-cmd
    dds-custom-cmd-lib
	${Boost_PROGRAM_OPTIONS_LIBRARY}
)

target_include_directories(task-custom-cmd
   PRIVATE
     $<TARGET_PROPERTY:dds-custom-cmd-lib,INTERFACE_INCLUDE_DIRECTORIES>
)

install(TARGETS task-custom-cmd DESTINATION tutorials/tutorial2)

##################################################################
# ui-custom-cmd
##################################################################

set( TUTSRV_SOURCE_FILES
	./ui-custom-cmd.cpp
)

add_executable(ui-custom-cmd ${TUTSRV_SOURCE_FILES})

target_link_libraries (
	ui-custom-cmd
    dds-custom-cmd-lib
	${Boost_PROGRAM_OPTIONS_LIBRARY}
)

target_include_directories(ui-custom-cmd
   PRIVATE
     $<TARGET_PROPERTY:dds-custom-cmd-lib,INTERFACE_INCLUDE_DIRECTORIES>
)

install(TARGETS ui-custom-cmd DESTINATION tutorials/tutorial2)

install(FILES ${dds-tutorial2_SOURCE_DIR}/tutorial2_topo.xml DESTINATION tutorials/tutorial2)
install(FILES ${dds-tutorial2_SOURCE_DIR}/tutorial2_hosts.cfg DESTINATION tutorials/tutorial2)
