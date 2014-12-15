# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project( dds-key-value-lib-tests )


include_directories(
	${Boost_INCLUDE_DIRS}
)

##################################################################
# Test_KeyValue
##################################################################

set( CLNT_SOURCE_FILES
	./Test_KeyValue.cpp
)

add_executable(dds-key-value-lib-tests ${CLNT_SOURCE_FILES})

target_link_libraries (
	dds-key-value-lib-tests
	dds-key-value-lib
	${Boost_SYSTEM_LIBRARY}
	${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
	${Boost_PROGRAM_OPTIONS_LIBRARY}
)

target_include_directories(dds-key-value-lib-tests
    PRIVATE
        $<TARGET_PROPERTY:dds-key-value-lib,INTERFACE_INCLUDE_DIRECTORIES>
)

install(TARGETS dds-key-value-lib-tests DESTINATION tests)

##################################################################
# task-test_key_value
##################################################################

set( KV_APP_SOURCE_FILES
	./task-test_key_value.cpp
)

add_executable(task-test_key_value ${KV_APP_SOURCE_FILES})

target_link_libraries (
	task-test_key_value
	dds-key-value-lib
)

target_include_directories(task-test_key_value
    PRIVATE
        $<TARGET_PROPERTY:dds-key-value-lib,INTERFACE_INCLUDE_DIRECTORIES>
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
	dds-key-value-lib
)

target_include_directories(task-async_test_key_value
    PRIVATE
        $<TARGET_PROPERTY:dds-key-value-lib,INTERFACE_INCLUDE_DIRECTORIES>
)

install(TARGETS task-async_test_key_value DESTINATION tests)
