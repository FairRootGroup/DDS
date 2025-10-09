# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [3.16.0] - 2025-10-09

### Added

- **dds-tools-lib**: Tools API now respects `DDS_LIGHTWEIGHT_PACKAGE` environment variable. When set, both `CSession::create()` and `SSubmitRequestData` constructor automatically enable lightweight mode.

### Fixed

- **DDSWorker.sh**: Fixed inverted logic bug that caused worker package deployment to fail when pre-compiled binaries were present. The script now correctly handles both full packages (with binaries) and lightweight packages (without binaries).
- **dds-submit-slurm**: Fixed critical bug in lightweight mode where #SBATCH directives were ignored by SLURM scheduler, causing job submission failures with "No partition specified" error. The issue was caused by executable code appearing before #SBATCH directives in the generated job script, violating SLURM's parsing requirements. Fixed by removing the lightweight validation code from the job script template and eliminating blank lines between #SBATCH directive placeholders.

## [3.15.0] - 2025-10-08

### Added

- **General**: Improved documentation for DDS Agent, DDS Intercom Library, and DDS Protocol Library with detailed examples and usage patterns.
- **dds-agent**: Comprehensive README file detailing architecture, shared memory transport, and task lifecycle management.
- **dds-intercom-lib**: New README file with examples for master-worker coordination, pipeline processing, and event-driven coordination.
- **dds-protocol-lib**: New README file explaining protocol components, transport layers, and usage examples.
- **dds-session**: New lightweight worker package mode documentation.
- **dds-submit**: Documentation for `--path` option.

### Changed

- **General**: Enhanced README files for better clarity and user guidance.
- **General**: Excluded system libraries (libc, libm, ld-linux) from the tarball to reduce package size and avoid conflicts with system installations.
- **dds-session**: README file to include detailed examples for lightweight session usage and environment variable configuration.
- **dds-submit**: README file to include `--path` option and improved descriptions.

## [3.14.0] - 2025-08-23

### Added

