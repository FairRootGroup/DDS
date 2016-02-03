# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project( pipe_log_engine )

#
# Where to lookup modules
#
set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

#
# Source files
#
set( SOURCE_FILES
     logEngine.cpp
)

set( SRC_HDRS
     logEngine.h
)

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${MiscCommon_LOCATION}
    ${Boost_INCLUDE_DIRS}
)

set(Boost_libs_for_pipe_log ${Boost_THREAD_LIBRARY} ${Boost_SYSTEM_LIBRARY})

#
# lib
#
if (CMAKE_GENERATOR STREQUAL "Xcode")
    add_library(pipe_log_engine SHARED ${SOURCE_FILES} ${SRC_HDRS})
else (CMAKE_GENERATOR STREQUAL "Xcode")
    add_library(pipe_log_engine SHARED ${SOURCE_FILES})
endif (CMAKE_GENERATOR STREQUAL "Xcode")

target_link_libraries (
	pipe_log_engine
	dds_user_defaults_lib
	${Boost_LOG_LIBRARY}
	${Boost_LOG_SETUP_LIBRARY}
	${Boost_libs_for_pipe_log} 
)

target_include_directories(pipe_log_engine
	PRIVATE
		$<TARGET_PROPERTY:dds_user_defaults_lib,INTERFACE_INCLUDE_DIRECTORIES>
)


install(TARGETS pipe_log_engine DESTINATION lib)

