# Installation

## Step 1: Get the source

* **From DDS git repository**

   ```shell
   git clone https://github.com/FairRootGroup/DDS.git DDS-master
   cd ./DDS-master
   ```

* **From DDS source tarball**
Unpack DDS tarball:

   ```shell
   tar -xzvf DDS-X.Y.Z-Source.tar.gz
   ```

   The tar command will created a new directory ```./DDS-X.Y.Z-Source```, where ```X.Y.Z``` represents a version of DDS.

   ```shell
   cd ./DDS-X.Y.Z-Source
   ```

## Step 2: Configure the source

You can adjust some configuration settings in the BuildSetup.cmake bootstrap file. The following is a list of variables:

| Variable | Variable |
|----------|----------|
| CMAKE_INSTALL_PREFIX | Install path prefix, prepended onto install directories.(default ```$HOME/DDS/<DDS_Version>```)|
| CMAKE_BUILD_TYPE | Set cmake build type. Possible options are: **None**, **Debug**, **Release**, **RelWithDebInfo**, **MinSizeRel** (default **Release**)|
| BUILD_DOCUMENTATION | Build source code documentation. Possible options are: **ON**/**OFF** (default **OFF**)|
| BUILD_TESTS | Build DDS tests. Possible options are: **ON**/**OFF** (default **OFF**)|

Now, prepare a build directory for an out-of-source build and configure the source:

```shell
mkdir build
cd build
cmake -C ../BuildSetup.cmake ..
```

**Note**  
If for some reason, for example a missing dependency, configuration failed. After you get the issue fixed, right before starting the cmake command it is recommended to delete everything in the build directory recursively. This will guaranty a clean build every time the source configuration is restarted.

## Step 3: Build and install

Issue the following commands to build and install DDS:

```shell
make -j
make install
```

### Installation Prefix

Please note, that by default DDS will be installed in ```$HOME/DDS/X.Y.Z```, where ```X.Y.Z``` is a version of DDS. However users can change this behavior by setting the install prefix path in the bootstrap script BuildSetup.cmake. Just uncomment the setting of *CMAKE_INSTALL_PREFIX* variable and change dummy *MY_PATH_HERE* to a desired path.

### WN package

Users have a possibility to additionally build DDS worker package for the local platform. In case if you have same OS types on all of the target machines and don't want to use WN packages from the DDS binary repository, just issue:

```shell
make -j wn_bin
make install
```

The commands will build and install a DDS worker package for the given platform.

We also recommend to build boost without [the icu library](http://site.icu-project.org/) support. This will reduce the size of the WN package dramatically. The following is boost build options you can use to switch of icu:

```shell
./bootstrap.sh --without-icu ...
./b2 --disable-icu ...
```

## Step 4: DDS Environment

In order to enable DDS's environment you need to source the DDS_env.sh script. Change to your newly installed DDS directory and issue:

```shell
cd [DDS INSTALL DIRECTORY]
source DDS_env.sh
```

You need to source this script every time before using DDS in a new system shell. Simplify it by sourcing the script in your bash profile.
