# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project(dds_misc-tests)

set(prefix dds_misc)
set(suffix tests)

#=============================================================================

set(test ${prefix}_MiscUtils-${suffix})
add_executable(${test} Test_MiscUtils.cpp)

target_link_libraries(${test}
  PUBLIC
  dds_misc_lib
  Boost::boost
  Boost::unit_test_framework
  Boost::system
  Boost::filesystem
)

install(TARGETS ${test} RUNTIME DESTINATION "${PROJECT_INSTALL_TESTS}")

#=============================================================================

set(test ${prefix}_MiscCli-${suffix})
add_executable(${test} Test_MiscCli.cpp)

target_link_libraries(${test}
  PUBLIC
  dds_misc_lib
  Boost::boost
  Boost::unit_test_framework
  Boost::system
  Boost::filesystem
  Boost::program_options
)

install(TARGETS ${test} RUNTIME DESTINATION "${PROJECT_INSTALL_TESTS}")

#=============================================================================

set(test ${prefix}_Process-${suffix})
add_executable(${test} Test_Process.cpp)

target_link_libraries(${test}
  PUBLIC
  dds_misc_lib
  Boost::boost
  Boost::unit_test_framework
  Boost::system
  Boost::filesystem
)

install(TARGETS ${test} RUNTIME DESTINATION "${PROJECT_INSTALL_TESTS}")

#=============================================================================

set(test ${prefix}_ProcessExecutable-${suffix})
add_executable(${test} Test_ProcessExecutable.cpp)

target_link_libraries(${test}
  PUBLIC
  dds_misc_lib
  Boost::boost
  Boost::system
  Boost::filesystem
  Boost::program_options
)

install(TARGETS ${test} RUNTIME DESTINATION "${PROJECT_INSTALL_TESTS}")

#=============================================================================

set(test ${prefix}_SysHelper-${suffix})
add_executable(${test} Test_SysHelper.cpp)

target_link_libraries(${test}
  PUBLIC
  dds_misc_lib
  Boost::boost
  Boost::unit_test_framework
  Boost::system
  Boost::filesystem
)

install(TARGETS ${test} RUNTIME DESTINATION "${PROJECT_INSTALL_TESTS}")

#=============================================================================

set(test ${prefix}_FindCfgFile-${suffix})
add_executable(${test} Test_FindCfgFile.cpp)

target_link_libraries(${test}
  PUBLIC
  dds_misc_lib
  Boost::boost
  Boost::unit_test_framework
  Boost::system
)

install(TARGETS ${test} RUNTIME DESTINATION "${PROJECT_INSTALL_TESTS}")

#=============================================================================

set(test ${prefix}_Logger-${suffix})
add_executable(${test} Test_Logger.cpp)

target_link_libraries(${test}
  PUBLIC
  dds_misc_lib
  Boost::boost
  Boost::unit_test_framework
)

install(TARGETS ${test} RUNTIME DESTINATION "${PROJECT_INSTALL_TESTS}")

#=============================================================================

set(test ${prefix}_Ncf-${suffix})
add_executable(${test} Test_Ncf.cpp)

target_link_libraries(${test}
  PUBLIC
  dds_misc_lib
  Boost::unit_test_framework
)

install(TARGETS ${test} RUNTIME DESTINATION "${PROJECT_INSTALL_TESTS}")

