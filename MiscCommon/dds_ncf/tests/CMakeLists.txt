# Copyright 2014 GSI, Inc. All rights reserved.
#
#
project(dds-ncf-tests)

add_executable(${PROJECT_NAME} test_ncf.cpp)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
	dds_ncf
  Boost::unit_test_framework
)

install(TARGETS ${PROJECT_NAME} 
  RUNTIME DESTINATION tests
)
