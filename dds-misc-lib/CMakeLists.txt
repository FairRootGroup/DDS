# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project(dds_misc_lib)

# BuildConstants header
set(BuildConstantsHeader ${PROJECT_BINARY_DIR}/BuildConstants.h)
set(DDS_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
set(DDS_BINDIR "${CMAKE_INSTALL_PREFIX}/${PROJECT_INSTALL_BINDIR}")
set(DDS_LIBDIR "${CMAKE_INSTALL_PREFIX}/${PROJECT_INSTALL_LIBDIR}")
configure_file(src/BuildConstants.h.in ${BuildConstantsHeader} @ONLY)

# Version header
set(VersionHeader ${PROJECT_BINARY_DIR}/Version.h)
configure_file(src/Version.h.in ${VersionHeader})

set(SOURCE_FILES
  src/SSHConfigFile.cpp
)

set(HEADER_FILES
  ${BuildConstantsHeader}
  src/CustomIterator.h
  src/MiscUtils.h
  src/def.h
  src/BoostHelper.h
  src/FindCfgFile.h
  src/SysHelper.h
  src/ErrorCode.h
  src/Process.h
  src/SessionIDFile.h
  src/INet.h
  src/CRC.h
  src/Logger.h
  src/MiscCli.h
  src/MiscSetup.h
  src/HexView.h
  src/MonitoringThread.h
  src/ConditionEvent.h
  src/TimeMeasure.h
  src/Environment.h
  src/DDSHelper.h
  src/Res.h
  src/ProgressDisplay.h
  src/stlx.h
)

set(HEADER_FILES_EXT
   src/EnvProp.h
   ${VersionHeader}
   src/SSHConfigFile.h
)

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${HEADER_FILES} ${HEADER_FILES_EXT})

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    Boost::boost
    Boost::filesystem
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
  $<INSTALL_INTERFACE:include>
)

install(TARGETS ${PROJECT_NAME}
  EXPORT DDSTargets
  LIBRARY DESTINATION "${PROJECT_INSTALL_LIBDIR}"
)
install(FILES ${HEADER_FILES_EXT} DESTINATION "${PROJECT_INSTALL_INCLUDEDIR}")

if(BUILD_TESTS)
  message(STATUS "Build ${PROJECT_NAME} unit tests - YES")
  add_subdirectory(tests)
else()
  message(STATUS "Build ${PROJECT_NAME} unit tests - NO")
endif()
