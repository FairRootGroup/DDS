# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project( dds-custom-cmd-lib )

#
# Where to lookup modules
#
set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

#
# configure files
# 
configure_file( ${dds-custom-cmd-lib_SOURCE_DIR}/src/version.h.in ${dds-custom-cmd-lib_BINARY_DIR}/version.h @ONLY )

#
# Source files
#
set( SOURCE_FILES
	./src/CustomCmd.cpp
	./src/CustomCmdGuard.cpp
	./src/AgentConnectionManager.cpp
	./src/AgentChannel.cpp
)

set( SRC_HDRS_INT
	./src/CustomCmdGuard.h
	./src/AgentConnectionManager.h
	./src/AgentChannel.h
)

set( SRC_HDRS
	./src/CustomCmd.h
)

include_directories(
	${dds-custom-cmd-lib_BINARY_DIR}
	${PROJECT_SOURCE_DIR}
	${MiscCommon_LOCATION}
	${Boost_INCLUDE_DIRS}
)

#
# lib
#
if (CMAKE_GENERATOR STREQUAL "Xcode")
	add_library(dds-custom-cmd-lib SHARED ${SOURCE_FILES} ${SRC_HDRS_INT} ${SRC_HDRS})
else (CMAKE_GENERATOR STREQUAL "Xcode")
	add_library(dds-custom-cmd-lib SHARED ${SOURCE_FILES})
endif (CMAKE_GENERATOR STREQUAL "Xcode")

target_link_libraries (
	dds-custom-cmd-lib
	dds-user-defaults-lib
	dds-protocol-lib
	${Boost_SYSTEM_LIBRARY}
	${Boost_FILESYSTEM_LIBRARY}
	${Boost_LOG_LIBRARY}
	${Boost_LOG_SETUP_LIBRARY}
	${Boost_THREAD_LIBRARY}
	${Boost_SIGNALS_LIBRARY}
)

target_include_directories(dds-custom-cmd-lib
	PRIVATE
		$<TARGET_PROPERTY:dds-user-defaults-lib,INTERFACE_INCLUDE_DIRECTORIES>
)

# API headers
target_include_directories(dds-custom-cmd-lib INTERFACE ${dds-custom-cmd-lib_SOURCE_DIR}/src)

install(TARGETS dds-custom-cmd-lib DESTINATION lib)
install(FILES ${SRC_HDRS} DESTINATION include)

#
# Unit tests
#
#if (BUILD_TESTS)
#	message (STATUS "Build dds-custom-cmd-lib tests - YES")
#	add_subdirectory ( ${dds-custom-cmd-lib_SOURCE_DIR}/tests ) 
#else (BUILD_TESTS)
#	message (STATUS "Build dds-custom-cmd-lib tests - NO")
#endif (BUILD_TESTS)

