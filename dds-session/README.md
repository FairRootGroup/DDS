# dds-session

Start/Stop DDS commander and manage DDS sessions. **UNIX/Linux/OSX**

## Synopsis

```shell
dds-session [[-h, --help] | [-v, --version]] {[start [--mixed] [--lightweight]] | [stop [SESSION_ID]] | [stop-all] | [list {all | run}] | [set-default SESSION_ID] | [clean [-f, --force]]}
```

## Description

Using this command users can perform a set of operations on DDS sessions, such as starting/stopping DDS server by creating new and stopping existing sessions. Users can also list available sessions or clean expired ones.

One user can start multiple DDS sessions. Each session will have its own DDS commander instance and will be sandboxed, i.e., won't interfere with other sessions of the same user.

## Options

* **-h, --help**  
Show usage options.

* **-v, --version**  
Show version information.

* **start**  
Start a new DDS session. DDS will automatically set the newly created session as the default one.
A single user can start as many DDS sessions as desired. Users are limited only by the resources of the underlying system.  
Each DDS session spawns its own commander server. All sessions are completely isolated from each other.  
At server start, DDS will test availability of DDS worker node binary packages and download them from the DDS repository if they are missing. If the user provides the `--mixed` parameter, then worker node packages for all systems (Linux, OS X) will be checked. By default, DDS checks only for a package compatible with the local system.  
To build a binary package for the local system, just issue:

  ```shell
  make -j wn_bin
  make -j install
  ```

  **Options for start command:**
  * `--mixed` - Use worker package for a mixed environment with agents on Linux and macOS simultaneously
  * `--lightweight` - Create lightweight worker packages without DDS binaries and libraries. This mode requires DDS to be pre-installed on worker nodes with proper environment variables set. Can also be enabled via `DDS_LIGHTWEIGHT_PACKAGE` environment variable.

* **stop** *[SESSION_ID]*  
Stop a given DDS session specified by `SESSION_ID`. If no `SESSION_ID` argument is provided, the command will stop the default DDS session. In this case, the command will ask the user to confirm the choice unless `--force` is used.

* **stop-all**  
Stop all running DDS sessions.

* **list** *{all | run}*  
List available DDS sessions. User must provide the filter criteria:
  * `all` - List absolutely all existing sessions, including expired ones
  * `run` - List only running DDS sessions

* **set-default** *SESSION_ID*  
Sets a given `SESSION_ID` as the default session ID.
The default session ID is used by all DDS commands when the user doesn't provide a session ID explicitly in the command line arguments.

* **clean** *[-f, --force]*  
Clean DDS sessions. This will remove all session-related temporary files and logs. **Be careful using this command. The operation cannot be undone.**
For safety reasons, the command confirms with the user the removal of each DDS session, but you can avoid this by providing the `-f` or `--force` argument.

## Environment Variables

* **DDS_LIGHTWEIGHT_PACKAGE**  
When set to `1`, `true`, `yes`, or `on` (case-insensitive), enables lightweight worker package mode. Command-line `--lightweight` option takes precedence over this environment variable.

## Prerequisites for Lightweight Packages

When using the `--lightweight` option or setting `DDS_LIGHTWEIGHT_PACKAGE=1`, DDS will skip worker package validation and create minimal packages (~50KB instead of ~15MB). Worker nodes **must** have:

* DDS pre-installed
* **DDS_COMMANDER_BIN_LOCATION** - Environment variable pointing to DDS binaries directory (e.g., `/opt/dds/bin`)
* **DDS_COMMANDER_LIBS_LOCATION** - Environment variable pointing to DDS libraries directory (e.g., `/opt/dds/lib`)

These variables should point to a valid DDS installation on the worker nodes. If not set or pointing to invalid locations, worker initialization will fail.

## Examples

### Start a new DDS session

```console
$ dds-session start

DDS session ID: cf84e72d-a3af-4fd8-af73-4337e9434612
Checking precompiled binaries for the local system only:
 dds-wrk-bin-2.1.12.g7619ef0-Darwin-universal.tar.gz - OK
Starting DDS commander...
Waiting for DDS Commander to appear online...
DDS commander appears online. Testing connection...
DDS commander is up and running.
------------------------
DDS commander server: 60753
------------------------
Startup time: 1061.46 ms
Default DDS session is set to cf84e72d-a3af-4fd8-af73-4337e9434612
Currently running DDS sessions:
cf84e72d-a3af-4fd8-af73-4337e9434612   [2018-08-22T11:53:34Z]   RUNNING
```

### List all sessions

```console
$ dds-session list all

   cfc8e86d-157b-404e-bde8-a32f8b3c1331   [2018-08-21T13:49:35Z]   STOPPED    
   5fdc6142-497c-433c-8333-721f05eabe31   [2018-08-21T14:10:39Z]   STOPPED
 * cf84e72d-a3af-4fd8-af73-4337e9434612   [2018-08-22T11:53:34Z]   RUNNING
```

### Start lightweight session using command-line option

```console
$ dds-session start --lightweight

DDS session ID: 12345678-1234-1234-1234-123456789abc
Starting DDS in lightweight mode - WN package validation skipped.
Note: Workers must have DDS pre-installed with DDS_COMMANDER_BIN_LOCATION and DDS_COMMANDER_LIBS_LOCATION set.
Starting DDS commander...
Waiting for DDS Commander to appear online...
DDS commander appears online. Testing connection...
DDS commander is up and running.
------------------------
DDS commander server: 12345
------------------------
Startup time: 142.56 ms
```

### Start lightweight session using environment variable

```console
$ export DDS_LIGHTWEIGHT_PACKAGE=1
$ dds-session start

DDS session ID: 87654321-4321-4321-4321-210987654321
Starting DDS in lightweight mode - WN package validation skipped.
Note: Workers must have DDS pre-installed with DDS_COMMANDER_BIN_LOCATION and DDS_COMMANDER_LIBS_LOCATION set.
Starting DDS commander...
...
```

### Stop a specific session

```console
$ dds-session stop cf84e72d-a3af-4fd8-af73-4337e9434612

Stopping DDS commander: cf84e72d-a3af-4fd8-af73-4337e9434612
Sending a graceful stop signal to Commander (pid/sessionID): 60753/cf84e72d-a3af-4fd8-af73-4337e9434612
dds-commander: self exiting (60753)...
```

### Clean sessions with force option

```console
$ dds-session clean --force

Removing: cfc8e86d-157b-404e-bde8-a32f8b3c1331
    DDS Work dir: /tmp/dds_wn_1234/...
    removed files count: 25
    DDS Log dir: /tmp/dds_log_1234/...
    removed files count: 8
```
