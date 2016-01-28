# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project( dds-custom-cmd )

#
# Where to lookup modules
#
set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

#
# configure files
#
configure_file( ${dds-custom-cmd_SOURCE_DIR}/src/version.h.in ${dds-custom-cmd_BINARY_DIR}/version.h @ONLY )

#
# Source files
#
set( SOURCE_FILES
	./src/main.cpp
)

#
# Header files
#
set( HEADER_FILES
	./src/Options.h
)

include_directories(
	${dds-custom-cmd_BINARY_DIR}
	${MiscCommon_LOCATION}
	${Boost_INCLUDE_DIRS}
)

#
# dds-custom-cmd executable
#
if (CMAKE_GENERATOR STREQUAL "Xcode")
    add_executable(dds-custom-cmd ${SOURCE_FILES} ${HEADER_FILES})
else (CMAKE_GENERATOR STREQUAL "Xcode")
    add_executable(dds-custom-cmd ${SOURCE_FILES})
endif (CMAKE_GENERATOR STREQUAL "Xcode")

target_link_libraries (
	dds-custom-cmd
	dds_intercom_lib
    dds-user-defaults-lib
	${Boost_PROGRAM_OPTIONS_LIBRARY}
	${Boost_SYSTEM_LIBRARY}
	${Boost_LOG_LIBRARY}
	${Boost_LOG_SETUP_LIBRARY}
	${Boost_THREAD_LIBRARY}
)

target_include_directories(dds-custom-cmd
    PRIVATE
		$<TARGET_PROPERTY:dds-user-defaults-lib,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE
        $<TARGET_PROPERTY:dds_intercom_lib,INTERFACE_INCLUDE_DIRECTORIES>
)

install(TARGETS dds-custom-cmd DESTINATION bin)

#
# Unit tests
#
#if (BUILD_TESTS)
#    message (STATUS "Build dds-custom-cmd unit tests - YES")
#    add_subdirectory ( ${dds-custom-cmd_SOURCE_DIR}/tests )
#else (BUILD_TESTS)
#    message (STATUS "Build dds-custom-cmd unit tests - NO")
#endif (BUILD_TESTS)
