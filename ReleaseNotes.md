# DDS Release Notes

## v3.11 (2024-09-05)

- DDS general
  - Fixed: compilation error on clang 15.
  - Fixed: new-delete-type-mismatch in BaseEventHandlersImpl.

## v3.10 (2024-04-28)

- DDS general
  - Fixed: a regression bug causing topology update to fail on the hash validation. (GH-480)

## v3.9 (2024-04-23)

- DDS general
  - Modified: compress topology files before broadcasting to agents. Significantly improves performance for big topology activations. (GH-478)  
  - Modified: Improved performance of the Core transport when transferring binary attachments. (GH-478)

## v3.8 (2024-01-19)

- DDS general
  - Fixed: On task done remove agents from the agent to tasks mapping.
  - Fixed: Replace std::iterator as it's deprecated (C++17).
  - Fixed: Tasks working directory is set to their slot directory instead of $DDS_LOCATION.
  - Fixed: Multiple stability issues.
  - Modified: support C++20 standard (GH-477).
  - Modified: Bump minimum version requirements for cmake (from 3.11.0 to 3.19)  and boost (from 1.67 to 1.75). (GH-428)
  - Modified: C++17 modernization of EnvProp.h/env_prop. (GH-368)
  - Added: 3rd party dependency on Protobuf (min v3.15).
  - Added: every DDS module logs now its pid, group id and parent pid. (GH-403)
  - Added: Support for Task Assets. (GH-406)
  - Added: Cancel running and pending SLURM jobs on DDS shutdown. (GH-429)
  - Added: Support for Apple's arm64 architecture. (GH-393)
  - Added: $DDS_CONFIG and `/etc/dds/DDS.cfg` are added to the DDS config search paths.   (GH-458)
  - Added: DDS libraries are now decorated with an ABI version. (GH-410)

- dds-agent
  - Fixed: Address potential crash in the external process termination routines.
  - Fixed: Revised handling of the slots container.
  - Fixed: Ignore SIGTERM while performing cleaning procedures. (GH-459)

- dds\_intercom\_lib
  - Fixed: Stability improvements.
  - Modified: Temporary increase intercom message size to 2048. (GH-440)
  - Modified: Set debug log severity on Custom command events. (GH-424)

- dds-session
  - Fixed: skip bad or non-session directories/files when performing clean and list operations.
  - Added: A data retention sanitization. Not running sessions older than the specified number of days ("server.data_retention") are auto deleted. (GH-435)

- dds-submit
  - Added: Users can specify a GroupName tag for each submission. This tag will be assigned to agents and can be used as a requirement in topologies. (GH-407)
  - Added: Users can provide a Submission Tag (`--submission-tag`). DDS RMS plug-ins will use this tag to name RMS jobs and directories. (GH-426)
  - Added: The command learned a new argument `--env-config/-e`. It can be used to define a custom environment script for each agent. (GH-430)
  - Added: The command learned a new argument `--min-instances`. It can be used to provide the minimum number of agents to spawn. (GH-434)
  - Added: The command learned a new argument `--enable-overbooking`. The flag instructs DDS RMS plug-ing to not specify any CPU requirement for RMS jobs. (GH-442)
  - Added: The command learned a new argument `--inline-config`. Content of this string will be added to the RMS job configuration file as is. It can be specified multiple times to   add multiline options. (GH-449)
  - Modified: WN package builder timeout interval was increased from 15 to 30 sec. (GH-468)
  - Modified: Improve validation of the WN package builder. (GH-468)

- dds-topology
  - Fixed: Stability improvements.
  - Fixed: A bug which caused `dds::topology_api::CTopoCreator` to ignore task assets.     (GH-452)
  - Fixed: Activating topology takes too long when task assets are used. (GH-454)
  - Fixed: a bug, which can cause a segfault when updating variables in topology.
  - Added: A new groupName requirement. It can be used on task and collection. (GH-407)
  - Added: Open API to read/update/add topology variable. The `CTopoVars` class.
  - Added: Support for Task Assets. (GH-406)
  - Added: Custom type of Task and Collection requirements. (GH-445)

- dds-ssh-plugin
  - Fixed: ssh cfg parser is passing cfg files of all plug-ins. (GH-413)
  - Added: Support for SubmissionID (GH-411)

