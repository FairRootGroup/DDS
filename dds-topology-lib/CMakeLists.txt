# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project(dds_topology_lib)

set(SOURCE_FILES
  src/TopoProperty.cpp
  src/TopoBase.cpp
  src/TopoUtils.cpp
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
  src/Topology.cpp
  src/TopoCreatorCore.cpp
  src/TopoCreator.cpp
  src/TopoAsset.cpp
)

set(HEADER_FILES
  src/TopoUtils.h
  src/TopoCore.h
  src/TopoParserXML.h
  src/TopoCreatorCore.h
)

set(HEADER_FILES_EXT
  src/Topology.h
  src/TopoCollection.h
  src/TopoContainer.h
  src/TopoElement.h
  src/TopoBase.h
  src/TopoDef.h
  src/TopoRequirement.h
  src/TopoTask.h
  src/TopoProperty.h
  src/TopoVars.h
  src/TopoTrigger.h
  src/TopoGroup.h
  src/TopoCreator.h
  src/TopoAsset.h
)

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${HEADER_FILES} ${HEADER_FILES_EXT})
#
# Set Target version
#
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION  ${DDS_VERSION}
    # SOVERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
)

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
  LIBRARY DESTINATION "${PROJECT_INSTALL_LIBDIR}"
)
install(FILES ${HEADER_FILES_EXT} DESTINATION "${PROJECT_INSTALL_INCLUDEDIR}")

if(BUILD_TESTS)
  message(STATUS "Build ${PROJECT_NAME} unit tests - YES")
  add_subdirectory(tests)
else()
  message(STATUS "Build ${PROJECT_NAME} unit tests - NO")
endif()
