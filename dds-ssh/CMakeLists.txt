# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project( dds-ssh )

#
# Where to lookup modules
#
set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

#
# configure files
# 
configure_file( ${PROJECT_SOURCE_DIR}/src/version.h.in ${PROJECT_BINARY_DIR}/src/version.h @ONLY )
#
# API Docs
#
#if(DOXYGEN_FOUND)
#   configure_file( ${PROJECT_SOURCE_DIR}/Doxyfile.in ${PROJECT_BINARY_DIR}/Doxyfile @ONLY )
#   add_custom_target(dds-ssh-api-docs ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/Doxyfile
#      COMMENT "Generating dds-ssh API docs")
#endif(DOXYGEN_FOUND)

#
# Source files
#
set( SOURCE_FILES
    ./src/main.cpp
    ./src/config.cpp
    ./src/worker.cpp
)
set( SRC_HDRS
     ./src/config.h
     ./src/local_types.h
     ./src/threadPool.h
     ./src/worker.h
)


include_directories(
    ${PROJECT_BINARY_DIR}/src
    ${MiscCommon_LOCATION}
    ${MiscCommon_LOCATION}/pipe_log_engine
    ${Boost_INCLUDE_DIRS} )

#
# dds-ssh executable
#
if (CMAKE_GENERATOR STREQUAL "Xcode")
    add_executable(dds-ssh ${SOURCE_FILES} ${SRC_HDRS})
else (CMAKE_GENERATOR STREQUAL "Xcode")
    add_executable(dds-ssh ${SOURCE_FILES})
endif (CMAKE_GENERATOR STREQUAL "Xcode")

target_link_libraries (
    dds-ssh
    pipe_log_engine
    dds_sys_files
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    ${Boost_THREAD_LIBRARY}
)

install(TARGETS dds-ssh DESTINATION bin)

#
# Unit tests
#
if (BUILD_TESTS)
    message (STATUS "Build dds-ssh unit tests - YES")
    add_subdirectory ( ${dds-ssh_SOURCE_DIR}/tests ) 
else (BUILD_TESTS)
    message (STATUS "Build dds-ssh unit tests - NO")
endif (BUILD_TESTS)

