# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project( dds-submit-ssh-tests )

include_directories(${dds-submit-ssh_SOURCE_DIR}/src)
#=============================================================================
add_executable(dds-submit-ssh_test_threadpool test_threadpool.cpp )

target_link_libraries (
    dds-submit-ssh_test_threadpool
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_THREAD_LIBRARY}
)

install(TARGETS dds-submit-ssh_test_threadpool DESTINATION tests)

