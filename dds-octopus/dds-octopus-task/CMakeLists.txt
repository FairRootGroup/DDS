# Copyright 2016 GSI, Inc. All rights reserved.
#
#
project(dds-octopus-task)

configure_file(src/version.h.in ${PROJECT_BINARY_DIR}/version.h @ONLY)

set(SOURCE_FILES
	src/main.cpp
	src/Task.cpp
	${dds-octopus_SOURCE_DIR}/src/OctopusProtocol.cpp
)

set(HEADER_FILES
	src/Task.h
	${dds-octopus_SOURCE_DIR}/src/OctopusProtocol.h
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(${PROJECT_NAME}
  MiscCommon
	dds_intercom_lib
  Boost::boost
  Boost::program_options
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
  $<BUILD_INTERFACE:${dds-octopus_SOURCE_DIR}/src>
)

install(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION tests/dds-octopus
)
