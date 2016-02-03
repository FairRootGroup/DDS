# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project( dds-test )

#
# Where to lookup modules
#
set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

#
# configure files
# 
configure_file( ${dds-test_SOURCE_DIR}/src/version.h.in ${dds-test_BINARY_DIR}/version.h @ONLY )

#
# Source files
#
set( SOURCE_FILES
	./src/main.cpp
	./src/TestChannel.cpp
)

#
# Header files
#
set( HEADER_FILES
	./src/Options.h
	./src/TestChannel.h
)

include_directories(
	${dds-test_BINARY_DIR}
	${MiscCommon_LOCATION}
	${Boost_INCLUDE_DIRS} 
)

#
# dds-test executable
#
if (CMAKE_GENERATOR STREQUAL "Xcode")
    add_executable(dds-test ${SOURCE_FILES} ${HEADER_FILES})
else (CMAKE_GENERATOR STREQUAL "Xcode")
    add_executable(dds-test ${SOURCE_FILES})
endif (CMAKE_GENERATOR STREQUAL "Xcode")

target_link_libraries (
	dds-test
	dds_user_defaults_lib
	dds_topology_lib
	dds_protocol_lib
	${Boost_PROGRAM_OPTIONS_LIBRARY}
	${Boost_SYSTEM_LIBRARY}
	${Boost_LOG_LIBRARY}
	${Boost_LOG_SETUP_LIBRARY}
	${Boost_THREAD_LIBRARY}
	${Boost_FILESYSTEM_LIBRARY}
)

target_include_directories(dds-test
	PRIVATE
		$<TARGET_PROPERTY:dds_user_defaults_lib,INTERFACE_INCLUDE_DIRECTORIES>
	PRIVATE
		$<TARGET_PROPERTY:dds_topology_lib,INTERFACE_INCLUDE_DIRECTORIES>
	PRIVATE
		$<TARGET_PROPERTY:dds_protocol_lib,INTERFACE_INCLUDE_DIRECTORIES>
)

install(TARGETS dds-test DESTINATION bin)

#
# Unit tests
#
#if (BUILD_TESTS)
#    message (STATUS "Build dds-test unit tests - YES")
#    add_subdirectory ( ${dds-test_SOURCE_DIR}/tests ) 
#else (BUILD_TESTS)
#    message (STATUS "Build dds-test unit tests - NO")
#endif (BUILD_TESTS)
