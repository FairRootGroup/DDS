# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project( DDS )

set(USER_DEFAULTS_CFG_VERSION "0.1")

#
# Check if cmake has the required version
#
# we need cmake v2.8.11+ in order to use target_include_directories
cmake_minimum_required( VERSION 2.8.11 FATAL_ERROR )

# Version
#
# take the version string from git and write it to a version file
# or take it from the version file if git is unavailable (a package distribution)
# Keep the version file in the source directory to satisfy the source package.
set( DDS_VERSION_FILE ${CMAKE_SOURCE_DIR}/etc/version )
if( EXISTS "${CMAKE_SOURCE_DIR}/.git" )

  execute_process(COMMAND git describe --match "[0-9].[0-9]" --abbrev=7 HEAD
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
option(BUILD_dds-commander "Build dds-commander" ON)
option(BUILD_dds-topology "Build dds-topology" ON)
option(BUILD_dds-topology-lib "Build dds-topology-lib" ON)
option(BUILD_dds-agent "Build dds-agent" ON)
option(BUILD_dds-ssh "Build dds-ssh" ON)
option(BUILD_dds-user-defaults "Build dds-user-defaults" ON)
option(BUILD_dds-info "Build dds-info" ON)
option(BUILD_dds-submit "Build dds-submit" ON)
option(BUILD_dds-agent-cmd "Build dds-agent-cmd" ON)
option(BUILD_dds-test "Build dds-test" ON)
option(BUILD_dds-key-value-lib "Build dds-key-value library" ON)
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
   #add_dependencies( apia-docs
   #                  MiscCommon-api-docs
   #                  pod-ssh-api-docs
   #                  pod-agent-api-docs
   #                  pod-info-api-docs
   #                  pod-remote-api-docs )

   add_custom_target( upload-api-docs 
       COMMAND rsync -avz -e ssh  ${DOC_OUTPUT_DIR} podwww@lxi001.gsi.de:~/web-docs/doc
       COMMENT "Upload API documentation to PoD's web server")
   add_dependencies( upload-api-docs api-docs )
endif(DOXYGEN_FOUND)

add_custom_target( format-code 
       COMMAND ${CMAKE_SOURCE_DIR}/utils/update_format.sh ${CMAKE_SOURCE_DIR}
       COMMENT "Updating source code formatting.")

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
# additional compiler and linker flags for C++11
#
if(APPLE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lc++")
elseif(UNIX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")
endif(APPLE)

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
# "bin" files
#
SET(DDS_BIN_FILES 
	${CMAKE_SOURCE_DIR}/bin/dds-server
	${CMAKE_SOURCE_DIR}/bin/dds-prep-worker
)

SET(DDS_BIN_PRIVATE_FILES
	${CMAKE_SOURCE_DIR}/bin/private/dds-addpayload
	${CMAKE_SOURCE_DIR}/bin/private/dds-ssh-submit-worker
)
#
# "etc" files
#
SET(DDS_ETC_FILES
	${DDS_VERSION_FILE}
	${CMAKE_SOURCE_DIR}/etc/DDSWorker.sh.in
)
#
# "share" files
#
SET(DDS_SHARE_FILES
     ${CMAKE_SOURCE_DIR}/res/topology.xsd
)

#
#
# Build DDS modules
#
#

#
# Search for BOOST
#
find_package(Boost 1.54 REQUIRED)
if(Boost_FOUND)
  set(local_boost_version "${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION}")

  set(Boost_Components log log_setup thread program_options filesystem system regex)
  if( BUILD_TESTS)
    set(Boost_Components ${Boost_Components} unit_test_framework)
  endif( BUILD_TESTS)
  
  find_package( Boost 1.54 REQUIRED COMPONENTS  ${Boost_Components} )
endif(Boost_FOUND)

# DDS Misc Common
message(STATUS "Build MiscCommon - YES")
add_subdirectory ( MiscCommon )

# DDS Protocol lib
message(STATUS "Build dds-protocol-lib - YES")
add_subdirectory( dds-protocol-lib )

# dds-daemonize
message(STATUS "Build dds-daemonize - YES")
add_subdirectory( dds-daemonize )

# dds-commander
if(BUILD_dds-commander)
    message(STATUS "Build dds-commander - YES")
    add_subdirectory ( dds-commander )
endif(BUILD_dds-commander)

# dds-topology
if(BUILD_dds-topology)
    message(STATUS "Build dds-topology - YES")
    add_subdirectory ( dds-topology )
endif(BUILD_dds-topology)

# dds-topology-lib
if(BUILD_dds-topology-lib)
    message(STATUS "Build dds-topology-lib - YES")
    add_subdirectory ( dds-topology-lib )
endif(BUILD_dds-topology-lib)

# dds-agent
if(BUILD_dds-agent)
    message(STATUS "Build dds-agent - YES")
    add_subdirectory ( dds-agent )
endif(BUILD_dds-agent)

# dds-ssh
if(BUILD_dds-ssh)
    message(STATUS "Build dds-ssh - YES")
    add_subdirectory ( dds-ssh )
endif(BUILD_dds-ssh)

# dds-user-defaults
if(BUILD_dds-user-defaults)
    message(STATUS "Build dds-user-defaults - YES")
    add_subdirectory ( dds-user-defaults )
endif(BUILD_dds-user-defaults)

# dds-info
if(BUILD_dds-info)
    message(STATUS "Build dds-info - YES")
    add_subdirectory ( dds-info )
endif(BUILD_dds-info)

# dds-submit
if(BUILD_dds-submit)
    message(STATUS "Build dds-submit - YES")
    add_subdirectory ( dds-submit )
endif(BUILD_dds-submit)

# dds-agent-cmd
if(BUILD_dds-agent-cmd)
    message(STATUS "Build dds-agent-cmd - YES")
    add_subdirectory ( dds-agent-cmd )
endif(BUILD_dds-agent-cmd)

# dds-test
if(BUILD_dds-test)
    message(STATUS "Build dds-test - YES")
    add_subdirectory ( dds-test )
endif(BUILD_dds-test)

# dds-key-value-lib
if(BUILD_dds-key-value-lib)
    message(STATUS "Build dds-key-value library - YES")
    add_subdirectory ( dds-key-value-lib )
endif(BUILD_dds-key-value-lib)

#
# Install
#
install(FILES ${DDS_MISC_FILES} DESTINATION .)
install(PROGRAMS ${CMAKE_SOURCE_DIR}/etc/DDS_env.sh DESTINATION .)
# install bin files
install(PROGRAMS ${DDS_BIN_FILES} DESTINATION bin)
install(PROGRAMS ${DDS_BIN_PRIVATE_FILES} DESTINATION bin/private)
# install etc files
install(FILES ${DDS_ETC_FILES} DESTINATION etc)
# install share files
install(FILES ${DDS_SHARE_FILES} DESTINATION share)
# tests runner
if( BUILD_TESTS )
  install(PROGRAMS run_test.sh DESTINATION tests)
endif( BUILD_TESTS )

# WN bin package custom target
# Because of $<TARGET_FILE:tgt> it works only in cmake 2.8.4
if( ${CMAKE_VERSION} VERSION_GREATER 2.8.3 )
   if( CMAKE_SIZEOF_VOID_P EQUAL 4 )
     set(TRG_ARCH x86)
   else( CMAKE_SIZEOF_VOID_P EQUAL 4 )
     set(TRG_ARCH amd64)
   endif( CMAKE_SIZEOF_VOID_P EQUAL 4 )
   
   if(APPLE)
       set( WN_PKG_NAME "dds-wrk-bin-${DDS_VERSION}-${CMAKE_SYSTEM_NAME}-universal" )
   elseif(UNIX)
        set( WN_PKG_NAME "dds-wrk-bin-${DDS_VERSION}-${CMAKE_SYSTEM_NAME}-${TRG_ARCH}" )
   endif(APPLE)

   set( WN_PKG_DIR "${CMAKE_BINARY_DIR}/dds-wrk-bin" )
   set( WN_PKG_NAME_ARCH "${WN_PKG_NAME}.tar.gz" )
   set( WN_PKG_REMOTE_DIR "/u/ddswww/web-docs/releases/add/${DDS_VERSION}" )

   if(APPLE)
     set( lib_cmd "bash" )
     set( lib_cmd_args "-c" "otool -L ${WN_PKG_DIR}/dds-agent  | grep boost | awk '{ system( \"cp \" $1 \" ${WN_PKG_DIR}\") }'" )
     # the log library needs a special handling as it depends on many other boost libraries
     set( lib_cmd_args_log "-c" "otool -L ${Boost_LOG_LIBRARY}  | grep -E 'boost|icu' | tail -n +2 | awk '{ system( \"cp \" $1 \" ${WN_PKG_DIR}\") }'" )
   elseif(UNIX)
     set( lib_cmd "bash" )
     set( lib_cmd_args "-c" "ldd ${WN_PKG_DIR}/dds-agent | grep boost | awk '{ system( \"cp \" $3 \" ${WN_PKG_DIR}\") }'" )
     # the log library needs a special handling as it depends on many other boost libraries
     set( lib_cmd_args_log "-c" "ldd ${Boost_LOG_LIBRARY} | grep -E 'boost|icu' | awk '{ system( \"cp \" $3 \" ${WN_PKG_DIR}\") }'" )
   endif(APPLE)
   
   add_custom_target( wn_bin
                      COMMAND -mkdir -p ${WN_PKG_DIR}
                      COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:dds-agent> "${WN_PKG_DIR}"
		              COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:dds-user-defaults> "${WN_PKG_DIR}"
                      COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:dds-user-defaults-lib> "${WN_PKG_DIR}"
		              COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:dds-protocol-lib> "${WN_PKG_DIR}"
					  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:dds-key-value-lib> "${WN_PKG_DIR}"
		              COMMAND ${lib_cmd} ${lib_cmd_args}
                      COMMAND ${lib_cmd} ${lib_cmd_args_log}
                      COMMAND ${CMAKE_COMMAND} -E tar czf ${WN_PKG_NAME_ARCH} "${WN_PKG_DIR}"
                      COMMAND -chmod go+xr ${WN_PKG_NAME_ARCH}
                      COMMENT "Generate WN binary package" 
                      VERBATIM )
   add_dependencies( wn_bin
			dds-agent
			dds-user-defaults-lib 
			dds-protocol-lib
			dds-key-value-lib)

   add_custom_target( wn_bin_upload
		      COMMAND ssh ddswww@lxi001 "mkdir -p ${WN_PKG_REMOTE_DIR} && chmod go+xr ${WN_PKG_REMOTE_DIR}"                      
                      COMMAND scp -p ${WN_PKG_NAME_ARCH} ddswww@lxi001:${WN_PKG_REMOTE_DIR}
                      COMMENT "Uploading WN binary package to DDS binary repository" 
                      VERBATIM )

   add_dependencies( wn_bin_upload
                     wn_bin )


   INSTALL(FILES ${CMAKE_BINARY_DIR}/${WN_PKG_NAME_ARCH} DESTINATION bin/wn_bins OPTIONAL)

endif(${CMAKE_VERSION} VERSION_GREATER 2.8.3)

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

