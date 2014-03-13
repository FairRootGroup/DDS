#************************************************************************
#
# CMakeLists.txt
# 
# Anar Manafov A.Manafov@gsi.de
# 
#
#        version number:    $LastChangedRevision$
#        created by:        Anar Manafov
#                           2011-03-07
#        last changed by:   $LastChangedBy$ $LastChangedDate$
#
#        Copyright (c) 2011 GSI, Scientific Computing group. All rights reserved.
#*************************************************************************
project( pipe_log_engine )

#
# Where to lookup modules
#
set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

#
# Source files
#
set( SOURCE_FILES
     logEngine.cpp
)

set( SRC_HDRS
     logEngine.h
)

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${MiscCommon_LOCATION}
    ${Boost_INCLUDE_DIRS}
)

set(Boost_libs_for_pipe_log ${Boost_THREAD_LIBRARY} ${Boost_SYSTEM_LIBRARY})

#
# lib
#
if (CMAKE_GENERATOR STREQUAL "Xcode")
    add_library(pipe_log_engine SHARED ${SOURCE_FILES} ${SRC_HDRS})
else (CMAKE_GENERATOR STREQUAL "Xcode")
    add_library(pipe_log_engine SHARED ${SOURCE_FILES})
endif (CMAKE_GENERATOR STREQUAL "Xcode")

target_link_libraries (
    pipe_log_engine
    ${Boost_libs_for_pipe_log} 
)

install(TARGETS pipe_log_engine DESTINATION lib)

