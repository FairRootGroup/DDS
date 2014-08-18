# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project( dds-getlog )

#
# Where to lookup modules
#
set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

#
# configure files
# 
configure_file( ${dds-getlog_SOURCE_DIR}/src/version.h.in ${dds-getlog_BINARY_DIR}/version.h @ONLY )

#
# API Docs
#
#if(DOXYGEN_FOUND)
#   configure_file( ${PROJECT_SOURCE_DIR}/Doxyfile.in ${PROJECT_BINARY_DIR}/Doxyfile @ONLY )
#   add_custom_target(pod-ssh-api-docs ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/Doxyfile
#      COMMENT "Generating pod-ssh API docs")
#endif(DOXYGEN_FOUND)

#
# Source files
#
set( SOURCE_FILES
	./src/main.cpp
	./src/GetLogChannel.cpp
)

#
# Header files
#
set( HEADER_FILES
	./src/Options.h
	./src/GetLogChannel.h
)

include_directories(
	${dds-getlog_BINARY_DIR}
	${MiscCommon_LOCATION}
	${Boost_INCLUDE_DIRS} 
)

#
# dds-getlog executable
#
if (CMAKE_GENERATOR STREQUAL "Xcode")
    add_executable(dds-getlog ${SOURCE_FILES} ${HEADER_FILES})
else (CMAKE_GENERATOR STREQUAL "Xcode")
    add_executable(dds-getlog ${SOURCE_FILES})
endif (CMAKE_GENERATOR STREQUAL "Xcode")

#message(STATUS "LOG_LIBRARY=${Boost_LOG_LIBRARY}")
#message(STATUS "FILESYSTEM_LIBRARY=${Boost_FILESYSTEM_LIBRARY}")

target_link_libraries (
	dds-getlog
	dds-user-defaults-lib
	dds-protocol-lib
	${Boost_PROGRAM_OPTIONS_LIBRARY}
	${Boost_SYSTEM_LIBRARY}
	${Boost_LOG_LIBRARY}
	${Boost_LOG_SETUP_LIBRARY}
	${Boost_THREAD_LIBRARY}
)

target_include_directories(dds-getlog
	PRIVATE
		$<TARGET_PROPERTY:dds-user-defaults-lib,INTERFACE_INCLUDE_DIRECTORIES>
	PRIVATE
		$<TARGET_PROPERTY:dds-protocol-lib,INTERFACE_INCLUDE_DIRECTORIES>
)

install(TARGETS dds-getlog DESTINATION bin)

#
# Unit tests
#
#if (BUILD_TESTS)
#    message (STATUS "Build dds-getlog unit tests - YES")
#    add_subdirectory ( ${dds-getlog_SOURCE_DIR}/tests ) 
#else (BUILD_TESTS)
#    message (STATUS "Build dds-getlog unit tests - NO")
#endif (BUILD_TESTS)