- **General**: Lightweight worker package support via `--lightweight` option and `DDS_LIGHTWEIGHT_PACKAGE` environment variable. This feature allows deployment of minimal packages (~50KB) instead of full packages (~15MB) when DDS is pre-installed on worker nodes, significantly improving deployment efficiency ([GH-491](https://github.com/FairRootGroup/DDS/issues/491)).
- **dds-session**: `--lightweight` command-line option to enable lightweight worker package mode for session startup ([GH-491](https://github.com/FairRootGroup/DDS/issues/491)).
- **dds-session**: `DDS_LIGHTWEIGHT_PACKAGE` environment variable support with command-line precedence for session startup ([GH-491](https://github.com/FairRootGroup/DDS/issues/491)).
- **dds-submit**: `--lightweight` command-line option to enable lightweight worker package mode ([GH-491](https://github.com/FairRootGroup/DDS/issues/491)).
- **dds-submit**: `DDS_LIGHTWEIGHT_PACKAGE` environment variable support with command-line precedence ([GH-491](https://github.com/FairRootGroup/DDS/issues/491)).
- **dds-tools-lib**: `enable_lightweight` flag to `ESubmitRequestFlags` enum for lightweight package support ([GH-491](https://github.com/FairRootGroup/DDS/issues/491)).

### Fixed

- **General**: boost::process compatibility with Boost 1.89+ by implementing conditional compilation to use boost::process v1 API when available, maintaining backward compatibility with older Boost versions ([GH-473](https://github.com/FairRootGroup/DDS/issues/473)).

### Changed

- **dds-session**: Enhanced startup logic to skip WN package validation when in lightweight mode, allowing DDS sessions to start without requiring precompiled worker packages ([GH-491](https://github.com/FairRootGroup/DDS/issues/491)).
- **dds-session**: Improved error messaging to suggest lightweight mode as an alternative when WN packages are missing ([GH-491](https://github.com/FairRootGroup/DDS/issues/491)).
- **dds-commander**: Enhanced worker package creation to support lightweight mode based on submit request flags ([GH-491](https://github.com/FairRootGroup/DDS/issues/491)).

## [3.13.0] - 2025-03-31

### Fixed

- **General**: Protobuf detection for modern and legacy installs.

## [3.12.0] - 2025-03-03

### Added

- **General**: The DDS Submit Tools API command now provides a response with job information. Currently, this feature is supported only by the Slurm plug-in ([GH-483](https://github.com/FairRootGroup/DDS/issues/483)).
- **General**: ToolsAPI documentation.
- **dds-commander**: Implemented agent health monitoring ([GH-484](https://github.com/FairRootGroup/DDS/issues/484)).
- **dds-user-defaults**: Agent health monitoring configuration ([GH-484](https://github.com/FairRootGroup/DDS/issues/484)).

### Fixed

- **General**: Build errors with boost 1.87.
- **General**: Task termination when no child processes exist.
- **General**: Multiple stability issues.
- **General**: Unit tests.

## [3.11.0] - 2024-09-05

### Fixed

- **General**: Compilation error on clang 15.
- **General**: New-delete-type mismatch in BaseEventHandlersImpl.

## [3.10.0] - 2024-04-28

### Fixed

- **General**: A regression bug causing topology updates to fail on hash validation ([GH-480](https://github.com/FairRootGroup/DDS/issues/480)).

## [3.9.0] - 2024-04-23

### Changed

- **General**: Compressed topology files before broadcasting to agents. This significantly improves performance for large topology activations ([GH-478](https://github.com/FairRootGroup/DDS/issues/478)).
- **General**: Improved performance of the Core transport when transferring binary attachments ([GH-478](https://github.com/FairRootGroup/DDS/issues/478)).

## [3.8.0] - 2024-01-19

### Added

- **General**: 3rd-party dependency on Protobuf (min v3.15).
- **General**: Every DDS module now logs its PID, group ID, and parent PID ([GH-403](https://github.com/FairRootGroup/DDS/issues/403)).
- **General**: Support for Task Assets ([GH-406](https://github.com/FairRootGroup/DDS/issues/406)).
- **General**: Cancel running and pending SLURM jobs on DDS shutdown ([GH-429](https://github.com/FairRootGroup/DDS/issues/429)).
- **General**: Support for Apple's arm64 architecture ([GH-393](https://github.com/FairRootGroup/DDS/issues/393)).
- **General**: `$DDS_CONFIG` and `/etc/dds/DDS.cfg` are added to the DDS config search paths ([GH-458](https://github.com/FairRootGroup/DDS/issues/458)).
- **General**: DDS libraries are now decorated with an ABI version ([GH-410](https://github.com/FairRootGroup/DDS/issues/410)).
- **dds_intercom_lib**: Temporarily increased intercom message size to 2048 ([GH-440](https://github.com/FairRootGroup/DDS/issues/440)).
- **dds-session**: A data retention sanitization feature. Sessions older than the specified number of days ("server.data_retention") are automatically deleted ([GH-435](https://github.com/FairRootGroup/DDS/issues/435)).
- **dds-submit**: Users can specify a GroupName tag for each submission. This tag will be assigned to agents and can be used as a requirement in topologies ([GH-407](https://github.com/FairRootGroup/DDS/issues/407)).
- **dds-submit**: Users can provide a Submission Tag (`--submission-tag`). DDS RMS plug-ins will use this tag to name RMS jobs and directories ([GH-426](https://github.com/FairRootGroup/DDS/issues/426)).
- **dds-submit**: The command learned a new argument `--env-config/-e`. It can be used to define a custom environment script for each agent ([GH-430](https://github.com/FairRootGroup/DDS/issues/430)).
- **dds-submit**: The command learned a new argument `--min-instances`. It can be used to provide the minimum number of agents to spawn ([GH-434](https://github.com/FairRootGroup/DDS/issues/434)).
- **dds-submit**: The command learned a new argument `--enable-overbooking`. The flag instructs DDS RMS plug-ins to not specify any CPU requirement for RMS jobs ([GH-442](https://github.com/FairRootGroup/DDS/issues/442)).
- **dds-submit**: The command learned a new argument `--inline-config`. The content of this string will be added to the RMS job configuration file as is. It can be specified multiple times to add multiline options ([GH-449](https://github.com/FairRootGroup/DDS/issues/449)).
- **dds-topology**: A new groupName requirement. It can be used on tasks and collections ([GH-407](https://github.com/FairRootGroup/DDS/issues/407)).
- **dds-topology**: Open API to read/update/add topology variables. The `CTopoVars` class.
- **dds-topology**: Support for Task Assets ([GH-406](https://github.com/FairRootGroup/DDS/issues/406)).
- **dds-topology**: Custom types of Task and Collection requirements ([GH-445](https://github.com/FairRootGroup/DDS/issues/445)).
- **dds-ssh-plugin**: Support for SubmissionID ([GH-411](https://github.com/FairRootGroup/DDS/issues/411)).
- **dds-slurm-plugin**: Support for SubmissionID ([GH-411](https://github.com/FairRootGroup/DDS/issues/411)).
- **dds-slurm-plugin**: Support for the minimum number of agents to spawn ([GH-434](https://github.com/FairRootGroup/DDS/issues/434)).
- **dds-localhost-plugin**: Support for SubmissionID ([GH-411](https://github.com/FairRootGroup/DDS/issues/411)).
- **dds-tools-api**: The ability to unsubscribe from either individual events or all events of requests ([GH-382](https://github.com/FairRootGroup/DDS/issues/382)).
- **dds-tools-api**: SAgentInfoResponseData provides the agent group name ([GH-415](https://github.com/FairRootGroup/DDS/issues/415)).
- **dds-tools-api**: SSubmitRequestData supports flags. See `SSubmitRequestData::setFlag` and `SSubmitRequestData::ESubmitRequestFlags` ([GH-442](https://github.com/FairRootGroup/DDS/issues/442)).
- **dds-tools-api**: Users can define additional job RMS configuration via `SSubmitRequestData::m_inlineConfig`. It will be inlined as is into the final job script ([GH-449](https://github.com/FairRootGroup/DDS/issues/449)).
- **dds-user-defaults**: A `server.data_retention` configuration key ([GH-435](https://github.com/FairRootGroup/DDS/issues/435)).

### Fixed

- **General**: On task completion, remove agents from the agent-to-tasks mapping.
- **General**: Replaced `std::iterator` as it's deprecated (C++17).
- **General**: Tasks' working directories are set to their slot directories instead of `$DDS_LOCATION`.
- **General**: Multiple stability issues.
- **dds-agent**: Addressed potential crashes in the external process termination routines.
- **dds-agent**: Revised handling of the slots container.
- **dds-agent**: Ignored SIGTERM while performing cleaning procedures ([GH-459](https://github.com/FairRootGroup/DDS/issues/459)).
- **dds_intercom_lib**: Stability improvements.
- **dds-session**: Skipped bad or non-session directories/files when performing clean and list operations.
- **dds-topology**: Stability improvements.
- **dds-topology**: A bug that caused `dds::topology_api::CTopoCreator` to ignore task assets ([GH-452](https://github.com/FairRootGroup/DDS/issues/452)).
- **dds-topology**: Activating topology took too long when task assets were used ([GH-454](https://github.com/FairRootGroup/DDS/issues/454)).
- **dds-topology**: A bug that could cause a segfault when updating variables in topology.
- **dds-ssh-plugin**: SSH config parser was passing config files of all plug-ins ([GH-413](https://github.com/FairRootGroup/DDS/issues/413)).
- **dds-slurm-plugin**: Ensured that scancel's SIGTERM is properly handled by all job steps and their scripts ([GH-459](https://github.com/FairRootGroup/DDS/issues/459)).
- **dds-user-defaults**: A dangling reference to a temporary in the User Defaults class.
- **dds-info**: Wrong exit code when called with `--help/--version` ([GH-470](https://github.com/FairRootGroup/DDS/issues/470)).

### Changed

- **General**: Support for the C++20 standard ([GH-477](https://github.com/FairRootGroup/DDS/issues/477)).
- **General**: Bumped minimum version requirements for CMake (from 3.11.0 to 3.19) and Boost (from 1.67 to 1.75) ([GH-428](https://github.com/FairRootGroup/DDS/issues/428)).
- **General**: C++17 modernization of `EnvProp.h/env_prop` ([GH-368](https://github.com/FairRootGroup/DDS/issues/368)).
- **dds_intercom_lib**: Set debug log severity on Custom command events ([GH-424](https://github.com/FairRootGroup/DDS/issues/424)).
- **dds-submit**: Increased the WN package builder timeout interval from 15 to 30 seconds ([GH-468](https://github.com/FairRootGroup/DDS/issues/468)).
- **dds-submit**: Improved validation of the WN package builder ([GH-468](https://github.com/FairRootGroup/DDS/issues/468)).
- **dds-slurm-plugin**: Replaced array job submission with nodes requirement ([GH-430](https://github.com/FairRootGroup/DDS/issues/430)).
- **dds-slurm-plugin**: Removed `#SBATCH --ntasks-per-node=1` ([GH-444](https://github.com/FairRootGroup/DDS/issues/444)).
- **dds-slurm-plugin**: The `#SBATCH --cpus-per-task=%DDS_NSLOTS%` requirement can now be disabled by providing the "enable-overbooking" flag (ToolsAPI or dds-submit) ([GH-442](https://github.com/FairRootGroup/DDS/issues/442)).
- **dds-slurm-plugin**: Prevented job termination when downing a single node of the job allocation ([GH-450](https://github.com/FairRootGroup/DDS/issues/450)).
- **dds-tools-api**: Logs of user processes that use Tools API are now moved to the DDS root log directory instead of the sessions directory.
- **dds-tools-api**: `CSession::waitForNumAgents` is renamed to `CSession::waitForNumSlots` ([GH-439](https://github.com/FairRootGroup/DDS/issues/439)).
- **dds-user-defaults**: Bumped the version to 0.5.
- **dds-agent-cmd**: getlog: Logs are now tarred without their source directory structure - as a flat stack of files ([GH-369](https://github.com/FairRootGroup/DDS/issues/369)).
- **dds-agent-cmd**: getlog: The command outputs the destination directory where downloaded archives will be stored. Also fixed the command's description ([GH-369](https://github.com/FairRootGroup/DDS/issues/369)).

## [3.6.0] - 2022-01-11

### Added

- **General**: A cmake option "CREATE_BOOST_SYMLINKS", which enables creation of boost (libboost_*) symlinks in `$DDS_LOCATION/lib/`. Default is OFF ([GH-199](https://github.com/FairRootGroup/DDS/issues/199), [GH-357](https://github.com/FairRootGroup/DDS/issues/357)).
- **General**: Setup DDS environment and create default config file in code. No need to exec DDS_env.sh script anymore ([GH-350](https://github.com/FairRootGroup/DDS/issues/350)).
- **General**: Add a onTaskDone event to ToolsAPI. Using ToolsAPI users are able now to subscribe and receive onTaskDone events whenever a user tasks exists ([GH-370](https://github.com/FairRootGroup/DDS/issues/370)).
- **General**: Refactor and open DDS SSH config API to the users. Rename `ncf` to `SSHConfigFile` ([GH-340](https://github.com/FairRootGroup/DDS/issues/340)).
- **General**: New command line option `--slot-list` of `dds-info` performing `SSlotInfoRequest` ([GH-374](https://github.com/FairRootGroup/DDS/issues/374)).
- **General**: DDS agent monitors available disk space and if the (configurable) threshold is reached, it will trigger a self-shutdown ([GH-392](https://github.com/FairRootGroup/DDS/issues/392)).
- **dds-tools-api**: `CSession::userDefaultsGetValueForKey` - returns a configuration value for a given configuration key.
- **dds-tools-api**: A new `SSlotInfoRequest` request returns a list of all active slots details ([GH-374](https://github.com/FairRootGroup/DDS/issues/374)).
- **dds-tools-api**: Task path to `OnTaskDone` reply.
- **dds-tools-api**: `Topology` request returns extended info of each activated or stopped task via new `STopologyResponseData` data class.
- **dds-tools-api**: A new `SAgentCommandRequest` request to shutdown agents by ID or by slotID ([GH-399](https://github.com/FairRootGroup/DDS/issues/399)).
- **dds-topology**: New `std::istream` based APIs.
- **dds-topology**: New `CTopology::getRuntimeTask` and `CTopology::getRuntimeCollection` methods which take either ID or runtime path as input.
- **dds-topology**: Task ID to `STopoRuntimeTask` and collection ID to `STopoRuntimeCollection`.
- **dds-topology**: On topology update/stop a task done event is now only sent when all processes of the given task are actually exited or killed ([GH-360](https://github.com/FairRootGroup/DDS/issues/360)).
- **dds-topology**: Scheduler supports multiple requirements for task or collection ([GH-395](https://github.com/FairRootGroup/DDS/issues/395)).
- **dds-slurm-plugin**: A couple of fixes in Slurm plugin found on a Virgo cluster.
- **dds-slurm-plugin**: Fixed path to demonised log file. This log is created if plug-in failed to start.
- **dds-ssh-plugin**: dds-submit-ssh doesn't exit if an exception is raised ([GH-363](https://github.com/FairRootGroup/DDS/issues/363)).
- **dds-user-defaults**: dds-user-defaults learned a new global option "agent.access_permissions". This option forces the given file mode on agent side files. At the moment it's only applied to user task log files (stdout and stderr) ([GH-389](https://github.com/FairRootGroup/DDS/issues/389)).
- **dds-user-defaults**: dds-user-defaults learned a new global option "agent.disk_space_threshold". The agent will trigger a self-shutdown if the free disk space is below this threshold ([GH-392](https://github.com/FairRootGroup/DDS/issues/392)).

### Fixed

- **General**: In some edge cases a topology update, performed during an intensive key-value exchange, can lead to a segmentation fault.
- **General**: When creating softlinks to boost prerequisite libs, skip linking if destination file exists ([GH-323](https://github.com/FairRootGroup/DDS/issues/323)).
- **General**: A bug, which prevented to kill user task processes if they ignore SIGTERM ([GH-359](https://github.com/FairRootGroup/DDS/issues/359)).
- **General**: Refined command line parsing using `boost::program_options::split_unix` ([GH-353](https://github.com/FairRootGroup/DDS/issues/353) and [GH-352](https://github.com/FairRootGroup/DDS/issues/352)).
- **General**: Clang 9 warning/error ([GH-249](https://github.com/FairRootGroup/DDS/issues/249)).
- **General**: `--version` option for `dds-session`.
- **dds-session**: Improved default SID storage and handling ([GH-318](https://github.com/FairRootGroup/DDS/issues/318)).
- **dds-session**: Silence the error "dds-session: error: Sessions holder dir doesn't exists" ([GH-376](https://github.com/FairRootGroup/DDS/issues/376)).

### Changed

- **General**: C++17 is now a required standard.
- **General**: Remove an obsolete dds-test tool ([GH-341](https://github.com/FairRootGroup/DDS/issues/341)).
- **General**: Remove obsolete internal statistics of channels ([GH-341](https://github.com/FairRootGroup/DDS/issues/341)).
- **General**: Refactor internal version implementation. Use a single `Version.h.in` configuration file for version instead of multiple files for each subproject. Install `Version.h` ([GH-342](https://github.com/FairRootGroup/DDS/issues/342)).
- **General**: Refactor `MiscCommon`.
- **dds-slurm-plugin**: DDS Session ID is added to the root path of job's wrk dir and the slurm sandbox wrk dir ([GH-349](https://github.com/FairRootGroup/DDS/issues/349)).
- **dds-ssh-plugin**: Remote destination directories are no longer required and will be created automatically at runtime ([GH-349](https://github.com/FairRootGroup/DDS/issues/349)).
- **dds-ssh-plugin**: Final remote destination directories are created in DDS session ID subfolder, i.e. directory format is "(root wrk dir from plugin cfg)/(sessionID)/(wn ID from plugin cfg)" ([GH-349](https://github.com/FairRootGroup/DDS/issues/349)).
- **dds-user-defaults**: Remove the `-V/--verbose` command line options ([GH-376](https://github.com/FairRootGroup/DDS/issues/376)).
- **dds-user-defaults**: A default cfg file creation workflow has been revised ([GH-376](https://github.com/FairRootGroup/DDS/issues/376)).

### Removed

- **General**: Obsolete test project. ODC is used as an integration platform for DDS.
## [3.4.0] - 2020-07-01

### Added

- **dds-tools-api**: Active topology filepath in commander info request.

### Changed

- **General**: General improvements and bug fixes.

### Added

- **dds-topology**: New max instances per host requirement ("maxinstances") for tasks and collections. One can define maximum allowed number of task/ collection instances per host.

## [3.2.0] - 2020-05-12

### Added

- **General**: Users now can specify custom environment scripts for each task ([GH-24](https://github.com/FairRootGroup/DDS/issues/24)).
- **dds-agent**: Intercom channel got a dedicated service. Now DDS main transport and Intercom work on different threads ([GH-279](https://github.com/FairRootGroup/DDS/issues/279)).
- **dds-tools-api**: New static API calls `CSession::getDefaultSessionIDString` and `CSession::getDefaultSessionID` to get the default session id ([GH-209](https://github.com/FairRootGroup/DDS/issues/209)).
- **dds-topology**: Getter of the filepath to the XML topology.
- **dds-topology**: New optional XML attribute which allows to set number of tasks in the collection.
- **localhost plug-in**: Requires now the `--slots` argument.

### Fixed

- **General**: If process is killed or crashed it can leave opened and locked interprocess mutex. It leads to hanging `boost::interprocess::message_queue::timed_send` function. The function tries to write to the queue which is locked by the mutex from the killed process. BOOST implements a workaround flag - `BOOST_INTERPROCESS_ENABLE_TIMEOUT_WHEN_LOCKING`. It forces the boost::interprocess to use timed mutexes instead of a simple ones.
- **dds-submit**: A bug, which caused the command to block if an unknown plug-in is requested.
- **dds-submit**: CLI returns 1 if submission failed ([GH-227](https://github.com/FairRootGroup/DDS/issues/227)).

### Changed

- **General**: Improved cleaning of child processes of user tasks.
- **dds-agent**: Significantly improved performance of stopping of user tasks.
- **dds-agent**: Improved the logic of stopping of user tasks. The algorithm now recursively enumerates absolutely all child process and send first graceful TERM followed by unconditional TERM after a given timeout.
- **dds-topology**: Major update of the topology API. Significantly improved topology construction API.
- **dds-topology**: New way of task and collection ID calculation. String which is used for CRC64 based ID calculation now includes path ID and an object hash string. This allows to detect also the content changes of the topology objects. Topology update uses the new feature in order to better detect difference between two topologies.
- **ssh plug-in**: Replace custom thread pool with boost asio's `thread_pool`.
- **ssh plug-in**: Improve performance and stability.

## [3.0.0] - 2019-12-11

The main highlight of this release is a general overhaul of the core engine of DDS agents.
Starting from this release, DDS supports multiple tasks per agent.
DDS now requires much less resources at runtime.
In compare to previous versions it's also significantly faster when using DDS Key-Value and DDS Custom Commands in user tasks.

### Added

- **General**: More Unit-/Functional-tests.
- **General**: DDS SM now supports multiple input queues.
- **General**: Add timestamp and log delivery time for Custom Commands.
- **dds-user-defaults**: New global option `agent.work_dir`. Using this setting users can define a working directory of agents.
- **dds-tools-api**: Agent count options moved from `SAgentInfoRequest` to a new `SAgentCountRequest`.
- **dds-tools-api**: Sync counterparts for each request in Tools API.
- **dds-tools-api**: Start/Stop DDS session multiple times within the same process.
- **dds-topology**: Parsing and reporting of the topology name: `dds-info --active-topology`, `dds-topology --topology-name <topo.xml>`, `CTopology::getName()`.
- **dds-topology**: Get filter iterator matching the task/collection runtime path in the topology.
- **dds-topology**: Calculation of the topology hash as CRC32.
- **dds-info**: `--wait` option to wait for the required number of agents online. Must be used together with `--active-count`, `--idle-count`, `--executing-count`.
- **dds-info**: `--active-count`, `--idle-count`, `--executing-count` option to get the number of active, idle or executing agents respectively. These option can be used together with `--wait` in order to wait for the required number of agents.

### Fixed

- **General**: A race condition in DDS Core in external process handling ([GH-252](https://github.com/FairRootGroup/DDS/issues/252)).
- **General**: Support list of values in `DDS_LD_LIBRARY_PATH` ([GH-262](https://github.com/FairRootGroup/DDS/issues/262)).
- **dds-commander**: Fix cases when multiple commanders trying to bind the same found free port in the same time ([GH-250](https://github.com/FairRootGroup/DDS/issues/250)).
- **dds-tools-api**: Newly created DDS session fails to send custom commands.
- **dds-topology**: Activate hangs on xml validation error ([GH-220](https://github.com/FairRootGroup/DDS/issues/220)).
- **dds-topology**: Check that agent's topology hash is the same as commander's one before task assignment. Fixes [GH-265](https://github.com/FairRootGroup/DDS/issues/265).
- **dds-info**: SIGSEGV in dds-info ([GH-261](https://github.com/FairRootGroup/DDS/issues/261)).

### Changed

- **General**: API breaking change! Names of all API headers are now streamlined.
- **General**: DDS SM channels learned to drain their write queue. It helps to reduce CPU usage and handle cases when a user task is finished, but still receiving Intercom messages (such as Custom Commands).

### Removed

- **dds-info**: `--wait-for-idle-agents`, `--wait-for-executing-agents` are obsolete. Replaced with `--idle-count`, `--executing-count` options used together with `--wait` option.

## [2.4.0] - 2019-06-18

### Added

- **dds_toolsapi_lib**: Initial release.
- **dds_intercom_lib**: Make custom command's condition regex aware ([GH-211](https://github.com/FairRootGroup/DDS/issues/211)).
- **General**: Support relative task executables ([GH-216](https://github.com/FairRootGroup/DDS/issues/216)).
- **General**: All DDS commands now learned about the `DDS_SESSION_ID` environment variable. If defined, it will be used instead of a default one ([GH-213](https://github.com/FairRootGroup/DDS/issues/213)).

### Fixed

- **General**: Don't copy reachable task to agent's directory ([GH-215](https://github.com/FairRootGroup/DDS/issues/215)).
- **General**: WnName requirement is only used for SSH plug-in. For other plug-ins the requirement is skipped with a warning message in the log ([GH-217](https://github.com/FairRootGroup/DDS/issues/217)).
- **General**: Build system writes temporary files only in build directory ([GH-182](https://github.com/FairRootGroup/DDS/issues/182)).
- **General**: Workaround wait_for bug in boost::process.
- **dds-topology**: dds-topology --activate hangs if there are no active agents ([GH-218](https://github.com/FairRootGroup/DDS/issues/218)).

### Changed

- **General**: Install DDS headers to "include/DDS" instead of "include".
- **General**: Add support for Boost version 1.70+.
- **General**: All DDS CLI commands have been rewritten to use DDS Tools API rather than DDS Core protocol.
- **dds-submit**: Introduced a lightweight worker package for the localhost plug-in. It doesn't contain libs and binaries. Deployment speed is x3 faster. Instead of ~15MB/agent disk space, DDS uses ~50 KB/agent now ([GH-210](https://github.com/FairRootGroup/DDS/issues/210)).
- **dds-submit**: The command reports now the time it took to submit the job.
- **dds-agent**: The watchdog now terminates/kills not only user tasks (parent processes), but also their children if they spawn any ([GH-212](https://github.com/FairRootGroup/DDS/issues/212)).
- **dds-topology**: Rename "id" to "name" in the topology XML file and topology classes. **WARNING**: this change is incompatible with an older XML topology files. Rename all "id" attributes and tags to "name" in the XML topology files in order to be compatible with current version of DDS.
- **dds-topology**: New `CTopology` class for public user API.

### Removed

- **dds-protocol**: `cmdDELETE_KEY` is obsolete and was removed.

## [2.2.0] - 2018-11-27

### Added

- **General**: Get rid of explicit include path mgmt and install CMake package.
- **General**: Improved error reporting for localhost plug-in. In case of failure logs are sent to the user.
- **General**: Decentralized key-value propagation. Lobby leader acts as a mini-commander which creates update key messages and forwards them either locally via shared memory if the receiver is in the same lobby or to the commander via network if the receiver is in a different lobby ([GH-196](https://github.com/FairRootGroup/DDS/issues/196)).
- **dds_intercom_lib**: Notification on Task Done `CIntercomService::subscribeOnTaskDone`.
- **dds-session**: Initial version of the tool ([GH-191](https://github.com/FairRootGroup/DDS/issues/191)).
- **dds-session**: Learned new commands: "start", "stop", "stop-all", "clean", "list", and "set-default" ([GH-192](https://github.com/FairRootGroup/DDS/issues/192)).
- **dds-info**: The command learned `--wait-for-idle-agents` parameter, which blocks the command infinitely until a required number of idle agents are online ([GH-205](https://github.com/FairRootGroup/DDS/issues/205)).
- **dds-topology**: Property scope. Properties having COLLECTION scope are propagated only to tasks in the same collection as a task sending a property. Properties with GLOBAL scope are sent to all dependent tasks.
- **dds-protocol**: Confirmation for `cmdASSIGN_USER_TASK` and `cmdACTIVATE_USER_TASK` ([GH-202](https://github.com/FairRootGroup/DDS/issues/202)).
- **dds-protocol**: Generic reply command `cmdREPLY` ([GH-201](https://github.com/FairRootGroup/DDS/issues/201)).

### Fixed

- **General**: In some edge cases a topology update, performed during an intensive key-value exchange, can lead to a segmentation fault.

### Changed

- **General**: Bump minimum required Boost version to 1.67.
- **General**: Bump minimum required cmake version to 3.11.0.
- **General**: dds-intercom key-value API changed to reflect recent changes in the protocol ([GH-196](https://github.com/FairRootGroup/DDS/issues/196)).
- **General**: Improve log dir detection algorithm for commander and agents. The new algorithm doesn't rely on `DDS_LOG_LOCATION` anymore.
- **dds-session**: Local mode is now a default start mode for DDS. The `--local` argument is removed.
- **dds-session**: A mixed mode is introduced (`--mixed`) to run DDS on Linux and OS X in the same time.
- **dds-topology**: Show proper error output from xmllint if topology XML file can't be validated.
- **dds-protocol**: New fields in `cmdUPDATE_KEY`: propertyID, value, sender task ID and receiver task ID ([GH-196](https://github.com/FairRootGroup/DDS/issues/196)).

### Removed

- **General**: Update key command from dds-agent-cmd.
- **General**: Remove an obsolete dds-test tool ([GH-341](https://github.com/FairRootGroup/DDS/issues/341)).
- **dds_intercom_lib**: Notification on key delete `CKeyValue::subscribeOnDelete`.
- **dds-server**: The command is removed. Use dds-session instead ([GH-192](https://github.com/FairRootGroup/DDS/issues/192)).

## [2.0.0] - 2018-03-12

### Added

- **General**: Introduced DDS Sessions ([GH-186](https://github.com/FairRootGroup/DDS/issues/186)).
- **General**: New test which throws exception in the user code.
- **dds-session**: Initial version of the tool ([GH-191](https://github.com/FairRootGroup/DDS/issues/191)).

### Fixed

- **dds-protocol**: Efficient transfer of binary attachments for shared memory channels.

### Changed

- **General**: Bump minimum required Boost version to 1.64.
- **General**: Code related to external processes execution has been ported to use boost::process library ([GH-190](https://github.com/FairRootGroup/DDS/issues/190)).
- **General**: Export `$DDS_SESSION_ID` for user's task which can be retrieved via `dds::env_prop` ([GH-187](https://github.com/FairRootGroup/DDS/issues/187)).
- **General**: Trap user code calls in try/catch ([GH-183](https://github.com/FairRootGroup/DDS/issues/183)).
- **dds-server**: The "restart" is no longer supported.
- **dds-server**: Introduced a "stop_all" option to stop all currently running DDS sessions.

## [1.8.0] - 2017-11-09

### Added

- **dds-session**: Initial version with commands: "start", "stop", "stop-all", "clean", "list", and "set-default" ([GH-191](https://github.com/FairRootGroup/DDS/issues/191), [GH-192](https://github.com/FairRootGroup/DDS/issues/192)).
- **dds_intercom_lib**: Notification on Task Done `CIntercomService::subscribeOnTaskDone`.
- **dds-info**: `--wait-for-idle-agents` parameter to block until required number of idle agents are online ([GH-205](https://github.com/FairRootGroup/DDS/issues/205)).
- **dds-topology**: Property scope - properties with COLLECTION scope are propagated only to tasks in the same collection, properties with GLOBAL scope are sent to all dependent tasks.
- **General**: Decentralized key-value propagation with lobby leader acting as mini-commander ([GH-196](https://github.com/FairRootGroup/DDS/issues/196)).
- **General**: Get rid of explicit include path management and install CMake package.
- **General**: Improved error reporting for localhost plug-in with logs sent to user on failure.

### Fixed

- **General**: In some edge cases a topology update during intensive key-value exchange could lead to segmentation fault.

### Changed

- **General**: Bump minimum required Boost version to 1.67.
- **General**: Bump minimum required cmake version to 3.11.0.
- **General**: dds-intercom key-value API changed to reflect recent changes in the protocol ([GH-196](https://github.com/FairRootGroup/DDS/issues/196)).
- **General**: Improve log dir detection algorithm for commander and agents. No longer relies on `DDS_LOG_LOCATION`.
- **dds-session**: Local mode is now default start mode for DDS. The `--local` argument is removed.
- **dds-session**: Mixed mode is introduced (`--mixed`) to run DDS on Linux and OS X simultaneously.
- **dds-server**: The command is removed. Use dds-session instead ([GH-192](https://github.com/FairRootGroup/DDS/issues/192)).
- **dds-topology**: Show proper error output from xmllint if topology XML file can't be validated.
- **dds-protocol**: New fields in `cmdUPDATE_KEY`: propertyID, value, sender task ID and receiver task ID ([GH-196](https://github.com/FairRootGroup/DDS/issues/196)).
- **dds-protocol**: Confirmation for `cmdASSIGN_USER_TASK` and `cmdACTIVATE_USER_TASK` ([GH-202](https://github.com/FairRootGroup/DDS/issues/202)).
- **dds-protocol**: Generic reply command `cmdREPLY` ([GH-201](https://github.com/FairRootGroup/DDS/issues/201)).

### Removed

- **General**: Obsolete test project. ODC is used as integration platform for DDS.
- **General**: Update key command from dds-agent-cmd.
- **dds_intercom_lib**: Notification on key delete `CKeyValue::subscribeOnDelete`.

## [2.0.0] - 2018-03-12

### Added

- **General**: Introduced DDS Sessions ([GH-186](https://github.com/FairRootGroup/DDS/issues/186)).
- **dds-session**: Initial version of the tool ([GH-191](https://github.com/FairRootGroup/DDS/issues/191)).
- **General**: New test which throws exception in user code.

### Fixed

- **dds-protocol**: Efficient transfer of binary attachments for shared memory channels.

### Changed

- **General**: Bump minimum required Boost version to 1.64.
- **General**: Code related to external processes execution ported to use boost::process library ([GH-190](https://github.com/FairRootGroup/DDS/issues/190)).
- **General**: Export `$DDS_SESSION_ID` for user's task which can be retrieved via `dds::env_prop` ([GH-187](https://github.com/FairRootGroup/DDS/issues/187)).
- **General**: Trap user code calls in try/catch ([GH-183](https://github.com/FairRootGroup/DDS/issues/183)).
- **dds-server**: The "restart" is no longer supported.
- **dds-server**: Introduced "stop_all" option to stop all currently running DDS sessions.

## [1.8.0] - 2017-11-09

### Added

- **General**: Lobby based deployment ([GH-78](https://github.com/FairRootGroup/DDS/issues/78)).
- **General**: Introduced Session ID ([GH-170](https://github.com/FairRootGroup/DDS/issues/170)).
- **General**: DDS cmake script learned `DDS_LD_LIBRARY_PATH` to help users who wants to build WN packages to workaround macOS's SIP when a custom installations of gcc/clang is used ([GH-175](https://github.com/FairRootGroup/DDS/issues/175)).
- **dds-protocol**: Handshake checks now protocol version of the client.
- **dds-protocol**: Handshake checks now session ID of the client to match server's one ([GH-170](https://github.com/FairRootGroup/DDS/issues/170)).
- **dds-protocol**: New API for pushing and processing of raw messages. Implemented for network and shared memory channels.
- **dds-protocol**: Special command `ECmdType::cmdRAW_MSG` for raw message event subscription.
- **dds-protocol**: Implement ID in protocol headers ([GH-178](https://github.com/FairRootGroup/DDS/issues/178)).
- **dds-protocol**: Multiple outputs for shared memory channel ([GH-78](https://github.com/FairRootGroup/DDS/issues/78)).
- **dds-user-defaults**: Command learned `--session-id-file` parameter, which shows the location of the session file on the local system.

### Fixed

- **General**: An issue that all the key-value update errors were processed as version mismatch errors, which is wrong. A new error type 'key-value not found' was introduced. DDS agent does not send back an updated key if the error was of type 'key-value not found'.
- **dds-topology**: dds-topology --validate works again ([GH-174](https://github.com/FairRootGroup/DDS/issues/174)).
- **dds-info**: The `dds-info -n` command hangs if there are no agents online ([GH-177](https://github.com/FairRootGroup/DDS/issues/177)).
- **dds-server**: Download WN packages only for supported systems. We don't support 32bit WN packages anymore.

### Changed

- **dds-protocol**: Create new message in the channel instead of clearing the current message. This allows to forward the message without additional copying.

## [1.6.0] - 2017-03-26

### Added

- **General**: Dependency look up and bundling of WN package using cmake ([GH-166](https://github.com/FairRootGroup/DDS/issues/166)).
- **General**: Bundle-like installation ([GH-167](https://github.com/FairRootGroup/DDS/issues/167)).
- **dds-topology**: New optional XML attribute which allows to set number of tasks in the collection.
- **dds-topology**: Support task triggers in topology. User can define condition and corresponding action for the trigger. Conditions and actions are predefined ([GH-151](https://github.com/FairRootGroup/DDS/issues/151)).

### Fixed

- **General**: Fix a bug where an unhandled exception could crash a user code when DDS environment is not set ([GH-168](https://github.com/FairRootGroup/DDS/issues/168)).
- **dds-info**: Send list of agents one by one to avoid protocol string limits ([GH-158](https://github.com/FairRootGroup/DDS/issues/158)).
- **dds-topology**: Proper check that `--disable-validation` option has to be used only with `--activate` and `--update` options.

### Changed

- **General**: BOOST libs from WN packages are built now without libicudata support to reduce the package size ([GH-141](https://github.com/FairRootGroup/DDS/issues/141)).
- **dds-commander**: Error message about insufficient number of agents shows now how many is required and how many agents are available ([GH-161](https://github.com/FairRootGroup/DDS/issues/161)).
- **dds-topology**: dds-topology's activate, stop, update and set commands are refactored ([GH-153](https://github.com/FairRootGroup/DDS/issues/153)).
- **dds-topology**: `dds-topology --set` is obsolete now, use `dds-topology --activate topo_file.xml` instead ([GH-153](https://github.com/FairRootGroup/DDS/issues/153)).
- **dds-topology**: Change declaration of the requirements in XML file. Check user's manual for the new syntax.
- **dds-protocol-lib**: Enhanced handling of the messages and events. Common base class for message and event handlers. Check at compile time consistency between event or command and callback function ([GH-169](https://github.com/FairRootGroup/DDS/issues/169)).

## [1.4.0] - 2016-10-31

### Added

- **General**: DDS SM now supports multiple input queues.
- **General**: Add timestamp and log delivery time for Custom Commands.
- **dds-protocol**: Shared memory message queue transport. New shared memory channel which is based on `boost::message_queue`. Pushing and receiving of commands is done via shared memory. In some cases this can significantly improve communication speed ([GH-129](https://github.com/FairRootGroup/DDS/issues/129), [GH-130](https://github.com/FairRootGroup/DDS/issues/130), [GH-131](https://github.com/FairRootGroup/DDS/issues/131)).
- **dds_intercom_lib**: Reconnect if connection fails ([GH-138](https://github.com/FairRootGroup/DDS/issues/138)).
- **dds_intercom_lib**: Possibility to subscribe to the error messages.
- **dds_intercom_lib**: New shared memory transport is used in dds_intercom_lib for key-value propagation and custom commands ([GH-129](https://github.com/FairRootGroup/DDS/issues/129), [GH-130](https://github.com/FairRootGroup/DDS/issues/130), [GH-131](https://github.com/FairRootGroup/DDS/issues/131)).
- **dds-topology**: dds-topology --update ([GH-129](https://github.com/FairRootGroup/DDS/issues/129)).
- **dds-octopus**: Initial release ([GH-150](https://github.com/FairRootGroup/DDS/issues/150)).
- **SSH plug-in**: Events from the submitter script are reflected on dds-submit output ([GH-139](https://github.com/FairRootGroup/DDS/issues/139)).
- **PBS plug-in**: Initial release ([GH-113](https://github.com/FairRootGroup/DDS/issues/113)).
- **LSF plug-in**: Initial release ([GH-148](https://github.com/FairRootGroup/DDS/issues/148)).

### Fixed

- **General**: Support versioning in key-value propagation ([GH-131](https://github.com/FairRootGroup/DDS/issues/131)).
- **General**: Purge local key-value store of agents on task stop ([GH-130](https://github.com/FairRootGroup/DDS/issues/130)).
- **dds-topology**: Wrong dds-topology --stop output ([GH-146](https://github.com/FairRootGroup/DDS/issues/146)).

### Changed

- **General**: Pipe log engine is improved to log events line by line, rather than using a fixed string length.
- **General**: Key-value updates from external utilities are not supported now.
- **General**: DDS SM channels learned to drain their write queue. It helps to reduce CPU usage and handle cases when a user task is finished, but still receiving Intercom messages (such as Custom Commands).
- **dds_intercom_lib**: Shared memory transport allows to improve the user API. DDS guarantees that update key notification callback will be called on each update key or delete key command. Users are responsible to store the local cache for key-value if required ([GH-129](https://github.com/FairRootGroup/DDS/issues/129), [GH-130](https://github.com/FairRootGroup/DDS/issues/130), [GH-131](https://github.com/FairRootGroup/DDS/issues/131)).

## [1.2.0] - 2016-06-07

### Added

- **General**: Get rid of explicit include path mgmt and install CMake package.
- **General**: Proper error message if DDS can't find xmllint ([GH-140](https://github.com/FairRootGroup/DDS/issues/140)).
- **dds_intercom_lib**: API for new plug-in system - `CRMSPluginProtocol` ([GH-108](https://github.com/FairRootGroup/DDS/issues/108)).
- **dds-submit**: Support of the new plug-in architecture ([GH-108](https://github.com/FairRootGroup/DDS/issues/108)).
- **dds-submit**: The command learned `--config/-c` parameter, which can be used to specify a configuration file for plug-ins ([GH-111](https://github.com/FairRootGroup/DDS/issues/111)).
- **dds-submit**: The command learned `--list/-l` parameter, which lists all available RMS plug-ins ([GH-112](https://github.com/FairRootGroup/DDS/issues/112)).
- **dds-submit**: Customizable plugin location. `--path` option which specifies the root directory of the plugins was added. If the directory is not provided - default path will be used ([GH-118](https://github.com/FairRootGroup/DDS/issues/118)).
- **dds-protocol-lib**: Maximum message size for key-value and custom commands ([GH-104](https://github.com/FairRootGroup/DDS/issues/104)).
- **dds-protocol-lib**: Sending of arrays ([GH-105](https://github.com/FairRootGroup/DDS/issues/105)).
- **dds-protocol-lib**: Sending of strings ([GH-106](https://github.com/FairRootGroup/DDS/issues/106)).
- **SLURM plug-in**: SLURM plug-in - initial release ([GH-109](https://github.com/FairRootGroup/DDS/issues/109)).
- **SSH plug-in**: New SSH plug-in - initial release ([GH-108](https://github.com/FairRootGroup/DDS/issues/108)).
- **localhost plug-in**: Initial release ([GH-115](https://github.com/FairRootGroup/DDS/issues/115)).

### Fixed

- **General**: cmake: Updated OSX RPATH settings.
- **General**: cmake: Fail with an explicit error when missing DDS worker package dependency ([GH-117](https://github.com/FairRootGroup/DDS/issues/117)).
- **General**: dds-intercom-lib: fails to parse JASON message with quotes ([GH-120](https://github.com/FairRootGroup/DDS/issues/120)).
- **dds-daemonize**: Failed to execute if the full path to the executable is provided ([GH-121](https://github.com/FairRootGroup/DDS/issues/121)).

### Changed

- **General**: dds-key-value-lib and dds-custom-cmd-lib are combined to a single library `dds_intercom_lib` ([GH-101](https://github.com/FairRootGroup/DDS/issues/101)).
- **General**: Use portable temporary directory path function.
- **dds-submit**: Drop support of `--ssh-rms-cfg` in favour of `--config` ([GH-111](https://github.com/FairRootGroup/DDS/issues/111)).
- **dds-submit**: Drop support of auto-config feature of dds-submit, when it remembers last used settings ([GH-111](https://github.com/FairRootGroup/DDS/issues/111)).
- **dds-submit**: Accept both `-n` and `-c` command line options.
- **dds-protocol-lib**: Improve protocol attachment architecture. Check maximum size for vectors and strings in commands. Size limitations: all vectors (except uint8_t) have a maximum size of uint16_t i.e. 2^16; all vector<uint8_t>'s have a maximum size of uint32_t i.e. 2^32; all std::string's have a maximum size of uint16_t i.e. 2^16.

## [1.0.0] - 2015-11-20

## [1.6.0] - 2017-03-26

### Added

- **General**: Dependency look up and bundling of WN package using cmake ([GH-166](https://github.com/FairRootGroup/DDS/issues/166)).
- **General**: Bundle-like installation ([GH-167](https://github.com/FairRootGroup/DDS/issues/167)).
- **dds-topology**: `dds-topology --set` is obsolete now, use `dds-topology --activate topo_file.xml` instead ([GH-153](https://github.com/FairRootGroup/DDS/issues/153)).
- **dds-topology**: Support task triggers in topology. User can define condition and corresponding action for the trigger ([GH-151](https://github.com/FairRootGroup/DDS/issues/151)).

### Fixed

- **General**: Bug where an unhandled exception could crash user code when DDS environment is not set ([GH-168](https://github.com/FairRootGroup/DDS/issues/168)).
- **dds-info**: Send list of agents one by one to avoid protocol string limits ([GH-158](https://github.com/FairRootGroup/DDS/issues/158)).
- **dds-topology**: Proper check that `--disable-validation` option has to be used only with `--activate` and `--update` options.

### Changed

- **General**: BOOST libs from WN packages are built now without libicudata support to reduce package size ([GH-141](https://github.com/FairRootGroup/DDS/issues/141)).
- **dds-commander**: Error message about insufficient number of agents shows now how many is required and how many agents are available ([GH-161](https://github.com/FairRootGroup/DDS/issues/161)).
- **dds-topology**: `dds-topology`'s activate, stop, update and set commands are refactored ([GH-153](https://github.com/FairRootGroup/DDS/issues/153)).
- **dds-topology**: Change declaration of the requirements in XML file. Check user's manual for the new syntax.
- **dds-protocol-lib**: Enhanced handling of the messages and events. Common base class for message and event handlers. Check at compile time consistency between event or command and callback function ([GH-169](https://github.com/FairRootGroup/DDS/issues/169)).

## [1.4.0] - 2016-10-31

### Added

- **dds-protocol**: Shared memory message queue transport. New shared memory channel based on `boost::message_queue` ([GH-129](https://github.com/FairRootGroup/DDS/issues/129), [GH-130](https://github.com/FairRootGroup/DDS/issues/130), [GH-131](https://github.com/FairRootGroup/DDS/issues/131)).
- **dds_intercom_lib**: Reconnect if connection fails ([GH-138](https://github.com/FairRootGroup/DDS/issues/138)).
- **dds_intercom_lib**: Possibility to subscribe to error messages.
- **dds_intercom_lib**: New shared memory transport for key-value propagation and custom commands ([GH-129](https://github.com/FairRootGroup/DDS/issues/129), [GH-130](https://github.com/FairRootGroup/DDS/issues/130), [GH-131](https://github.com/FairRootGroup/DDS/issues/131)).
- **dds-topology**: `dds-topology --update` ([GH-129](https://github.com/FairRootGroup/DDS/issues/129)).
- **dds-octopus**: Initial release ([GH-150](https://github.com/FairRootGroup/DDS/issues/150)).
- **PBS plug-in**: Initial release ([GH-113](https://github.com/FairRootGroup/DDS/issues/113)).
- **LSF plug-in**: Initial release ([GH-148](https://github.com/FairRootGroup/DDS/issues/148)).

### Fixed

- **General**: Purge local key-value store of agents on task stop ([GH-130](https://github.com/FairRootGroup/DDS/issues/130)).
- **dds-topology**: Wrong dds-topology --stop output ([GH-146](https://github.com/FairRootGroup/DDS/issues/146)).

### Changed

- **General**: Pipe log engine is improved to log events line by line, rather than using a fixed string length.
- **General**: Key-value updates from external utilities are not supported now.
- **General**: Support versioning in key-value propagation ([GH-131](https://github.com/FairRootGroup/DDS/issues/131)).
- **General**: DDS SM channels learned to drain their write queue. Helps reduce CPU usage and handle cases when user task is finished but still receiving Intercom messages.
- **dds_intercom_lib**: Shared memory transport allows to improve the user API. DDS guarantees that update key notification callback will be called on each update key or delete key command ([GH-129](https://github.com/FairRootGroup/DDS/issues/129), [GH-130](https://github.com/FairRootGroup/DDS/issues/130), [GH-131](https://github.com/FairRootGroup/DDS/issues/131)).
- **SSH plug-in**: Events from the submitter script are reflected on dds-submit output ([GH-139](https://github.com/FairRootGroup/DDS/issues/139)).

## [1.2.0] - 2016-06-07

### Added

- **dds_intercom_lib**: API for new plug-in system - `CRMSPluginProtocol` ([GH-108](https://github.com/FairRootGroup/DDS/issues/108)).
- **dds-submit**: Support of the new plug-in architecture ([GH-108](https://github.com/FairRootGroup/DDS/issues/108)).
- **dds-submit**: `--config/-c` parameter to specify configuration file for plug-ins ([GH-111](https://github.com/FairRootGroup/DDS/issues/111)).
- **dds-submit**: `--list/-l` parameter to list all available RMS plug-ins ([GH-112](https://github.com/FairRootGroup/DDS/issues/112)).
- **dds-submit**: `--path` option to specify root directory of plugins ([GH-118](https://github.com/FairRootGroup/DDS/issues/118)).
- **dds-protocol-lib**: Maximum message size for key-value and custom commands ([GH-104](https://github.com/FairRootGroup/DDS/issues/104)).
- **dds-protocol-lib**: Sending of arrays ([GH-105](https://github.com/FairRootGroup/DDS/issues/105)).
- **dds-protocol-lib**: Sending of strings ([GH-106](https://github.com/FairRootGroup/DDS/issues/106)).
- **SLURM plug-in**: Initial release ([GH-109](https://github.com/FairRootGroup/DDS/issues/109)).
- **SSH plug-in**: New SSH plug-in - initial release ([GH-108](https://github.com/FairRootGroup/DDS/issues/108)).
- **localhost plug-in**: Initial release ([GH-115](https://github.com/FairRootGroup/DDS/issues/115)).

### Fixed

- **General**: cmake: Updated OSX RPATH settings.
- **General**: cmake: Fail with explicit error when missing DDS worker package dependency ([GH-117](https://github.com/FairRootGroup/DDS/issues/117)).
- **General**: dds-intercom-lib: fails to parse JASON message with quotes ([GH-120](https://github.com/FairRootGroup/DDS/issues/120)).
- **General**: Proper error message if DDS can't find xmllint ([GH-140](https://github.com/FairRootGroup/DDS/issues/140)).
- **dds-daemonize**: Failed to execute if the full path to the executable is provided ([GH-121](https://github.com/FairRootGroup/DDS/issues/121)).

### Changed

- **General**: dds-key-value-lib and dds-custom-cmd-lib are combined to a single library `dds_intercom_lib` ([GH-101](https://github.com/FairRootGroup/DDS/issues/101)).
- **General**: Use portable temporary directory path function.
- **General**: Install DDS headers to "include/DDS" instead of "include".
- **dds-submit**: Drop support of `--ssh-rms-cfg` in favour of `--config` ([GH-111](https://github.com/FairRootGroup/DDS/issues/111)).
- **dds-submit**: Drop support of auto-config feature, when it remembers last used settings ([GH-111](https://github.com/FairRootGroup/DDS/issues/111)).
- **dds-submit**: Accept both `-n` and `-c` command line options.
- **dds-protocol-lib**: Improve protocol attachment architecture. Check maximum size for vectors and strings in commands.

## [1.0.0] - 2015-11-20

### Added

- **General**: Give users a possibility to specify task requirement based on worker node name in the SSH configuration. Name can be specified as regular expression ([GH-88](https://github.com/FairRootGroup/DDS/issues/88)).
- **General**: Statistics accumulation: message size, message queue size for read and write operations is accumulated ([GH-99](https://github.com/FairRootGroup/DDS/issues/99)).
- **General**: New dds-stat command is introduced with possible options: enable, disable and get for statistics accumulation ([GH-99](https://github.com/FairRootGroup/DDS/issues/99)).
- **General**: Possibility to send custom commands from user tasks or utils. New library dds-custom-cmd-lib is introduced ([GH-100](https://github.com/FairRootGroup/DDS/issues/100)).
- **General**: DDS Tutorial2 which introduces the use of the new custom dds-custom-cmd-lib library ([GH-100](https://github.com/FairRootGroup/DDS/issues/100)).
- **General**: DDS environment properties API - DDSEnvProp ([GH-92](https://github.com/FairRootGroup/DDS/issues/92)).
- **dds-submit**: The command learned a localhost RMS ([GH-93](https://github.com/FairRootGroup/DDS/issues/93)).

### Fixed

- **General**: Git error when using out of source builds ([GH-85](https://github.com/FairRootGroup/DDS/issues/85)).
- **General**: A class name lookup issues, which could result in unpredictable behavior during run-time (agent and key-value-lib had classes with the same name and same header protection).
- **General**: Check `DDS_LOCATION` before agent start ([GH-98](https://github.com/FairRootGroup/DDS/issues/98)).
- **General**: Since Mac OS 10.11 (El Capitan) `DYLD_LIBRARY_PATH` is not exported in the sub-shell environment. We explicitly set `DYLD_LIBRARY_PATH` to the libraries directory.
- **dds-key-value**: Removed sys. signals handler. A user process now is responsible to catch signals if needed ([GH-97](https://github.com/FairRootGroup/DDS/issues/97)).
- **dds-server**: Check that `DDS_LOCATION` is set ([GH-86](https://github.com/FairRootGroup/DDS/issues/86)).

### Changed

- **General**: Extend error message in case if a worker package is missing ([GH-89](https://github.com/FairRootGroup/DDS/issues/89)).
- **dds scout**: Log pre-execution env to make sure environment is correct ([GH-67](https://github.com/FairRootGroup/DDS/issues/67)).

## [0.10.0] - 2015-07-16

### Added

- **General**: Handlers of the monitoring thread can be registered now with custom call intervals ([GH-63](https://github.com/FairRootGroup/DDS/issues/63)).
- **General**: Accumulated push message function ([GH-64](https://github.com/FairRootGroup/DDS/issues/64)).
- **General**: Include std c++ lib into worker package ([GH-61](https://github.com/FairRootGroup/DDS/issues/61)).
- **General**: Nicer logging on monitoring thread actions ([GH-80](https://github.com/FairRootGroup/DDS/issues/80)).
- **General**: Additional log levels. DDS has learned 3 new levels of protocol log events ([GH-49](https://github.com/FairRootGroup/DDS/issues/49)).
- **General**: Group name, collection name, task name and task path are exported as environment variables for each task ([GH-95](https://github.com/FairRootGroup/DDS/issues/95)).
- **General**: DDS Tutorial1.
- **dds-commander**: Since dds-commander is a daemon and doesn't have a console, it now has a dedicated log file for its std-out/-err called "dds-commander.out.log". File is located in the log directory.
- **dds-topology**: Output time spent on activation ([GH-62](https://github.com/FairRootGroup/DDS/issues/62)).
- **dds-topology**: The command learned `--set` parameter, which is used to set up topology for the current deployment ([GH-56](https://github.com/FairRootGroup/DDS/issues/56)).
- **dds-topology**: The command learned `--disable-validation`, which is used to disable topology validation. It can be used only together with `--set` ([GH-56](https://github.com/FairRootGroup/DDS/issues/56)).
- **dds-topology**: Scheduling and requirements for the collections ([GH-76](https://github.com/FairRootGroup/DDS/issues/76)).
- **dds-topology**: Index for tasks and collections which are in groups ([GH-72](https://github.com/FairRootGroup/DDS/issues/72)).
- **dds-topology**: New test for task and collection indices ([GH-72](https://github.com/FairRootGroup/DDS/issues/72)).
- **dds-topology**: Variable definition in the topology ([GH-71](https://github.com/FairRootGroup/DDS/issues/71)).
- **dds-key-value**: Multiple subscribers for key-value notifications ([GH-70](https://github.com/FairRootGroup/DDS/issues/70)).
- **dds-key-value**: If task can only read property then property will not be propagated ([GH-55](https://github.com/FairRootGroup/DDS/issues/55)).
- **dds-key-value**: User task can subscribe to error events, for example, error will be send if property can not be propagated ([GH-55](https://github.com/FairRootGroup/DDS/issues/55)).

### Fixed

- **General**: Fix implementation of `cmdSHUTDOWN` ([GH-65](https://github.com/FairRootGroup/DDS/issues/65)).
- **General**: Remove shared memory on exit.
- **General**: Fix monitoring thread to prevent breaks if custom callbacks throw exceptions ([GH-80](https://github.com/FairRootGroup/DDS/issues/80)).
- **dds scout**: New lock algorithm, instead of the lockfile command.
- **dds-agent**: Reconnect to DDS commander if connection was dropped ([GH-77](https://github.com/FairRootGroup/DDS/issues/77)).
- **dds-agent**: After reconnection to commander server key update won't be propagate from the effected agent ([GH-81](https://github.com/FairRootGroup/DDS/issues/81)).
- **dds-key-value**: Stability improvements.
- **dds-key-value**: Multiple protections for a case when a user process calls key/value API, but the corresponding agent is offline ([GH-87](https://github.com/FairRootGroup/DDS/issues/87)).

### Changed

- **General**: Name of task output file changed to `user_task_<datetime>_<task_id>_<out/err>.log` ([GH-75](https://github.com/FairRootGroup/DDS/issues/75)).
- **dds-submit**: Removed `--topo` parameter ([GH-56](https://github.com/FairRootGroup/DDS/issues/56)).
- **dds-submit**: Removed `--disable-xml-validation` parameter ([GH-56](https://github.com/FairRootGroup/DDS/issues/56)).
- **dds-agent**: Optimized key-value persistence to shared memory.
- **dds-agent**: User log file name starts from the name of the task ([GH-96](https://github.com/FairRootGroup/DDS/issues/96)).
- **dds-user-defaults**: Use string log severity values instead of numbers ([GH-49](https://github.com/FairRootGroup/DDS/issues/49)).

## [0.8.0] - 2015-02-17

### Added

- **General**: Log rotation: maximum total size of the stored log files is 1GB ([GH-36](https://github.com/FairRootGroup/DDS/issues/36)).
- **General**: Log rotation: minimum free space on the drive after which older log files will be deleted is 2GB ([GH-36](https://github.com/FairRootGroup/DDS/issues/36)).
- **General**: User's task stdout/err on WNs are automatically written in dedicated log files, `user_task_<TASK_ID>_out.log` and `user_task_<TASK_ID>_err.log` accordingly ([GH-26](https://github.com/FairRootGroup/DDS/issues/26)).
- **General**: Progress display for `dds-agent-cmd getlog`, `dds-topology --activate` and `dds-test -t` in percent. Optionally full verbose messages can be displayed with `--verbose` option ([GH-42](https://github.com/FairRootGroup/DDS/issues/42)).
- **General**: Broadcast property deletion on task exit ([GH-28](https://github.com/FairRootGroup/DDS/issues/28)).
- **General**: Property propagation types ([GH-30](https://github.com/FairRootGroup/DDS/issues/30)).
- **dds-commander**: State of agents ([GH-27](https://github.com/FairRootGroup/DDS/issues/27)).
- **dds-ssh**: The ssh plug-in has been extended to support multiple agents per host ([GH-25](https://github.com/FairRootGroup/DDS/issues/25)).
- **dds-ssh**: Each DDS scout uses separate stderr/-out file (scout.log), when more than one worker requested per machine.
- **dds-key-value**: Users are now able to subscribe on properties update events ([GH-29](https://github.com/FairRootGroup/DDS/issues/29)).
- **dds-key-value**: Shared memory storage for key-value ([GH-35](https://github.com/FairRootGroup/DDS/issues/35)).
- **dds-info**: taskId and task name to console output (`dds-info -l`) ([GH-33](https://github.com/FairRootGroup/DDS/issues/33)).
- **dds-info**: Possibility to get property list and property values from agents ([GH-52](https://github.com/FairRootGroup/DDS/issues/52)).
- **dds-topology**: Users are now able to stop (restart) execution of tasks by calling `dds-topology --stop`. To restart call: `dds-topology --stop` and `dds-topology --activate` ([GH-31](https://github.com/FairRootGroup/DDS/issues/31)).

### Fixed

- **General**: Idle time calculation for dds-commander and dds-agent ([GH-32](https://github.com/FairRootGroup/DDS/issues/32)).
- **General**: A bug, which prevented log files to rotate.
- **General**: Reaching the idle timeout causes Commander and Agents to exit even if user processes are still running ([GH-54](https://github.com/FairRootGroup/DDS/issues/54)).
- **dds-protocol-lib**: A bug in the dds-agent, which could cause a SEGFAULT when trying to access a deleted channel object on disconnect.
- **dds-protocol-lib**: Stability improvements. Handling edge cases which could occur during channel destruction.
- **dds-topology**: A bug, which caused a crash when topology activate is called before dds-submit ([GH-51](https://github.com/FairRootGroup/DDS/issues/51)).

### Changed

- **dds-user-defaults**: Default log level is changed to 1 (instead of 0).
- **dds-user-defaults**: Log rotation: default log rotations size in MB instead of bytes. (default is 10 MB).
- **dds-protocol-lib**: The DDS transport learned to accumulate commands before sending, instead of sending them one by one ([GH-38](https://github.com/FairRootGroup/DDS/issues/38)).
- **dds-protocol-lib**: Hand-shake messages are prioritized now. DDS doesn't send/accept any other message until hand-shake is successful ([GH-37](https://github.com/FairRootGroup/DDS/issues/37)).
- **dds-protocol-lib**: Revised write message algorithms. It is also faster now.
- **dds-protocol-lib**: Implemented callbacks (signals) in BaseChannelImpl for different channel events like connect, disconnect, handshakeOK, handshakeFailed ([GH-41](https://github.com/FairRootGroup/DDS/issues/41)).

## [0.6.0] - 2014-12-05

### Added

- **General**: Key-value propagation support ([GH-12](https://github.com/FairRootGroup/DDS/issues/12)).
- **General**: Key-value propagation API lib ([GH-11](https://github.com/FairRootGroup/DDS/issues/11)).
- **General**: Simple scheduler for SSH which takes into account requirements ([GH-20](https://github.com/FairRootGroup/DDS/issues/20)).
- **General**: Startup time of agents. It can be requested via git-info -l ([GH-3](https://github.com/FairRootGroup/DDS/issues/3)).
- **dds-topology**: A possibility to use comments in the topology XML file ([GH-15](https://github.com/FairRootGroup/DDS/issues/15)).
- **dds-topology**: Task activation functionality is moved from dds-submit to dds-topology ([GH-16](https://github.com/FairRootGroup/DDS/issues/16)).
- **dds-agent-cmd**: New command for communication with agents ([GH-17](https://github.com/FairRootGroup/DDS/issues/17)).
- **dds-agent-cmd**: getlog functionality moved to dds-agent-cmd ([GH-17](https://github.com/FairRootGroup/DDS/issues/17)).
- **dds-agent-cmd**: dds-agent-cmd learned a new command - update-key. It forces an update of a given task's property in the topology ([GH-12](https://github.com/FairRootGroup/DDS/issues/12)).

### Fixed

- **General**: Arguments of the task executable could contain slashes.

### Changed

- **General**: Build WN packages without ICU support ([GH-14](https://github.com/FairRootGroup/DDS/issues/14)).
- **dds-protocol-lib**: Version changed to v2.0.
- **dds-protocol-lib**: The protocol has learned a new command - `cmdUPDATE_KEY` ([GH-12](https://github.com/FairRootGroup/DDS/issues/12)).
- **dds-protocol-lib**: BinaryAttachment command learned to resolve environment variables in source files paths.
- **dds-topology**: dds-topology renamed to dds-topology-lib. dds-topology is executable now.

## [0.4.0] - 2014-10-24

### Added

- **General**: DDS learned how to expand given user tasks commands with arguments given as a single string. (in the Topology->Task->exec parameter).
- **General**: If a user's task is defined in the topology as not reachable, then DDS will take care of delivering it to worker nodes ([GH-6](https://github.com/FairRootGroup/DDS/issues/6)).
- **dds-topology**: Topology learned a new users' task attribute - "reachable". It defines whether executable is available on worker nodes ([GH-6](https://github.com/FairRootGroup/DDS/issues/6)).
- **dds-submit**: Show more informative messages in case if the ssh plug-in failed to deploy agents.
- **dds-submit**: The command remembers now all options of the last successful call.
- **dds-submit**: The command learned a new command line option `--config`. It gives the possibility to specify a configuration file with predefined dds-submit options.

### Fixed

- **dds-topology**: Respond with an error if the given topo file is missing.
- **dds-submit**: Stop server communication channel if a fatal error is received from the server.

### Changed

- **General**: Improved stability.
- **General**: All DDS CLI commands use now common code to find suitable DDS commander server.
- **General**: Updated User's manual.
- **dds-topology**: The topology description schema has been revised. See User's manual for more details.
- **dds-submit**: Properly reflect server messages to stdout when agents are submitted/activated.
- **dds-protocol-lib**: The protocol message header size has been reduced from 12 to 8 bytes.
- **dds-protocol-lib**: The protocol message header is validated now using CRC.
- **dds-protocol-lib**: Split binary files uploads into multiple message chunks, instead of using one message per file.

## [0.2.0] - 2014-09-03

### Added

- **General**: The first stable internal release.

## [0.10.0] - 2015-07-16

### Added

- **General**: Handlers of the monitoring thread can be registered with custom call intervals ([GH-63](https://github.com/FairRootGroup/DDS/issues/63)).
- **General**: Accumulated push message function ([GH-64](https://github.com/FairRootGroup/DDS/issues/64)).
- **General**: Include std c++ lib into worker package ([GH-61](https://github.com/FairRootGroup/DDS/issues/61)).
- **General**: Additional log levels. DDS learned 3 new levels of protocol log events ([GH-49](https://github.com/FairRootGroup/DDS/issues/49)).
- **General**: Group name, collection name, task name and task path are exported as environment variables for each task ([GH-95](https://github.com/FairRootGroup/DDS/issues/95)).
- **General**: DDS Tutorial1.
- **dds-commander**: Dedicated log file for std-out/-err called "dds-commander.out.log" in log directory.
- **dds-topology**: `--set` parameter to set up topology for current deployment ([GH-56](https://github.com/FairRootGroup/DDS/issues/56)).
- **dds-topology**: `--disable-validation` parameter to disable topology validation ([GH-56](https://github.com/FairRootGroup/DDS/issues/56)).
- **dds-topology**: Scheduling and requirements for collections ([GH-76](https://github.com/FairRootGroup/DDS/issues/76)).
- **dds-topology**: Index for tasks and collections which are in groups ([GH-72](https://github.com/FairRootGroup/DDS/issues/72)).
- **dds-topology**: Variable definition in topology ([GH-71](https://github.com/FairRootGroup/DDS/issues/71)).
- **dds-key-value**: Multiple subscribers for key-value notifications ([GH-70](https://github.com/FairRootGroup/DDS/issues/70)).
- **dds-key-value**: If task can only read property then property will not be propagated ([GH-55](https://github.com/FairRootGroup/DDS/issues/55)).
- **dds-key-value**: User task can subscribe to error events ([GH-55](https://github.com/FairRootGroup/DDS/issues/55)).

### Fixed

- **General**: Fix implementation of `cmdSHUTDOWN` ([GH-65](https://github.com/FairRootGroup/DDS/issues/65)).
- **General**: Remove shared memory on exit.
- **General**: Fix monitoring thread to prevent breaks if custom callbacks throw exceptions ([GH-80](https://github.com/FairRootGroup/DDS/issues/80)).
- **dds-agent**: Reconnect to DDS commander if connection was dropped ([GH-77](https://github.com/FairRootGroup/DDS/issues/77)).
- **dds-agent**: After reconnection to commander server key update won't be propagated from the affected agent ([GH-81](https://github.com/FairRootGroup/DDS/issues/81)).
- **dds-key-value**: Stability improvements.
- **dds-key-value**: Multiple protections for case when user process calls key/value API, but corresponding agent is offline ([GH-87](https://github.com/FairRootGroup/DDS/issues/87)).

### Changed

- **General**: Name of task output file changed to `user_task_<datetime>_<task_id>_<out/err>.log` ([GH-75](https://github.com/FairRootGroup/DDS/issues/75)).
- **General**: Nicer logging on monitoring thread actions ([GH-80](https://github.com/FairRootGroup/DDS/issues/80)).
- **dds scout**: New lock algorithm, instead of lockfile command.
- **dds-topology**: Output time spent on activation ([GH-62](https://github.com/FairRootGroup/DDS/issues/62)).
- **dds-submit**: Removed `--topo` parameter ([GH-56](https://github.com/FairRootGroup/DDS/issues/56)).
- **dds-submit**: Removed `--disable-xml-validation` parameter ([GH-56](https://github.com/FairRootGroup/DDS/issues/56)).
- **dds-agent**: Optimized key-value persistence to shared memory.
- **dds-agent**: User log file name starts from the name of the task ([GH-96](https://github.com/FairRootGroup/DDS/issues/96)).
- **dds-user-defaults**: Use string log severity values instead of numbers ([GH-49](https://github.com/FairRootGroup/DDS/issues/49)).

## [0.8.0] - 2015-02-17

### Added

- **General**: Log rotation: maximum total size of stored log files is 1GB ([GH-36](https://github.com/FairRootGroup/DDS/issues/36)).
- **General**: Log rotation: minimum free space on drive after which older log files will be deleted is 2GB ([GH-36](https://github.com/FairRootGroup/DDS/issues/36)).
- **General**: User's task stdout/err on WNs are automatically written in dedicated log files, `user_task_<TASK_ID>_out.log` and `user_task_<TASK_ID>_err.log` ([GH-26](https://github.com/FairRootGroup/DDS/issues/26)).
- **General**: Progress display for `dds-agent-cmd getlog`, `dds-topology --activate` and `dds-test -t` in percent ([GH-42](https://github.com/FairRootGroup/DDS/issues/42)).
- **General**: Broadcast property deletion on task exit ([GH-28](https://github.com/FairRootGroup/DDS/issues/28)).
- **General**: Property propagation types ([GH-30](https://github.com/FairRootGroup/DDS/issues/30)).
- **dds-commander**: State of agents ([GH-27](https://github.com/FairRootGroup/DDS/issues/27)).
- **dds-ssh**: Support for multiple agents per host ([GH-25](https://github.com/FairRootGroup/DDS/issues/25)).
- **dds-key-value**: Users can now subscribe to properties update events ([GH-29](https://github.com/FairRootGroup/DDS/issues/29)).
- **dds-key-value**: Shared memory storage for key-value ([GH-35](https://github.com/FairRootGroup/DDS/issues/35)).
- **dds-info**: taskId and task name to console output (`dds-info -l`) ([GH-33](https://github.com/FairRootGroup/DDS/issues/33)).
- **dds-info**: Possibility to get property list and property values from agents ([GH-52](https://github.com/FairRootGroup/DDS/issues/52)).
- **dds-topology**: Users can now stop (restart) execution of tasks by calling `dds-topology --stop` ([GH-31](https://github.com/FairRootGroup/DDS/issues/31)).

### Fixed

- **General**: Idle time calculation for dds-commander and dds-agent ([GH-32](https://github.com/FairRootGroup/DDS/issues/32)).
- **General**: Bug which prevented log files to rotate.
- **General**: Reaching idle timeout causes Commander and Agents to exit even if user processes are still running ([GH-54](https://github.com/FairRootGroup/DDS/issues/54)).
- **dds-protocol-lib**: Bug in dds-agent which could cause SEGFAULT when trying to access deleted channel object on disconnect.
- **dds-protocol-lib**: Stability improvements. Handling edge cases during channel destruction.
- **dds-topology**: Bug which caused crash when topology activate is called before dds-submit ([GH-51](https://github.com/FairRootGroup/DDS/issues/51)).

### Changed

- **General**: Default log level is changed to 1 (instead of 0).
- **General**: Log rotation: default log rotations size in MB instead of bytes (default is 10 MB).
- **dds-ssh**: Each DDS scout uses separate stderr/-out file (scout.log), when more than one worker requested per machine.
- **dds-protocol-lib**: DDS transport learned to accumulate commands before sending, instead of sending them one by one ([GH-38](https://github.com/FairRootGroup/DDS/issues/38)).
- **dds-protocol-lib**: Hand-shake messages are prioritized. DDS doesn't send/accept any other message until hand-shake is successful ([GH-37](https://github.com/FairRootGroup/DDS/issues/37)).
- **dds-protocol-lib**: Revised write message algorithms. Faster now.
- **dds-protocol-lib**: Implemented callbacks (signals) in BaseChannelImpl for different channel events like connect, disconnect, handshakeOK, handshakeFailed ([GH-41](https://github.com/FairRootGroup/DDS/issues/41)).

## [0.6.0] - 2014-12-05

### Added

- **General**: Key-value propagation support ([GH-12](https://github.com/FairRootGroup/DDS/issues/12)).
- **General**: Key-value propagation API lib ([GH-11](https://github.com/FairRootGroup/DDS/issues/11)).
- **General**: Simple scheduler for SSH which takes into account requirements ([GH-20](https://github.com/FairRootGroup/DDS/issues/20)).
- **General**: Startup time of agents. Can be requested via git-info -l ([GH-3](https://github.com/FairRootGroup/DDS/issues/3)).
- **dds-topology**: Possibility to use comments in topology XML file ([GH-15](https://github.com/FairRootGroup/DDS/issues/15)).
- **dds-topology**: Task activation functionality moved from dds-submit ([GH-16](https://github.com/FairRootGroup/DDS/issues/16)).
- **dds-agent-cmd**: New command for communication with agents ([GH-17](https://github.com/FairRootGroup/DDS/issues/17)).
- **dds-agent-cmd**: getlog functionality moved from other tools ([GH-17](https://github.com/FairRootGroup/DDS/issues/17)).
- **dds-agent-cmd**: Command learned update-key. Forces update of given task's property in topology ([GH-12](https://github.com/FairRootGroup/DDS/issues/12)).

### Fixed

- **General**: Arguments of the task executable could contain slashes.

### Changed

- **General**: Build WN packages without ICU support ([GH-14](https://github.com/FairRootGroup/DDS/issues/14)).
- **dds-protocol-lib**: Version changed to v2.0.
- **dds-protocol-lib**: Protocol learned new command - `cmdUPDATE_KEY` ([GH-12](https://github.com/FairRootGroup/DDS/issues/12)).
- **dds-protocol-lib**: BinaryAttachment command learned to resolve environment variables in source files paths.
- **dds-topology**: dds-topology renamed to dds-topology-lib. dds-topology is executable now.

## [0.4.0] - 2014-10-24

### Added

- **General**: DDS learned how to expand given user tasks commands with arguments given as a single string.
- **General**: If user's task is defined in topology as not reachable, then DDS will take care of delivering it to worker nodes ([GH-6](https://github.com/FairRootGroup/DDS/issues/6)).
- **dds-topology**: Topology learned new users' task attribute - "reachable". It defines whether executable is available on worker nodes ([GH-6](https://github.com/FairRootGroup/DDS/issues/6)).
- **dds-submit**: Show more informative messages in case if ssh plug-in failed to deploy agents.
- **dds-submit**: Command remembers now all options of the last successful call.
- **dds-submit**: Command learned new command line option `--config`. Gives possibility to specify configuration file with predefined dds-submit options.

### Fixed

- **dds-topology**: Respond with error if given topo file is missing.
- **dds-submit**: Stop server communication channel if fatal error is received from server.

### Changed

- **General**: Improved stability.
- **General**: All DDS CLI commands use now common code to find suitable DDS commander server.
- **General**: Updated User's manual.
- **dds-topology**: Topology description schema has been revised. See User's manual for more details.
- **dds-submit**: Properly reflect server messages to stdout when agents are submitted/activated.
- **dds-protocol-lib**: Protocol message header size reduced from 12 to 8 bytes.
- **dds-protocol-lib**: Protocol message header is validated now using CRC.
- **dds-protocol-lib**: Split binary files uploads into multiple message chunks, instead of using one message per file.

## [0.2.0] - 2014-09-03

### Added

- **General**: First stable internal release.