- dds-slurm-plugin
  - Fixed: Make sure that scancel's SIGTERM is properly handled by all job steps and their scripts. (GH-459)
  - Added: Support for SubmissionID (GH-411)
  - Added: Support of minimum number of agents to spawn. (GH-434)
  - Modified: Replace array job submission with nodes requirement. (GH-430)
  - Modified: Remove `#SBATCH --ntasks-per-node=1`. (GH-444)
  - Modified: The `#SBATCH --cpus-per-task=%DDS_NSLOTS%` requirement is now can be disabled by providing the "enable-overbooking" flag (ToolsAPI or dds-submit). (GH-442)
  - Modified: Prevent job termination when downing a single node of the job allocation. (GH-450)

- dds-localhost-plugin
  - Added: Support for SubmissionID (GH-411)

- dds-tools-api
  - Modified: Logs of user processes which use Tools API are moved now to the DDS root log   directory, instead of sessions directory.
  - Modified: `CSession::waitForNumAgents` is renamed to `CSession::waitForNumSlots`. (GH-439)
  - Added: An ability to unsubscribe from either individual events or all events of   requests. (GH-382)
  - Added: SAgentInfoResponseData provides the agent group name. (GH-415)
  - Added: SSubmitRequestData supports flags. See `SSubmitRequestData::setFlag` and `SSubmitRequestData::ESubmitRequestFlags`. (GH-442)
  - Added: Users can define additional job RMS configuration via `SSubmitRequestData::m_inlineConfig`. It will be inlined as is into the final job script. (GH-449)

- dds-user-defaults
  - Fixed: a dangling reference to a temporary in User Defaults class.
  - Modified: Bump the version to 0.5.
  - Added: A `server.data_retention` configuration key. (GH-435)

- dds-info
  - Fixed: wrong exit code when called with `--help/--version`. (GH-470)

- dds-agent-cmd
  - Modified: getlog: now logs are tar'ed without their source directory structure - as a flat stack of files. (GH-369)  
  - Modified: getlog: the command outputs the destination directory where downloaded archives will be stored into. Also fixed command's description. (GH-369)

## v3.6 (2022-01-11)

- DDS general
  - Removed: obsolete test project. ODC is used as an integration platform for DDS.
  - Fixed: in some edge cases a topology update, performed during an intensive key-value exchange, can lead to a segmentation fault.
  - Fixed: When creating softlinks to boost prerequisite libs, skip linking if destination file exists. (GH-323)
  - Fixed: a bug, which prevented to kill user task processes if they ignore SIGTERM. (GH-359)
  - Fixed: Refined command line parsing using `boost::program_options::split_unix`. (GH-353 and GH-352)
  - Fixed: Clang 9 warning/error. (GH-249)
  - Modified: C++17 is now a required standard.
  - Modified: Remove an obsolete dds-test tool. (GH-341)
  - Modified: Remove obsolete internal statistics of channels. (GH-341)
  - Added: A cmake option "CREATE_BOOST_SYMLINKS", which enables creation of boost (libboost_*) symlinks in $DDS_LOCATION/lib/. Default   is OFF. (GH-199, GH-357)
  - Modified: Refactor internal version implementation. Use a single `Version.h.in` configuration file for version instead of multiple   files for each subproject. Install `Version.h`. (GH-342)
  - Fixed: `--version` option for `dds-session`.
  - Added: setup DDS environment and create default config file in code. No need to exec DDS_env.sh script anymore. (GH-350)
  - Added: Add a onTaskDone event to ToolsAPI. Using ToolsAPI users are able now to subscribe and receive onTaskDone events whenever a   user tasks exists.(GH-370)
  - Modified: Refactor `MiscCommon`.
  - Added: Refactor and open DDS SSH config API to the users. Rename `ncf` to `SSHConfigFile`. (GH-340)
  - Added: new command line option `--slot-list` of  `dds-info` performing `SSlotInfoRequest`. (GH-374)
  - Added: DDS agent monitors available disk space and if the (configurable) threshold is reached, it will trigger a self-shutdown. (GH-392)

- dds-tools-api
  - Added: CSession::userDefaultsGetValueForKey - returns a configuration value for a given configuration key.
  - Added: a new `SSlotInfoRequest` request returns a list of all active slots details. (GH-374)
  - Added: task path to `OnTaskDone` reply.
  - Added: `Topology` request returns extended info of each activated or stopped task via new `STopologyResponseData` data class.
  - Added: A new `SAgentCommandRequest` request to shutdown agents by ID or by slotID. (GH-399)

- dds-topology
  - Added: new std::istream based APIs.
  - Added: new CTopology::getRuntimeTask and CTopology::getRuntimeCollection methods which take either ID or runtime path as input.
  - Added: task ID to STopoRuntimeTask and collection ID to STopoRuntimeCollection.
  - Added: On topology update/stop a task done event is now only sent when all processes of the given task are actually exited or   killed. (GH-360)
  - Added: scheduler supports multiple requirements for task or collection. (GH-395)

