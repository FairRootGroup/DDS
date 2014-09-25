# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project( dds-key-value-lib-tests )


include_directories(
	${Boost_INCLUDE_DIRS}
)

##################################################################
# Test_ProtocolMessage
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
)

target_include_directories(dds-key-value-lib-tests
    PRIVATE
        $<TARGET_PROPERTY:dds-key-value-lib,INTERFACE_INCLUDE_DIRECTORIES>
)

install(TARGETS dds-key-value-lib-tests DESTINATION tests)

