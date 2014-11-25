# DDS Release Notes

## v0.6 (NOT YET RELEASED)
### DDS common   
Modified: Build WN packages without ICU support. (GH-14)   

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

