# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project( MiscCommon )
#
# Version
#
set(MISCCOMMON_VERSION "5.0.0")
#
# Where to lookup modules
#
set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")
#
# API Docs
#
#if(DOXYGEN_FOUND)
#   configure_file( ${PROJECT_SOURCE_DIR}/Doxyfile.in ${PROJECT_BINARY_DIR}/Doxyfile @ONLY )
#   add_custom_target(MiscCommon-api-docs ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/Doxyfile
#       COMMENT "Generating MiscCommon API docs")
#endif(DOXYGEN_FOUND)


#
# Build pipe_log_engine
#
#message(STATUS "Build the pipe_log_engine lib - YES")
#add_subdirectory( ${MiscCommon_SOURCE_DIR}/pipe_log_engine )

#
# Build pod_protocol
#
#message(STATUS "Build the pod_protocol lib - YES")
#add_subdirectory( ${MiscCommon_SOURCE_DIR}/pod_protocol )

#
# Build proof_status_file
#
#message(STATUS "Build the proof_status_file lib - YES")
#add_subdirectory( ${MiscCommon_SOURCE_DIR}/proof_status_file )

#
# Build SSHTunnel
#
#message(STATUS "Build the SSHTunnel lib - YES")
#add_subdirectory( ${MiscCommon_SOURCE_DIR}/SSHTunnel )

#
# Build dds_sys_files
#
#message(STATUS "Build the dds_sys_files lib - YES")
#add_subdirectory( ${MiscCommon_SOURCE_DIR}/dds_sys_files )

#
## Unit tests
#
#if (BUILD_TESTS)
#    message (STATUS "Build MiscCommon unit tests - YES")
#    add_subdirectory ( ${MiscCommon_SOURCE_DIR}/tests ) 
#else (BUILD_TESTS)
#    message (STATUS "Build MiscCommon unit tests - NO")
#endif (BUILD_TESTS)
