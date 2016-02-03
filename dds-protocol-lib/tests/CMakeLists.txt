# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project( dds_protocol_lib-tests )


include_directories(
	${Boost_INCLUDE_DIRS}
)

##################################################################
# Test_ProtocolMessage
##################################################################

set( CLNT_SOURCE_FILES
   ./Test_ProtocolMessage.cpp
   ./TestCmd.cpp
)

set( CLNT_HEADER_FILES
	./TestCmd.h
)

if (CMAKE_GENERATOR STREQUAL "Xcode")
	add_executable(dds_protocol_lib-ProtocolMessage-tests ${CLNT_SOURCE_FILES} ${CLNT_HEADER_FILES})
else (CMAKE_GENERATOR STREQUAL "Xcode")
	add_executable(dds_protocol_lib-ProtocolMessage-tests ${CLNT_SOURCE_FILES})
endif (CMAKE_GENERATOR STREQUAL "Xcode")

target_link_libraries (
	dds_protocol_lib-ProtocolMessage-tests
	dds_protocol_lib
	${Boost_SYSTEM_LIBRARY}
	${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
)

target_include_directories(dds_protocol_lib-ProtocolMessage-tests
    PRIVATE
        $<TARGET_PROPERTY:dds_protocol_lib,INTERFACE_INCLUDE_DIRECTORIES>
	PRIVATE
		$<TARGET_PROPERTY:dds_user_defaults_lib,INTERFACE_INCLUDE_DIRECTORIES>
)

install(TARGETS dds_protocol_lib-ProtocolMessage-tests DESTINATION tests)
