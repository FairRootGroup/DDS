# dds-info

Query different kinds of information from DDS commander server. **UNIX/Linux/OSX**

## Synopsis

```shell
dds-info [[-h, --help] | [-v, --version]] [[-s, --session arg] | [--commander-pid] | [--status] | [-n, --active-count] | [-l, --agent-list] | [--slot-list] | [--idle-count] | [--executing-count] | [--wait arg] | [--active-topology]]
```

## Description

The command can be used to query different kinds of information from DDS commander server, including agent status, slot counts, topology information, and server process details.

## Options

* **-h, --help**  
Show usage options.

* **-v, --version**  
Show version information.

* **-s, --session** *arg*  
DDS Session ID.

* **--commander-pid**  
Return the process ID (PID) of the commander server.

* **--status**  
Query current status of DDS commander server.

* **-n, --active-count**  
Return the number of online slots.

* **-l, --agent-list**  
Show detailed information about all online agents.

* **--slot-list**  
Show detailed information about all online slots.

* **--idle-count**  
Return the number of idle slots.

* **--executing-count**  
Return the number of executing slots.

* **--wait** *arg*  
The command will block infinitely until a required number of slots are available. Must be used together with `--active-count`, `--idle-count`, or `--executing-count`. The argument specifies the minimum number of slots to wait for.

* **--active-topology**  
Return the name and path of the active topology.

## Examples

### Check commander server status

```console
$ dds-info --status
DDS commander server process (12345) is running...
```

### Get commander server process ID

```console
$ dds-info --commander-pid
12345
```

### Get number of active slots

```console
$ dds-info --active-count
50
```

### Get number of idle slots

```console
$ dds-info --idle-count
25
```

### Get number of executing slots

```console
$ dds-info --executing-count
25
```

### List all online agents

```console
$ dds-info --agent-list
Agent 0: id (12345678), pid (98765), group name (common), startup time (1.23s), slots total/executing/idle (10/5/5), host (user@hostname), wrkDir ("/tmp/dds_wn_12345")
Agent 1: id (12345679), pid (98766), group name (common), startup time (1.45s), slots total/executing/idle (10/0/10), host (user@hostname), wrkDir ("/tmp/dds_wn_12346")
```

### List all online slots

```console
$ dds-info --slot-list
Slot 0: agentID (12345678), slotID (1), taskID (task_1), state (executing), host (hostname), wrkDir ("/tmp/dds_wn_12345/slot_1")
Slot 1: agentID (12345678), slotID (2), taskID (0), state (idle), host (hostname), wrkDir ("/tmp/dds_wn_12345/slot_2")
```

### Get active topology information

```console
$ dds-info --active-topology
active topology: example_topology; path: /path/to/topology.xml
```

### Wait for a specific number of active slots

```console
$ dds-info --active-count --wait 100
Active agents online: 100
Idle agents online: 50
Executing agents online: 50
```

### Wait for idle slots to become available

```console
$ dds-info --idle-count --wait 10
Active agents online: 75
Idle agents online: 10
Executing agents online: 65
```
