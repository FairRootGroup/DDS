# dds-agent-cmd

Send commands to DDS agents. **UNIX/Linux/OSX**

## Synopsis

```shell
dds-agent-cmd [[-h, --help] | [-v, --version]] [--verbose] [-s, --session arg] {getlog [-a, --all]}
```

## Description

This utility allows you to send commands to DDS agents.  
Currently supports retrieving log files from worker nodes.

## Options

* **-h, --help**  
Show usage options.

* **-v, --version**  
Show version information.

* **--verbose**  
Enable verbose output.

* **-s, --session** *arg*  
DDS Session ID.

* **getlog**  
Download all log files from active agents. All files from agents' working directories with the extension `.log` will be tar/zip'ed into a single file and downloaded to the DDS commander server machine into the directory specified by `server.log_dir` DDS configuration option and placed in the subdirectory "agents" (default: `~/.DDS/log/agents`). For more details about this configuration option, see the [User Defaults Configuration Reference](../docs/user-defaults-configuration.md).

* **-a, --all**  
Send command to all active agents. Must be used with `getlog` command.

## Examples

### Download log files from all active agents

```console
$ dds-agent-cmd getlog --all
Retrieving log files from worker nodes...
Files will be saved in ~/.DDS/sessions/12345678-1234-1234-1234-123456789abc/log/agents
Log files downloaded successfully.
```

### Download log files with verbose output

```console
$ dds-agent-cmd getlog --all --verbose
Sending getlog command to all active agents...
Agent 12345678: Collecting log files...
Agent 12345679: Collecting log files...
Files are being downloaded to ~/.DDS/sessions/12345678-1234-1234-1234-123456789abc/log/agents
Download completed successfully.
```

### Download log files for specific session

```console
$ dds-agent-cmd getlog --all --session 87654321-4321-4321-4321-210987654321
Retrieving log files from worker nodes...
Files will be saved in ~/.DDS/sessions/87654321-4321-4321-4321-210987654321/log/agents
Log files downloaded successfully.
```
