# Copyright 2018 GSI, Inc. All rights reserved.
#
#
project(dds-session)

set(SOURCE_FILES
  src/main.cpp
  src/Start.cpp
  src/Stop.cpp
)

set(HEADER_FILES
  src/Options.h
  src/Start.h
  src/Stop.h
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_${PROJECT_CXX_STANDARD})

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  dds_misc_lib
  dds_user_defaults_lib
  dds_protocol_lib
  dds_tools_lib
  Boost::boost
  Boost::program_options
  Boost::log
  Boost::log_setup
  Boost::thread
  Boost::filesystem
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
)

install(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION "${PROJECT_INSTALL_BINDIR}"
)

#if(BUILD_TESTS)
#  message(STATUS "Build ${PROJECT_NAME} unit tests - YES")
#  add_subdirectory(tests) 
#else()
#  message(STATUS "Build ${PROJECT_NAME} unit tests - NO")
#endif()
