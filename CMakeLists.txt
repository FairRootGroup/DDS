# Copyright 2014-2020 GSI, Inc. All rights reserved.
#
#
project( DDS )
set(BUILD_SHARED_LIBS ON)

set(DDS_USER_DEFAULTS_CFG_VERSION "0.4")
set(DDS_PROTOCOL_VERSION "2")

#
# Check if cmake has the required version
#
cmake_minimum_required( VERSION 3.11.0 FATAL_ERROR )

string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWER)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

set(CMAKE_CXX_STANDARD 17 CACHE STRING "C++ standard version to use (default is 17)")
message(STATUS "Requiring C++${CMAKE_CXX_STANDARD}")

# Version
#
# take the version string from git and write it to a version file
# or take it from the version file if git is unavailable (a package distribution)
# Keep the version file in the source directory to satisfy the source package.
#
# Note:
# The version file is generated in the source dir as it needs to be included into the source package (cpack)
set( DDS_VERSION_FILE ${CMAKE_SOURCE_DIR}/version )
if( EXISTS "${CMAKE_SOURCE_DIR}/.git" )

	# The command produce a verion like "X.Y.Z.gHASH"
	# The hash suffix is "-g" + 7-char abbreviation for the tip commit of parent.
	# The "g" prefix stands for "git" and is used to allow describing the version of a software depending on the SCM the software is managed with.
  execute_process(COMMAND git --git-dir "${CMAKE_SOURCE_DIR}/.git" describe --match "[0-9]*\\.[0-9]*" --abbrev=7 HEAD
                  COMMAND sed -e "s/-/./g"
                  OUTPUT_VARIABLE DDS_VERSION
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process( COMMAND bash -c "echo \"${DDS_VERSION}\" > ${DDS_VERSION_FILE}" )

else( EXISTS "${CMAKE_SOURCE_DIR}/.git" )

  execute_process(COMMAND cat ${DDS_VERSION_FILE}
                  OUTPUT_VARIABLE DDS_VERSION
                  OUTPUT_STRIP_TRAILING_WHITESPACE)

endif( EXISTS "${CMAKE_SOURCE_DIR}/.git" )
message(STATUS "Building DDS version: ${DDS_VERSION}")

#
# Install directory
#
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set (CMAKE_INSTALL_PREFIX "$ENV{HOME}/DDS/${DDS_VERSION}" CACHE PATH "Install path prefix, prepended onto install directories." FORCE)
endif (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

#
# NOTE: the GNUInstallDirs should be included after the CMAKE_INSTALL_PREFIX is set.
# Otherwise the CMAKE_INSTALL_FULL_* variables will not respect the custom prefix.
#
#
# Set appropriate defaults for install subdirectories.
# Especially for correct packaging.
# Define CMAKE_INSTALL_*DIR and CMAKE_INSTALL_FULL_*DIR variables
include(GNUInstallDirs)

# Define install dirs
set(PROJECT_INSTALL_BINDIR ${CMAKE_INSTALL_BINDIR})
set(PROJECT_INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR})
set(PROJECT_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME_LOWER})
set(PROJECT_INSTALL_DATADIR ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME_LOWER})
set(PROJECT_INSTALL_TESTS tests)
set(PROJECT_INSTALL_DOCDIR ${CMAKE_INSTALL_DATADIR}/doc/${PROJECT_NAME_LOWER})
set(PROJECT_INSTALL_SYSCONFDIR ${CMAKE_INSTALL_SYSCONFDIR})
set(PROJECT_INSTALL_PLUGINSDIR plugins)
set(PROJECT_INSTALL_TUTORIALSDIR tutorials)

