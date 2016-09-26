# Copyright 2016 GSI, Inc. All rights reserved.
#
#
project( dds-octopus-task )

#
# Where to lookup modules
#
set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

#
# configure files
# 
configure_file( ${dds-octopus-task_SOURCE_DIR}/src/version.h.in ${dds-octopus-task_BINARY_DIR}/version.h @ONLY )

#
# Source files
#
set( SOURCE_FILES
	./src/main.cpp
	./src/Task.cpp
	./../src/OctopusProtocol.cpp
)

#
# Header files
#
set( HEADER_FILES
	./src/Task.h
	./../src/OctopusProtocol.h
)

include_directories(
	${dds-octopus-task_BINARY_DIR}
	${dds-octopus-task_SOURCE_DIR}/../src/
	${MiscCommon_LOCATION}
	${Boost_INCLUDE_DIRS} 
)

#
# dds-octopus executable
#
if (CMAKE_GENERATOR STREQUAL "Xcode")
    add_executable(dds-octopus-task ${SOURCE_FILES} ${HEADER_FILES})
else (CMAKE_GENERATOR STREQUAL "Xcode")
    add_executable(dds-octopus-task ${SOURCE_FILES})
endif (CMAKE_GENERATOR STREQUAL "Xcode")

target_link_libraries (
	dds-octopus-task
	dds_intercom_lib
	${Boost_PROGRAM_OPTIONS_LIBRARY}
)

target_include_directories(dds-octopus-task
    PRIVATE
        $<TARGET_PROPERTY:dds_intercom_lib,INTERFACE_INCLUDE_DIRECTORIES>
)

install(TARGETS dds-octopus-task DESTINATION tests/dds-octopus)
