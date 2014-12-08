# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project( dds-topology-lib )

#
# Where to lookup modules
#
set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

#
# configure files
# 
configure_file( ${dds-topology-lib_SOURCE_DIR}/src/version.h.in ${dds-topology-lib_BINARY_DIR}/version.h @ONLY )

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
	./src/TopoProperty.cpp
	./src/TopoBase.cpp
	./src/TopoUtils.cpp
	./src/TopoFactory.cpp
	./src/Topology.cpp
	./src/Task.cpp
	./src/TaskCollection.cpp
	./src/TaskGroup.cpp
	./src/TopoElement.cpp
	./src/TaskContainer.cpp
	./src/TopologyParserXML.cpp
	./src/TopoIndex.cpp
    ./src/Requirement.cpp
)

#
# Header files
#
set( HEADER_FILES
	./src/TopoProperty.h
	./src/TopoBase.h
	./src/TopoUtils.h
	./src/TopoFactory.h
	./src/Topology.h
	./src/Task.h
	./src/TaskCollection.h
	./src/TaskGroup.h
	./src/TopoElement.h
	./src/TaskContainer.h
	./src/TopologyParserXML.h
	./src/TopoIndex.h
    ./src/Requirement.h
)

include_directories(
	${dds-topology-lib_BINARY_DIR}
	${MiscCommon_LOCATION}
	${Boost_INCLUDE_DIRS}
)

#
# dds-topology-lib executable
#
if (CMAKE_GENERATOR STREQUAL "Xcode")
#    add_executable(dds-topology-lib ${SOURCE_FILES} ${HEADER_FILES})
    add_library(dds-topology-lib SHARED ${SOURCE_FILES} ${HEADER_FILES})
else (CMAKE_GENERATOR STREQUAL "Xcode")
#    add_executable(dds-topology-lib ${SOURCE_FILES})
    add_library(dds-topology-lib SHARED ${SOURCE_FILES})
endif (CMAKE_GENERATOR STREQUAL "Xcode")

target_link_libraries ( dds-topology-lib
	dds-user-defaults-lib
	${Boost_FILESYSTEM_LIBRARY}
    ${Boost_REGEX_LIBRARY}
)

# API headers
#target_include_directories(dds-topology-lib INTERFACE ${dds-topology-lib_SOURCE_DIR}/src)


target_include_directories(dds-topology-lib
    INTERFACE
        ${dds-topology-lib_SOURCE_DIR}/src
	PRIVATE
		$<TARGET_PROPERTY:dds-user-defaults-lib,INTERFACE_INCLUDE_DIRECTORIES>
)

install(TARGETS dds-topology-lib DESTINATION lib)

#
# Unit tests
#
if (BUILD_TESTS)
    message (STATUS "Build dds-topology-lib unit tests - YES")
    add_subdirectory ( ${dds-topology-lib_SOURCE_DIR}/tests ) 
else (BUILD_TESTS)
    message (STATUS "Build dds-topology-lib unit tests - NO")
endif (BUILD_TESTS)



