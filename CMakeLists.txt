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

  execute_process(COMMAND git describe --abbrev=4 HEAD
                  COMMAND sed -e "s/-/./g"
                  OUTPUT_VARIABLE DDS_VERSION
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process( COMMAND bash -c "echo \"${DDS_VERSION}\" > ${DDS_VERSION_FILE}" )

else( EXISTS "${CMAKE_SOURCE_DIR}/.git" )

  execute_process(COMMAND cat ${DDS_VERSION_FILE}
                  OUTPUT_VARIABLE DDS_VERSION
                  OUTPUT_STRIP_TRAILING_WHITESPACE)

endif( EXISTS "${CMAKE_SOURCE_DIR}/.git" )

# modules
option(BUILD_dds_topology "Build dds-topology" ON)

# additional compiler flags for C++11
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
# additional linker flags for C++11
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lc++")

# Search for BOOST
find_package(Boost 1.55 REQUIRED)
if(Boost_FOUND)
  set(local_boost_version "${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION}")

  set(Boost_Components thread program_options filesystem system)  
  if( BUILD_TESTS)
    set(Boost_Components ${Boost_Components} unit_test_framework)
  endif( BUILD_TESTS)
  
  find_package( Boost 1.55 REQUIRED COMPONENTS  ${Boost_Components} )  
endif(Boost_FOUND)


if(BUILD_dds_topology)
    message(STATUS "Build dds-topology - YES")
    add_subdirectory ( dds-topology )
endif(BUILD_dds_topology)

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