- dds-session
  - Modified: improved default SID storage and handling. (GH-318)
  - Fixed: silence the error "dds-session: error: Sessions holder dir doesn't exists". (GH-376)

- dds-slurm-plugin
  - Fixed: a couple of fixes in Slurm plugin found on a Virgo cluster.
  - Fixed: fixed path to demonised log file. This log is created if plug-in failed to start.
  - Modified: DDS Session ID is added to the root path of job's wrk dir and the slurm sandbox wrk dir. (GH-349)

- dds-ssh-plugin
  - Modified: Remote destination directories are no longer required and will be created automatically at runtime. (GH-349)
  - Modified: Final remote destination directories are created in DDS session ID subfolder, i.e. directory format is "(root wrk dir from   plugin cfg)/(sessionID)/(wn ID from plugin cfg)". (GH-349)
  - Fixed: dds-submit-ssh doesn't exit if an exception is raised. (GH-363)

- dds-user-defaults
  - Modified: remove the "-V/--verbose" command line options. (GH-376)
  - Modified: a default cfg file creation workflow has been revised. (GH-376)
  - Added: dds-user-defaults learned a new global option "agent.access_permissions". This option forces the given file mode on agent   side files. At the moment it's only applied to user task log files (stdout and stderr). (GH-389)
  - Added: dds-user-defaults learned a new global option "agent.disk_space_threshold". The agent will trigger a self-shutdown if the free disk space is below this threshold. (GH-392)

## v3.4 (2020-07-01)

- DDS general
  - Modified: General improvements and bug fixes.

- dds-tools-api
  - Added: active topology filepath in commander info request.

- dds-topology
  - Added: new max instances per host requirement ("maxinstances") for tasks and collections. One can define maximum allowed number of task/ collection instances per host.

## v3.2 (2020-05-12)

- DDS general
  - Added: Users now can specify custom environment scripts for each task. (GH-24)
  - Modified: Improved cleaning of child processes of user tasks.
  - Fixed: If process is killed or crashed it can leave opened and locked interprocess mutex. It leads to hanging boost::interprocess::message_queue::timed_send function. The function tries to write to the queue which is locked by the mutex from the killed process. BOOST implements a workaround flag - BOOST_INTERPROCESS_ENABLE_TIMEOUT_WHEN_LOCKING. It forces the boost::interprocess to use timed mutexes instead of a simple ones.

- dds-agent
  - Modified: Intercom channel got a dedicated service. Now DDS main transport and Intercom work on different threads. (GH-279)
  - Modified: Significantly improved performance of stopping of user tasks.
  - Modified: Improved the logic of stopping of user tasks. The algorithm now recursively enumerates absolutely all child process and send first graceful TERM followed by unconditional TERM after a given timeout.

- dds-tools-api
  - Added: new static API calls CSession::getDefaultSessionIDString and CSession::getDefaultSessionID to get the default session id. (GH-209)

- dds-topology
  - Added: getter of the filepath to the XML topology.
  - Added: new optional XML attribute which allows to set number of tasks in the collection.
  - Modified: major update of the topology API. Significantly improved topology construction API.
  - Modified: new way of task and collection ID calculation. String which is used for CRC64 based ID calculation now includes path ID and an object hash string. This allows to detect also the content changes of the topology objects. Topology update uses the new feature in order to better detect difference between two topologies.

- dds-submit
  - Fixed: a bug, which caused the command to block if an unknown plug-in is requested.
  - Fixed: CLI returns 1 if submission failed. (GH-227)

- localhost plug-in
  - Modified: Requires now the --slots argument.

- ssh plug-in
  - Modified: Replace custom thread pool with boost asio's thread__pool.
  - Modified: Improve performance and stability.

## v3.0 (2019-12-11)

The main highlight of this release is a general overhaul of the core engine of DDS agents.
Starting from this release, DDS supports multiple tasks per agent.
DDS now requires much less resources at runtime.
In compare to previous versions it's also significantly faster when using DDS Key-Value and DDS Custom Commands in user tasks.

- DDS general
  - Fixed: a race condition in DDS Core in external process handling. (GH-252)
  - Fixed: Support list of values in DDS_LD_LIBRARY_PATH. (GH-262)
  - Modified: API breaking change! Names of all API headers are now streamlined.
  - Modified: DDS SM channels learned to drain their write queue. It helps to reduce CPU usage and handle cases when a user task is finished, but still receiving Intercom messages (such as Custom Commands).
  - Added: More Unit-/Functional-tests.
  - Added: DDS SM now supports multiple input queues.
  - Added: Add timestamp and log delivery time for Custom Commands.

