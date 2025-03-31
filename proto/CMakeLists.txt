# Copyright 2022-2025 GSI, Inc. All rights reserved.
#
#
project(dds_proto_lib)

#
# All protobuf IDL files
#

file(GLOB PROTOBUF_FILELIST "${CMAKE_CURRENT_SOURCE_DIR}/*.proto")

# Create the library target first
add_library(${PROJECT_NAME})

# Try modern approach first (CONFIG mode with protobuf_generate)
if(TARGET protobuf::protoc AND COMMAND protobuf_generate)
  message(STATUS "Using modern Protobuf approach with protobuf_generate")
  
  protobuf_generate(
    TARGET ${PROJECT_NAME}
    LANGUAGE cpp
    PROTOS ${PROTOBUF_FILELIST}
  )
# If modern command not available but we have the imported targets, try to include the module
elseif(TARGET protobuf::protoc AND EXISTS "${Protobuf_DIR}/protobuf-generate.cmake")
  message(STATUS "Including protobuf-generate.cmake module from ${Protobuf_DIR}")
  
  include("${Protobuf_DIR}/protobuf-generate.cmake")
  protobuf_generate(
    TARGET ${PROJECT_NAME}
    LANGUAGE cpp
    PROTOS ${PROTOBUF_FILELIST}
  )
# Fall back to traditional approach
elseif(COMMAND protobuf_generate_cpp)
  message(STATUS "Using traditional Protobuf approach with protobuf_generate_cpp")
  
  protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTOBUF_FILELIST})
  target_sources(${PROJECT_NAME} PRIVATE ${PROTO_SRCS} ${PROTO_HDRS})
  set_source_files_properties(${PROTO_SRCS} ${PROTO_HDRS} PROPERTIES GENERATED TRUE)
# Last resort: try to find the commands in standard modules
else()
  message(STATUS "Attempting to find Protobuf commands in standard modules")
  
  if(EXISTS "${CMAKE_ROOT}/Modules/FindProtobuf.cmake")
    include("${CMAKE_ROOT}/Modules/FindProtobuf.cmake")
    
    if(COMMAND protobuf_generate_cpp)
      protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTOBUF_FILELIST})
      target_sources(${PROJECT_NAME} PRIVATE ${PROTO_SRCS} ${PROTO_HDRS})
      set_source_files_properties(${PROTO_SRCS} ${PROTO_HDRS} PROPERTIES GENERATED TRUE)
    else()
      message(FATAL_ERROR "Could not find protobuf_generate_cpp command. Please ensure Protobuf is properly installed.")
    endif()
  else()
    message(FATAL_ERROR "Could not find Protobuf CMake modules. Please ensure Protobuf is properly installed.")
  endif()
endif()

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_${PROJECT_CXX_STANDARD})

# Link with Protobuf libraries (use imported targets when available)
if(TARGET protobuf::libprotobuf)
  target_link_libraries(${PROJECT_NAME} PUBLIC protobuf::libprotobuf)
else()
  target_link_libraries(${PROJECT_NAME} PUBLIC ${Protobuf_LIBRARIES})
  target_include_directories(${PROJECT_NAME} PUBLIC ${Protobuf_INCLUDE_DIRS})
endif()

# Always include the build directory for generated files
target_include_directories(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>)

#
# Set Target version
#
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION  ${DDS_VERSION}
    # SOVERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
)

install(TARGETS ${PROJECT_NAME}
  EXPORT DDSTargets
  LIBRARY DESTINATION "${PROJECT_INSTALL_LIBDIR}"
)
