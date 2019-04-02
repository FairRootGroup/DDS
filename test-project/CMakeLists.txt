# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project(test-project)

cmake_minimum_required( VERSION 3.11.0 FATAL_ERROR )

set ( DDS_INSTALL_DIR "$ENV{DDS_LOCATION}" )

if (DDS_INSTALL_DIR STREQUAL "")
    message(FATAL_ERROR "DDS_LOCATION is not set. Please specify DDS install directory.")
elseif (DDS_INSTALL_DIR STREQUAL "")
    message(STATUS "DDS install directory is set to ${DDS_INSTALL_DIR}")
endif (DDS_INSTALL_DIR STREQUAL "")

if(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++ -Wthread-safety")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lc++")
elseif(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")
endif(APPLE)

set(Boost_Components log log_setup thread program_options filesystem system regex)
find_package(Boost 1.67 REQUIRED COMPONENTS  ${Boost_Components})
if(Boost_FOUND)
    set(local_boost_version "${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION}")
endif(Boost_FOUND)

add_executable(test-exec src/TestProject.cpp)

if(APPLE)
  target_link_libraries(test-exec
    Boost::boost
    ${DDS_INSTALL_DIR}/lib/libdds_intercom_lib.dylib
    ${DDS_INSTALL_DIR}/lib/libdds_topology_lib.dylib
  )
elseif(UNIX)
  target_link_libraries(test-exec
    Boost::boost
    ${DDS_INSTALL_DIR}/lib/libdds_intercom_lib.so
    ${DDS_INSTALL_DIR}/lib/libdds_topology_lib.so
  )
endif(APPLE)

target_include_directories(test-exec
    PRIVATE
    ${DDS_INSTALL_DIR}/include
)