- dds-user-defaults
  - Added: new global option agent.work_dir. Using this setting users can define a working directory of agents.

- dds-commander
  - Fixed: fix cases when multiple commanders trying to bind the same found free port in the same time. (GH-250)

- dds-tools-api
  - Fixed: newly created DDS session fails to send custom commands.
  - Added: Agent count options moved from SAgentInfoRequest to a new SAgentCountRequest.
  - Added: Sync counterparts for each request in Tools API.
  - Added: Start/Stop DDS session multiple times within the same process.

- dds-topology
  - Added: parsing and reporting of the topology name: "dds-info --active-topology", "dds-topology --topology-name <topo.xml>", CTopology::getName().
  - Fixed: activate hangs on xml validation error. (GH-220).
  - Added: get filter iterator matching the task/collection runtime path in the topology.
  - Added: calculation of the topology hash as CRC32.
  - Fixed: check that agent's topology hash is the same as commander's one before task assignment. Fixes GH-265.

- dds-info
  - Fixed: SIGSEGV in dds-info. (GH-261)
  - Added: --wait option to wait for the required number of agents online. Must be used together with  --active-count, --idle-count, --executing-count.
  - Added: --active-count, --idle-count, --executing-count option to get the number of active, idle or executing agents respectively.   - These option can be used together with --wait in order to wait for the required number of agents.
  - Removed: --wait-for-idle-agents, --wait-for-executing-agents are obsolete. Replaced with -idle-count, --executing-count options used together with --wait option.  

## v2.4 (2019-06-18)

- DDS general
  - Fixed: don't copy reachable task to agent's directory. (GH-215)
  - Fixed: WnName requirement is only used for SSH plug-in. For other plug-ins the requirement is skipped with a warning message in the log. (GH-217)
  - Fixed: build system writes temporary files only in build directory. (GH-182)
  - Fixed: Workaround wait_for bug in boost::process.
  - Added: support relative task executables. (GH-216)
  - Added: All DDS commands now learned about the DDS_SESSION_ID environment variable. If defined, it will be used instead of a default one. (GH-213)
  - Modified: install DDS headers to "include/DDS" instead of "include"
  - Modified: Add support for Boost version 1.70+.  
  - Modified: All DDS CLI commands have been rewritten to use DDS Tools API rather than DDS Core protocol.

- dds\_toolsapi\_lib
  - Added: Initial release

- dds\_intercom\_lib
  - Added: make custom command's condition regex aware. (GH-211)

- dds-submit
  - Modified: Introduced a lightweight worker package for the localhost plug-in. It doesn't contain libs and binaries. Deployment speed is x3 faster. Instead of ~15MB/agent disk space, DDS uses ~50 KB/agent now. (GH-210).
  - Added: The command reports now the time it took to submit the job.

- dds-agent
  - Modified: The watchdog now terminates/kills not only user tasks (parent processes), but also their children if they spawn any. (GH-212)

- dds-topology
  - Fixed: dds-topology --activate hangs if there are no active agents. (GH-218)
  - Modified: rename "id" to "name" in the topology XML file and topology classes. WARNING: this change is incompatible with an older XML topology files. Rename all "id" attributes and tags to "name" in the XML topology files in order to be compatible with current version of DDS.
  - Added: new CTopology class for public user API.

- dds-protocol
  - Removed: cmdDELETE_KEY is obsolete and was removed.

## v2.2 (2018-11-27)

- DDS general
  - Removed: update key command from dds-agent-cmd.
  - Modified: Bump minimum required Boost version to 1.67.
  - Modified: Bump minimum required cmake version to 3.11.0.
  - Modified: dds-intercom key-value API changed to reflect recent changes in the protocol. (GH-196)
  - Modified: Improve log dir detection algorithm for commander and agents. The new algorithm doesn't rely on DDS_LOG_LOCATION anymore.
  - Added: Get rid of explicit include path mgmt and install CMake package.
  - Added: Improved error reporting for localhost plug-in. In case of failure logs are sent to the user.
  - Added: decentralized key-value propagation. Lobby leader acts as a mini-commander which creates update key messages and forwards them either locally via shared memory if the receiver is in the same lobby or  to the commander via network if the receiver is in a different lobby.  (GH-196)

- dds\_intercom\_lib
  - Removed: notification on key delete CKeyValue::subscribeOnDelete.
  - Added: notification on Task Done CIntercomService::subscribeOnTaskDone.

