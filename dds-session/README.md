# dds-session

Start/Stop DDS commander and manage DDS sessions. **UNIX/Linux/OSX**

## Synopsis

```shell
dds-session {[[start --mixed] | [stop SESSION_ID] | [stop_all] | [list all | run]] | [set-default SESSION_ID] | [clean -f]}
```

## Description

Using this command users can perform a set of operations on DDS sessions, such as start/stop DDS server by creating new and stopping existing sessions. Users can also list available sessions or clean expired ones.

One user can start multiple DDS sessions. Each session will have its own DDS commander instance and will be sandboxed, i.e. won't disturb other sessions of the same user.

## Options

* **start**  
Start a new DDS session. DDS will automatically set the newly created session as a default one.
A single user can start as many DDS sessions as desired. Users are limited only by the resources of underlying system.  
Each DDS session spawns its own commander server. All sessions are completely isolated from each other.  
At the server start DDS will test availability of DDS WN bin. packages and download them from the DDS repository if they are missing. If the user provides `--mixed parameter`, then WN packages for all systems (Linux, OS X) will be checked. By default DDS checks only for a package compatible with the local system only.  
To build a binary package for the local system, just issue:

  ```shell
  make -j wn_bin
  make -j install
  ```

* **stop**  
Stop a given DDS session specified by `SESSION_ID`. If no `SESSION_ID` argument is provided, the command will stop the default DDS session. But in this case the command will ask user to confirm the choice.

* **stop_all**  
Stop all running DDS sessions.

* **list**  
List available DDS sessions. User must provide the filter criteria, either all or run
With all the command will list absolutely all existing sessions, including expired ones.
With run the command will list only running DDS sessions.

* **set-default**  
Sets a given `SESSION_ID` as a default session ID.
The default session ID is used by all DDS commands, when user doesn't provide a session ID explicitly in the command line arguments.

* **clean**  
The command cleans DDS sessions. It will remove all session related temporary files and logs. Be careful using this command. The operation can't be undone.
For safety reason the command confirms with the user removal of each DDS session, but you can avoid this by providing the `-f` argument.

**Usage example:**

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
    
$ dds-session list all

   cfc8e86d-157b-404e-bde8-a32f8b3c1331   [2018-08-21T13:49:35Z]   STOPPED    
   5fdc6142-497c-433c-8333-721f05eabe31   [2018-08-21T14:10:39Z]   STOPPED
 * cf84e72d-a3af-4fd8-af73-4337e9434612   [2018-08-22T11:53:34Z]   RUNNING

    
    
$ dds-session stop cf84e72d-a3af-4fd8-af73-4337e9434612

Stopping DDS commander: cf84e72d-a3af-4fd8-af73-4337e9434612
Sending a graceful stop signal to Commander (pid/sessionID): 60753/cf84e72d-a3af-4fd8-af73-4337e9434612
dds-commander: self exiting (60753)...
   
```
