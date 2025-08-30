# SLURM RMS plug-in

## Sandbox directory

If your home directory is not shared on the SLURM cluster, then you must define a sandbox directory, which DDS will use to store SLURM job script and all jobs' working directories will be also located there. Please note, that at the moment DDS doesn't clean jobs' working directories, therefore you are responsible to remove them if needed.

In order to set sandbox directory a DDS global option `server.sandbox_dir` have to be changed, which is located in the DDS configuration file `DDS.cfg` (default location: `$HOME/.DDS/DDS.cfg`).

For more details about this and other configuration options, see the [User Defaults Configuration Reference](../../docs/user-defaults-configuration.md).

## User configuration

Using [dds-submit -c My_SLURM.cfg](../../dds-submit/README.md) command you can provide additional configuration options for DDS SLURM jobs.  
For example, the following command will submit 10 DDS agents (each with 50 task slots) and will use additional SLURM configuration options provided in the `My_SLURM.cfg`:

```shell
dds-submit -r slurm -n 10 --slots 50 -c My_SLURM.cfg
```

> [!NOTE]  
> The content of the custom SLURM job configuration file can be any `sbatch` parameter, except `srun` and `--array`.
>
> For example, My_SLURM.cfg can contain:
>
>   ```bash
>   #SBATCH -A "account"
>   #SBATCH --time=00:30:00
>   ```

## Usage example

Submit 10 DDS agents to SLURM cluster. On the SLURM submitter machine execute:

```shell
$ dds-submit -r slurm -n 10

 dds-submit: Contacting DDS commander on lxbk0200.gsi.de:20001 ...
 dds-submit: Connection established.
 dds-submit: Requesting server to process job submission...
 dds-submit: Server reports: Creating new worker package...
 dds-submit: Server reports: RMS plug-in: /u/manafov/DDS/1.1.61.g474ddc6/plugins/dds-submit-slurm/dds-submit-slurm
 dds-submit: Server reports: Initializing RMS plug-in...
 dds-submit: Server reports: RMS plug-in is online. Startup time: 17ms.
 dds-submit: Server reports: Plug-in: Generating SLURM Job script...
 dds-submit: Server reports: Plug-in: Preparing job submission...
 dds-submit: Server reports: Plug-in: pipe log engine: Submitting DDS Job on the SLURM cluster...

 dds-submit: Server reports: Plug-in: pipe log engine: SLURM: Submitted batch job 9539993

 dds-submit: Server reports: Plug-in: DDS agents have been submitted
```

Check the status of your SLURM jobs:

```shell
scontrol show job 9539993
```

Check the status of your DDS agents:

```shell
dds-info -ln
```

Once agents are online, use DDS as normal.
