# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project( dds-key-value-lib )

#
# Where to lookup modules
#
set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

#
# configure files
# 
configure_file( ${dds-key-value-lib_SOURCE_DIR}/src/version.h.in ${dds-key-value-lib_BINARY_DIR}/version.h @ONLY )

#
# Source files
#
set( SOURCE_FILES
	./src/KeyValue.cpp
	./src/KeyValueGuard.cpp
	./src/AgentConnectionManager.cpp
	./src/AgentChannel.cpp
)

set( SRC_HDRS_INT
	./src/KeyValueGuard.h
	./src/AgentConnectionManager.h
	./src/AgentChannel.h
)

set( SRC_HDRS
	./src/KeyValue.h
)

include_directories(
	${dds-key-value-lib_BINARY_DIR}
	${PROJECT_SOURCE_DIR}
	${MiscCommon_LOCATION}
	${Boost_INCLUDE_DIRS}
)

#
# lib
#
if (CMAKE_GENERATOR STREQUAL "Xcode")
	add_library(dds-key-value-lib SHARED ${SOURCE_FILES} ${SRC_HDRS_INT} ${SRC_HDRS})
else (CMAKE_GENERATOR STREQUAL "Xcode")
	add_library(dds-key-value-lib SHARED ${SOURCE_FILES})
endif (CMAKE_GENERATOR STREQUAL "Xcode")

target_link_libraries (
	dds-key-value-lib
	dds-user-defaults-lib
	dds-protocol-lib
	${Boost_SYSTEM_LIBRARY}
	${Boost_FILESYSTEM_LIBRARY}
	${Boost_LOG_LIBRARY}
	${Boost_LOG_SETUP_LIBRARY}
	${Boost_THREAD_LIBRARY}
)

target_include_directories(dds-key-value-lib
	PRIVATE
		$<TARGET_PROPERTY:dds-user-defaults-lib,INTERFACE_INCLUDE_DIRECTORIES>
)

# API headers
target_include_directories(dds-key-value-lib INTERFACE ${dds-key-value-lib_SOURCE_DIR}/src)

install(TARGETS dds-key-value-lib DESTINATION lib)
install(FILES ${SRC_HDRS} DESTINATION include)

#
# Unit tests
#
if (BUILD_TESTS)
	message (STATUS "Build dds-key-value-lib tests - YES")
	add_subdirectory ( ${dds-key-value-lib_SOURCE_DIR}/tests ) 
else (BUILD_TESTS)
	message (STATUS "Build dds-key-value-lib tests - NO")
endif (BUILD_TESTS)

