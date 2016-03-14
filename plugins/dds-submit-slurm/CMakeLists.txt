# Copyright 2016 GSI, Inc. All rights reserved.
#
#
project( dds-submit-slurm )

#
# Where to lookup modules
#
set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

#
# configure files
# 
configure_file( ${PROJECT_SOURCE_DIR}/src/version.h.in ${PROJECT_BINARY_DIR}/src/version.h @ONLY )

#
# Source files
#
set( SOURCE_FILES
	./src/main.cpp
)
set( SRC_HDRS
)


include_directories(
	${PROJECT_BINARY_DIR}/src
	${MiscCommon_LOCATION}
	${Boost_INCLUDE_DIRS} )

#
# dds-slurm executable
#
if (CMAKE_GENERATOR STREQUAL "Xcode")
    add_executable(dds-submit-slurm ${SOURCE_FILES} ${SRC_HDRS})
else (CMAKE_GENERATOR STREQUAL "Xcode")
    add_executable(dds-submit-slurm ${SOURCE_FILES})
endif (CMAKE_GENERATOR STREQUAL "Xcode")

target_link_libraries (
	dds-submit-slurm
	dds_user_defaults_lib
	dds_intercom_lib
	pipe_log_engine
	${Boost_PROGRAM_OPTIONS_LIBRARY}
	${Boost_LOG_LIBRARY}
	${Boost_LOG_SETUP_LIBRARY}
	${Boost_THREAD_LIBRARY}
)

target_include_directories(dds-submit-slurm
	PRIVATE
		$<TARGET_PROPERTY:dds_user_defaults_lib,INTERFACE_INCLUDE_DIRECTORIES>
	PRIVATE
		$<TARGET_PROPERTY:dds_intercom_lib,INTERFACE_INCLUDE_DIRECTORIES>
)

install(TARGETS dds-submit-slurm DESTINATION plugins/dds-submit-slurm)
install(PROGRAMS ${dds-submit-slurm_SOURCE_DIR}/src/dds-submit-slurm-worker DESTINATION plugins/dds-submit-slurm)
install(PROGRAMS ${dds-submit-slurm_SOURCE_DIR}/src/job.slurm.in DESTINATION plugins/dds-submit-slurm)

