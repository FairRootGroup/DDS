# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project(dds-user-defaults)

configure_file(src/version.h.in ${PROJECT_BINARY_DIR}/version.h @ONLY)

set(LIB_SOURCE_FILES
  src/UserDefaults.cpp
)

set(SOURCE_FILES
  src/main.cpp
  ${LIB_SOURCE_FILES}
)

set(HEADER_FILES
  src/UserDefaults.h
  src/Options.h
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(${PROJECT_NAME}
  PUBLIC
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  MiscCommon
  Boost::boost
  Boost::program_options
  Boost::system
  Boost::filesystem
)

add_library(dds_user_defaults_lib SHARED ${LIB_SOURCE_FILES} ${HEADER_FILES})

# we want to have the same name for the library and for the exe
set_target_properties(dds_user_defaults_lib PROPERTIES
  OUTPUT_NAME ${PROJECT_NAME}
)

target_include_directories(dds_user_defaults_lib
  PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
  $<INSTALL_INTERFACE:include>
)

target_link_libraries(dds_user_defaults_lib
  PUBLIC
  MiscCommon
  Boost::boost
  Boost::program_options
  Boost::system
  Boost::filesystem
)

install(TARGETS ${PROJECT_NAME} dds_user_defaults_lib
  EXPORT DDSTargets
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
)

if(BUILD_TESTS)
  message(STATUS "Build ${PROJECT_NAME} unit tests - YES")
  # add_subdirectory(tests) 
else()
  message(STATUS "Build ${PROJECT_NAME} unit tests - NO")
endif()
