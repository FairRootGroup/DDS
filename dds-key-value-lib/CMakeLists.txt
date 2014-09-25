# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project( dds-key-value-lib )

#
# Where to lookup modules
#
set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

#
# Source files
#
set( SOURCE_FILES
	./src/KeyValue.cpp
)

set( SRC_HDRS
	./src/KeyValue.h
)

include_directories(
	${PROJECT_SOURCE_DIR}
	${MiscCommon_LOCATION}
	${Boost_INCLUDE_DIRS}
)

#
# lib
#
if (CMAKE_GENERATOR STREQUAL "Xcode")
	add_library(dds-key-value-lib SHARED ${SOURCE_FILES} ${SRC_HDRS})
else (CMAKE_GENERATOR STREQUAL "Xcode")
	add_library(dds-key-value-lib SHARED ${SOURCE_FILES})
endif (CMAKE_GENERATOR STREQUAL "Xcode")

target_link_libraries (
	dds-key-value-lib
	${Boost_SYSTEM_LIBRARY}
)

# API headers
target_include_directories(dds-key-value-lib INTERFACE ${dds-key-value-lib_SOURCE_DIR}/src)

install(TARGETS dds-key-value-lib DESTINATION lib)


#
# Unit tests
#
if (BUILD_TESTS)
	message (STATUS "Build dds-key-value-lib tests - YES")
	add_subdirectory ( ${dds-key-value-lib_SOURCE_DIR}/tests ) 
else (BUILD_TESTS)
	message (STATUS "Build dds-key-value-lib tests - NO")
endif (BUILD_TESTS)

