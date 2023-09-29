# dds-user-defaults

Get and set global DDS options. **UNIX/Linux/OSX**

## Synopsis

```shell
dds-user-defaults [[-h, --help] | [-v, --version] | [-V, --verbose] | [-p, --path] | [-d, --default]] [-c, --config arg] [-s, --session arg] [--ignore-default-sid] [--default-session-id] [--default-session-id-file] [-f, --force] [[--key arg] | [--wrkpkg] | [--wrkscript] | [--rms-sandbox-dir] | [--user-env-script] | [--server-info-file]]
```

## Description

The **dds-user-defaults** command can be used to get and set global DDS options. It also can be used to get different static settings, related to the current deployment.

## Options

* **-h, --help**  
Shows usage options.

* **-v, --version**  
Shows version information.

* **-V, --verbose**  
Causes the command to verbose additional information and error messages.

* **-p, --path**  
Shows default DDS user defaults config file path.

* **-d, --default**  
Generates a default DDS configuration file.

* **-f, --force**  
If the destination file exists, removes it and creates a new file, without prompting for confirmation.  
Can only be used with the `-d, --default` options.

* **-c, --config** *arg*  
This options can be used together with other options to specify non-default location of the DDS configuration file.  
By default the command uses `~/.DDS/DDS.cfg`.

* **-s, --session** *arg*  
Use the specified DDS Session ID instead of a default one.

* **--ignore-default-sid**  
Force to ignore a default sid.

* **--default-session-id**  
Show the current default session ID.

* **--default-session-id-file**  
Show the full path of the default session ID file.

* **--key *arg***  
Gets a value for the given key from the DDS user defaults.

* **--wrkpkg**  
Shows the full path of the worker package. The path must be evaluated before use.

* **--wrkscript**  
Shows the full path of the worker script. The path must be evaluated before use.

* **--rms-sandbox-dir**  
Shows the full path of the RMS sandbox directory. It returns server.sandbox_dir if it's not empty, otherwise server.work_dir is returned. The path must be evaluated before use.

* **--user-env-script**  
Shows the full path of user's environment script for workers (if present). The path must be evaluated before use.

* **--server-info-file**  
Shows the full path of the DDS server info file. The path must be evaluated before use.
