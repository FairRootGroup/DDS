# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project( dds-commander )

#
# Where to lookup modules
#
set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

#
# configure files
#
configure_file( ${dds-commander_SOURCE_DIR}/src/version.h.in ${dds-commander_BINARY_DIR}/version.h @ONLY )

#
# Source files
#
set( SOURCE_FILES
	./src/main.cpp
	./src/ConnectionManager.cpp
	./src/AgentChannel.cpp
    ./src/UIChannelInfo.cpp
    ./src/SSHScheduler.cpp
    ./src/ChannelId.cpp
		./src/KeyValueManager.cpp
)

#
# Header files
#
set( HEADER_FILES
	./src/Options.h
	./src/ConnectionManager.h
	./src/AgentChannel.h
    ./src/UIChannelInfo.h
    ./src/SSHScheduler.h
    ./src/ChannelId.h
		./src/KeyValueManager.h
)

include_directories(
	${dds-commander_BINARY_DIR}
	${MiscCommon_LOCATION}
	${Boost_INCLUDE_DIRS}
)

#
# dds-commander executable
#
if (CMAKE_GENERATOR STREQUAL "Xcode")
    add_executable(dds-commander ${SOURCE_FILES} ${HEADER_FILES})
else (CMAKE_GENERATOR STREQUAL "Xcode")
    add_executable(dds-commander ${SOURCE_FILES})
endif (CMAKE_GENERATOR STREQUAL "Xcode")

target_link_libraries (
	dds-commander
	dds_user_defaults_lib
	dds_topology_lib
	dds_protocol_lib
    dds_ncf
    dds_intercom_lib
	${Boost_PROGRAM_OPTIONS_LIBRARY}
	${Boost_SYSTEM_LIBRARY}
	${Boost_LOG_LIBRARY}
	${Boost_LOG_SETUP_LIBRARY}
	${Boost_THREAD_LIBRARY}
	${Boost_FILESYSTEM_LIBRARY}
)

target_include_directories(dds-commander
	PRIVATE
		$<TARGET_PROPERTY:dds_user_defaults_lib,INTERFACE_INCLUDE_DIRECTORIES>
	PRIVATE
		$<TARGET_PROPERTY:dds_topology_lib,INTERFACE_INCLUDE_DIRECTORIES>
	PRIVATE
		$<TARGET_PROPERTY:dds_protocol_lib,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE
        $<TARGET_PROPERTY:dds_ncf,INTERFACE_INCLUDE_DIRECTORIES>
)

install(TARGETS dds-commander DESTINATION bin)

#
# Unit tests
#
if (BUILD_TESTS)
    message (STATUS "Build dds-commander unit tests - YES")
    add_subdirectory ( ${dds-commander_SOURCE_DIR}/tests )
else (BUILD_TESTS)
    message (STATUS "Build dds-commander unit tests - NO")
endif (BUILD_TESTS)
