# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project( dds-ncf-tests )


#=============================================================================
add_executable(dds-ncf-test test_ncf.cpp )

target_link_libraries (
    dds-ncf-test
	dds_ncf
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_THREAD_LIBRARY}
	${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
)

install(TARGETS dds-ncf-test DESTINATION tests)
