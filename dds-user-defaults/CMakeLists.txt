# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project(dds-user-defaults)

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
  dds_misc_lib
  Boost::boost
  Boost::program_options
  Boost::system
  Boost::filesystem
)

set(target "dds_user_defaults_lib")
add_library(${target} SHARED ${LIB_SOURCE_FILES} ${HEADER_FILES})
add_library(${PROJECT_NAME}::${target_lib} ALIAS ${target})
#
# Set Target version
#
set_target_properties(${target} PROPERTIES
    VERSION  ${DDS_VERSION}
    # SOVERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
)

# we want to have the same name for the library and for the exe
set_target_properties(${target} PROPERTIES
  OUTPUT_NAME ${PROJECT_NAME}
)

target_include_directories(${target}
  PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
  $<INSTALL_INTERFACE:include>
)

target_link_libraries(${target}
  PUBLIC
  dds_misc_lib
  Boost::boost
  Boost::program_options
  Boost::system
  Boost::filesystem
)

install(TARGETS ${PROJECT_NAME} ${target}
  EXPORT DDSTargets
  RUNTIME DESTINATION "${PROJECT_INSTALL_BINDIR}"
  LIBRARY DESTINATION "${PROJECT_INSTALL_LIBDIR}"
)

if(BUILD_TESTS)
  message(STATUS "Build ${PROJECT_NAME} unit tests - YES")
  add_subdirectory(tests) 
else()
  message(STATUS "Build ${PROJECT_NAME} unit tests - NO")
endif()
