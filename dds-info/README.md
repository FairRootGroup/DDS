# dds-info

It an be used to query different kinds of information from DDS commander server. **UNIX/Linux/OSX**

## Synopsis

```shell
dds-info [[-h, --help] | [-v, --version]] [[-s, --session arg] | [--commander-pid] | [--status] | [-n, --active-count] | [-l, --agents-list] | [--idle-count] | [--executing-count] | [--wait-count arg] | [--active-topology]]
```

## Description

The command can be used to query different kinds of information from DDS commander server.

## Options

* **-h, --help**  
Shows usage options.

* **-v, --version**  
Shows version information.

* **-s, --session arg**  
DDS Session ID.

* **--commander-pid**  
Return the pid of the commander server.

* **--status**  
Query current status of DDS commander server.

* **-n, --active-count**  
Returns a number of online slots.

* **-l, --agents-list**  
Show detailed info about all online agents.

* **--idle-count**  
Returns a number of idle slots.

* **--executing-count**  
Returns a number of executing slots.

* **--wait-count** *arg*  
The command will block infinitely until a required number of agents are available. Must be used together with `--active-count`, `--idle-count` or `--executing-count`.

* **--active-topology**  
Returns the name of the active topology.
