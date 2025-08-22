# dds-submit

Submits and activates DDS agents. **UNIX/Linux/OSX**

## Synopsis

```shell
dds-submit [[-h, --help] | [-v, --version]] [-l, --list] [-r, --rms arg] [-s, --session arg] [-c, --config arg] [-n, --number arg] [--min-instances arg] [--slots arg] [-g, --group-name arg] [-t, --submission-tag arg] [-e, --env-config arg] [--inline-config arg] [--enable-overbooking] [--lightweight]
```

## Description

The command is used to submit DDS agents to allocate resources for user tasks. Once enough agents are online use the [dds-topology](../dds-topology/README.md) command to activate the agents - i.e. distribute user tasks across agents and start them.

## Options

* **-h, --help**  
Shows usage options.  

* **-v, --version**  
Shows version information.

* **-l, --list**  
List all available RMS plug-ins.

* **-r, --rms** *arg*  
Defines a destination resource management system plug-in. Use `--list` to find out names of available RMS plug-ins.

* **-s, --session** *arg*  
DDS Session ID.

* **--path** *arg*  
A plug-in's directory search path. It can be used for external RMS plug-ins.

* **-c, --config** *arg*  
A plug-in's configuration file. It can be used to provide additional RMS options. It should contain only RMS options. To define custom environment per agent, use --env-config.

* **-e, --env-config** *arg*  
A path to a user environment script. Will be executed once per agent (valid for all task slots of the agent).

* **--inline-config** *arg*  
Content of this string will be added to the RMS job configuration file as is. It can be specified multiple times, example: `dds-submit -r slurm -n 5 --slots=10 --inline-config="#SBATCH --cpus-per-task=124" --inline-config="#SBATCH --test=45"`.

* **-n, --number** *arg*  
Defines a number of DDS agents to spawn. Default: 1.

* **--min-instances** *arg*  
Request that a minimum of "--min-instances" and maximum of "--number" agents to spawn. This option is RMS plug-in depended. At the moment only the slurm plug-in supports it. If set to 0, the minimum is ignored. Default: 0.

* **--slots** *arg*  
Defines a number of task slots per agent.

* **-g, --group-name** *arg*  
Defines a group name of agents of this submission. Default: "common".

* **-t, --submission-tag** *arg*  
It can be used to define a submission tag. DDS RMS plug-ins will use this tag to name DDS RMS jobs and directories they create on the worker nodes. Default: "dds_agent_job".

* **--enable-overbooking**  
The flag instructs DDS RMS plug-in to not specify any CPU requirement for RMS jobs. For example, the SLURM plug-in will not add the "#SBATCH --cpus-per-task" option to the job script. Otherwise DDS will try to require as many CPU per agent as tasks slots.

* **--lightweight**  
Create a lightweight worker package without DDS binaries and libraries (~50KB instead of ~15MB). Requires DDS to be pre-installed on worker nodes with `DDS_COMMANDER_BIN_LOCATION` and `DDS_COMMANDER_LIBS_LOCATION` environment variables properly set to point to the DDS installation paths. This option provides 3x faster deployment speed and significantly reduces network transfer overhead. Can also be enabled via `DDS_LIGHTWEIGHT_PACKAGE` environment variable.

## Environment Variables

* **DDS_LIGHTWEIGHT_PACKAGE**  
When set to `1`, `true`, `yes`, or `on` (case-insensitive), enables lightweight worker package mode. Command-line `--lightweight` option takes precedence over this environment variable.

## Prerequisites for Lightweight Packages

When using the `--lightweight` option, the following environment variables **must** be available on worker nodes at runtime:

* **DDS_COMMANDER_BIN_LOCATION** - Path to DDS binaries directory (e.g., `/opt/dds/bin`)
* **DDS_COMMANDER_LIBS_LOCATION** - Path to DDS libraries directory (e.g., `/opt/dds/lib`)

These variables should point to a valid DDS installation on the worker nodes. If not set or pointing to invalid locations, worker initialization will fail. Ensure your batch system job scripts or worker node environment properly export these variables.

## Examples

### Lightweight SLURM Submission

```shell
# Submit to SLURM with lightweight package (DDS must be pre-installed on compute nodes)
dds-submit -r slurm -n 10 --slots 8 --lightweight

# With environment script to set DDS paths
dds-submit -r slurm -n 10 --slots 8 --lightweight --env-config env_setup.sh
```

### SSH Lightweight Submission

```shell
# Submit via SSH with lightweight package
dds-submit -r ssh -c ssh_hosts.cfg --lightweight
```

### Environment Variable Usage

```shell
# Enable lightweight mode globally via environment variable
export DDS_LIGHTWEIGHT_PACKAGE=1
dds-submit -r slurm -n 10 --slots 8

# Command line option takes precedence over environment variable
export DDS_LIGHTWEIGHT_PACKAGE=1
dds-submit -r slurm -n 10 --slots 8   # lightweight enabled (from env var)

# Environment variable can be set to various values
export DDS_LIGHTWEIGHT_PACKAGE=true   # or "yes", "on", "1"
export DDS_LIGHTWEIGHT_PACKAGE=false  # or "no", "off", "0", or unset
```