#
# Options
#
option(BUILD_DOCUMENTATION "Build documentation" OFF)
# Create boost (libboost_*) symlinks in DDS_LOCATION/lib/
option(CREATE_BOOST_SYMLINKS "Create boost (libboost_*) symlinks in DDS_LOCATION/lib/" OFF)
# tests
option(BUILD_TESTS "Build DDS unit tests" OFF)
# modules
option(BUILD_dds-commander "Build dds-commander" ON)
option(BUILD_dds-topology "Build dds-topology" ON)
option(BUILD_dds_topology_lib "Build dds_topology_lib" ON)
option(BUILD_dds-agent "Build dds-agent" ON)
option(BUILD_dds-user-defaults "Build dds-user-defaults" ON)
option(BUILD_dds-info "Build dds-info" ON)
option(BUILD_dds-submit "Build dds-submit" ON)
option(BUILD_dds-agent-cmd "Build dds-agent-cmd" ON)
option(BUILD_dds-tutorials "Build dds-tutorials" ON)
option(BUILD_dds-custom-cmd "Build dds-custom-cmd" ON)
option(BUILD_dds_intercom_lib "Build dds_intercom_lib" ON)
option(BUILD_dds-submit-ssh "Build dds-submit-ssh" ON)
option(BUILD_dds-submit-localhost "Build dds-submit-localhost" ON)
option(BUILD_dds-submit-slurm "Build dds-submit-slurm" ON)
option(BUILD_dds-submit-pbs "Build dds-submit-pbs" ON)
option(BUILD_dds-submit-lsf "Build dds-submit-lsf" ON)
option(BUILD_dds-session "Build dds-session" ON)
option(BUILD_dds_tools_lib "Build dds-tools-lib" ON)
option(BUILD_dds_pipe_log_engine_lib "Build dds-pipe-log-engine-lib" ON)
#
# Custom targets
#
find_package(Doxygen)
if(DOXYGEN_FOUND)
	message(STATUS "Build API docs - YES")
	#
	# API Documentation output directory
	#
    set(DOC_OUTPUT_DIR ${CMAKE_BINARY_DIR}/documentation/api-docs)
    file(MAKE_DIRECTORY ${DOC_OUTPUT_DIR})

	configure_file( ${PROJECT_SOURCE_DIR}/doxyfile.in ${PROJECT_BINARY_DIR}/doxyfile @ONLY )
	add_custom_target( api-docs
		${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/doxyfile
		WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
		COMMENT "Generating API docs" )

	add_custom_target( upload-api-docs
		COMMAND rsync -avz --whole-file --progress --chmod=Du=rwx,Dgo=rx,Fu=rw,Fog=r -e ssh  ${DOC_OUTPUT_DIR} ddswww@lxi001.gsi.de:~/web-docs/doc
		COMMENT "Upload API documentation to DDS's web server")
	add_dependencies( upload-api-docs api-docs )
endif(DOXYGEN_FOUND)

# Source Code Formatting using clang-format
add_custom_target( format-code
       COMMAND ${CMAKE_SOURCE_DIR}/utils/update_format.sh ${CMAKE_SOURCE_DIR}
       COMMENT "Updating source code formatting.")

#
# This is needed to properly install PoD modules
#
set(IS_SET_DDS_INSTALL_PREFIX 1 CACHE INTERNAL "")
#
# Where to lookup modules
#
set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR};${CMAKE_SOURCE_DIR}/cmake")
#
# MiscCommon location
#
set(MiscCommon_LOCATION ${CMAKE_SOURCE_DIR}/MiscCommon)
set(MiscCommon_LOCATION
    ${MiscCommon_LOCATION}
    ${MiscCommon_LOCATION}/pipe_log_engine
)
#
# Custom compiler definitions
#
# If process is killed or crased it can leave opened and locked interprocess mutex. It leads to hanging boost::interprocess::message_queue::timed_send function. The function tries to write to the queue which is locked by the mutex from the killed process. BOOST implements a workaround flag - BOOST_INTERPROCESS_ENABLE_TIMEOUT_WHEN_LOCKING. It forces the boost::interprocess to use timed mutexes instead of a simple ones.
add_compile_definitions(BOOST_INTERPROCESS_ENABLE_TIMEOUT_WHEN_LOCKING)
# boost::interprocess mutexes timeout duration.
add_compile_definitions(BOOST_INTERPROCESS_TIMEOUT_WHEN_LOCKING_DURATION_MS=5000)
# lots of warnings and all warnings as errors
add_compile_options(-Wall -Wextra)

