# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project( dds-topology )

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
#   add_custom_target(pod-ssh-api-docs ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/Doxyfile
#      COMMENT "Generating pod-ssh API docs")
#endif(DOXYGEN_FOUND)

#
# Source files
#
set( SOURCE_FILES
	./src/main.cpp
	./src/DDSTopoProperty.cpp
	./src/DDSTopoBase.cpp
	./src/DDSTopoUtils.cpp
	./src/DDSTopoFactory.cpp
	./src/DDSTopology.cpp
	./src/DDSTask.cpp
	./src/DDSTaskCollection.cpp
	./src/DDSTaskGroup.cpp
	./src/DDSPort.cpp
	./src/DDSTopoElement.cpp
	./src/DDSTaskContainer.cpp
	./src/DDSTopologyParserXML.cpp
)

#
# Header files
#
set( HEADER_FILES
	./src/DDSTopoProperty.h
	./src/DDSTopoBase.h
	./src/DDSTopoUtils.h
	./src/DDSTopoFactory.h
	./src/DDSTopology.h
	./src/DDSTask.h
	./src/DDSTaskCollection.h
	./src/DDSTaskGroup.h
	./src/DDSPort.h
	./src/DDSTopoElement.h
	./src/DDSTaskContainer.h
	./src/DDSTopologyParserXML.h
)

include_directories(
#    ${PROJECT_BINARY_DIR}/src
#    ${MiscCommon_LOCATION}
#    ${MiscCommon_LOCATION}/pipe_log_engine
    ${Boost_INCLUDE_DIRS} )

#
# dds-topology executable
#
if (CMAKE_GENERATOR STREQUAL "Xcode")
#    add_executable(dds-topology ${SOURCE_FILES} ${HEADER_FILES})
    add_library(dds-topology SHARED ${SOURCE_FILES} ${HEADER_FILES})
else (CMAKE_GENERATOR STREQUAL "Xcode")
#    add_executable(dds-topology ${SOURCE_FILES})
    add_library(dds-topology SHARED ${SOURCE_FILES})
endif (CMAKE_GENERATOR STREQUAL "Xcode")

#target_link_libraries (
#    dds-topology
#    ${Boost_PROGRAM_OPTIONS_LIBRARY}
#)

install(TARGETS dds-topology DESTINATION bin)

#
# Unit tests
#
if (BUILD_TESTS)
    message (STATUS "Build dds-topology unit tests - YES")
    add_subdirectory ( ${dds-topology_SOURCE_DIR}/tests ) 
else (BUILD_TESTS)
    message (STATUS "Build dds-topology unit tests - NO")
endif (BUILD_TESTS)

