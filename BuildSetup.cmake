# Copyright 2014 GSI, Inc. All rights reserved.
#
#
# issue the following commands in order to build DDS from source:
#  1) mkdir build
#  2) cd build
#  3) cmake -C ../BuildSetup.cmake ..
#  4) make [-j] wn_bin  /This step is optional. Only if you need to generate WN package on your local system./
#  5) make [-j] install
#

#
# Install prefix
#
#SET (CMAKE_INSTALL_PREFIX "MY_PATH_HERE" CACHE PATH "Install path prefix, prepended onto install directories." FORCE)

#
# BUILD TYPE
#
# set cmake build type, default value is: RelWithDebInfo
# possible options are: None Debug Release RelWithDebInfo MinSizeRel
set( CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build" FORCE )

#set(CMAKE_VERBOSE_MAKEFILE TRUE CACHE BOOL "This is useful for debugging only." FORCE)

# This is needed if you want to use gLite plug-in and have several version of BOOST installed
#set( Boost_USE_MULTITHREADED OFF CACHE BOOL "BOOST" FORCE )


#
# Disable the search for boost-cmake.
# This is needed to be able to build DDS worker packages.
# Otherwise cmake look up algorithms can't find boost libs on wn_bin build
#
#set(Boost_NO_BOOST_CMAKE TRUE CACHE BOOL "" FORCE)


#
# Create boost (libboost_*) symlinks in DDS_LOCATION/lib/
#
set( CREATE_BOOST_SYMLINKS FALSE CACHE BOOL "Create boost (libboost_*) symlinks in DDS_LOCATION/lib/" FORCE )

#
# Documentation
#
# set( BUILD_DOCUMENTATION ON CACHE BOOL "Build source code documentation" FORCE )

#
# Tests
#
set( BUILD_TESTS ON CACHE BOOL "Build DDS tests" FORCE )

#
# C++ Standard
#
# set(CMAKE_CXX_STANDARD 20 CACHE STRING "Default value for CXX_STANDARD property of targets")
