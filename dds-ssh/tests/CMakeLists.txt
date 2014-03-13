# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project( dds-ssh-tests )

include_directories(${dds-ssh_SOURCE_DIR}/src)
#=============================================================================
add_executable(dds-ssh_test_config test_config.cpp ${dds-ssh_SOURCE_DIR}/src/config.cpp)

target_link_libraries (
    dds-ssh_test_config
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
)

install(TARGETS dds-ssh_test_config DESTINATION tests)
#=============================================================================
add_executable(dds-ssh_test_threadpool test_threadpool.cpp )

target_link_libraries (
    dds-ssh_test_threadpool
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_THREAD_LIBRARY}
)

install(TARGETS dds-ssh_test_threadpool DESTINATION tests)

