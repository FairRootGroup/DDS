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
	./src/CommanderServer.cpp
	./src/TalkToAgent.cpp
)

#
# Header files
#
set( HEADER_FILES
	./src/Options.h
	./src/CommanderServer.h
	./src/TalkToAgent.h
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
	dds-user-defaults-lib
	${Boost_PROGRAM_OPTIONS_LIBRARY}
	${Boost_SYSTEM_LIBRARY}
)

target_include_directories(dds-commander
	PRIVATE
		$<TARGET_PROPERTY:dds-user-defaults-lib,INTERFACE_INCLUDE_DIRECTORIES>
)

install(TARGETS dds-commander DESTINATION bin)

#
# Unit tests
#
#if (BUILD_TESTS)
#    message (STATUS "Build dds-commander unit tests - YES")
#    add_subdirectory ( ${dds-commander_SOURCE_DIR}/tests ) 
#else (BUILD_TESTS)
#    message (STATUS "Build dds-commander unit tests - NO")
#endif (BUILD_TESTS)
