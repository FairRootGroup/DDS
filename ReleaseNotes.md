# DDS Release Notes

## v0.3 (NOT YET RELEASED)
### dds-topology
Fixed: respond with an error if the given topo file is missing.   

### dds-submit
Modified: Stop server communication channel if a fatal error is received from the server.   
Added: Properly reflect server messages to stdout when agents are submitted/activated.   
Added: Show more informative messages in case if the ssh plug-in failed to deploy agents.   
Added: The command remembers now all options of the last successful call.   
Added: The command learned a new command line option "--config". IT gives the possibility to specify a configuration file with predefined dds-submit options.   

### dds-protocol-lib
Improved: The protocol message header size has been reduced from 12 to 8 bytes.   
Improved: The protocol message header is validated now using CRC.

### DDS common
Improved: all DDS CLI commands use now common code to find suitable DDS commander server.   

## v0.2 (2014-09-03)

The first stable internal release.

