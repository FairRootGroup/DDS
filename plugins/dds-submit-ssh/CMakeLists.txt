# Copyright 2016 GSI, Inc. All rights reserved.
#
#
project( dds-submit-ssh )

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
	./src/worker.cpp
)
set( SRC_HDRS
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
    add_executable(dds-submit-ssh ${SOURCE_FILES} ${SRC_HDRS})
else (CMAKE_GENERATOR STREQUAL "Xcode")
    add_executable(dds-submit-ssh ${SOURCE_FILES})
endif (CMAKE_GENERATOR STREQUAL "Xcode")

target_link_libraries (
	dds-submit-ssh
	dds_user_defaults_lib
    dds_intercom_lib
	dds_sys_files
	dds_ncf
	pipe_log_engine
	${Boost_PROGRAM_OPTIONS_LIBRARY}
	${Boost_LOG_LIBRARY}
	${Boost_LOG_SETUP_LIBRARY}
	${Boost_THREAD_LIBRARY}
)

target_include_directories(dds-submit-ssh
	PRIVATE
		$<TARGET_PROPERTY:dds_user_defaults_lib,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE
        $<TARGET_PROPERTY:dds_intercom_lib,INTERFACE_INCLUDE_DIRECTORIES>
)

install(TARGETS dds-submit-ssh DESTINATION plugins/dds-submit-ssh)
install(PROGRAMS ${dds-submit-ssh_SOURCE_DIR}/src/dds-submit-ssh-worker DESTINATION plugins/dds-submit-ssh)
#
# Unit tests
#
if (BUILD_TESTS)
    message (STATUS "Build dds-submit-ssh unit tests - YES")
    add_subdirectory ( ${dds-submit-ssh_SOURCE_DIR}/tests ) 
else (BUILD_TESTS)
    message (STATUS "Build dds-submit-ssh unit tests - NO")
endif (BUILD_TESTS)

