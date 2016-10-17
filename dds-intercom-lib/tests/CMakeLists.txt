# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project( dds_intercom_lib-tests )


include_directories(
	${Boost_INCLUDE_DIRS}
)

##################################################################
# Unit Test
##################################################################

set( CLNT_SOURCE_FILES
	./Test.cpp
)

add_executable(dds_intercom_lib-tests ${CLNT_SOURCE_FILES})

target_link_libraries (
	dds_intercom_lib-tests
	dds_intercom_lib
	${Boost_SYSTEM_LIBRARY}
	${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
	${Boost_PROGRAM_OPTIONS_LIBRARY}
)

target_include_directories(dds_intercom_lib-tests
    PRIVATE
        $<TARGET_PROPERTY:dds_intercom_lib,INTERFACE_INCLUDE_DIRECTORIES>
)

install(TARGETS dds_intercom_lib-tests DESTINATION tests)

##################################################################
# task-test_key_value
##################################################################

set( KV_APP_SOURCE_FILES
	./task-test_key_value.cpp
)

add_executable(task-test_key_value ${KV_APP_SOURCE_FILES})

target_link_libraries (
	task-test_key_value
	dds_intercom_lib
)

target_include_directories(task-test_key_value
    PRIVATE
        $<TARGET_PROPERTY:dds_intercom_lib,INTERFACE_INCLUDE_DIRECTORIES>
)

install(TARGETS task-test_key_value DESTINATION tests)


install(FILES ${dds_intercom_lib-tests_SOURCE_DIR}/topo_test_key-value-1.xml DESTINATION tests)
install(FILES ${dds_intercom_lib-tests_SOURCE_DIR}/topo_test_key-value-2.xml DESTINATION tests)
