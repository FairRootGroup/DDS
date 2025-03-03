# dds-submit

Submits and activates DDS agents. **UNIX/Linux/OSX**

## Synopsis

```shell
dds-submit [[-h, --help] | [-v, --version]] [-l, --list] [-r, --rms arg] [-s, --session arg] [-c, --config arg] [-n, --number arg] [--min-instances arg] [--slots arg] [-g, --group-name arg] [-t, --submission-tag arg] [-e, --env-config arg] [--inline-config arg] [--enable-overbooking]
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
