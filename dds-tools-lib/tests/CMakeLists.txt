# Copyright 2019 GSI, Inc. All rights reserved.
#
#
project(dds_tools_lib-tests)

#=============================================================================

add_executable(dds_tools_lib-protocol-tests TestProtocol.cpp)

target_link_libraries(dds_tools_lib-protocol-tests
  PUBLIC
    dds_topology_lib
  	dds_tools_lib
	dds_user_defaults_lib
	Boost::unit_test_framework
	Boost::filesystem
)

install(TARGETS
  dds_tools_lib-protocol-tests

  RUNTIME DESTINATION "${PROJECT_INSTALL_TESTS}"
)

install(FILES 
  test_protocol_1.json

  DESTINATION "${PROJECT_INSTALL_TESTS}"
)


#=============================================================================

add_executable(dds_tools_lib-session-tests TestSession.cpp)

target_link_libraries(dds_tools_lib-session-tests
  PUBLIC
    dds_topology_lib
  	dds_tools_lib
	dds_user_defaults_lib
	Boost::unit_test_framework
	Boost::filesystem
)

install(TARGETS
  dds_tools_lib-session-tests

  RUNTIME DESTINATION "${PROJECT_INSTALL_TESTS}"
)