#
# additional compiler and linker flags
#
if(APPLE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -Wthread-safety")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lc++")
elseif(UNIX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
	set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} "-Wl,--enable-new-dtags")
	set(CMAKE_SHARED_LINKER_FLAGS ${CMAKE_SHARED_LINKER_FLAGS} "-Wl,--enable-new-dtags")
endif()

set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH "${PROJECT_INSTALL_LIBDIR}")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
list(FIND "${CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES}" "${CMAKE_INSTALL_FULL_LIBDIR}" isSystemDir)
if("${isSystemDir}" STREQUAL "-1")
   message(STATUS "Using full rpath: ${CMAKE_INSTALL_FULL_LIBDIR}")
   set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_FULL_LIBDIR}")
endif()

#
# global variables
#
set(RESOURCE_DIR "${CMAKE_SOURCE_DIR}/res")
set(TOPOLOGY_SCHEMA_PATH "${RESOURCE_DIR}/topology.xsd")

#
# "additional" files
#
SET(DDS_DOC_FILES
    ${CMAKE_SOURCE_DIR}/LICENSE
    ${CMAKE_SOURCE_DIR}/ReleaseNotes.md
)
#
# "bin" files
#
SET(DDS_BIN_FILES
	${CMAKE_SOURCE_DIR}/bin/dds-prep-worker
)

SET(DDS_BIN_PRIVATE_FILES
	${CMAKE_SOURCE_DIR}/bin/private/dds-addpayload
)
#
# "etc" files
#
SET(DDS_ETC_FILES
	${DDS_VERSION_FILE}
	${CMAKE_SOURCE_DIR}/etc/DDSWorker.sh.in
	${CMAKE_SOURCE_DIR}/etc/dds_user_task_wrapper.sh.in
)
#
# "share" files
#
SET(DDS_SHARE_FILES
     ${CMAKE_SOURCE_DIR}/res/topology.xsd
)

#
#
# Build DDS modules
#
#

#
# Search for BOOST
#
set(Boost_Components log log_setup thread program_options filesystem system regex)
if(BUILD_TESTS)
  set(Boost_Components ${Boost_Components} unit_test_framework)
endif(BUILD_TESTS)

find_package(Boost 1.67 REQUIRED COMPONENTS  ${Boost_Components})
if(Boost_FOUND)
  set(local_boost_version "${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION}")
endif(Boost_FOUND)

# DDS Misc Common
message(STATUS "Build dds_misc_lib - YES")
add_subdirectory ( dds-misc-lib )

# DDS Protocol lib
message(STATUS "Build dds_protocol_lib - YES")
add_subdirectory( dds-protocol-lib )

# dds-daemonize
message(STATUS "Build dds-daemonize - YES")
add_subdirectory( dds-daemonize )

# dds-commander
if(BUILD_dds-commander)
    message(STATUS "Build dds-commander - YES")
    add_subdirectory ( dds-commander )
endif(BUILD_dds-commander)

# dds-topology
if(BUILD_dds-topology)
    message(STATUS "Build dds-topology - YES")
    add_subdirectory ( dds-topology )
endif(BUILD_dds-topology)

# dds_topology_lib
if(BUILD_dds_topology_lib)
    message(STATUS "Build dds_topology_lib - YES")
    add_subdirectory ( dds-topology-lib )
endif(BUILD_dds_topology_lib)

# dds-agent
if(BUILD_dds-agent)
    message(STATUS "Build dds-agent - YES")
    add_subdirectory ( dds-agent )
endif(BUILD_dds-agent)

# dds-user-defaults
if(BUILD_dds-user-defaults)
    message(STATUS "Build dds-user-defaults - YES")
    add_subdirectory ( dds-user-defaults )
endif(BUILD_dds-user-defaults)

# dds-info
if(BUILD_dds-info)
    message(STATUS "Build dds-info - YES")
    add_subdirectory ( dds-info )
endif(BUILD_dds-info)

# dds-submit
if(BUILD_dds-submit)
    message(STATUS "Build dds-submit - YES")
    add_subdirectory ( dds-submit )
endif(BUILD_dds-submit)