- dds-protocol
  - Added: Confirmation for cmdASSIGN_USER_TASK and cmdACTIVATE_USER_TASK. (GH-202)
  - Added: Generic reply command cmdREPLY. (GH-201)
  - Modified: new fields in cmdUPDATE_KEY: propertyID, value, sender task ID and receiver task ID. (GH-196)

- dds-session
  - Added: Initial version of the tool. (GH-191)
  - Added: Learned new commands: "start", "stop", "stop-all", "clean", "list", and "set-default". (GH-192)
  - Modified: Local mode is now a default start mode for DDS. The --local argument is removed.
  - Modified: A mixed mode is introduced (--mixed) to run DDS on Linux and OS X in the same time.

- dds-server
  - Modified: The command is removed. Use dds-session instead. (GH-192)

- dds-info
  - Added: The command learned "--wait-for-idle-agents" parameter, which blocks the command infinitely until a required number of idle agents are online. (GH-205)

- dds-topology
  - Modified: Show proper error output from xmllint if topology XML file can't be validated.
  - Added: Property scope. Properties having COLLECTION scope are propagated only to tasks in the same collection as a task sending a property. Properties with GLOBAL scope are sent to all dependent tasks.

## v2.0 (2018-03-12)

- DDS general
  - Added: Introduced DDS Sessions. (GH-186)
  - Modified: Bump minimum required Boost version to 1.64.
  - Modified: Code related to external processes execution has been ported to use boost::process library. (GH-190)
  - Modified: Export $DDS_SESSION_ID for user's task which can be retrieved via dds::env_prop. (GH-187)
  - Modified: Trap user code calls in try/catch. (GH-183)
  - Added: New test which throws exception in the user code.

- dds-server
  - Modified: The "restart" is no longer supported.
  - Added: Introduced a "stop\_all" option to stop all currently running DDS sessions.

- dds-protocol
  - Fixed: efficient transfer of binary attachments for shared memory channels.

- dds-session
  - Added: Initial version of the tool. (GH-191)

## v1.8 (2017-11-09)

- DDS general
  - Fixed: an issue that all the key-value update errors were processed as version mismatch errors, which is wrong. A new error type 'key-value not found' was introduced. DDS agent does not send back an updated key if the error was of type 'key-value not found'.
  - Added: Lobby based deployment. (GH-78)
  - Added: Introduced Session ID. (GH-170)
  - Added: DDS cmake script learned DDS_LD_LIBRARY_PATH to help users who wants to build WN packages to workaround macos's SIP when a custom installations of gcc/clang is used. (GH-175)

- dds-protocol
  - Fixed: an issue when decimal type is passed as an argument to the callback function.
  - Added: Handshake checks now protocol version of the client.
  - Added: Handshake checks now session ID of the client to match server's one.  (GH-170)
  - Added: new API for pushing and processing of raw messages. Implemented for network and shared memory channels.
  - Added: special command ECmdType::cmdRAW_MSG  for raw message event subscription.
  - Added: Implement ID in protocol headers. (GH-178)
  - Modified: create new message in the channel instead of clearing the current message. This allows to forward the message without additional copying.
  - Added: Multiple outputs for shared memory channel. (GH-78)

- dds-topology
  - Fixed: dds-topology --validate works again. (GH-174)

- dds-user-defaults
  - Added: dds-user-defaults command learned "--session-id-file" parameter, which shows the location of the session file on the local system.

- dds-info
  - Fixed: the "dds-info -n" command hangs if there are no agents online. (GH-177)

- dds-server
  - Fixed: Download WN packages only for supported systems. We don't support 32bit WN packages anymore.

## v1.6 (2017-03-26)

- DDS general
  - Added: Dependency look up and bundling of WN package using cmake. (GH-166)
  - Added: Bundle-like installation. (GH-167)
  - Modified: BOOST libs from WN packages are built now without libicudata support to reduce the package size. (GH-141)
  - Fixed: Fix a bug where an unhandled exception could crash a user code when DDS environment is not set. (GH-168)

- dds-commander
  - Modified: Error message about insufficient number of agents shows now how many is required and how many agents are available. (GH-161)

- dds-info
  - Fixed: send list of agents one by one to avoid protocol string limits. (GH-158)

- dds-topology
  - Modified: dds-topology's activate, stop, update and set commands are refactored. (GH-153)
  - Modified: "dds-topology --set" is obsolete now, use "dds-topology --activate topo_file.xml" instead. (GH-153)
  - Modified: Change declaration of the requirements in XML file. Check user's manual for the new syntax.
  - Modified: Support task triggers in topology. User can define condition and corresponding action for the trigger. Conditions and actions are predefined. (GH-151)
  - Fixed: dds-topology: proper check that --disable-validation option has to be used only with --activate and --update options.

