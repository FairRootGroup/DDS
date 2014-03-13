# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project( dds_sys_files )

#
# Where to lookup modules
#
set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

#
# Source files
#
set( SOURCE_FILES
     DDSSysFiles.cpp
)

set( SRC_HDRS
     DDSSysFiles.h
)

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${MiscCommon_LOCATION}
    ${Boost_INCLUDE_DIRS}
)

#
# lib
#
if (CMAKE_GENERATOR STREQUAL "Xcode")
    add_library(dds_sys_files SHARED ${SOURCE_FILES} ${SRC_HDRS})
else (CMAKE_GENERATOR STREQUAL "Xcode")
    add_library(dds_sys_files SHARED ${SOURCE_FILES})
endif (CMAKE_GENERATOR STREQUAL "Xcode")

target_link_libraries (
    dds_sys_files
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
)

install(TARGETS dds_sys_files DESTINATION lib)