# dds-agent-cmd
if(BUILD_dds-agent-cmd)
    message(STATUS "Build dds-agent-cmd - YES")
    add_subdirectory ( dds-agent-cmd )
endif(BUILD_dds-agent-cmd)

# dds-tutorials
if(BUILD_dds-tutorials)
    message(STATUS "Build dds-tutorials - YES")
    add_subdirectory ( dds-tutorials )
endif(BUILD_dds-tutorials)

# dds-custom-cmd
if(BUILD_dds-custom-cmd)
    message(STATUS "Build dds-custom-cmd - YES")
    add_subdirectory ( dds-custom-cmd )
endif(BUILD_dds-custom-cmd)

# dds_intercom_lib
if(BUILD_dds_intercom_lib)
    message(STATUS "Build dds_intercom_lib - YES")
    add_subdirectory ( dds-intercom-lib )
endif(BUILD_dds_intercom_lib)

# dds-submit-ssh
if(BUILD_dds-submit-ssh)
    message(STATUS "Build dds-submit-ssh - YES")
    add_subdirectory ( plugins/dds-submit-ssh )
endif(BUILD_dds-submit-ssh)

# dds-submit-localhost
if(BUILD_dds-submit-localhost)
    message(STATUS "Build dds-submit-localhost - YES")
    add_subdirectory ( plugins/dds-submit-localhost )
endif(BUILD_dds-submit-localhost)

# dds-submit-slurm
if(BUILD_dds-submit-slurm)
    message(STATUS "Build dds-submit-slurm - YES")
    add_subdirectory ( plugins/dds-submit-slurm )
endif(BUILD_dds-submit-slurm)

# dds-submit-pbs
if(BUILD_dds-submit-pbs)
    message(STATUS "Build dds-submit-pbs - YES")
    add_subdirectory ( plugins/dds-submit-pbs )
endif(BUILD_dds-submit-pbs)

# dds-submit-lsf
if(BUILD_dds-submit-lsf)
    message(STATUS "Build dds-submit-lsf - YES")
    add_subdirectory ( plugins/dds-submit-lsf )
endif(BUILD_dds-submit-lsf)

# dds-session
if(BUILD_dds-session)
    message(STATUS "Build dds-session - YES")
    add_subdirectory ( dds-session )
endif(BUILD_dds-session)

# dds_tools_lib
if(BUILD_dds_tools_lib)
    message(STATUS "Build dds_tools_lib - YES")
    add_subdirectory ( dds-tools-lib )
endif(BUILD_dds_tools_lib)

# dds-pipe-log-engine-lib
if(BUILD_dds_pipe_log_engine_lib)
    message(STATUS "Build dds_pipe_log_engine_lib - YES")
    add_subdirectory ( dds-pipe-log-engine-lib )
endif(BUILD_dds_pipe_log_engine_lib)

