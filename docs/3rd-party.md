# Building 3rd-party

## BOOST Libs

### BOOST on macOS

```bash
./bootstrap.sh --prefix=[INSTALL DIR] --without-icu
./b2 --disable-icu --prefix=[INSTALL DIR] -j8 --layout=system threading=multi link=shared,static cxxstd=17 install

cd [INSTALL_DIR]/lib
find . -name '*.dylib' -exec bash -c 'nm=$(basename $1);install_name_tool $1 -id [INSTALL_DIR]/lib/$nm' -- {} \;
```

### BOOST on Linux

```bash
./bootstrap.sh --prefix=[INSTALL DIR] --without-icu
./b2 --disable-icu --prefix=[INSTALL DIR] -j8 --layout=system threading=multi link=shared,static cxxflags="-std=c++11" install
```

## clang-format

### clang-format on macOS

[LLVM binary builds](http://releases.llvm.org/download.html) An LLVM version 15.0.7 should be used: [Download](https://github.com/llvm/llvm-project/releases/tag/llvmorg-15.0.7)

### devtools-3 on CentOS

[Instructions](https://www.softwarecollections.org/en/scls/rhscl/devtoolset-3/)
