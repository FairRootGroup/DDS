# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project(dds-agent)

configure_file(src/version.h.in ${PROJECT_BINARY_DIR}/version.h @ONLY)

set(SOURCE_FILES
	src/main.cpp
	src/CommanderChannel.cpp
	src/AgentConnectionManager.cpp
	src/SMIntercomChannel.cpp
	src/SMFWChannel.cpp
	src/SMCommanderChannel.cpp
  src/SMLeaderChannel.cpp
)

set(HEADER_FILES
	src/Options.h
	src/CommanderChannel.h
	src/AgentConnectionManager.h
	src/SMIntercomChannel.h
	src/SMFWChannel.h
	src/SMCommanderChannel.h
  src/SMLeaderChannel.h
)

if(APPLE)
  set(RT_LIB )
elseif(UNIX)
  # On Linux we have to link against rt library which is needed for boost::interprocess
  set(RT_LIB rt)
endif()

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  MiscCommon
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
  RUNTIME DESTINATION bin
)

if(BUILD_TESTS)
  message(STATUS "Build ${PROJECT_NAME} unit tests - YES")
  # add_subdirectory(tests)
else()
  message(STATUS "Build ${PROJECT_NAME} unit tests - NO")
endif()