#
# Install
#
install(FILES ${DDS_DOC_FILES} DESTINATION "${PROJECT_INSTALL_DOCDIR}")
install(PROGRAMS ${CMAKE_SOURCE_DIR}/etc/DDS_env.sh DESTINATION .)
# install bin files
install(PROGRAMS ${DDS_BIN_FILES} DESTINATION "${PROJECT_INSTALL_BINDIR}")
install(PROGRAMS ${DDS_BIN_PRIVATE_FILES} DESTINATION "${PROJECT_INSTALL_BINDIR}/private")
# install etc files
install(FILES ${DDS_ETC_FILES} DESTINATION "${PROJECT_INSTALL_SYSCONFDIR}")
# install share files
install(FILES ${DDS_SHARE_FILES} DESTINATION share)
# tests runner
if( BUILD_TESTS )
    # Configure and install test topologies
    macro(ddsConfigTopoPropertyTest MULTIPLIER MAX_VALUE TOPO_FILE)
        set(DDS_VAR_MULTIPLIER ${MULTIPLIER})
        set(DDS_VAR_MAX_VALUE ${MAX_VALUE})
        configure_file(${CMAKE_SOURCE_DIR}/res/tests/property_test.xml.in ${CMAKE_CURRENT_BINARY_DIR}/res/tests/${TOPO_FILE} @ONLY)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/res/tests/${TOPO_FILE} DESTINATION "${PROJECT_INSTALL_TESTS}")
    endmacro()
    ddsConfigTopoPropertyTest(1 10 property_test.xml)
    ddsConfigTopoPropertyTest(2 10 property_test_up.xml)
    ddsConfigTopoPropertyTest(1 1000000 property_test_inf.xml)
    ddsConfigTopoPropertyTest(2 1000000 property_test_inf_up.xml)
    
    configure_file(run_test.sh.in ${CMAKE_CURRENT_BINARY_DIR}/run_test.sh @ONLY)
    install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/run_test.sh DESTINATION "${PROJECT_INSTALL_TESTS}")
    install(PROGRAMS ${CMAKE_SOURCE_DIR}/res/tests/run_test_property.sh DESTINATION "${PROJECT_INSTALL_TESTS}")
    install(PROGRAMS ${CMAKE_SOURCE_DIR}/res/tests/run_test_property_local.sh DESTINATION "${PROJECT_INSTALL_TESTS}")
    install(PROGRAMS ${CMAKE_SOURCE_DIR}/res/tests/run_test_property_slurm.sh DESTINATION "${PROJECT_INSTALL_TESTS}")
    install(FILES ${CMAKE_SOURCE_DIR}/res/tests/env_test.xml DESTINATION "${PROJECT_INSTALL_TESTS}")
	install(FILES ${CMAKE_SOURCE_DIR}/res/tests/sleep_test.xml DESTINATION "${PROJECT_INSTALL_TESTS}")
    install(FILES ${CMAKE_SOURCE_DIR}/res/tests/custom_cmd_test.xml DESTINATION "${PROJECT_INSTALL_TESTS}")
endif( BUILD_TESTS )


#
# Build DDS WN Package
#
if( CMAKE_SIZEOF_VOID_P EQUAL 4 )
	set(TRG_ARCH x86)
else( CMAKE_SIZEOF_VOID_P EQUAL 4 )
	set(TRG_ARCH amd64)
endif( CMAKE_SIZEOF_VOID_P EQUAL 4 )

if(APPLE)
	set( WN_PKG_NAME "dds-wrk-bin-${DDS_VERSION}-${CMAKE_SYSTEM_NAME}-universal" )
elseif(UNIX)
	set( WN_PKG_NAME "dds-wrk-bin-${DDS_VERSION}-${CMAKE_SYSTEM_NAME}-${TRG_ARCH}" )
endif(APPLE)

set( WN_PKG_DIR "${CMAKE_BINARY_DIR}/dds-wrk-bin" )
set( WN_PKG_NAME_ARCH "${WN_PKG_NAME}.tar.gz" )
set( WN_PKG_REMOTE_DIR "/u/ddswww/web-docs/releases/add/${DDS_VERSION}" )

set(DDS_BOOST_LIB_DIR ${Boost_LIBRARY_DIR})
if("${DDS_BOOST_LIB_DIR}" STREQUAL "")
	set(DDS_BOOST_LIB_DIR ${Boost_LIBRARY_DIR_RELEASE})
endif()

if(ENV{DDS_LD_LIBRARY_PATH})
  # because of SIP on macOS we can't use (DY)LD_LIBRARY_PATH.
  # But we need to search also in custom location for libstdc++ in case if user installs a custom version of gcc/clang.
  # WORKAROUND: We therefore introduce DDS_LD_LIBRARY_PATH, which user can use to specify custom library path(s)
  file(TO_CMAKE_PATH "$ENV{DDS_LD_LIBRARY_PATH}" ENV_LD_LIBRARY_PATH)
else()
  file(TO_CMAKE_PATH "$ENV{LD_LIBRARY_PATH}" ENV_LD_LIBRARY_PATH)
endif()

set(PREREQ_DIRS "$<TARGET_FILE_DIR:dds-user-defaults>::$<TARGET_FILE_DIR:dds_protocol_lib>::$<TARGET_FILE_DIR:dds_intercom_lib>::$<TARGET_FILE_DIR:dds_topology_lib>::$<TARGET_FILE_DIR:dds_misc_lib>::${DDS_BOOST_LIB_DIR}")
foreach(p IN LISTS ENV_LD_LIBRARY_PATH)
  set(PREREQ_DIRS "${PREREQ_DIRS}::${p}")
