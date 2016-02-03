# Copyright 2014 GSI, Inc. All rights reserved.
#dds-agent-cmd
#
project( dds-agent-cmd )

#
# Where to lookup modules
#
set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

#
# configure files
# 
configure_file( ${dds-agent-cmd_SOURCE_DIR}/src/version.h.in ${dds-agent-cmd_BINARY_DIR}/version.h @ONLY )

#
# Source files
#
set( SOURCE_FILES
	./src/main.cpp
	./src/GenericChannel.cpp
)

#
# Header files
#
set( HEADER_FILES
	./src/Options.h
	./src/GenericChannel.h
)

include_directories(
	${dds-agent-cmd_BINARY_DIR}
	${MiscCommon_LOCATION}
	${Boost_INCLUDE_DIRS} 
)

#
# dds-agent-cmd executable
#
if (CMAKE_GENERATOR STREQUAL "Xcode")
    add_executable(dds-agent-cmd ${SOURCE_FILES} ${HEADER_FILES})
else (CMAKE_GENERATOR STREQUAL "Xcode")
    add_executable(dds-agent-cmd ${SOURCE_FILES})
endif (CMAKE_GENERATOR STREQUAL "Xcode")

#message(STATUS "LOG_LIBRARY=${Boost_LOG_LIBRARY}")
#message(STATUS "FILESYSTEM_LIBRARY=${Boost_FILESYSTEM_LIBRARY}")

target_link_libraries (
	dds-agent-cmd
	dds_user_defaults_lib
	dds_protocol_lib
	${Boost_PROGRAM_OPTIONS_LIBRARY}
	${Boost_SYSTEM_LIBRARY}
	${Boost_LOG_LIBRARY}
	${Boost_LOG_SETUP_LIBRARY}
	${Boost_THREAD_LIBRARY}
)

target_include_directories(dds-agent-cmd
	PRIVATE
		$<TARGET_PROPERTY:dds_user_defaults_lib,INTERFACE_INCLUDE_DIRECTORIES>
	PRIVATE
		$<TARGET_PROPERTY:dds_protocol_lib,INTERFACE_INCLUDE_DIRECTORIES>
)

install(TARGETS dds-agent-cmd DESTINATION bin)

#
# Unit tests
#
#if (BUILD_TESTS)
#    message (STATUS "Build dds-agent-cmd unit tests - YES")
#    add_subdirectory ( ${dds-agent-cmd_SOURCE_DIR}/tests ) 
#else (BUILD_TESTS)
#    message (STATUS "Build dds-agent-cmd unit tests - NO")
#endif (BUILD_TESTS)
