# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project(dds-daemonize)

add_executable(${PROJECT_NAME} src/main.cpp)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_${PROJECT_CXX_STANDARD})

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  dds_misc_lib
  Boost::boost
  Boost::program_options
  Boost::system
  Boost::filesystem
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
)

install(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION "${PROJECT_INSTALL_BINDIR}"
)

if(BUILD_TESTS)
  message(STATUS "Build ${PROJECT_NAME} unit tests - YES")
  add_subdirectory(tests) 
else()
  message(STATUS "Build ${PROJECT_NAME} unit tests - NO")
endif()