endforeach()

set(DDS_AGENT_BIN_PATH $<TARGET_FILE:dds-agent>)
set(DDS_PREREQ_SOURCE_BIN_PATH $<TARGET_FILE:dds-commander>)

set(PREREQ_DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}")

# WORKAROUND: don't generate helper script when building an xcode project
# The reason: "Evaluation file to be written multiple times for different configurations or languages with different content"
if (NOT CMAKE_GENERATOR STREQUAL "Xcode")
	#
	# WORKAROUND: the issue is configure_file currently does not appear to honor generator expressions,
	# but it is still needed to configure @...@ expressions.
	# We therefore use two-step procedure to configure a file.
	#
	# 1. Deal with @...@ configurable items:
	configure_file(${CMAKE_SOURCE_DIR}/cmake/modules/DDS_CollectPrerequisitesGen.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/cmake/modules/DDS_CollectPrerequisitesGen.cmake.configured @ONLY)
	# 2. Deal with items configured with generator expressions (variables like "$<TARGET_FILE_DIR:" evaluted on this stage):
	FILE(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/cmake/modules/DDS_CollectPrerequisitesGen.cmake
			  INPUT ${CMAKE_CURRENT_BINARY_DIR}/cmake/modules/DDS_CollectPrerequisitesGen.cmake.configured CONDITION 1)

	if( BUILD_TESTS )
		set(DDS_PREREQ_SOURCE_BIN_PATH $<TARGET_FILE:dds_topology_lib-tests>)
		#
		# WORKAROUND: the issue is configure_file currently does not appear to honor generator expressions,
		# but it is still needed to configure @...@ expressions.
		# We therefore use two-step procedure to configure a file.
		#
		# 1. Deal with @...@ configurable items:
		configure_file(${CMAKE_SOURCE_DIR}/cmake/modules/DDS_CollectPrerequisitesGen.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/cmake/modules/DDS_CollectPrerequisitesGen_Tests.cmake.configured @ONLY)
		# 2. Deal with items configured with generator expressions (variables like "$<TARGET_FILE_DIR:" evaluted on this stage):
		FILE(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/cmake/modules/DDS_CollectPrerequisitesGen_Tests.cmake
			  	INPUT ${CMAKE_CURRENT_BINARY_DIR}/cmake/modules/DDS_CollectPrerequisitesGen_Tests.cmake.configured CONDITION 1)
	endif( BUILD_TESTS )
endif()

