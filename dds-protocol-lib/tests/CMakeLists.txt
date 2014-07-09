# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project( dds-protocol-lib-tests )

#
# Source files
#
set( SRV_SOURCE_FILES
	./server.cpp
)
set( CLNT_SOURCE_FILES
	./client.cpp
)

include_directories(
	${Boost_INCLUDE_DIRS}
)

add_executable(dds-protocol-lib-server-tests ${SRV_SOURCE_FILES})

target_link_libraries (
	dds-protocol-lib-server-tests
	dds-protocol-lib
	${Boost_SYSTEM_LIBRARY}
)

target_include_directories(dds-protocol-lib-server-tests
    PRIVATE
        $<TARGET_PROPERTY:dds-protocol-lib,INTERFACE_INCLUDE_DIRECTORIES>
)

install(TARGETS dds-protocol-lib-server-tests DESTINATION tests)



add_executable(dds-protocol-lib-client-tests ${CLNT_SOURCE_FILES})

target_link_libraries (
	dds-protocol-lib-client-tests
	dds-protocol-lib
	${Boost_SYSTEM_LIBRARY}
)

target_include_directories(dds-protocol-lib-client-tests
    PRIVATE
        $<TARGET_PROPERTY:dds-protocol-lib,INTERFACE_INCLUDE_DIRECTORIES>
)

install(TARGETS dds-protocol-lib-client-tests DESTINATION tests)

