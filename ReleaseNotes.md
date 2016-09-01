# DDS Release Notes

## v1.4 (Not Yet Released)

### DDS common
Modified: pipe log engine is improved to log events line by line, rather than using a fixed string length.   

### DDS protocol
Added: shared memory message queue transport. New shared memory channel which is based on  boost::message_queue. Pushing and receiving of commands is done via shared memory. In some cases this can significantly improve communication speed. (GH-129, GH-130, GH-131)

### SSH plug-in
Modified: events from the submitter script are reflected on dds-submit output. (GH-139)   

### dds_intercom\_lib
Added: reconnect if connection fails. (GH-138)
Added: possibility to subscribe to the error messages.
Added: new shared memory transport is used in dds_intercom_lib for key-value propagation and custom commands. (GH-129, GH-130, GH-131)
Modified: Shared memory transport allows to improve the user API. DDS garantees that update key notification callback will be called on each update key or delete key command. Users are responsible to store the local cache for key-value if required. (GH-129, GH-130, GH-131)

### dds-topology
Fixed: wrong dds-topology --stop output. (GH-146)   

## v1.2 (2016-06-07)
###DDS common
Fixed: cmake: Updated OSX RPATH settings.   
Fixed: cmake: Fail with an explicit error when missing DDS worker package dependency. (GH-117)   
Fixed: dds-intercom-lib: fails to parse JASON message with quotes. (GH-120)   
Added: dds-intercom-lib: API for new plug-in system - CRMSPluginProtocol. (GH-108)   
Modified: dds-key-value-lib and dds-custom-cmd-lib are combined to a single library dds_intercom_lib. (GH-101)   
Modified: Use portable temporary directory path function.  
Modified: Proper error message if DDS can't find xmllint. (GH-140)    

### dds-submit
Added: Support of the new plug-in architecture. (GH-108)   
Added: the command learned "--config/-c" parameter, which can be used to specify a configuration file for plug-ins. (GH-111)   
Added: the command learned "--list/-l" parameter, which lists all available RMS plug-ins. (GH-112)   
Modified: drop support of "--ssh-rms-cfg" in favor of "--config". (GH-111)   
Modified: drop support of auto-config feature of dds-submit, when it remembers last used settings. (GH-111)   


### dds-protocol-lib
Added: maximum message size for key-value and custom commands (GH-104)  
Added: sending of arrays (GH-105)   
Added: sending of strings (GH-106)   
Added: improve protocol attachment architecture. Check maximum size for vectors and strings in commands. Size limitations:   
  -   all vectors (except uint8_t) have a maximum size of uint16_t i.e. 2^16;   
  -   all vector<uint8_t>'s have a maximum size of uint32_t i.e. 2^32;   
  -   all std::string's have a maximum size of uint16_t i.e. 2^16.   

### SLURM plug-in
Added: SLURM plug-in - initial release. (GH-109)   

### SSH plug-in
Added: New SSH plug-in - initial release. (GH-108)   

### localhost plug-in
Added: Initial release. (GH-115)    

### dds-daemonize
Fixed: failed to execute if the full path to the executable is provided. (GH-121)   

### dds-submit
Added: customizable plugin location. --path option which specifies the root directory of the plugins was edded. If the directory is not provided - default path will be used. (GH-118)   
Modified: accept both -n and -c command line options.   

## v1.0 (2015-11-20)
###DDS common
Fixed: git error when using out of source builds (GH-85)    
Fixed: a class name lookup issues, which could result in unpredictable behavior during run-time (agent and key-value-lib had classes with the same name and same header protection).    
Fixed: check DDS_LOCATION before agent start. (GH-98)   
Fixed: since Mac OS 10.11 (El Capitan) DYLD_LIBRARY_PATH is not exported in the sub-shell environment. We explicitly set DYLD_LIBRARY_PATH to the libraries directory.  
Added: Give users a possibility to specify task requirement based on worker node name in the SSH configuration. Name can be specified as regular expression. (GH-88)   
Added: extend error message in case if a worker package is missing. (GH-89)   
Added: statistics accumulation: message size, message queue size for read and write operations is accumulated. (GH-99)   
Added: new dds-stat command is introduced with possible options: enable, disable and get for statistics accumulation. (GH-99)    
Added: possibility to send custom commands from user tasks or utils. New library dds-custom-cmd-lib is introduced. (GH-100)   
Added: DDS Tutorial2 which introduces the use of the new custom dds-custom-cmd-lib library. (GH-100)   
Added: DDS environment properties API - DDSEnvProp (GH-92)   

