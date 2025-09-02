# Release Notes

## Unreleased

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

### 🚀 For Users

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