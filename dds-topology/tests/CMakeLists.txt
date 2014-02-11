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
add_executable(dds-topology_tests Test.cpp )

target_link_libraries (
    dds-topology_tests
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
)

install(TARGETS dds-topology_tests DESTINATION tests)

