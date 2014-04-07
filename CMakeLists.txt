# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project( DDS )

#
# Check if cmake has the required version
#
# we need cmake v. < v2.6.2 in order to use VERSION_EQUAL and other VERSION_*
cmake_minimum_required( VERSION 2.6.2 FATAL_ERROR )

# Version
#
# take the version string from git and write it to a version file
# or take it from the version file if git is unavailable (a package distribution)
# Keep the version file in the source directory to satisfy the source package.
set( DDS_VERSION_FILE ${CMAKE_SOURCE_DIR}/etc/version )
if( EXISTS "${CMAKE_SOURCE_DIR}/.git" )

  execute_process(COMMAND git describe --match "[0-9].[0-9].[0-9]" --abbrev=7 HEAD
                  COMMAND sed -e "s/-/./g"
                  OUTPUT_VARIABLE DDS_VERSION
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process( COMMAND bash -c "echo \"${DDS_VERSION}\" > ${DDS_VERSION_FILE}" )

else( EXISTS "${CMAKE_SOURCE_DIR}/.git" )

  execute_process(COMMAND cat ${DDS_VERSION_FILE}
                  OUTPUT_VARIABLE DDS_VERSION
                  OUTPUT_STRIP_TRAILING_WHITESPACE)

endif( EXISTS "${CMAKE_SOURCE_DIR}/.git" )
message(STATUS "Building DDS version: ${DDS_VERSION}")
#
# Options
#
option(BUILD_DOCUMENTATION "Build documentation" OFF)
# tests
option(BUILD_TESTS "Build DDS unit tests" OFF)
# modules
option(BUILD_dds_topology "Build dds-topology" ON)
option(BUILD_dds_ssh "Build dds-ssh" ON)
#
# Documentation output directory
#
find_package(Doxygen)
if(DOXYGEN_FOUND)
   set(DOC_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/documentation/api-docs)
endif(DOXYGEN_FOUND)
#
# Custom targets
#
if(DOXYGEN_FOUND)
   add_custom_target( api-docs COMMENT "Generating API docs" )
   add_dependencies( api-docs
                     MiscCommon-api-docs
                     pod-ssh-api-docs
                     pod-agent-api-docs
                     pod-info-api-docs
                     pod-remote-api-docs )

   add_custom_target( upload-api-docs 
       COMMAND rsync -avz -e ssh  ${DOC_OUTPUT_DIR} podwww@lxi001.gsi.de:~/web-docs/doc
       COMMENT "Upload API documentation to PoD's web server")
   add_dependencies( upload-api-docs api-docs )
endif(DOXYGEN_FOUND)
#
# Install directory
#
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set (CMAKE_INSTALL_PREFIX "$ENV{HOME}/DDS/${DDS_VERSION}" CACHE PATH "Install path prefix, prepended onto install directories." FORCE)
endif (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
#
# This is needed to properly install PoD modules
#
set(IS_SET_DDS_INSTALL_PREFIX 1 CACHE INTERNAL "")
#
# Where to lookup modules
#
set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}; ${CMAKE_SOURCE_DIR}/cmake")
#
# MiscCommon location
#
set(MiscCommon_LOCATION ${CMAKE_SOURCE_DIR}/MiscCommon)
set(MiscCommon_LOCATION
    ${MiscCommon_LOCATION}
    ${MiscCommon_LOCATION}/pipe_log_engine
    ${MiscCommon_LOCATION}/dds_sys_files
)
#
# additional compiler flags for C++11
#
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
#
# additional linker flags for C++11
#
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lc++")
#
# global variables
#
set(RESOURCE_DIR "${CMAKE_SOURCE_DIR}/res")
set(TOPOLOGY_SCHEMA_PATH "${RESOURCE_DIR}/topology.xsd")

#
# "additional" files
#
SET(DDS_MISC_FILES
    ${CMAKE_SOURCE_DIR}/LICENSE
    ${CMAKE_SOURCE_DIR}/ReleaseNotes.md
)
#
# "etc" files
#
SET(DDS_ETC_FILES
     ${DDS_VERSION_FILE}
)
#
#
# Build DDS modules
#
#

#
# Search for BOOST
#
find_package(Boost 1.55 REQUIRED)
if(Boost_FOUND)
  set(local_boost_version "${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION}")

  set(Boost_Components thread program_options filesystem system)  
  if( BUILD_TESTS)
    set(Boost_Components ${Boost_Components} unit_test_framework)
  endif( BUILD_TESTS)
  
  find_package( Boost 1.55 REQUIRED COMPONENTS  ${Boost_Components} )
endif(Boost_FOUND)

# DDS Misc Common
message(STATUS "Build MiscCommon - YES")
add_subdirectory ( MiscCommon )

# dds_topology
if(BUILD_dds_topology)
    message(STATUS "Build dds-topology - YES")
    add_subdirectory ( dds-topology )
endif(BUILD_dds_topology)

# dds_ssh
#if(BUILD_dds_ssh)
#    message(STATUS "Build dds-ssh - YES")
#    add_subdirectory ( dds-ssh )
#endif(BUILD_dds_ssh)


#
# Install
#
install(FILES ${DDS_MISC_FILES} DESTINATION .)
install(PROGRAMS ${CMAKE_SOURCE_DIR}/etc/DDS_env.sh DESTINATION .)
# install etc files
install(FILES ${DDS_ETC_FILES} DESTINATION etc)
# tests runner
if( BUILD_TESTS )
  install(PROGRAMS run_test.sh DESTINATION tests)
endif( BUILD_TESTS )

#
# Package
#
SET(CPACK_GENERATOR "TGZ")
set(CPACK_SOURCE_GENERATOR "TGZ")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_SOURCE_IGNORE_FILES
    "inst_tmp"
    "etc/xpd.cf$"
    "documentation/Diagrams"
    "documentation/html_help/.*\\\\.html$"
    "documentation/html_help/.*\\\\.png$"
    "documentation/html_help/.*\\\\.css$"
    "documentation/html_help/.*\\\\.ps$"
    "documentation/api-docs"
    "app/pod-agent/doc"
    "app/pod-console/doc"
    "Logo"
    "/\\\\."
    "~$;/_"
    "^${PROJECT_SOURCE_DIR}/build"
    "CMakeFiles/"
    "CMakeCache"
    "gz$"
    "Makefile\\\\."
    ".project"
    ".cproject"
    ".settings"
    "cmake_install"
    "CPack"
    "\\\\.svn/"
    "Makefile$"
    "PoDFiles.txt$")

set( CPACK_SOURCE_PACKAGE_FILE_NAME "DDS-${DDS_VERSION}-Source")

include(CPack)