- dds-protocol-lib
  - Enhanced handling of the messages and events. Common base class for message and event handlers. Check at compile time consistency between event or command and callback function. (GH-169)

## v1.4 (2016-10-31)

- DDS general
  - Modified: pipe log engine is improved to log events line by line, rather than using a fixed string length.  
  - Modified: key-value updates from external utilities are not supported now.
  - Modified: support versioning in key-value propagation. (GH-131)  
  - Fixed: Purge local key-value store of agents on task stop. (GH-130)

- DDS protocol
  - Added: shared memory message queue transport. New shared memory channel which is based on  boost::message_queue. Pushing and receiving of commands is done via shared memory. In some cases this can significantly improve communication speed. (GH-129, GH-130, GH-131)

- SSH plug-in
  - Modified: events from the submitter script are reflected on dds-submit output. (GH-139)

- PBS plug-in
  - Added: Initial release. (GH-113)

- LSF plug-in
  - Added: Initial release. (GH-148)

- dds\_intercom\_lib
  - Added: reconnect if connection fails. (GH-138)
  - Added: possibility to subscribe to the error messages.
  - Added: new shared memory transport is used in dds_intercom_lib for key-value propagation and custom commands. (GH-129, GH-130, GH-131)
  - Modified: Shared memory transport allows to improve the user API. DDS guarantees that update key notification callback will be called on each update key or delete key command. Users are responsible to store the local cache for key-value if required. (GH-129, GH-130, GH-131)  

- dds-topology
  - Fixed: wrong dds-topology --stop output. (GH-146)
  - Added: dds-topology --update. (GH-129)

- dds-octopus
  - Added: Initial release. (GH-150)

## v1.2 (2016-06-07)

- DDS general
  - Fixed: cmake: Updated OSX RPATH settings.
  - Fixed: cmake: Fail with an explicit error when missing DDS worker package dependency. (GH-117)
  - Fixed: dds-intercom-lib: fails to parse JASON message with quotes. (GH-120)
  - Added: dds-intercom-lib: API for new plug-in system - CRMSPluginProtocol. (GH-108)
  - Modified: dds-key-value-lib and dds-custom-cmd-lib are combined to a single library dds_intercom_lib. (GH-101)
  - Modified: Use portable temporary directory path function.  
  - Modified: Proper error message if DDS can't find xmllint. (GH-140)

- dds-submit
  - Added: Support of the new plug-in architecture. (GH-108)
  - Added: the command learned "--config/-c" parameter, which can be used to specify a configuration file for plug-ins. (GH-111)
  - Added: the command learned "--list/-l" parameter, which lists all available RMS plug-ins. (GH-112)
  - Modified: drop support of "--ssh-rms-cfg" in favour of "--config". (GH-111)
  - Modified: drop support of auto-config feature of dds-submit, when it remembers last used settings. (GH-111)

- dds-protocol-lib
  - Added: maximum message size for key-value and custom commands (GH-104)  
  - Added: sending of arrays (GH-105)
  - Added: sending of strings (GH-106)
  - Added: improve protocol attachment architecture. Check maximum size for vectors and strings in commands. Size limitations:
    - all vectors (except uint8_t) have a maximum size of uint16_t i.e. 2^16;
    - all vector<uint8_t>'s have a maximum size of uint32_t i.e. 2^32;
    - all std::string's have a maximum size of uint16_t i.e. 2^16.

- SLURM plug-in
  - Added: SLURM plug-in - initial release. (GH-109)

- SSH plug-in
  - Added: New SSH plug-in - initial release. (GH-108)

- localhost plug-in
  - Added: Initial release. (GH-115)

- dds-daemonize
  - Fixed: failed to execute if the full path to the executable is provided. (GH-121)

- dds-submit
  - Added: customizable plugin location. --path option which specifies the root directory of the plugins was added. If the directory is not provided - default path will be used. (GH-118)
  - Modified: accept both -n and -c command line options.

## v1.0 (2015-11-20)

