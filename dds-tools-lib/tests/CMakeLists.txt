# Copyright 2019 GSI, Inc. All rights reserved.
#
#
project(dds_tools_lib-tests)

#=============================================================================

add_executable(dds_tools_lib-tests Test.cpp)

target_link_libraries(dds_tools_lib-tests
  PUBLIC
  	dds_tools_lib
	dds_user_defaults_lib
	Boost::unit_test_framework
	Boost::filesystem
)

install(TARGETS
  dds_tools_lib-tests

  RUNTIME DESTINATION tests
)

install(FILES 
  test_protocol_1.json

  DESTINATION tests
)