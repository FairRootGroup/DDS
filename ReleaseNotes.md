# Release Notes

## [3.18.0] - 2026-03-18

### � Bug Fixes

#### Undefined Variable in dds-user-defaults

- **`add_library(ALIAS)` used `${target_lib}` instead of `${target}`**: The alias target name expanded to `dds-user-defaults::` (empty suffix) because `target_lib` was never defined. Fixed to use the correct variable `${target}` → `dds-user-defaults::dds_user_defaults_lib`.

#### Malformed Generator Expressions in All 5 Submit Plugins

- **`${PROJECT_BINARY_DIR/src}` → `${PROJECT_BINARY_DIR}/src`**: The `/src` was inside the CMake variable braces, causing the expression to evaluate to an empty string. Affected: `dds-submit-ssh`, `dds-submit-localhost`, `dds-submit-slurm`, `dds-submit-pbs`, `dds-submit-lsf`.

#### Duplicate Link Libraries

- **`dds_misc_lib` listed twice** in `dds-commander`, `dds-submit-ssh`, and `dds-submit-localhost` — removed the duplicates.
#### CMake Library Symlink Path Matching

- **Fixed regex pattern from `/lib` to `/lib$`**: The original pattern incorrectly matched paths ending with `/lib64` or similar, preventing the symlink creation. The exact match anchor ensures the symlink is only created when the lib directory is truly outside the standard location.

#### ARM Platform Architecture Detection

- **macOS ARM64 vs Linux aarch64**: Added platform-specific architecture strings for ARM builds. macOS uses `arm64` while Linux uses `aarch64` for the same architecture family. This ensures precompiled worker binaries are correctly identified and deployed on both platforms.
### 🔧 Build Infrastructure

#### Boost.Process v1 Is Header-Only — No Library Component Needed

- **Removed `process` from Boost component search**: `Boost.Process` v1 is a header-only library, so it must not be listed as a compiled component in `find_package(Boost ... COMPONENTS ...)`. The previous configuration caused CMake to search for a `libboost_process` binary that does not exist, along with a fallback alias workaround.
- **Cleaned up `dds-misc-lib` link libraries**: `Boost::process` removed from `target_link_libraries` in `dds-misc-lib`. Include paths are already provided through the `Boost::boost` header-only interface target, so Boost.Process v1 headers remain fully accessible without any explicit link target.

#### Dev Container Enhancements

- **Locale generation**: Added en_US.UTF-8 locale generation to the dev container. This is required by Boost.Process and `std::locale` for proper string and process handling in the containerized environment.
- **Git configuration**: Added `postStartCommand` to mark the workspace as a safe directory for git operations, resolving ownership mismatches between the mounted workspace (owned by root) and the container user (dds).
- **Dependency cleanup**: Replaced openssh-client with libxml2-utils in the Dockerfile, and removed extensive SSH-agent forwarding documentation from devcontainer.json as it's no longer needed.

#### Dead Code Cleanup

- **Removed `MiscCommon_LOCATION`**: Variable pointed to a non-existent directory and was never referenced by any CMakeLists.txt.
- **Removed `IS_SET_DDS_INSTALL_PREFIX`**: Cache variable was set but never read anywhere in the project.
- **Consolidated `CMAKE_MODULE_PATH`**: Merged two separate `set()` calls (where the second silently overwrote the first) into a single assignment including all three search directories.

## [3.17.0] - 2026-03-16

### 🐛 Bug Fixes

#### Build Warning Fixes

- **Self-assignment warning in `Process.h`**: Replaced the no-op self-assignment `_filterForRealUserID = _filterForRealUserID` with `(void)_filterForRealUserID` to silence the unused-argument warning cleanly (`-Wself-assign`)
- **Deprecated `boost::asio::deadline_timer`**: Replaced all uses of the deprecated `deadline_timer` / `boost::posix_time` API with `boost::asio::system_timer` / `std::chrono` in `Process.h` and `BaseChannelImpl.h` (`-Wdeprecated-declarations`)
- **`[[nodiscard]]` warnings in Protobuf calls**: Cast the return values of `UnpackTo()` (dds-commander) and `PackFrom()` (dds-submit-slurm) to `void` to suppress `-Wunused-result`

#### Linker Error Fix — Boost.Process v2 Static Initializers

