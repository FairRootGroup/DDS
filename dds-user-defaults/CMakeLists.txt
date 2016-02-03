# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project( dds-user-defaults)

#
# Where to lookup modules
#
set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

#
# configure files
# 
configure_file( ${dds-user-defaults_SOURCE_DIR}/src/version.h.in ${dds-user-defaults_BINARY_DIR}/version.h @ONLY )

#
# Source files
#
set( LIB_SOURCE_FILES
    ./src/UserDefaults.cpp
)

set( SOURCE_FILES
    ./src/main.cpp
    ${LIB_SOURCE_FILES}
)

#
# Header files
#
set( HEADER_FILES
   ./src/UserDefaults.h
   ./src/Options.h
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
    add_library(dds_user_defaults_lib SHARED ${LIB_SOURCE_FILES} ${HEADER_FILES})
    add_executable(dds-user-defaults ${SOURCE_FILES} ${HEADER_FILES})
else (CMAKE_GENERATOR STREQUAL "Xcode")
    add_library(dds_user_defaults_lib SHARED ${LIB_SOURCE_FILES})
    add_executable(dds-user-defaults ${SOURCE_FILES})
endif (CMAKE_GENERATOR STREQUAL "Xcode")

# we want to have the same name for the library and for the exe
SET_TARGET_PROPERTIES(dds_user_defaults_lib PROPERTIES OUTPUT_NAME dds-user-defaults)

# API headers
target_include_directories(dds_user_defaults_lib INTERFACE ${dds-user-defaults_SOURCE_DIR}/src)

target_link_libraries (
    dds-user-defaults
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
)

target_link_libraries (
    dds_user_defaults_lib
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
)

install(TARGETS dds-user-defaults DESTINATION bin)
install(TARGETS dds_user_defaults_lib DESTINATION lib)


#
# Unit tests
#
if (BUILD_TESTS)
    message (STATUS "Build dds-user-defaults unit tests - YES")
 #   add_subdirectory ( ${dds-user-defaults_SOURCE_DIR}/tests ) 
else (BUILD_TESTS)
    message (STATUS "Build dds-user-defaults unit tests - NO")
endif (BUILD_TESTS)
