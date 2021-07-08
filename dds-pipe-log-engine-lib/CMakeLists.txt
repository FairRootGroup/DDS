# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project(dds_pipe_log_engine_lib)

add_library(dds_pipe_log_engine_lib SHARED
  src/PipeLogEngine.cpp
  src/PipeLogEngine.h
)

target_link_libraries(dds_pipe_log_engine_lib
  PUBLIC
  dds_misc_lib
  dds_user_defaults_lib
  Boost::boost
  Boost::log
  Boost::log_setup
  Boost::thread
  Boost::system
)

target_include_directories(dds_pipe_log_engine_lib
  PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
)

install(TARGETS dds_pipe_log_engine_lib
  LIBRARY DESTINATION "${PROJECT_INSTALL_LIBDIR}"
)
