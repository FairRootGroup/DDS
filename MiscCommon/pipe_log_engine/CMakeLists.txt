# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project(pipe_log_engine)

add_library(pipe_log_engine SHARED
  logEngine.cpp
  logEngine.h
)

target_link_libraries(pipe_log_engine
  PUBLIC
  MiscCommon
	dds_user_defaults_lib
  Boost::boost
	Boost::log
  Boost::log_setup
  Boost::thread
  Boost::system
)

target_include_directories(pipe_log_engine
  PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
)

install(TARGETS pipe_log_engine
  LIBRARY DESTINATION "${PROJECT_INSTALL_LIBDIR}"
)
