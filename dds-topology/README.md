# dds-topology

Topology related commands. **UNIX/Linux/OSX**

## Synopsis

```shell
dds-topology [[-h, --help] | [-v, --version] | [-V, --verbose] [[--disable-validation]] | [-s, --session arg] | [--activate arg] | [--stop] | [--update arg] | [--validate arg] | [--topology-name arg]]
```

## Description

This command allows to perform topology related tasks.

## Options

* **-h, --help**  
Shows usage options.

* **-v, --version**  
Shows version information.

* **-V, --verbose**  
Causes the command to verbose additional information and error messages.

* **--disable-validation**  
Switches off topology validation.

* **--s, --session** *arg*  
DDS Session ID.

* **--activate** *arg*  
Requests DDS to activate agents, i.e. distribute and start user tasks according to the given topology.

* **--update** *arg*  
Requests DDS to update currently running topology with a new one.

* **--stop**  
Requests DDS to stop execution of user tasks. Stop the active topology.

* **--validate** *arg*  
Validates topology file against DDS's XSD schema.

* **--topology-name** *arg*  
Get the name of the topology for a given topology file.
