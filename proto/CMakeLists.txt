# Copyright 2022 GSI, Inc. All rights reserved.
#
#
project(dds_proto_lib)

#
# All protobuf IDL files
#

file(GLOB PROTOBUF_FILELIST "${CMAKE_CURRENT_SOURCE_DIR}/*.proto")
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTOBUF_FILELIST})

SET_SOURCE_FILES_PROPERTIES(${PROTO_SRC} ${PROTO_INCL} PROPERTIES GENERATED TRUE)

add_library(${PROJECT_NAME} ${PROTO_SRCS} ${PROTO_INCL})

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_${PROJECT_CXX_STANDARD})

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  protobuf::libprotobuf
)
#
# Set Target version
#
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION  ${DDS_VERSION}
    # SOVERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
  ${Protobuf_INCLUDE_DIRS}
)

install(TARGETS ${PROJECT_NAME}
  EXPORT DDSTargets
  LIBRARY DESTINATION "${PROJECT_INSTALL_LIBDIR}"
)
