# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project( dds-protocol-lib )

#
# Where to lookup modules
#
set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

#
# Source files
#
set( SOURCE_FILES
     Protocol.cpp 
     ProtocolCommands.cpp
)

set( SRC_HDRS
     Protocol.h 
     ProtocolCommands.h
)

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${MiscCommon_LOCATION}
)

#
# lib
#
if (CMAKE_GENERATOR STREQUAL "Xcode")
    add_library(dds-protocol-lib SHARED ${SOURCE_FILES} ${SRC_HDRS})
else (CMAKE_GENERATOR STREQUAL "Xcode")
    add_library(dds-protocol-lib SHARED ${SOURCE_FILES})
endif (CMAKE_GENERATOR STREQUAL "Xcode")

target_link_libraries (
    dds-protocol-lib
)

install(TARGETS dds-protocol-lib DESTINATION lib)

