# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project(dds_pipe_log_engine_lib)

add_library(${PROJECT_NAME} SHARED
  src/PipeLogEngine.cpp
  src/PipeLogEngine.h
)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_${PROJECT_CXX_STANDARD})
#
# Set Target version
#
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION  ${DDS_VERSION}
    # SOVERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  dds_misc_lib
  dds_user_defaults_lib
  Boost::boost
  Boost::log
  Boost::log_setup
  Boost::thread
  Boost::system
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
)

install(TARGETS ${PROJECT_NAME}
  LIBRARY DESTINATION "${PROJECT_INSTALL_LIBDIR}"
)
