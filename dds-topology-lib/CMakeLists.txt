# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project(dds_topology_lib)

configure_file(src/version.h.in ${PROJECT_BINARY_DIR}/version.h @ONLY)

set(SOURCE_FILES
	src/TopoProperty.cpp
	src/TopoBase.cpp
	src/TopoUtils.cpp
	src/TopoFactory.cpp
	src/Topology.cpp
	src/Task.cpp
	src/TaskCollection.cpp
	src/TaskGroup.cpp
	src/TopoElement.cpp
	src/TaskContainer.cpp
	src/TopologyParserXML.cpp
	src/TopoIndex.cpp
  src/Requirement.cpp
  src/TopoVars.cpp
	src/Trigger.cpp
)

set(HEADER_FILES
	src/TopoProperty.h
	src/TopoBase.h
	src/TopoUtils.h
	src/TopoFactory.h
	src/Topology.h
	src/Task.h
	src/TaskCollection.h
	src/TaskGroup.h
	src/TopoElement.h
	src/TaskContainer.h
	src/TopologyParserXML.h
	src/TopoIndex.h
  src/Requirement.h
  src/TopoVars.h
	src/Trigger.h
)

# add_executable(dds_topology_lib ${SOURCE_FILES} ${HEADER_FILES})
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(${PROJECT_NAME}
  PUBLIC
	dds_user_defaults_lib
  Boost::boost
  Boost::filesystem
  Boost::regex
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
)

install(TARGETS ${PROJECT_NAME}
  LIBRARY DESTINATION lib
)

if(BUILD_TESTS)
  message(STATUS "Build ${PROJECT_NAME} unit tests - YES")
  add_subdirectory(tests)
else()
  message(STATUS "Build ${PROJECT_NAME} unit tests - NO")
endif()