- DDS general
  - Fixed: git error when using out of source builds (GH-85)
  - Fixed: a class name lookup issues, which could result in unpredictable behavior during run-time (agent and key-value-lib had classes with the same name and same header protection).
  - Fixed: check DDS_LOCATION before agent start. (GH-98)
  - Fixed: since Mac OS 10.11 (El Capitan) DYLD_LIBRARY_PATH is not exported in the sub-shell environment. We explicitly set DYLD_LIBRARY_PATH to the libraries directory.  
  - Added: Give users a possibility to specify task requirement based on worker node name in the SSH configuration. Name can be specified as regular expression. (GH-88)
  - Added: extend error message in case if a worker package is missing. (GH-89)
  - Added: statistics accumulation: message size, message queue size for read and write operations is accumulated. (GH-99)
  - Added: new dds-stat command is introduced with possible options: enable, disable and get for statistics accumulation. (GH-99)
  - Added: possibility to send custom commands from user tasks or utils. New library dds-custom-cmd-lib is introduced. (GH-100)
  - Added: DDS Tutorial2 which introduces the use of the new custom dds-custom-cmd-lib library. (GH-100)
  - Added: DDS environment properties API - DDSEnvProp (GH-92)

- dds-key-value
  - Fixed: Removed sys. signals handler. A user process now is responsible to catch signals if needed. (GH-97)

- dds-submit
  - Added: the command learned a localhost RMS. (GH-93)

- dds-server
  - Fixed: Check that DDS_LOCATION is set. (GH-86)

- dds scout
  - Modified: log pre-execution env to make sure environment is correct. (GH-67)

## v0.10 (2015-07-16)

- DDS general
  - Added: handlers of the monitoring thread can be registered now with custom call intervals. (GH-63)
  - Added: accumulated push message function. (GH-64)
  - Added: include std c++ lib into worker package. (GH-61)
  - Added: nicer logging on monitoring thread actions. (GH-80)
  - Added: additional log levels. DDS has learned 3 new levels of protocol log events. (GH-49)
  - Added: group name, collection name, task name and task path are exported as environment variables for each task. (GH-95)
  - Added: DDS Tutorial1
  - Fixed: fix implementation of cmdSHUTDOWN. (GH-65)
  - Fixed: remove shared memory on exit.
  - Fixed: fix monitoring thread to prevent breaks if custom callbacks throw exceptions. (GH-80)  
  - Modified: Name of task output file changed to "user_task_<datetime>_<task_id>_<out/err>.log". (GH-75)

- dds scout
  - Modified: New lock algorithm, instead of the lockfile command.

- dds-commander
  - Added: Since dds-commander is a daemon and doesn't have a console, it now has a dedicated log file for its std-out/-err called "dds-commander.out.log". File is located in the log directory.

- dds-topology
  - Added: output time spent on activation. (GH-62)
  - Added: the command learned "--set" parameter, which is used to set up topology for the current deployment. (GH-56)
  - Added: the command learned "--disable-validation", which is used to disable topology validation. It can be used only together with "--set". (GH-56)
  - Added: scheduling and requirements for the collections. (GH-76)
  - Added: index for tasks and collections which are in groups. (GH-72)
  - Added: new test for task and collection indices. (GH-72)
  - Added: variable definition in the topology. (GH-71)

- dds-submit
  - Modified: removed "--topo" parameter. (GH-56)
  - Modified: removed "--disable-xml-validation" parameter. (GH-56)

- dds-agent
  - Fixed: reconnect to DDS commander if connection was dropped. (GH-77)
  - Fixed: after reconnection to commander server key update won't be propagate from the effected agent. (GH-81)
  - Modified: Optimized key-value persistence to shared memory.
  - Modified: User log file name starts from the name of the task. (GH-96)

- dds-key-value
  - Fixed: stability improvements.
  - Fixed: multiple protections for a case when a user process calls key/value API, but the corresponding agent is offline. (GH-87)
  - Added: Multiple subscribers for key-value notifications. (GH-70)
  - Added: If task can only read property then property will not be propagated. (GH-55)
  - Added: User task can subscribe to error events, for example, error will be send if property can not be propagated. (GH-55)

- dds-user-defaults
  - Modified: use string log severity values instead of numbers. (GH-49)

## v0.8 (2015-02-17)

- DDS general
  - Fixed: idle time calculation for dds-commander and dds-agent. (GH-32)
  - Fixed: a bug, which prevented log files to rotate.
  - Fixed: reaching the idle timeout causes Commander and Agents to exit even if user processes are still running. (GH-54)
  - Added: Log rotation: maximum total size of the stored log files is 1GB. (GH-36)
  - Added: Log rotation: minimum free space on the drive after which older log files will be deleted is 2GB. (GH-36)
  - Added: User's task stdout/err on WNs are automatically written in dedicated log files, user_task_<TASK_ID>_out.log and user_task_<TASK_ID>_err.log accordingly. (GH-26)
  - Added: Progress display for "dds-agent-cmd getlog", "dds-topology --activate" and "dds-test -t" in percent. Optionally full verbose messages can be displayed with --verbose option. (GH-42)
  - Added: Broadcast property deletion on task exit. (GH-28)
  - Added: property propagation types. (GH-30)