### dds-key-value
Fixed: Removed sys. signals handler. A user process now is responsible to catch signals if needed. (GH-97)    

### dds-submit
Added: the command learned a localhost RMS. (GH-93)   

### dds-server
Fixed: Check that DDS_LOCATION is set. (GH-86)    

### dds scout
Modified: log pre-execution env to make sure environment is correct. (GH-67)   

## v0.10 (2015-07-16)
### DDS common
Added: handlers of the monitoring thread can be registered now with custom call intervals. (GH-63)   
Added: accumulated push message function. (GH-64)   
Added: include std c++ lib into worker package. (GH-61)   
Added: nicer logging on monitoring thread actions. (GH-80)     
Added: additional log levels. DDS has learned 3 new levels of protocol log events. (GH-49)    
Added: group name, collection name, task name and task path are exported as environment variables for each task. (GH-95)    
Added: DDS Tutorial1    
Fixed: fix implementation of cmdSHUTDOWN. (GH-65)   
Fixed: remove shared memory on exit.    
Fixed: fix monitoring thread to prevent breaks if custom callbacks throw exceptions. (GH-80)  
Modified: Name of task output file changed to "user_task_<datetime>_<task_id>_<out/err>.log". (GH-75)   

### dds scout
Modified: New lock algorithm, instead of the lockfile command.    

### dds-commander
Added: Since dds-commander is a daemon and doesn't have a console, it now has a dedicated log file for its std-out/-err called "dds-commander.out.log". File is located in the log directory.   

### dds-topology
Added: output time spent on activation. (GH-62)    
Added: the command learned "--set" parameter, which is used to set up topology for the current deployment. (GH-56)   
Added: the command learned "--disiable-validation", which is used to disable topology validation. It can be used only together with "--set". (GH-56)   
Added: scheduling and requirements for the collections. (GH-76)    
Added: index for tasks and collections which are in groups. (GH-72)   
Added: new test for task and collection indices. (GH-72)   
Added: variable definition in the topology. (GH-71)

### dds-submit
Modified: removed "--topo" parameter. (GH-56)   
Modified: removed "--disable-xml-validation" parameter. (GH-56)   

### dds-agent
Fixed: reconnect to DDS commander if connection was dropped. (GH-77)   
Fixed: after reconnection to commander server key update won't be propagate from the effected agent. (GH-81)    
Modified: Optimized key-value persistence to shared memory.   
Modified: User log file name starts from the name of the task. (GH-96)    

### dds-key-value
Fixed: stability improvements.    
Fixed: multiple protections for a case when a user process calls key/value API, but the corresponding agent is offline. (GH-87)    
Added: Multiple subscribers for key-value notifications. (GH-70)   
Added: If task can only read property then property will not be propagated. (GH-55)   
Added: User task can subscribe to error events, for example, error will be send if property can not be propagated. (GH-55)   

### dds-user-defaults
Modified: use string log severity values instead of numbers. (GH-49)    

## v0.8 (2015-02-17)
### DDS common
Fixed: idle time calculation for dds-commander and dds-agent. (GH-32)   
Fixed: a bug, which prevented log files to rotate.    
Fixed: reaching the idle timeout causes Commander and Agents to exit even if user processes are still running. (GH-54)    
Added: Log rotation: maximum total size of the stored log files is 1GB. (GH-36)    
Added: Log rotation: minimum free space on the drive after which older log files will be deleted is 2GB. (GH-36)    
Added: User's task stdout/err on WNs are automatically written in dedicated log files, user_task_<TASK_ID>_out.log and user_task_<TASK_ID>_err.log accordingly. (GH-26)   
Added: Progress display for "dds-agent-cmd getlog", "dds-topology --activate" and "dds-test -t" in percent. Optionally full verbose messages can be displayed with --verbose option. (GH-42)   
Added: Broadcast property deletion on task exit. (GH-28)   
Added: property propagation types. (GH-30)   

