# Copyright 2016 GSI, Inc. All rights reserved.
#
#
project( dds-octopus )

#
# Where to lookup modules
#
set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

#
# configure files
# 
configure_file( ${dds-octopus_SOURCE_DIR}/src/version.h.in ${dds-octopus_BINARY_DIR}/version.h @ONLY )

#
# Source files
#
set( SOURCE_FILES
	./src/main.cpp
	./src/OctopusTestImpl.cpp
	./src/OctopusProtocol.cpp
	./src/TestPing.cpp
	./src/TestBigCustomCmd.cpp
)

#
# Header files
#
set( HEADER_FILES
	./src/Options.h
	./src/OctopusTestImpl.h
	./src/OctopusProtocol.h
	./src/TestPing.h
	./src/TestBigCustomCmd.h
)

include_directories(
	${dds-octopus_BINARY_DIR}
	${MiscCommon_LOCATION}
	${Boost_INCLUDE_DIRS} 
)

#
# dds-octopus executable
#
if (CMAKE_GENERATOR STREQUAL "Xcode")
    add_executable(dds-octopus ${SOURCE_FILES} ${HEADER_FILES})
else (CMAKE_GENERATOR STREQUAL "Xcode")
    add_executable(dds-octopus ${SOURCE_FILES})
endif (CMAKE_GENERATOR STREQUAL "Xcode")

target_link_libraries (
	dds-octopus
	dds_intercom_lib
	${Boost_PROGRAM_OPTIONS_LIBRARY}
)

target_include_directories(dds-octopus
    PRIVATE
        $<TARGET_PROPERTY:dds_intercom_lib,INTERFACE_INCLUDE_DIRECTORIES>
)

install(TARGETS dds-octopus DESTINATION tests/dds-octopus)
install(PROGRAMS ${dds-octopus_SOURCE_DIR}/src/dds-octopus-start DESTINATION tests/dds-octopus)
install(FILES ${dds-octopus_SOURCE_DIR}/src/octopus_topo_1.xml DESTINATION tests/dds-octopus)

add_subdirectory( dds-octopus-task )
