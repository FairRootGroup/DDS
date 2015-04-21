# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project( dds-topology )

#
# Where to lookup modules
#
set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

#
# configure files
# 
configure_file( ${dds-topology_SOURCE_DIR}/src/version.h.in ${dds-topology_BINARY_DIR}/version.h @ONLY )

#
# Source files
#
set( SOURCE_FILES
	./src/main.cpp
	./src/ActivateChannel.cpp
)

#
# Header files
#
set( HEADER_FILES
	./src/Options.h
	./src/ActivateChannel.h
)

include_directories(
	${dds-topology_BINARY_DIR}
	${MiscCommon_LOCATION}
	${Boost_INCLUDE_DIRS} 
)

#
# dds-topology executable
#
if (CMAKE_GENERATOR STREQUAL "Xcode")
    add_executable(dds-topology ${SOURCE_FILES} ${HEADER_FILES})
else (CMAKE_GENERATOR STREQUAL "Xcode")
    add_executable(dds-topology ${SOURCE_FILES})
endif (CMAKE_GENERATOR STREQUAL "Xcode")

target_link_libraries (
	dds-topology
	dds-user-defaults-lib
	dds-protocol-lib
    dds-topology-lib
	${Boost_PROGRAM_OPTIONS_LIBRARY}
	${Boost_SYSTEM_LIBRARY}
	${Boost_LOG_LIBRARY}
	${Boost_LOG_SETUP_LIBRARY}
	${Boost_THREAD_LIBRARY}
	${Boost_FILESYSTEM_LIBRARY}
)

target_include_directories(dds-topology
	PRIVATE
		$<TARGET_PROPERTY:dds-user-defaults-lib,INTERFACE_INCLUDE_DIRECTORIES>
	PRIVATE
		$<TARGET_PROPERTY:dds-protocol-lib,INTERFACE_INCLUDE_DIRECTORIES>
)

install(TARGETS dds-topology DESTINATION bin)

#
# Unit tests
#
#if (BUILD_TESTS)
#    message (STATUS "Build dds-topology unit tests - YES")
#    add_subdirectory ( ${dds-topology_SOURCE_DIR}/tests ) 
#else (BUILD_TESTS)
#    message (STATUS "Build dds-topology unit tests - NO")
#endif (BUILD_TESTS)
