# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project(SSHTunnel)

add_library(${PROJECT_NAME} SHARED
  SSHTunnel.cpp
  SSHTunnel.h
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  MiscCommon
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
)

install(TARGETS ${PROJECT_NAME}
  LIBRARY DESTINATION lib
)
install(PROGRAMS ssh-tunnel DESTINATION bin/private)
