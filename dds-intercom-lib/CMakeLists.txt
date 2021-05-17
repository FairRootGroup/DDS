# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project(dds_intercom_lib)

set(SOURCE_FILES
	src/Intercom.cpp
	src/dds_rms_plugin_protocol.cpp
	src/IntercomServiceCore.cpp
	src/SMAgentChannel.cpp
    src/AgentChannel.cpp
)

set(SRC_HDRS_INT
	src/IntercomServiceCore.h
	src/SMAgentChannel.h
    src/AgentChannel.h
)

set(SRC_HDRS
	src/Intercom.h
	src/IntercomErrorCodes.h
)

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${SRC_HDRS_INT} ${SRC_HDRS})

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  MiscCommon
	dds_user_defaults_lib
	dds_protocol_lib
	Boost::boost
	Boost::system
	Boost::filesystem
	Boost::log
	Boost::log_setup
	Boost::thread
	$<$<PLATFORM_ID:Linux>:rt> # Clang 9 warning/error GH-249
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
  $<INSTALL_INTERFACE:include>
  $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
)

install(TARGETS ${PROJECT_NAME}
  EXPORT DDSTargets
  LIBRARY DESTINATION "${PROJECT_INSTALL_LIBDIR}"
)
install(FILES ${SRC_HDRS} DESTINATION "${PROJECT_INSTALL_INCLUDEDIR}")

if(BUILD_TESTS)
  message(STATUS "Build ${PROJECT_NAME} tests - YES")
  add_subdirectory(tests)
else()
  message(STATUS "Build ${PROJECT_NAME} tests - NO")
endif()
