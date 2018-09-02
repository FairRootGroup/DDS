# Copyright 2016 GSI, Inc. All rights reserved.
#
#
project(dds-submit-pbs)

configure_file(src/version.h.in ${PROJECT_BINARY_DIR}/src/version.h @ONLY)

add_executable(${PROJECT_NAME} src/main.cpp)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  MiscCommon
	dds_user_defaults_lib
	dds_intercom_lib
	pipe_log_engine
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

install(TARGETS ${PROJECT_NAME} DESTINATION plugins/${PROJECT_NAME})
install(PROGRAMS
  src/dds-submit-pbs-worker
  src/job.pbs.in

  DESTINATION plugins/${PROJECT_NAME}
)

#if(BUILD_TESTS)
#  message(STATUS "Build ${PROJECT_NAME} unit tests - YES")
#  add_subdirectory(tests) 
#else()
#  message(STATUS "Build ${PROJECT_NAME} unit tests - NO")
#endif()
