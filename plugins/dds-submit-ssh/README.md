# SSH RMS plug-in

## Resource definition

DDS's SSH plug-in is capable to deploy DDS agents on any resource machine available for password-less access (public key, ssh agent, etc.).  
In order to define resources for the SSH plug-in we use a comma-separated values (CSV) configuration file, in cases if you want to deploy agents on several computing nodes.  
The ssh plug-in can also spawn agents on the local machine only. In this case you don't need a configuration file - just use `dds-submit -n X`, where X is a desired number of agents to spawn.  

As for the configuration file. Fields are normally separated by commas. If you want to put a comma in a field, you need to decorate it with quotes around. Also 3 escape sequences are supported.

**DDS's SSH plug-in configuration fields:**

1. An id (must be any unique string).  
This id string is used just to distinguish different DDS workers in the plug-in.
1. A host name with or without a login, in a form: `login@host.fqdn`
1. Additional SSH params (could be empty).
1. A remote working directory.
1. A number of agents to spawn.

**An example of the SSH plug-in configuration file:**

```csv
r1, anar@lxg0527.gsi.de, -p24, /tmp/test, 10
# this is a comment
r2, user@lxi001.gsi.de,,/home/user/dds,10
125, user2@host, , /tmp/test,
```

## Usage example

Call using a given configuration file:

```shell
dds-submit -r ssh -c your-ssh-Resource-definition-config-file
```

Call using a local system only to spawn 10 DDS agents on it:

```shell
dds-submit -r ssh -n 10
```