#
# TODO: replace DDS_CollectPrerequisites.cmake by DDS_CollectPrerequisitesGen.cmake and make the script more generic
#
add_custom_target( wn_bin
	COMMAND ${CMAKE_COMMAND} -E make_directory ${WN_PKG_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:dds-agent> "${WN_PKG_DIR}"
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:dds-user-defaults> "${WN_PKG_DIR}"
	COMMAND ${CMAKE_COMMAND} -DPREREQ_DESTINATION=${WN_PKG_DIR} -DDDS_AGENT_BIN_PATH=${DDS_AGENT_BIN_PATH}
		-DPREREQ_DIRS=${PREREQ_DIRS} -DDDS_BOOST_LIB_DIR=${DDS_BOOST_LIB_DIR} -P "${CMAKE_SOURCE_DIR}/cmake/modules/DDS_CollectPrerequisites.cmake"
	COMMAND ${CMAKE_COMMAND} -E tar czf ${WN_PKG_NAME_ARCH} "${WN_PKG_DIR}"
	COMMAND chmod go+xr ${WN_PKG_NAME_ARCH}
	COMMENT "Generate WN binary package"
	VERBATIM )

add_dependencies( wn_bin
	dds-agent
	dds_user_defaults_lib
	dds_protocol_lib
	dds_intercom_lib)

add_custom_target( wn_bin_upload
	COMMAND ssh ddswww@lxi001 "mkdir -p ${WN_PKG_REMOTE_DIR} && chmod go+xr ${WN_PKG_REMOTE_DIR}"
	COMMAND scp -p ${WN_PKG_NAME_ARCH} ddswww@lxi001:${WN_PKG_REMOTE_DIR}
	COMMENT "Uploading WN binary package to DDS binary repository"
	VERBATIM )

add_dependencies( wn_bin_upload
	wn_bin )

INSTALL(FILES ${CMAKE_BINARY_DIR}/${WN_PKG_NAME_ARCH} DESTINATION "${PROJECT_INSTALL_BINDIR}/wn_bins" OPTIONAL)


# WORKAROUND: don't generate helper script when building an xcode project
# The reason: "Evaluation file to be written multiple times for different configurations or languages with different content"
if (NOT CMAKE_GENERATOR STREQUAL "Xcode")
	# WORKAROUND: create destination "lib" directory.
	# INSTALL SCRIPT is called before any other lib is installed, therefore we need to create a dest. dir.
	INSTALL(DIRECTORY DESTINATION "${PROJECT_INSTALL_LIBDIR}")
	INSTALL(SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/cmake/modules/DDS_CollectPrerequisitesGen.cmake)
	if( BUILD_TESTS )
		INSTALL(SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/cmake/modules/DDS_CollectPrerequisitesGen_Tests.cmake)
	endif( BUILD_TESTS )
endif()

#
# Package
#
SET(CPACK_GENERATOR "TGZ")
set(CPACK_SOURCE_GENERATOR "TGZ")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_SOURCE_IGNORE_FILES
    "inst_tmp"
    "etc/xpd.cf$"
    "documentation/Diagrams"
    "documentation/html_help/.*\\\\.html$"
    "documentation/html_help/.*\\\\.png$"
    "documentation/html_help/.*\\\\.css$"
    "documentation/html_help/.*\\\\.ps$"
    "documentation/api-docs"
    "app/dds-agent/doc"
    "app/dds-console/doc"
    "Logo"
    "/\\\\."
    "~$;/_"
    "^${PROJECT_SOURCE_DIR}/build"
    "CMakeFiles/"
    "CMakeCache"
    "gz$"
    "Makefile\\\\."
    ".project"
    ".cproject"
    ".settings"
    "cmake_install"
    "CPack"
    "\\\\.svn/"
    "Makefile$"
	"DDSFiles.txt$")

set( CPACK_SOURCE_PACKAGE_FILE_NAME "DDS-${DDS_VERSION}-Source")

include(CPack)

# Generate and install CMake package
include(CMakePackageConfigHelpers)
set(PACKAGE_INSTALL_DESTINATION
  ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}-${DDS_VERSION}
)
install(EXPORT ${PROJECT_NAME}Targets
  NAMESPACE ${PROJECT_NAME}::
  DESTINATION ${PACKAGE_INSTALL_DESTINATION}
  EXPORT_LINK_INTERFACE_LIBRARIES
)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
  VERSION ${DDS_VERSION}
  COMPATIBILITY AnyNewerVersion
)
configure_package_config_file(
  ${CMAKE_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
  INSTALL_DESTINATION ${PACKAGE_INSTALL_DESTINATION}
  PATH_VARS CMAKE_INSTALL_PREFIX
)
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
  DESTINATION ${PACKAGE_INSTALL_DESTINATION}
)

install(FILES ${CMAKE_SOURCE_DIR}/dds.h DESTINATION "${PROJECT_INSTALL_INCLUDEDIR}")

# Create a symlink to the lib dir if its location is outside of DDS_LOCATION.
# For example, when "cmake -DCMAKE_INSTALL_LIBDIR=..." is specified then the location of the lib dir can be different.
# Some DDS modules expects to find libs in $DDS_LOCATION/lib, therefore a link is created to keep backward compatibility.
if( NOT "${CMAKE_INSTALL_FULL_LIBDIR}" MATCHES "${CMAKE_INSTALL_PREFIX}/lib")
	install(CODE "EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_INSTALL_FULL_LIBDIR} ${CMAKE_INSTALL_PREFIX}/lib)")
endif()
