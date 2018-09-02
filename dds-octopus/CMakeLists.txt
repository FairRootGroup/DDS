# Copyright 2016 GSI, Inc. All rights reserved.
#
#
project(dds-octopus)

configure_file(src/version.h.in ${PROJECT_BINARY_DIR}/version.h @ONLY)

set(SOURCE_FILES
	src/main.cpp
	src/OctopusTestImpl.cpp
	src/OctopusProtocol.cpp
	src/TestPing.cpp
	src/TestBigCustomCmd.cpp
)

set( HEADER_FILES
	src/Options.h
	src/OctopusTestImpl.h
	src/OctopusProtocol.h
	src/TestPing.h
	src/TestBigCustomCmd.h
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  MiscCommon
	dds_intercom_lib
  Boost::boost
  Boost::program_options
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
)

install(TARGETS  ${PROJECT_NAME}        DESTINATION tests/${PROJECT_NAME})
install(PROGRAMS src/dds-octopus-start  DESTINATION tests/${PROJECT_NAME})
install(FILES    src/octopus_topo_1.xml DESTINATION tests/${PROJECT_NAME})

add_subdirectory(dds-octopus-task)
