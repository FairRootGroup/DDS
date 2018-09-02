# Copyright 2016 GSI, Inc. All rights reserved.
#
#
project(dds_ncf)

add_library(${PROJECT_NAME} SHARED
  src/ncf.cpp
  src/ncf.h
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  MiscCommon
  Boost::boost
  Boost::program_options
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
)

if(BUILD_TESTS)
  message(STATUS "Build ${PROJECT_NAME} unit tests - YES")
  add_subdirectory(tests) 
else()
  message(STATUS "Build ${PROJECT_NAME} unit tests - NO")
endif()

install(TARGETS ${PROJECT_NAME}
  LIBRARY DESTINATION lib
)
