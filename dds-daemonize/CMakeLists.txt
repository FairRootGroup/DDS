# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project( dds-daemonize)


set(USER_DEFAULTS_CFG_VERSION "0.1")

#
# Where to lookup modules
#
set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

#
# configure files
# 
configure_file( ${dds-daemonize_SOURCE_DIR}/src/version.h.in ${dds-daemonize_BINARY_DIR}/version.h @ONLY )

#
# Source files
#
set( SOURCE_FILES
    ./src/main.cpp
)

#
# Header files
#
set( HEADER_FILES
)

include_directories(
	${dds-daemonize_BINARY_DIR}
	${MiscCommon_LOCATION}
	${Boost_INCLUDE_DIRS}
)

#
# dds-daemonize executable
#
if (CMAKE_GENERATOR STREQUAL "Xcode")
    add_executable(dds-daemonize ${SOURCE_FILES} ${HEADER_FILES})
else (CMAKE_GENERATOR STREQUAL "Xcode")
    add_executable(dds-daemonize ${SOURCE_FILES})
endif (CMAKE_GENERATOR STREQUAL "Xcode")


target_link_libraries (
	dds-daemonize
	${Boost_PROGRAM_OPTIONS_LIBRARY}
	${Boost_SYSTEM_LIBRARY}
)

install(TARGETS dds-daemonize DESTINATION bin)

#
# Unit tests
#
if (BUILD_TESTS)
    message (STATUS "Build dds-daemonize unit tests - YES")
    add_subdirectory ( ${dds-daemonize_SOURCE_DIR}/tests ) 
else (BUILD_TESTS)
    message (STATUS "Build dds-daemonize unit tests - NO")
endif (BUILD_TESTS)

