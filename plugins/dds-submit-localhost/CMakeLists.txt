# Copyright 2016 GSI, Inc. All rights reserved.
#
#
project(dds-submit-localhost)

add_executable(${PROJECT_NAME} src/main.cpp)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_${PROJECT_CXX_STANDARD})

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  dds_misc_lib
  dds_user_defaults_lib
  dds_intercom_lib
  dds_misc_lib
  Boost::boost
  Boost::program_options
  Boost::log
  Boost::log_setup
  Boost::thread
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR/src}>
)

install(TARGETS ${PROJECT_NAME} DESTINATION "${PROJECT_INSTALL_PLUGINSDIR}/${PROJECT_NAME}")

#if(BUILD_TESTS)
#  message(STATUS "Build ${PROJECT_NAME} unit tests - YES")
#  add_subdirectory(tests) 
#else()
#  message(STATUS "Build ${PROJECT_NAME} unit tests - NO")
#endif()
