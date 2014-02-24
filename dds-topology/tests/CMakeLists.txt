#************************************************************************
#
# CMakeLists.txt
# 
# Anar Manafov A.Manafov@gsi.de
# 
#
#        version number:    $LastChangedRevision$
#        created by:        Anar Manafov
#                           2009-12-25
#        last changed by:   $LastChangedBy$ $LastChangedDate$
#
#        Copyright (c) 2009-2010 GSI GridTeam. All rights reserved.
#*************************************************************************
project( dds-topology-tests )

include_directories(${dds-topology_SOURCE_DIR}/src ${Boost_INCLUDE_DIRS})
#=============================================================================


set( SOURCE_FILES
    Test.cpp
    ${dds-topology_SOURCE_DIR}/src/DDSTopology.cpp
    ${dds-topology_SOURCE_DIR}/src/DDSTask.cpp
    ${dds-topology_SOURCE_DIR}/src/DDSTaskCollection.cpp
    ${dds-topology_SOURCE_DIR}/src/DDSTaskGroup.cpp
)
add_executable(dds-topology_tests ${SOURCE_FILES} )

target_link_libraries (
    dds-topology_tests
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
)

install(TARGETS dds-topology_tests DESTINATION tests)


# Copy schema file and test XML files to the build directory.
add_custom_target(copy_to_build ALL
            COMMAND ${CMAKE_COMMAND} -E copy ${TOPOLOGY_SCHEMA_PATH} "${dds-topology-tests_BINARY_DIR}/Debug"
            COMMAND ${CMAKE_COMMAND} -E copy ${dds-topology-tests_SOURCE_DIR}/test_topology.xml "${dds-topology-tests_BINARY_DIR}/Debug"
            COMMENT "Copying XML Schema file and test topology XML to build directory."
            VERBATIM)
