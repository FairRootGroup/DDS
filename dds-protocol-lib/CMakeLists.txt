# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project(dds_protocol_lib)

set(SOURCE_FILES
	src/ProtocolCommands.cpp
	src/ProtocolMessage.cpp
	src/BasicCmd.cpp
	src/AgentsInfoCmd.cpp
	src/SimpleMsgCmd.cpp
	src/UUIDCmd.cpp
	src/AssignUserTaskCmd.cpp
	src/BinaryAttachmentCmd.cpp
	src/HostInfoCmd.cpp
	src/SubmitCmd.cpp
	src/VersionCmd.cpp
	src/BinaryAttachmentReceivedCmd.cpp
	src/BinaryAttachmentStartCmd.cpp
	src/UpdateKeyCmd.cpp
	src/ProgressCmd.cpp
	src/UserTaskDoneCmd.cpp
	src/GetPropValuesCmd.cpp
    src/CustomCmdCmd.cpp
    src/UpdateTopologyCmd.cpp
    src/ReplyCmd.cpp
)

set(SRC_HDRS
	src/ProtocolCommands.h
	src/ProtocolMessage.h
	src/BaseChannelImpl.h
	src/BaseSMChannelImpl.h
	src/ClientChannelImpl.h
	src/ServerChannelImpl.h
	src/ChannelEventHandlersImpl.h
	src/ChannelMessageHandlersImpl.h
	src/CommandAttachmentImpl.h
	src/ConnectionManagerImpl.h
	src/BasicCmd.h
	src/AgentsInfoCmd.h
	src/SimpleMsgCmd.h
	src/UUIDCmd.h
	src/AssignUserTaskCmd.h
	src/BinaryAttachmentCmd.h
	src/HostInfoCmd.h
	src/SubmitCmd.h
	src/VersionCmd.h
	src/BinaryAttachmentReceivedCmd.h
	src/BinaryAttachmentStartCmd.h
	src/UpdateKeyCmd.h
	src/ProgressCmd.h
	src/UserTaskDoneCmd.h
	src/GetPropValuesCmd.h
    src/CustomCmdCmd.h
    src/UpdateTopologyCmd.h
    src/BaseEventHandlersImpl.h
    src/ChannelInfo.h
    src/ProtocolDef.h
    src/ReplyCmd.h
)

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${SRC_HDRS})
#
# Set Target version
#
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION  ${DDS_VERSION}
    # SOVERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  dds_misc_lib
  dds_user_defaults_lib
  Boost::boost
  Boost::system
  Boost::log
  Boost::log_setup
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
)

install(TARGETS ${PROJECT_NAME}
  EXPORT DDSTargets
  LIBRARY DESTINATION "${PROJECT_INSTALL_LIBDIR}"
)

if(BUILD_TESTS)
  message(STATUS "Build ${PROJECT_NAME} tests - YES")
	add_subdirectory(tests)
else()
  message(STATUS "Build ${PROJECT_NAME} tests - NO")
endif()
