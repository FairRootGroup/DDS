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
	src/TopoCore.cpp
	src/TopoTask.cpp
	src/TopoCollection.cpp
	src/TopoGroup.cpp
	src/TopoElement.cpp
	src/TopoContainer.cpp
	src/TopoParserXML.cpp
    src/TopoRequirement.cpp
    src/TopoVars.cpp
	src/TopoTrigger.cpp
)

set(HEADER_FILES
	src/TopoProperty.h
	src/TopoBase.h
	src/TopoUtils.h
	src/TopoFactory.h
	src/TopoCore.h
	src/TopoTask.h
	src/TopoCollection.h
	src/TopoGroup.h
	src/TopoElement.h
	src/TopoContainer.h
	src/TopoParserXML.h
    src/TopoRequirement.h
    src/TopoVars.h
	src/TopoTrigger.h
    src/TopoDef.h
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
  $<INSTALL_INTERFACE:include>
)

install(TARGETS ${PROJECT_NAME}
  EXPORT DDSTargets
  LIBRARY DESTINATION lib
)

if(BUILD_TESTS)
  message(STATUS "Build ${PROJECT_NAME} unit tests - YES")
  add_subdirectory(tests)
else()
  message(STATUS "Build ${PROJECT_NAME} unit tests - NO")
endif()
