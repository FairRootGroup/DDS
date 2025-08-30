# dds-topology

Topology related commands. **UNIX/Linux/OSX**

## Synopsis

```shell
dds-topology [[-h, --help] | [-v, --version]] [[-V, --verbose] [--disable-validation] [-s, --session arg]] {[--activate arg] | [--update arg] | [--stop] | [--validate arg] | [--required-agents arg] | [--topology-name arg]}
```

## Description

This command allows you to perform topology-related tasks, including activating, updating, stopping, and validating DDS topologies.

## Options

* **-h, --help**  
Show usage options.

* **-v, --version**  
Show version information.

* **-V, --verbose**  
Cause the command to output additional information and error messages.

* **--disable-validation**  
Switch off topology validation during activate, update, required-agents, or topology-name operations.

* **-s, --session** *arg*  
DDS Session ID.

* **--activate** *arg*  
Request DDS to activate agents, i.e., distribute and start user tasks according to the given topology file.

* **--update** *arg*  
Request DDS to update the currently running topology with a new one.

* **--stop**  
Request DDS to stop execution of user tasks. Stop the active topology.

* **--validate** *arg*  
Validate topology file against DDS's XSD schema.

* **--required-agents** *arg*  
Get the required number of agents for the given topology file.

* **--topology-name** *arg*  
Get the name of the topology from the given topology file.

## Examples

### Validate a topology file

```console
$ dds-topology --validate my_topology.xml
Validating topology file: my_topology.xml
Topology is valid.
```

### Activate a topology

```console
$ dds-topology --activate my_topology.xml
Requesting to activate topology: my_topology.xml
Topology has been activated.
```

### Update a running topology

```console
$ dds-topology --update new_topology.xml
Requesting to update topology: new_topology.xml
Topology has been updated.
```

### Stop the active topology

```console
$ dds-topology --stop
Requesting to stop execution of user tasks.
DDS agents have been stopped.
```

### Get topology name

```console
$ dds-topology --topology-name my_topology.xml
MyTopologyName
```

### Get required number of agents

```console
$ dds-topology --required-agents my_topology.xml
25
```

### Activate with verbose output

```console
$ dds-topology --activate my_topology.xml --verbose
Validating topology file: my_topology.xml
Topology is valid.
Requesting to activate topology: my_topology.xml
Distributing tasks to agents...
All tasks have been successfully distributed.
Topology has been activated.
```

### Activate without validation

```console
$ dds-topology --activate my_topology.xml --disable-validation
Requesting to activate topology: my_topology.xml (validation disabled)
Topology has been activated.
```
