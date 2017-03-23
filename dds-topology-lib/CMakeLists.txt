# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project( dds_topology_lib )

#
# Where to lookup modules
#
set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

#
# configure files
#
configure_file( ${dds_topology_lib_SOURCE_DIR}/src/version.h.in ${dds_topology_lib_BINARY_DIR}/version.h @ONLY )

#
# Source files
#
set( SOURCE_FILES
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
    ./src/TopoVars.cpp
		./src/Trigger.cpp
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
    ./src/TopoVars.h
		./src/Trigger.h
)

include_directories(
	${dds_topology_lib_BINARY_DIR}
	${MiscCommon_LOCATION}
	${Boost_INCLUDE_DIRS}
)

#
# dds_topology_lib executable
#
if (CMAKE_GENERATOR STREQUAL "Xcode")
#    add_executable(dds_topology_lib ${SOURCE_FILES} ${HEADER_FILES})
    add_library(dds_topology_lib SHARED ${SOURCE_FILES} ${HEADER_FILES})
else (CMAKE_GENERATOR STREQUAL "Xcode")
#    add_executable(dds_topology_lib ${SOURCE_FILES})
    add_library(dds_topology_lib SHARED ${SOURCE_FILES})
endif (CMAKE_GENERATOR STREQUAL "Xcode")

target_link_libraries ( dds_topology_lib
	dds_user_defaults_lib
	${Boost_FILESYSTEM_LIBRARY}
    ${Boost_REGEX_LIBRARY}
)

# API headers
#target_include_directories(dds_topology_lib INTERFACE ${dds_topology_lib_SOURCE_DIR}/src)


target_include_directories(dds_topology_lib
    INTERFACE
        ${dds_topology_lib_SOURCE_DIR}/src
	PRIVATE
		$<TARGET_PROPERTY:dds_user_defaults_lib,INTERFACE_INCLUDE_DIRECTORIES>
)

install(TARGETS dds_topology_lib DESTINATION lib)

#
# Unit tests
#
if (BUILD_TESTS)
    message (STATUS "Build dds_topology_lib unit tests - YES")
    add_subdirectory ( ${dds_topology_lib_SOURCE_DIR}/tests )
else (BUILD_TESTS)
    message (STATUS "Build dds_topology_lib unit tests - NO")
endif (BUILD_TESTS)