- **`dds_tools_lib-session-tests` link failure**: Removed a redundant `#include <boost/process.hpp>` from `TestSession.cpp`. On Boost 1.90 the top-level header pulls in `boost/process/v2/error.hpp` and `boost/process/v2/shell.hpp`, which contain static initializers that require the compiled `libboost_process` (v2) library — a library that is not linked in this project. The `bp` namespace alias was already provided by `Process.h` via `boost/process/v1.hpp`.

### 🔧 Build Infrastructure

#### Dev Container Support

- **New `.devcontainer/` configuration**: Added a reproducible VS Code Dev Container based on `debian:bookworm-slim` with a multi-stage Dockerfile
  - **Stage 1 (builder)**: Compiles Boost 1.90.0, abseil-cpp 20240722.1, and Protobuf v34.0 from source
  - **Stage 2 (dev)**: Ships only the pre-built artifacts alongside Clang/LLVM 21 (compiler, clangd, clang-format, lld), CMake 4.2.3, ccache, git and make
  - **Supports `linux/amd64` and `linux/arm64`** via `TARGETARCH`
  - **ccache** persisted in a named Docker volume (`dds-ccache`) for fast incremental rebuilds
  - **`postCreateCommand`** automatically runs CMake configure (with ccache launchers) on first container open
  - **VS Code extensions**: `vscode-clangd` (replaces the C/C++ IntelliSense engine) and `cmake-tools` pre-installed

## [3.16.0] - 2025-10-09

### 🎉 New Features

#### Tools API Environment Variable Support

- **Automatic Lightweight Mode**: Tools API now automatically detects and respects the `DDS_LIGHTWEIGHT_PACKAGE` environment variable
- **No More `make wn_bin`**: When using lightweight mode, you no longer need to build the worker binary package with `make wn_bin` - a huge time saver!
- **Simplified API Usage**: Users no longer need to manually set the `enable_lightweight` flag when the environment variable is set
- **Consistent Behavior**: Tools API now behaves consistently with command-line tools (`dds-session` and `dds-submit`)
- **Smaller Packages**: Worker packages reduced from ~15MB to ~50KB in lightweight mode

### 🐛 Bug Fixes

#### Critical SLURM Plugin Fix for Lightweight Mode

- **Job Submission Failure**: Fixed critical bug in SLURM plugin that caused job submissions to fail with "No partition specified or system default partition" error when using lightweight mode
- **Root Cause**: The job script template incorrectly placed executable validation code before #SBATCH directives, violating SLURM's parsing requirements. SLURM stops processing #SBATCH options when it encounters the first executable line, causing all subsequent directives (including `--partition`) to be ignored
- **Template Bug**: The placeholder `#DDS_LIGHTWEIGHT_VALIDATION` appeared in both a comment line and the code section. The `boost::replace_all()` function replaced both occurrences, breaking the comment syntax and injecting executable code before #SBATCH directives
- **Resolution**:
  - Removed lightweight validation code from the job script template entirely
  - Eliminated blank lines between #SBATCH directive placeholders
  - Validation logic moved to worker nodes where it's actually needed (DDSWorker.sh)
- **Impact**: SLURM now correctly parses all #SBATCH directives including partition specifications, resource requirements, and job options

#### Critical Worker Package Deployment Fix

- **DDSWorker.sh Logic Error**: Fixed inverted logic bug that caused worker package deployment to fail when pre-compiled binaries were present
- **Impact**: This bug prevented users from deploying full worker packages (with binaries) even though the binaries were correctly packaged
- **Resolution**: The script now correctly:
  - Extracts and uses binaries when they exist (full package mode)
  - Validates lightweight mode requirements when binaries are absent (lightweight package mode)

### 🚀 For Users

#### If You Use SLURM with Lightweight Mode

If you experienced SLURM job submission failures with errors like:

```text
Batch job submission failed: No partition specified or system default partition
```

This was caused by a critical bug in the SLURM plugin template that placed executable code before #SBATCH directives. SLURM stopped parsing directives when it encountered this code, ignoring your partition specifications and other settings.

**The fix requires rebuilding DDS:**

```bash
cd /path/to/DDS/build
make
make install
```

After rebuilding, your SLURM submissions with lightweight mode will work correctly, and all #SBATCH directives (including partition, CPU requirements, etc.) will be properly recognized.

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
