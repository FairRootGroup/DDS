# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project(MiscCommon)

set(MISCCOMMON_VERSION "5.0.0")

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME}
  INTERFACE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
)

install(TARGETS ${PROJECT_NAME} EXPORT DDSTargets)
#if(DOXYGEN_FOUND)
#   configure_file( ${PROJECT_SOURCE_DIR}/Doxyfile.in ${PROJECT_BINARY_DIR}/Doxyfile @ONLY )
#   add_custom_target(MiscCommon-api-docs ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/Doxyfile
#       COMMENT "Generating MiscCommon API docs")
#endif(DOXYGEN_FOUND)

#message(STATUS "Build the pipe_log_engine lib - YES")
add_subdirectory(pipe_log_engine) 

#message(STATUS "Build the SSHTunnel lib - YES")
#add_subdirectory( ${MiscCommon_SOURCE_DIR}/SSHTunnel )

message(STATUS "Build the dds_ncf lib - YES")
add_subdirectory(dds_ncf)

if(BUILD_TESTS)
  message(STATUS "Build ${PROJECT_NAME} unit tests - YES")
  add_subdirectory(tests)
else()
  message(STATUS "Build ${PROJECT_NAME} unit tests - NO")
endif()

install(FILES EnvProp.h DESTINATION "${PROJECT_INSTALL_INCLUDEDIR}")

# BuildConstants header
set(DDS_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
set(DDS_BINDIR "${CMAKE_INSTALL_PREFIX}/${PROJECT_INSTALL_BINDIR}")
set(DDS_LIBDIR "${CMAKE_INSTALL_PREFIX}/${PROJECT_INSTALL_LIBDIR}")
configure_file(BuildConstants.h.in ${CMAKE_BINARY_DIR}/BuildConstants.h @ONLY)

# Version header
set(VersionHeader ${CMAKE_BINARY_DIR}/Version.h)
configure_file(Version.h.in ${VersionHeader})
install(FILES ${VersionHeader} DESTINATION "${PROJECT_INSTALL_INCLUDEDIR}")

