# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project( dds_protocol_lib )

#
# Where to lookup modules
#
set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

#
# configure files
#
configure_file( ${dds_protocol_lib_SOURCE_DIR}/src/version.h.in ${dds_protocol_lib_BINARY_DIR}/version.h @ONLY )

#
# Source files
#
set( SOURCE_FILES
	./src/ProtocolCommands.cpp
	./src/ProtocolMessage.cpp
	./src/BasicCmd.cpp
	./src/AgentsInfoCmd.cpp
	./src/SimpleMsgCmd.cpp
	./src/UUIDCmd.cpp
	./src/AssignUserTaskCmd.cpp
	./src/BinaryAttachmentCmd.cpp
	./src/HostInfoCmd.cpp
	./src/SubmitCmd.cpp
	./src/VersionCmd.cpp
	./src/BinaryAttachmentReceivedCmd.cpp
	./src/BinaryAttachmentStartCmd.cpp
	./src/UpdateKeyCmd.cpp
    ./src/UpdateKeyErrorCmd.cpp
	./src/DeleteKeyCmd.cpp
	./src/ProgressCmd.cpp
	./src/UserTaskDoneCmd.cpp
	./src/GetPropValuesCmd.cpp
	./src/StatImpl.cpp
    ./src/CustomCmdCmd.cpp
    ./src/UpdateTopologyCmd.cpp
)

set( SRC_HDRS
	./src/ProtocolCommands.h
	./src/ProtocolMessage.h
	./src/BaseChannelImpl.h
	./src/BaseSMChannelImpl.h
	./src/ClientChannelImpl.h
	./src/ServerChannelImpl.h
	./src/ChannelEventHandlersImpl.h
	./src/ChannelMessageHandlersImpl.h
	./src/CommandAttachmentImpl.h
	./src/ConnectionManagerImpl.h
	./src/BasicCmd.h
	./src/AgentsInfoCmd.h
	./src/SimpleMsgCmd.h
	./src/UUIDCmd.h
	./src/AssignUserTaskCmd.h
	./src/BinaryAttachmentCmd.h
	./src/HostInfoCmd.h
	./src/SubmitCmd.h
	./src/VersionCmd.h
	./src/BinaryAttachmentReceivedCmd.h
	./src/BinaryAttachmentStartCmd.h
	./src/UpdateKeyCmd.h
    ./src/UpdateKeyErrorCmd.h
	./src/DeleteKeyCmd.h
	./src/ProgressCmd.h
	./src/UserTaskDoneCmd.h
	./src/GetPropValuesCmd.h
	./src/StatImpl.h
    ./src/CustomCmdCmd.h
    ./src/UpdateTopologyCmd.h
    ./src/BaseEventHandlersImpl.h
)

include_directories(
	${PROJECT_SOURCE_DIR}
	${MiscCommon_LOCATION}
	${Boost_INCLUDE_DIRS}
)

#
# lib
#
if (CMAKE_GENERATOR STREQUAL "Xcode")
	add_library(dds_protocol_lib SHARED ${SOURCE_FILES} ${SRC_HDRS})
else (CMAKE_GENERATOR STREQUAL "Xcode")
	add_library(dds_protocol_lib SHARED ${SOURCE_FILES})
endif (CMAKE_GENERATOR STREQUAL "Xcode")

target_link_libraries (
	dds_protocol_lib
	${Boost_SYSTEM_LIBRARY}
    ${Boost_LOG_LIBRARY}
    ${Boost_LOG_SETUP_LIBRARY}
)

target_include_directories(dds_protocol_lib
	PRIVATE
		$<TARGET_PROPERTY:dds_user_defaults_lib,INTERFACE_INCLUDE_DIRECTORIES>
)

# API headers
target_include_directories(dds_protocol_lib INTERFACE ${dds_protocol_lib_SOURCE_DIR}/src)

install(TARGETS dds_protocol_lib DESTINATION lib)


#
# Unit tests
#
if (BUILD_TESTS)
	message (STATUS "Build dds_protocol_lib tests - YES")
	add_subdirectory ( ${dds_protocol_lib_SOURCE_DIR}/tests )
else (BUILD_TESTS)
	message (STATUS "Build dds_protocol_lib tests - NO")
endif (BUILD_TESTS)
