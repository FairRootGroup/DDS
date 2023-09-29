# dds-agent-cmd

Send commands to agent. **UNIX/Linux/OSX**

## Synopsis

```shell
dds-agent-cmd [[-h, --help] | [-v, --version] | [command, --command arg] | [-s, --session arg]] {[getlog arg] {[-a, --all]} | [update-key arg] {[--key arg] | [--value arg]}}
```

## Description

This utility allows to send commands to DDS agents.  
For the currently available commands see [Options](#options)

## Options

* **getlog** *arg*  
Download all log files from active agents. All files from agents' working directories with the extension `.log` will be tar/zip'ed into a single file and downloaded on DDS commander server machine into the directory specified by `server.log_dir` DDS configuration option and placed in the subdirectory "agents" (default:`~/.DDS/log/agents`)  
Usage example:

  ```shell
  dds-agent-cmd getlog -a
  ```

* **update-key** *arg*  
It forces an update of a given task's property in the topology. Name of the property and a new value should be provided additionally (see `--key` and `--value`)  
Usage example:

  ```shell
  dds-agent-cmd update-key --key mykey --value new_value
  ```

* **--key**  
Defines the key to update

* **--value**  
Defines a new value of the given key.

* **-a, --all**  
Send command to all active agents.

* **--s, --session** *arg*  
DDS Session ID.
