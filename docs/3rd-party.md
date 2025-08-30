# Building 3rd-party Dependencies

This document provides instructions for building the third-party dependencies required for DDS development.

## BOOST Libraries

DDS requires BOOST 1.75 or higher with C++17 support.

### BOOST on macOS

```bash
./bootstrap.sh --prefix=[INSTALL_DIR] --without-icu
./b2 --disable-icu --prefix=[INSTALL_DIR] -j8 --layout=system threading=multi link=shared,static cxxstd=17 install

cd [INSTALL_DIR]/lib
find . -name '*.dylib' -exec bash -c 'nm=$(basename $1);install_name_tool $1 -id [INSTALL_DIR]/lib/$nm' -- {} \;
```

### BOOST on Linux

```bash
./bootstrap.sh --prefix=[INSTALL_DIR] --without-icu
./b2 --disable-icu --prefix=[INSTALL_DIR] -j8 --layout=system threading=multi link=shared,static cxxstd=17 install
```

**Note:** We recommend building boost without [ICU library](http://site.icu-project.org/) support to reduce the size of worker node packages.

## Code Formatting

### clang-format

DDS uses clang-format for consistent code formatting.

#### clang-format on macOS

Download from [LLVM binary builds](http://releases.llvm.org/download.html).  
DDS requires LLVM version 15.0.7: [Download](https://github.com/llvm/llvm-project/releases/tag/llvmorg-15.0.7)

#### clang-format on Linux

Install via package manager or download from LLVM releases.

## Development Tools

### devtools-3 on CentOS/RHEL

For older CentOS/RHEL systems, you may need newer development tools:
[Software Collections Instructions](https://www.softwarecollections.org/en/scls/rhscl/devtoolset-3/)

## Build Requirements Summary

* C++17 compiler (GCC 7+, Clang 5+, or equivalent)
* CMake 3.19+
* BOOST 1.75+ (built with C++17 support)
* clang-format 15.0.7 (for development)
