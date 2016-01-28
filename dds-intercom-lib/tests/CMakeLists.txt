# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project( dds-intercom-lib-tests )


include_directories(
	${Boost_INCLUDE_DIRS}
)

##################################################################
# Test_KeyValue
##################################################################

set( CLNT_SOURCE_FILES
	./Test_KeyValue.cpp
)

add_executable(dds-intercom-lib-tests ${CLNT_SOURCE_FILES})

target_link_libraries (
	dds-intercom-lib-tests
	dds_intercom_lib
	${Boost_SYSTEM_LIBRARY}
	${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
	${Boost_PROGRAM_OPTIONS_LIBRARY}
)

target_include_directories(dds-intercom-lib-tests
    PRIVATE
        $<TARGET_PROPERTY:dds_intercom_lib,INTERFACE_INCLUDE_DIRECTORIES>
)

install(TARGETS dds-intercom-lib-tests DESTINATION tests)

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

##################################################################
# task-async_test_key_value.cpp
##################################################################

set( KV_ASYNC_TEST_APP_SOURCE_FILES
	./task-async_test_key_value.cpp
)

add_executable(task-async_test_key_value ${KV_ASYNC_TEST_APP_SOURCE_FILES})

target_link_libraries (
	task-async_test_key_value
	dds_intercom_lib
)

target_include_directories(task-async_test_key_value
    PRIVATE
        $<TARGET_PROPERTY:dds_intercom_lib,INTERFACE_INCLUDE_DIRECTORIES>
)

install(TARGETS task-async_test_key_value DESTINATION tests)


install(FILES ${dds-intercom-lib-tests_SOURCE_DIR}/topo_test_key-value-1.xml DESTINATION tests)
install(FILES ${dds-intercom-lib-tests_SOURCE_DIR}/topo_test_key-value-2.xml DESTINATION tests)
