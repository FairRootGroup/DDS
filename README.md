# DDS
Dynamic Deployment System

## Requirements

*  a C++11 compiler
*  cmake 2.8.11+
*  BOOST 1.54+ (built by a C++11 compiler, with C++11 enabled)
*  OS: Mac OSX, Linux

## Install
### Build from source

~~~~~~~~~~~~~~~~~~
cd <DDS_SOURCE_DIR>
mkdir build
cd build
cmake -C ../BuildSetup.cmake ..
make -j install
~~~~~~~~~~~~~~~~~~

Optionally you can call the following to build a DDS worker package if the machines you want to deploy and the machine you build on are the same.
If you skip this step, then DDS will use the default precompiled packages from the dds web site.

~~~~~~~~~~~~~~~~~~
make -j wn_bin
make -j install
~~~~~~~~~~~~~~~~~~

Please note, that by default DDS will be installed in $HOME/DDS/X.Y.Z, where X.Y.Z is a version of DDS. However users can change this behaviour by setting the install prefix path in the bootstrap script BuildSetup.cmake. Just uncomment the setting of CMAKE_INSTALL_PREFIX variable and change dummy MY_PATH_HERE to a desired path.

## Quick Start

### Initialise DDS environment

### Start DDS server