### dds-commander
Added: State of agents. (GH-27)    

### dds-user-defaults
Modified: Default log level is changed to 1 (instead of 0).   
Modified: Log rotation: default log rotations size in MB instead of bytes. (default is 10 MB).   

### dds-ssh
Added: the ssh plug-in has been extended to support multiple agents per host. (GH-25)   
Added: each DDS scout uses separate stderr/-out file (scout.log), when more than one worker requested per machine.   

### dds-key-value
Added: users are now able to subscribe on properties update events. (GH-29)   
Added: shared memory storage for key-value. (GH-35)   

### dds-protocol-lib
Modified: The DDS transport learned to accumulate commands before sending, instead of sending them one by one. (GH-38)   
Modified: Hand-shake messages are prioritized now. DDS doesn't send/accept any other message until hand-shake is successful. (GH-37)   
Fixed: Revised write message algorithms. It is also faster now.    
Fixed: a bug in the dds-agent, which could cause a SEGFAULT when trying to access a deleted channel object on disconnect.    
Added: Implemented callbacks (signals) in BaseChannelImpl for different channel events like connect, disconnect, handshakeOK, handshakeFailed. (GH-41)   
Fixed: Stability improvements. Handling edge cases which could occur during channel destruction.

### dds-info
Added: taskId and task name to console output (dds-info -l). (GH-33)    
Added: possibility to get property list and property values from agents. (GH-52)

### dds-topology
Added: Users are now able to stop (restart) execution of tasks by calling "dds-topology --stop". To restart call: "dds-topology --stop" and "dds-topology --activate". (GH-31)    
Fixed: a bug, which caused a crash when topology activate is called before dds-submit. (GH-51)   

## v0.6 (2014-12-05)
### DDS common   
Modified: Build WN packages without ICU support. (GH-14)   
Added: key-value propagation support. (GH-12)   
Added: key-value propagation API lib. (GH-11)   
Fixed: Arguments of the task executable could contain slashes.   
Added: Simple scheduler for SSH which takes into account requirements. (GH-20)   
Added: Startup time of agents. It can be requested via git-info -l. (GH-3)   

### dds-protocol-lib
Modified: Version changed to v2.0.   
Added: The protocol has learned a new command - cmdUPDATE_KEY. (GH-12)   
Added: BinaryAttachment command learned to resolve environment variables in source files paths.

### dds-topology
Added: a possibility to use comments in the topology XML file. (GH-15)   
Renamed: dds-topology renamed to dds-topology-lib. dds-topology is executable now.   
Added: task activation functionality is moved from dds-submit to dds-topology.(GH-16)   

###dds-agent-cmd
Added: new command for communication with agents.(GH-17)   
Added: getlog functionality moved to dds-agent-cmd.(GH-17)   
Added: dds-agent-cmd learned a new command - update-key. It forces an update of a given task's property in the topology. (GH-12)   

## v0.4 (2014-10-24)
### DDS common
Added: DDS learned how to expand given user tasks commands with arguments given as a single string. (in the Topology->Task->exec parameter).   
Added: if a user's task is defined in the topology as not reachable, then DDS will take care of delivering it to worker nodes. (GH-6)   
Improved: all DDS CLI commands use now common code to find suitable DDS commander server.   
Modified: Updated User's manual.   
Modified: Improved stability.  

### dds-topology
Fixed: respond with an error if the given topo file is missing.   
Modified: the topology description schema has been revised. See User's manual for more details.   
Added: topology learned a new users' task attribute - "reachable". It defines whether executable is available on worker nodes. (GH-6)

### dds-submit
Modified: Stop server communication channel if a fatal error is received from the server.   
Added: Properly reflect server messages to stdout when agents are submitted/activated.   
Added: Show more informative messages in case if the ssh plug-in failed to deploy agents.   
Added: The command remembers now all options of the last successful call.   
Added: The command learned a new command line option "--config". It gives the possibility to specify a configuration file with predefined dds-submit options.   

### dds-protocol-lib
Improved: The protocol message header size has been reduced from 12 to 8 bytes.   
Improved: The protocol message header is validated now using CRC.   
Improved: Split binary files uploads into multiple message chunks, instead of using one message per file.

## v0.2 (2014-09-03)

The first stable internal release.
