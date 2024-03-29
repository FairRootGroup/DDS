# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project(dds-agent)

set(SOURCE_FILES
	src/main.cpp
	src/CommanderChannel.cpp
	src/AgentConnectionManager.cpp
    src/SMIntercomChannel.cpp
)

set(HEADER_FILES
	src/Options.h
	src/CommanderChannel.h
	src/AgentConnectionManager.h
    src/SMIntercomChannel.h
)

if(APPLE)
  set(RT_LIB )
elseif(UNIX)
  # On Linux we have to link against rt library which is needed for boost::interprocess
  set(RT_LIB rt)
endif()

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_${PROJECT_CXX_STANDARD})

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  dds_misc_lib
  dds_protocol_lib
  dds_topology_lib
  dds_user_defaults_lib
  dds_intercom_lib
  Boost::boost
  Boost::program_options
  Boost::system
  Boost::log
  Boost::log_setup
  Boost::thread
  Boost::filesystem
  ${RT_LIB}
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
)

install(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION "${PROJECT_INSTALL_BINDIR}"
)

if(BUILD_TESTS)
  message(STATUS "Build ${PROJECT_NAME} unit tests - YES")
  # add_subdirectory(tests)
else()
  message(STATUS "Build ${PROJECT_NAME} unit tests - NO")
endif()
