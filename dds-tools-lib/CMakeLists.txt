# Copyright 2019 GSI, Inc. All rights reserved.
#
#
project(dds_tools_lib)

set(SOURCE_FILES
    src/Tools.cpp
    src/ToolsProtocol.cpp
)

set(HEADER_FILES
)

set(HEADER_FILES_EXT
    src/Tools.h
    src/ToolsProtocol.h
    src/ToolsProtocolCore.h
)

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${HEADER_FILES} ${HEADER_FILES_EXT})

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_${PROJECT_CXX_STANDARD})
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
    dds_intercom_lib
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
