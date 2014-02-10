project( dds-topology  )

#
# Where to lookup modules
#
set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

#
# configure files
# 
configure_file( ${PROJECT_SOURCE_DIR}/src/version.h.in ${PROJECT_BINARY_DIR}/src/version.h @ONLY )
#
# API Docs
#
#if(DOXYGEN_FOUND)
#   configure_file( ${PROJECT_SOURCE_DIR}/Doxyfile.in ${PROJECT_BINARY_DIR}/Doxyfile @ONLY )
#   add_custom_target(pod-ssh-api-docs ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/Doxyfile
#      COMMENT "Generating pod-ssh API docs")
#endif(DOXYGEN_FOUND)

#
# Source files
#
#set( SOURCE_FILES
#    ./src/main.cpp
#    ./src/config.cpp
#    ./src/worker.cpp
#)
#set( SRC_HDRS
#     ./src/config.h
#     ./src/local_types.h
#     ./src/threadPool.h
#     ./src/worker.h
#)


#include_directories(
#    ${PROJECT_BINARY_DIR}/src
#    ${MiscCommon_LOCATION}
#    ${MiscCommon_LOCATION}/pipe_log_engine
#    ${Boost_INCLUDE_DIRS} )

#
# dds-topology executable
#
#if (CMAKE_GENERATOR STREQUAL "Xcode")
#    add_executable(pod-ssh ${SOURCE_FILES} ${SRC_HDRS})
#else (CMAKE_GENERATOR STREQUAL "Xcode")
#    add_executable(pod-ssh ${SOURCE_FILES})
#endif (CMAKE_GENERATOR STREQUAL "Xcode")

#target_link_libraries (
#    pod-ssh
#    pipe_log_engine
#    pod_sys_files
#    ${Boost_PROGRAM_OPTIONS_LIBRARY}
#    ${Boost_THREAD_LIBRARY}
#)

#install(TARGETS pod-ssh DESTINATION bin)

#
# Unit tests
#
#if (BUILD_TESTS)
#    message (STATUS "Build pod-ssh unit tests - YES")
#    add_subdirectory ( ${pod-ssh_SOURCE_DIR}/tests ) 
#else (BUILD_TESTS)
#    message (STATUS "Build pod-ssh unit tests - NO")
#endif (BUILD_TESTS)

