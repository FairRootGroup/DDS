# dds-user-defaults

Get and set global DDS options. **UNIX/Linux/OSX**

## Synopsis

```shell
dds-user-defaults [[-h, --help] | [-v, --version]] [[-p, --path] | [-d, --default [-f, --force]] | [-c, --config arg] [-s, --session arg] [--submission-id arg] [--ignore-default-sid] [--default-session-id] [--default-session-id-file] [[--key arg] | [--wrkpkg] | [--wrkscript] | [--rms-sandbox-dir] | [--user-env-script] | [--server-info-file] | [--session-id-file]]]
```

## Description

The **dds-user-defaults** command can be used to get and set global DDS options. It can also be used to get different static settings related to the current deployment.

For a comprehensive reference of all available configuration options, their defaults, and descriptions, see the [User Defaults Configuration Reference](../docs/user-defaults-configuration.md).

## Options

* **-h, --help**  
Show usage options.

* **-v, --version**  
Show version information.

* **-p, --path**  
Show default DDS user defaults config file path.

* **-d, --default**  
Generate a default DDS configuration file.

* **-f, --force**  
If the destination file exists, remove it and create a new file, without prompting for confirmation.  
Can only be used with the `-d, --default` option.

* **-c, --config** *arg*  
This option can be used together with other options to specify non-default location of the DDS configuration file.  
By default the command uses `~/.DDS/DDS.cfg`.

* **-s, --session** *arg*  
Use the specified DDS Session ID instead of the default one.

* **--submission-id** *arg*  
Specify the Submission ID. Required for --wrkpkg and --wrkscript options.

* **--ignore-default-sid**  
Force ignoring the default session ID.

* **--default-session-id**  
Show the current default session ID.

* **--default-session-id-file**  
Show the full path of the default session ID file.

* **--key** *arg*  
Get a value for the given key from the DDS user defaults.

* **--wrkpkg**  
Show the full path of the worker package. The path must be evaluated before use.

* **--wrkscript**  
Show the full path of the worker script. The path must be evaluated before use.

* **--rms-sandbox-dir**  
Show the full path of the RMS sandbox directory. It returns server.sandbox_dir if it's not empty, otherwise server.work_dir is returned. The path must be evaluated before use.

* **--user-env-script**  
Show the full path of user's environment script for workers (if present). The path must be evaluated before use.

* **--server-info-file**  
Show the full path of the DDS server info file. The path must be evaluated before use.

* **--session-id-file**  
Show the full path of the session ID file of the local environment.

## Examples

### Show config file path

```console
$ dds-user-defaults --path
/home/user/.DDS/DDS.cfg
```

### Generate default config file

```console
$ dds-user-defaults --default
Generating the default DDS configuration file...
Generating the default DDS configuration file - DONE.
```

### Generate default config file with force overwrite

```console
$ dds-user-defaults --default --force
Generating the default DDS configuration file...
Generating the default DDS configuration file - DONE.
```

### Get a specific configuration key value

```console
$ dds-user-defaults --key server.work_dir
$HOME/.DDS

$ dds-user-defaults --key server.log_dir
$HOME/.DDS/log

$ dds-user-defaults --key server.port_range_min
20000
```

### Get current default session ID

```console
$ dds-user-defaults --default-session-id
12345678-1234-1234-1234-123456789abc
```

### Get server info file path

```console
$ dds-user-defaults --server-info-file
/home/user/.DDS/sessions/12345678-1234-1234-1234-123456789abc/server_info.cfg
```

### Get RMS sandbox directory

```console
$ dds-user-defaults --rms-sandbox-dir
/home/user/.DDS
```

### Get worker package path

```console
$ dds-user-defaults --wrkpkg --submission-id sub123
/home/user/.DDS/sessions/12345678-1234-1234-1234-123456789abc/wn_bins/dds-wrk-bin-3.0.0-Linux-x86_64.tar.gz
```

### Get worker script path

```console
$ dds-user-defaults --wrkscript --submission-id sub123
/home/user/.DDS/sessions/12345678-1234-1234-1234-123456789abc/wn_bins/dds_wn_worker.sh
```

### Use custom config file

```console
$ dds-user-defaults --config /path/to/custom/DDS.cfg --key server.work_dir
/custom/work/directory
```

### Use specific session ID

```console
$ dds-user-defaults --session 87654321-4321-4321-4321-210987654321 --session-id-file
/home/user/.DDS/sessions/87654321-4321-4321-4321-210987654321/session_id
```
