# localhost RMS plug-in

## Introduction

DDS's localhost plug-in is capable to deploy DDS agents on a local machine. Unlike SSH plug-in, localhost plug-in doesn't require a password-less access (public key, ssh agent, etc.).  
The configuration file is not required for localhost plug-in.  
The plug-in spawns 1 agent with a defined number of task slots on the local machine only. Just use [dds-submit --slots X](../../dds-submit/README.md), where X is a desired number of task slots.

## Usage example

Call using a local system only to spawn 1 DDS agent with 10 task slots:

```shell
dds-submit -r localhost --slots 10
```
