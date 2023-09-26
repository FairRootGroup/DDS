# dds-submimt

Submits and activates DDS agents UNIX/Linux/OSX

## Synopsis

```shell
dds-submit [[-h, --help] | [-v, --version]] [-l, --list] [-r, --rms arg] [-s, --session arg] {[-c, --config arg] | [-n, --number arg] | [-s, --slots arg]}
```

## Description

The command is used to submit DDS agents to allocate resources for user tasks. Once enough agents are online use the [dds-topology](../dds-topology/README.md) command to activate the agents - i.e. distribute user tasks across agents and start them.

## Options

* **-h, --help**  
Shows usage options.  

* **-v, --version**  
Shows version information.

* **--l, --list** *arg*  
List all available RMS plug-ins.

* **--r, --rms** *arg*  
Defines a destination resource management system plug-in. Use `--list` to find out names of available RMS plug-ins.

* **--s, --session** *arg*  
DDS Session ID.

* **--path** *arg*  
Defines a path to the root plug-ins directory. If not specified than default root plug-ins directory is used.

* **-c, --config** *arg*  
A plug-in's configuration file. It can be used to provide additional RMS options.

* **-n, --number** *arg*  
Defines a number of agents to spawn. This option can not be mixed with `--config`.

* **-s, --slots** *arg*  
Defines a number of task slots per agent. This option can not be mixed with `--config`.
