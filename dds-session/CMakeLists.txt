# Copyright 2018 GSI, Inc. All rights reserved.
#
#
project( dds-session )

#
# Where to lookup modules
#
set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

#
# configure files
# 
configure_file( ${dds-session_SOURCE_DIR}/src/version.h.in ${dds-session_BINARY_DIR}/version.h @ONLY )

#
# Source files
#
set( SOURCE_FILES
	./src/main.cpp
	./src/Start.cpp
	./src/InfoChannel.cpp
	./src/Stop.cpp
)

#
# Header files
#
set( HEADER_FILES
	./src/Options.h
	./src/Start.h
	./src/InfoChannel.h
	./src/Stop.h
)

include_directories(
	${dds-session_BINARY_DIR}
	${MiscCommon_LOCATION}
	${Boost_INCLUDE_DIRS} 
)

#
# dds-commander executable
#
if (CMAKE_GENERATOR STREQUAL "Xcode")
    add_executable(dds-session ${SOURCE_FILES} ${HEADER_FILES})
else (CMAKE_GENERATOR STREQUAL "Xcode")
    add_executable(dds-session ${SOURCE_FILES})
endif (CMAKE_GENERATOR STREQUAL "Xcode")

#message(STATUS "LOG_LIBRARY=${Boost_LOG_LIBRARY}")
#message(STATUS "FILESYSTEM_LIBRARY=${Boost_FILESYSTEM_LIBRARY}")

target_link_libraries (
	dds-session
	dds_user_defaults_lib
	dds_protocol_lib
	${Boost_PROGRAM_OPTIONS_LIBRARY}
	${Boost_SYSTEM_LIBRARY}
	${Boost_LOG_LIBRARY}
	${Boost_LOG_SETUP_LIBRARY}
	${Boost_THREAD_LIBRARY}
	${Boost_FILESYSTEM_LIBRARY}
)

target_include_directories(dds-session
	PRIVATE
		$<TARGET_PROPERTY:dds_user_defaults_lib,INTERFACE_INCLUDE_DIRECTORIES>
	PRIVATE
		$<TARGET_PROPERTY:dds_protocol_lib,INTERFACE_INCLUDE_DIRECTORIES>
)

install(TARGETS dds-session DESTINATION bin)

#
# Unit tests
#
#if (BUILD_TESTS)
#    message (STATUS "Build dds-session unit tests - YES")
#    add_subdirectory ( ${dds-session_SOURCE_DIR}/tests ) 
#else (BUILD_TESTS)
#    message (STATUS "Build dds-submmit unit tests - NO")
#endif (BUILD_TESTS)
