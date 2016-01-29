# Copyright 2016 GSI, Inc. All rights reserved.
#
#
project( dds_ncf )

#
# Where to lookup modules
#
set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

#
# Source files
#
set( SOURCE_FILES
     ./src/ncf.cpp
)

set( SRC_HDRS
     ./src/ncf.h
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
    add_library(dds_ncf SHARED ${SOURCE_FILES} ${SRC_HDRS})
else (CMAKE_GENERATOR STREQUAL "Xcode")
    add_library(dds_ncf SHARED ${SOURCE_FILES})
endif (CMAKE_GENERATOR STREQUAL "Xcode")

target_link_libraries (
    dds_ncf
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
)


#
# Unit tests
#
if (BUILD_TESTS)
    message (STATUS "Build dds_ncf unit tests - YES")
    add_subdirectory ( ${dds_ncf_SOURCE_DIR}/tests ) 
else (BUILD_TESTS)
    message (STATUS "Build dds_ncf unit tests - NO")
endif (BUILD_TESTS)

install(TARGETS dds_ncf DESTINATION lib)

