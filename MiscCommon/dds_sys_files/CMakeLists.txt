# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project(dds_sys_files)

add_library(${PROJECT_NAME} SHARED
  DDSSysFiles.cpp
  DDSSysFiles.h
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  MiscCommon
  Boost::program_options
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
)

install(TARGETS ${PROJECT_NAME}
  LIBRARY DESTINATION lib
)
