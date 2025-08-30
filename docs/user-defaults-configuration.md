# DDS User Defaults Configuration Reference

This document provides a comprehensive reference for all DDS user defaults configuration options. User defaults allow you to customize DDS behavior and specify various settings for the server, agents, and logging.

## Overview

DDS user defaults are stored in a configuration file, by default located at `~/.DDS/DDS.cfg`. You can:

- Generate a default configuration file: `dds-user-defaults --default`
- View current configuration file path: `dds-user-defaults --path`
- Get specific configuration values: `dds-user-defaults --key <key_name>`
- Use custom configuration file: `dds-user-defaults --config /path/to/custom.cfg`

For detailed CLI usage, see the [dds-user-defaults command reference](../dds-user-defaults/README.md).

## Configuration Sections

### [server] Section

Server-related configuration options that control DDS commander behavior.

#### Core Directory Settings

| Setting       | Default Value    | Description                                                                                                                                   |
| ------------- | ---------------- | --------------------------------------------------------------------------------------------------------------------------------------------- |
| `work_dir`    | `$HOME/.DDS`     | Main working directory for DDS. All DDS data, session information, and temporary files are stored here.                                       |
| `sandbox_dir` | `$HOME/.DDS`     | Directory used for worker packages. Useful when RMS can't access the main working directory. In most cases, should be the same as `work_dir`. |
| `log_dir`     | `$HOME/.DDS/log` | Directory where DDS logs are stored. Each session creates a subdirectory here.                                                                |

#### Logging Configuration

| Setting                  | Default Value | Description                                                                                                                                                                             |
| ------------------------ | ------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `log_severity_level`     | `info`        | Minimum log level to record. Options: `p_l` (protocol low), `p_m` (protocol medium), `p_h` (protocol high), `dbg` (debug), `inf` (info), `wrn` (warning), `err` (error), `fat` (fatal). |
| `log_rotation_size`      | `10`          | Maximum log file size in MB before rotation occurs.                                                                                                                                     |
| `log_has_console_output` | `true`        | Whether to output log messages to console in addition to log files.                                                                                                                     |

#### Network Configuration

| Setting                    | Default Value | Description                                                                               |
| -------------------------- | ------------- | ----------------------------------------------------------------------------------------- |
| `commander_port_range_min` | `20000`       | Minimum port number for DDS commander connections. Must be open for incoming connections. |
| `commander_port_range_max` | `21000`       | Maximum port number for DDS commander connections. Must be open for incoming connections. |

#### Session Management

| Setting          | Default Value | Description                                                                                              |
| ---------------- | ------------- | -------------------------------------------------------------------------------------------------------- |
| `idle_time`      | `1800`        | Idle time in seconds after which DDS processes may be terminated by monitoring thread.                   |
| `data_retention` | `7`           | Number of days to keep DDS sessions. Non-running sessions older than this will be automatically deleted. |

#### Agent Health Monitoring

| Setting                       | Default Value | Description                                                                                                     |
| ----------------------------- | ------------- | --------------------------------------------------------------------------------------------------------------- |
| `agent_health_check_interval` | `30`          | How often (in seconds) the Commander checks agent health status.                                                |
| `agent_health_check_timeout`  | `30`          | Timeout in seconds for agent health checks. If an agent doesn't respond within this time, it's considered dead. |

### [agent] Section

Agent-related configuration options that control worker node behavior.

#### Agent Directory Settings

| Setting    | Default Value | Description                                                                                                                                                                                            |
| ---------- | ------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `work_dir` | *(empty)*     | Custom working directory for agents. If empty, agents use a directory under `server.sandbox_dir`. **Note:** This option is ignored by localhost and SSH plug-ins. It's recommended to keep this empty. |

#### File Permissions

| Setting              | Default Value | Description                                                                                                                                                                                                                        |
| -------------------- | ------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `access_permissions` | `0660`        | File permissions (in octal) applied to agent-side files, particularly user task log files (stdout/stderr). Common values: `0644` (read for all, write for owner), `0660` (read/write for owner/group), `0444` (read-only for all). |

#### Resource Management

| Setting                | Default Value | Description                                                                                                                   |
| ---------------------- | ------------- | ----------------------------------------------------------------------------------------------------------------------------- |
| `disk_space_threshold` | `500`         | Minimum free disk space in MB. Agent will shut down if free space falls below this threshold. Set to `0` to disable checking. |

## Configuration File Example

Here's a complete example configuration file with commonly used settings:

```ini
# DDS user defaults
# version: 2.2

[server]
# Core directories
work_dir=$HOME/.DDS
sandbox_dir=$HOME/.DDS
log_dir=$HOME/.DDS/log

# Logging configuration
log_severity_level=info
log_rotation_size=10
log_has_console_output=true

# Network settings
commander_port_range_min=20000
commander_port_range_max=21000

# Session management
idle_time=1800
data_retention=7

# Agent health monitoring
agent_health_check_interval=30
agent_health_check_timeout=30

[agent]
# Agent configuration
work_dir=
access_permissions=0660
disk_space_threshold=500
```

## Common Use Cases

### Custom Working Directory

To use a custom working directory (e.g., on a faster filesystem):

```ini
[server]
work_dir=/fast/storage/dds
sandbox_dir=/fast/storage/dds
log_dir=/fast/storage/dds/log
```

### High-Performance Setup

For high-throughput deployments:

```ini
[server]
log_severity_level=wrn
log_has_console_output=false
agent_health_check_interval=60
agent_health_check_timeout=60
```

### Development/Debug Setup

For development and debugging:

```ini
[server]
log_severity_level=dbg
log_has_console_output=true
agent_health_check_interval=10
agent_health_check_timeout=15
```

### Restricted Network Environment

For environments with limited port ranges:

```ini
[server]
commander_port_range_min=22000
commander_port_range_max=22010
```

## File Permissions Reference

The `agent.access_permissions` setting uses octal notation:

| Octal | Binary | Permission |
| ----- | ------ | ---------- |
| 4     | 100    | Read       |
| 2     | 010    | Write      |
| 1     | 001    | Execute    |

Combine values for each user class (owner, group, world):

- `0644`: Owner read/write, group/world read-only
- `0660`: Owner/group read/write, world no access
- `0755`: Owner read/write/execute, group/world read/execute
- `0444`: Read-only for everyone

## Log Severity Levels

| Level           | Code  | Description                             |
| --------------- | ----- | --------------------------------------- |
| Protocol Low    | `p_l` | Low-level protocol events and higher    |
| Protocol Medium | `p_m` | Medium-level protocol events and higher |
| Protocol High   | `p_h` | High-level protocol events and higher   |
| Debug           | `dbg` | General debug events and higher         |
| Info            | `inf` | Informational messages and higher       |
| Warning         | `wrn` | Warning messages and higher             |
| Error           | `err` | Error messages and higher               |
| Fatal           | `fat` | Fatal error messages only               |

## Environment Variable Expansion

Configuration values support environment variable expansion using `$VARIABLE` or `${VARIABLE}` syntax:

```ini
work_dir=$HOME/.DDS
log_dir=${HOME}/logs/dds
sandbox_dir=${TMPDIR}/dds-sandbox
```

## Related Documentation

- [dds-user-defaults CLI Reference](../dds-user-defaults/README.md) - Command-line interface documentation
- [How to Start](how-to-start.md) - Getting started with DDS
- [Requirements](requirements.md) - Network and system requirements