- dds-commander
  - Added: State of agents. (GH-27)

- dds-user-defaults
  - Modified: Default log level is changed to 1 (instead of 0).
  - Modified: Log rotation: default log rotations size in MB instead of bytes. (default is 10 MB).

- dds-ssh
  - Added: the ssh plug-in has been extended to support multiple agents per host. (GH-25)
  - Added: each DDS scout uses separate stderr/-out file (scout.log), when more than one worker requested per machine.

- dds-key-value
  - Added: users are now able to subscribe on properties update events. (GH-29)
  - Added: shared memory storage for key-value. (GH-35)

- dds-protocol-lib
  - Modified: The DDS transport learned to accumulate commands before sending, instead of sending them one by one. (GH-38)
  - Modified: Hand-shake messages are prioritized now. DDS doesn't send/accept any other message until hand-shake is successful. (GH-37)
  - Fixed: Revised write message algorithms. It is also faster now.
  - Fixed: a bug in the dds-agent, which could cause a SEGFAULT when trying to access a deleted channel object on disconnect.
  - Added: Implemented callbacks (signals) in BaseChannelImpl for different channel events like connect, disconnect, handshakeOK, handshakeFailed. (GH-41)
  - Fixed: Stability improvements. Handling edge cases which could occur during channel destruction.

- dds-info
  - Added: taskId and task name to console output (dds-info -l). (GH-33)
  - Added: possibility to get property list and property values from agents. (GH-52)

- dds-topology
  - Added: Users are now able to stop (restart) execution of tasks by calling "dds-topology --stop". To restart call: "dds-topology --stop" and "dds-topology --activate". (GH-31)
  - Fixed: a bug, which caused a crash when topology activate is called before dds-submit. (GH-51)

## v0.6 (2014-12-05)

- DDS general
  - Modified: Build WN packages without ICU support. (GH-14)
  - Added: key-value propagation support. (GH-12)
  - Added: key-value propagation API lib. (GH-11)
  - Fixed: Arguments of the task executable could contain slashes.
  - Added: Simple scheduler for SSH which takes into account requirements. (GH-20)
  - Added: Startup time of agents. It can be requested via git-info -l. (GH-3)

- dds-protocol-lib
  - Modified: Version changed to v2.0.
  - Added: The protocol has learned a new command - cmdUPDATE_KEY. (GH-12)
  - Added: BinaryAttachment command learned to resolve environment variables in source files paths.

- dds-topology
  - Added: a possibility to use comments in the topology XML file. (GH-15)
  - Renamed: dds-topology renamed to dds-topology-lib. dds-topology is executable now.
  - Added: task activation functionality is moved from dds-submit to dds-topology.(GH-16)

- dds-agent-cmd
  - Added: new command for communication with agents.(GH-17)
  - Added: getlog functionality moved to dds-agent-cmd.(GH-17)
  - Added: dds-agent-cmd learned a new command - update-key. It forces an update of a given task's property in the topology. (GH-12)

## v0.4 (2014-10-24)

- DDS general
  - Added: DDS learned how to expand given user tasks commands with arguments given as a single string. (in the Topology->Task->exec parameter).
  - Added: if a user's task is defined in the topology as not reachable, then DDS will take care of delivering it to worker nodes. (GH-6)
  - Improved: all DDS CLI commands use now common code to find suitable DDS commander server.
  - Modified: Updated User's manual.
  - Modified: Improved stability.  

- dds-topology
  - Fixed: respond with an error if the given topo file is missing.
  - Modified: the topology description schema has been revised. See User's manual for more details.
  - Added: topology learned a new users' task attribute - "reachable". It defines whether executable is available on worker nodes. (GH-6)

- dds-submit
  - Modified: Stop server communication channel if a fatal error is received from the server.
  - Added: Properly reflect server messages to stdout when agents are submitted/activated.
  - Added: Show more informative messages in case if the ssh plug-in failed to deploy agents.
  - Added: The command remembers now all options of the last successful call.
  - Added: The command learned a new command line option "--config". It gives the possibility to specify a configuration file with predefined dds-submit options.

- dds-protocol-lib
  - Improved: The protocol message header size has been reduced from 12 to 8 bytes.
  - Improved: The protocol message header is validated now using CRC.
  - Improved: Split binary files uploads into multiple message chunks, instead of using one message per file.

## v0.2 (2014-09-03)

The first stable internal release.
