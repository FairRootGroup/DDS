# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project( SSHTunnel )

#
# Where to lookup modules
#
set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

#
# Source files
#
set( SOURCE_FILES
     SSHTunnel.cpp
)

set( SRC_HDRS
     SSHTunnel.h
)

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${MiscCommon_LOCATION}
)

#
# lib
#
if (CMAKE_GENERATOR STREQUAL "Xcode")
    add_library(SSHTunnel SHARED ${SOURCE_FILES} ${SRC_HDRS})
else (CMAKE_GENERATOR STREQUAL "Xcode")
    add_library(SSHTunnel SHARED ${SOURCE_FILES})
endif (CMAKE_GENERATOR STREQUAL "Xcode")

target_link_libraries (
    SSHTunnel
)


install(TARGETS SSHTunnel DESTINATION lib)
install(PROGRAMS ssh-tunnel DESTINATION bin/private)

