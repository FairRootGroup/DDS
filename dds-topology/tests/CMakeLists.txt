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
project( pod-ssh-tests )

include_directories(${pod-ssh_SOURCE_DIR}/src)
#=============================================================================
add_executable(pod-ssh_test_config test_config.cpp ${pod-ssh_SOURCE_DIR}/src/config.cpp)

target_link_libraries (
    pod-ssh_test_config
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
)

install(TARGETS pod-ssh_test_config DESTINATION tests)
#=============================================================================
add_executable(pod-ssh_test_threadpool test_threadpool.cpp )

target_link_libraries (
    pod-ssh_test_threadpool
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_THREAD_LIBRARY}
)

install(TARGETS pod-ssh_test_threadpool DESTINATION tests)

