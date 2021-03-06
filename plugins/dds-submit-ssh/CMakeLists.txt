# Copyright 2016 GSI, Inc. All rights reserved.
#
#
project(dds-submit-ssh)

set(SOURCE_FILES
	src/main.cpp
	src/worker.cpp
)

set(SRC_HDRS
	src/local_types.h
	src/worker.h
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${SRC_HDRS})

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  MiscCommon
	dds_user_defaults_lib
  dds_intercom_lib
	dds_ncf
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

install(TARGETS ${PROJECT_NAME} DESTINATION "${PROJECT_INSTALL_PLUGINSDIR}/${PROJECT_NAME}")
install(PROGRAMS src/dds-submit-ssh-worker DESTINATION "${PROJECT_INSTALL_PLUGINSDIR}/${PROJECT_NAME}")

if(BUILD_TESTS)
  message(STATUS "Build ${PROJECT_NAME} unit tests - YES")
#  add_subdirectory(tests) 
else()
  message(STATUS "Build ${PROJECT_NAME} unit tests - NO")
endif()
