# How to Start

## Environment

In order to enable DDS environment you need to source the DDS_env.sh script. The script is located in the directory where you installed DDS.

```shell
cd [DDS INSTALLATION]
source DDS_env.sh
```

## Server

Use the `dds-session` command to start/stop/list DDS sessions.

```shell
dds-session start
```

### Starting DDS in Lightweight Mode

DDS can be started in lightweight mode for environments where DDS is pre-installed on worker nodes:

```shell
# Using command-line option
dds-session start --lightweight

# Using environment variable
export DDS_LIGHTWEIGHT_PACKAGE=1
dds-session start
```

In lightweight mode, DDS skips worker package validation and creates minimal deployment packages, significantly improving startup times. This is particularly useful in environments with pre-configured DDS installations.

## Deploy Agents

In order to deploy agents you can use different [DDS plug-ins](../plugins/README.md#rms-plug-ins).

One of the fastest way to deploy DDS is to use [the localhost plug-in](../plugins/dds-submit-localhost/README.md).

But if you want to use multiple hosts, then [the SSH plug-in](../plugins/dds-submit-ssh/README.md) is the right tool.  
When you don't have an RMS or you want to use a Cloud based system or even if you want just to use resources around you, like computers of your colleagues, then the plug-in is the best way to go.

First of all you need to [define resources](../plugins/dds-submit-ssh/README.md#resource-definition).

Then use `dds-submit` to deploy DDS agents on the given resources:

```shell
dds-submit --rms ssh -c FULL_PATH_TO_YOUR_SSHPLUGIN_RESOURCE_FILE
```

### Lightweight Worker Packages

For faster deployment when DDS is pre-installed on worker nodes, you can use lightweight worker packages:

```shell
# Enable globally via environment variable
export DDS_LIGHTWEIGHT_PACKAGE=1
dds-submit --rms slurm -n 10 --slots 8

# Or use command-line option
dds-submit --rms slurm -n 10 --slots 8 --lightweight
```

This reduces package size from ~15MB to ~50KB and provides 3x faster deployment speed.

**Important:** For lightweight packages to work, worker nodes must have these environment variables set at runtime:

- `DDS_COMMANDER_BIN_LOCATION` - Path to DDS binaries (e.g., `/opt/dds/bin`)
- `DDS_COMMANDER_LIBS_LOCATION` - Path to DDS libraries (e.g., `/opt/dds/lib`)

These should point to a valid DDS installation on the worker nodes. Configure your batch system or environment scripts to export these variables before DDS worker startup.

## Check availability of Agents

Using `dds-info` you can query different kinds of information from DDS. For example you can check how many agents are already online:

```shell
dds-info -n
```

or query more detailed info about agents:

```shell
dds-info -l
```

## Activate Topology

Once you get enough online agents, you can activate them. Activation of agents means, that DDS will use the given topology to distribute user tasks across available resources (agents):

```shell
dds-topology --activate FULL_PATH_TO_YOUR_TOPOLOGY_FILE
```

DDS will automatically check whether available resources are actually sufficient to execute the given topology.
