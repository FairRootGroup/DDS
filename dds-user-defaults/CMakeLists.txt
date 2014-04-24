# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project( dds-user-defaults)


set(USER_DEFAULTS_CFG_VERSION "0.1")

#
# Where to lookup modules
#
set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

#
# configure files
# 
configure_file( ${dds-user-defaults_SOURCE_DIR}/src/version.h.in ${dds-user-defaults_BINARY_DIR}/version.h @ONLY )

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
)

#
# Header files
#
set( HEADER_FILES

)

include_directories(
    ${dds-user-defaults_BINARY_DIR}
    ${MiscCommon_LOCATION}
    ${Boost_INCLUDE_DIRS}
)

#
# dds-user-defaults executable
#
if (CMAKE_GENERATOR STREQUAL "Xcode")
    add_executable(dds-user-defaults ${SOURCE_FILES} ${HEADER_FILES})
else (CMAKE_GENERATOR STREQUAL "Xcode")
    add_executable(dds-user-defaults ${SOURCE_FILES})
endif (CMAKE_GENERATOR STREQUAL "Xcode")

target_link_libraries (
    dds-user-defaults
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
)

install(TARGETS dds-user-defaults DESTINATION bin)

#
# Unit tests
#
if (BUILD_TESTS)
    message (STATUS "Build dds-user-defaults unit tests - YES")
 #   add_subdirectory ( ${dds-user-defaults_SOURCE_DIR}/tests ) 
else (BUILD_TESTS)
    message (STATUS "Build dds-user-defaults unit tests - NO")
endif (BUILD_TESTS)

