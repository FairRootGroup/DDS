# Release Notes

## [Unreleased]

### 🎉 New Features

#### Tools API Environment Variable Support

- **Automatic Lightweight Mode**: Tools API now automatically detects and respects the `DDS_LIGHTWEIGHT_PACKAGE` environment variable
- **No More `make wn_bin`**: When using lightweight mode, you no longer need to build the worker binary package with `make wn_bin` - a huge time saver!
- **Simplified API Usage**: Users no longer need to manually set the `enable_lightweight` flag when the environment variable is set
- **Consistent Behavior**: Tools API now behaves consistently with command-line tools (`dds-session` and `dds-submit`)
- **Smaller Packages**: Worker packages reduced from ~15MB to ~50KB in lightweight mode

### 🐛 Bug Fixes

#### Critical Worker Package Deployment Fix

- **DDSWorker.sh Logic Error**: Fixed inverted logic bug that caused worker package deployment to fail when pre-compiled binaries were present
- **Impact**: This bug prevented users from deploying full worker packages (with binaries) even though the binaries were correctly packaged
- **Resolution**: The script now correctly:
  - Extracts and uses binaries when they exist (full package mode)
  - Validates lightweight mode requirements when binaries are absent (lightweight package mode)

### 🚀 For Users

#### If You Use Tools API

Before this fix, you had to explicitly set the lightweight flag:

```cpp
submitInfo.setFlag(SSubmitRequestData::ESubmitRequestFlags::enable_lightweight, true);
```

Now, simply set the environment variable before running your application:

```bash
export DDS_LIGHTWEIGHT_PACKAGE=1
./my_dds_app
```

The Tools API will automatically:

- Start sessions with `--lightweight` flag
- Configure submit requests for lightweight mode

#### If You Experienced Worker Package Failures

If you previously encountered errors like:

```text
Error: Can't find WN pre-compiled bin.: /path/to/dds-wrk-bin-3.14-Linux-x86_64.tar.gz
```

This was caused by the DDSWorker.sh bug and is now fixed. Your worker packages will deploy correctly regardless of whether they contain pre-compiled binaries or are in lightweight mode.

### 📝 Complete Changelog

For a complete list of all changes, see [CHANGELOG.md](CHANGELOG.md).

---

## [3.15.0] - 2025-10-08

### 🎉 New Features

#### Enhanced Documentation and User Experience

- **Comprehensive Component Documentation**: Added detailed README files for core DDS components including DDS Agent, DDS Intercom Library, and DDS Protocol Library with practical examples and usage patterns
- **DDS Agent Architecture Guide**: New comprehensive documentation covering shared memory transport, task lifecycle management, and internal architecture
- **Intercom Library Examples**: Added practical examples for master-worker coordination, pipeline processing, and event-driven coordination patterns
- **Protocol Library Guide**: Detailed explanation of protocol components, transport layers, and implementation examples
- **Session Management**: Enhanced documentation for lightweight worker package mode and environment variable configuration
- **Submit Tool Improvements**: Better documentation for the `--path` option and improved command descriptions

### 📋 Key Documentation Updates

- **dds-agent**: New comprehensive README with architecture details and usage examples
- **dds-intercom-lib**: Complete API documentation with real-world coordination patterns
- **dds-protocol-lib**: In-depth protocol documentation with transport layer explanations
- **dds-session**: Enhanced README with lightweight session examples and configuration guides
- **dds-submit**: Improved documentation including new `--path` option details

### � Build & Packaging Improvements

- **Optimized Tarball**: System libraries (libc, libm, ld-linux) are now excluded from the tarball to reduce package size and avoid conflicts with system installations

### �🚀 For Users

These documentation improvements make DDS much easier to understand and use:

- New users can quickly get started with comprehensive guides
- Advanced users can leverage detailed examples for complex coordination patterns
- System administrators have better understanding of architecture and deployment options

### 📝 Complete Changelog

For a complete list of all changes, see [CHANGELOG.md](CHANGELOG.md).

### 💡 What's Coming Next

Stay tuned for the official release which will include these documentation enhancements to improve your DDS experience.

---

## [3.14.0] - 2023-08-23

### 🎉 Major Features

#### Lightweight Worker Package Support

- **Faster Deployments**: New lightweight worker package mode reduces package size from ~15MB to ~50KB when DDS is pre-installed on worker nodes
- **Efficient Resource Usage**: Significantly improved deployment efficiency for large-scale distributed computing environments

### 🚀 Key Features

- **dds-session**: Added `--lightweight` option and `DDS_LIGHTWEIGHT_PACKAGE` environment variable support
- **dds-submit**: New lightweight mode with command-line and environment variable configuration
- **Enhanced Compatibility**: Fixed boost::process compatibility with Boost 1.89+ while maintaining backward compatibility

### 🐛 Bug Fixes

- **Boost Compatibility**: Resolved compatibility issues with Boost 1.89+ using conditional compilation
- **Worker Package Validation**: Improved startup logic to handle missing worker packages gracefully

### 💥 Breaking Changes

None in this release.

### 📝 Full Changelog

For a complete list of all changes, see [CHANGELOG.md](CHANGELOG.md).

### 🚀 How to Upgrade

1. Download the latest DDS release
2. Follow standard installation procedures
3. Use new `--lightweight` flag with `dds-session` and `dds-submit` when DDS is pre-installed on worker nodes

### 📋 Known Issues

See [GitHub Issues](https://github.com/FairRootGroup/DDS/issues) for current known issues.
