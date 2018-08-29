# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project( dds-tutorial1 )

##################################################################
# task-type-one
##################################################################

set( TUTSRV_SOURCE_FILES
	./task-type-one.cpp
)

add_executable(task-type-one ${TUTSRV_SOURCE_FILES})

target_link_libraries (
	task-type-one
	dds_intercom_lib
	${Boost_PROGRAM_OPTIONS_LIBRARY}
)

target_include_directories(task-type-one
    PRIVATE
        $<TARGET_PROPERTY:dds_intercom_lib,INTERFACE_INCLUDE_DIRECTORIES>
    PUBLIC
        ${Boost_INCLUDE_DIRS}
)

install(TARGETS task-type-one DESTINATION tutorials/tutorial1)

##################################################################
# tutorial1-client
##################################################################

set( TUTCLN_SOURCE_FILES
	./task-type-two.cpp
)

add_executable(task-type-two ${TUTCLN_SOURCE_FILES})

target_link_libraries (
	task-type-two
	dds_intercom_lib
	${Boost_PROGRAM_OPTIONS_LIBRARY}
)

target_include_directories(task-type-two
    PRIVATE
        $<TARGET_PROPERTY:dds_intercom_lib,INTERFACE_INCLUDE_DIRECTORIES>
    PUBLIC
        ${Boost_INCLUDE_DIRS}
)

install(TARGETS task-type-two DESTINATION tutorials/tutorial1)


install(FILES ${dds-tutorial1_SOURCE_DIR}/tutorial1_topo.xml DESTINATION tutorials/tutorial1)
install(FILES ${dds-tutorial1_SOURCE_DIR}/tutorial1_hosts.cfg DESTINATION tutorials/tutorial1)
