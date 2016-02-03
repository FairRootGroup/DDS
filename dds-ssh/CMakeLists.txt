# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project( dds-ssh )

#
# Where to lookup modules
#
set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

#
# configure files
# 
configure_file( ${PROJECT_SOURCE_DIR}/src/version.h.in ${PROJECT_BINARY_DIR}/src/version.h @ONLY )

#
# Source files
#
set( SOURCE_FILES
	./src/main.cpp
	./src/config.cpp
	./src/worker.cpp
)
set( SRC_HDRS
	./src/config.h
	./src/local_types.h
	./src/threadPool.h
	./src/worker.h
)


include_directories(
	${PROJECT_BINARY_DIR}/src
	${MiscCommon_LOCATION}
	${MiscCommon_LOCATION}/pipe_log_engine
	${Boost_INCLUDE_DIRS} )

#
# dds-ssh executable
#
if (CMAKE_GENERATOR STREQUAL "Xcode")
    add_executable(dds-ssh ${SOURCE_FILES} ${SRC_HDRS})
else (CMAKE_GENERATOR STREQUAL "Xcode")
    add_executable(dds-ssh ${SOURCE_FILES})
endif (CMAKE_GENERATOR STREQUAL "Xcode")

target_link_libraries (
	dds-ssh
	dds_user_defaults_lib
	dds_sys_files
	pipe_log_engine
	${Boost_PROGRAM_OPTIONS_LIBRARY}
	${Boost_LOG_LIBRARY}
	${Boost_LOG_SETUP_LIBRARY}
	${Boost_THREAD_LIBRARY}
)

target_include_directories(dds-ssh
	PRIVATE
		$<TARGET_PROPERTY:dds_user_defaults_lib,INTERFACE_INCLUDE_DIRECTORIES>
)


install(TARGETS dds-ssh DESTINATION bin)

#
# Unit tests
#
if (BUILD_TESTS)
    message (STATUS "Build dds-ssh unit tests - YES")
    add_subdirectory ( ${dds-ssh_SOURCE_DIR}/tests ) 
else (BUILD_TESTS)
    message (STATUS "Build dds-ssh unit tests - NO")
endif (BUILD_TESTS)

