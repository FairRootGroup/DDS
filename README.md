# The Dynamic Deployment System (DDS)
DDS - is a tool-set that automates and significantly simplifies a deployment of user defined processes and their dependencies on any resource management system using a given topology.

## Basic concepts
DDS:
- implements a single-responsibility-principle command line tool-set and APIs,
- treats users’ tasks as black boxes,
- doesn’t depend on RMS (provides deployment via SSH, when no RMS is present),
- supports workers behind FireWalls (outgoing connection from WNs required),
- doesn’t require pre-installation on WNs,
- deploys private facilities on demand with isolated sandboxes,
- provides a key-value properties propagation service for tasks,
- provides a rules based execution of tasks.

## Building 3rd-party

### BOOST on macOS

~~~~~~~
./bootstrap.sh --prefix=[INSTALL DIR] --without-icu
./b2 --disable-icu --prefix=[INSTALL DIR] -j8 --layout=system threading=multi link=shared,static cxxflags="-std=c++11 -stdlib=libc++ -Wthread-safety" linkflags="-lc++ -stdlib=libc++" install

cd [INSTALL_DIR]/lib
find . -name '*.dylib' -exec bash -c 'nm=$(basename $1);install_name_tool $1 -id [INSTALL_DIR]/lib/$nm' -- {} \;
~~~~~~~

### clang-format on macOS

[LLVM binary builds](http://releases.llvm.org/download.html)

### devtools-3 on CentOS

[Instructions](https://www.softwarecollections.org/en/scls/rhscl/devtoolset-3/)
