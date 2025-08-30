# Tutorials

## Tutorial 1

This tutorial demonstrates how to deAfter executing ui-custom-cmd there will be an output to the console with receiving and sending custom commands. Also check output files of tasks.

## Tutorial 3

This tutorial demonstrates how to use DDS in an automated script environment. It shows a complete workflow from starting a DDS session to activating a topology and cleaning up afterwards.

The tutorial is provided as a shell script that automates the entire DDS workflow, making it useful for batch processing, testing, or integration with other automation tools.

After DDS is installed the tutorial can be found in `$DDS_LOCATION/tutorials/tutorial3`

### Tutorial 3 - Files of the tutorial

* **run_tutorial3.sh**: Main automation script that demonstrates the complete DDS workflow.

### Tutorial 3 - Usage example

Before running the tutorial, you need to configure the script variables:

1. Set the `location` variable to your DDS installation directory
2. Set the `topologyFile` variable to point to your topology file  
3. Set the `requiredNofAgents` variable to the number of agents needed for your topology

```shell
cd $DDS_LOCATION/tutorials/tutorial3
# Edit run_tutorial3.sh to set proper paths and requirements
./run_tutorial3.sh
```

### Tutorial 3 - What the script does

The automation script performs the following steps:

1. Sources DDS environment
2. Starts a new DDS session
3. Submits the required number of agents using the localhost plug-in
4. Waits for agents to come online
5. Activates the specified topology
6. Runs for a specified duration (configurable)
7. Stops the DDS session and cleans up

This tutorial is particularly useful for:

* Automated testing environments
* Batch processing workflows  
* Integration with external job schedulers
* Learning the complete DDS command sequenceloy a simple topology of 2 types of tasks (TaskTypeOne and TaskTypeTwo).  
By default, there will be deployed one instance of TaskTypeTwo and 5 instances of TaskTypeOne. Additionally TaskTypeTwo subscribes on key-value property from TaskTypeOne, which name is TaskIndexProperty.  
Once TaskTypeTwo receives values of TaskIndexProperty from all TaskTypeOne, it will set the ReplyProperty property.  
Number of instances can be changed in the topology file (`tutorial1_topo.xml`) using the `--instances` option of TaskTypeOne. Please note that number of worker nodes in the SSH-plugin configuration file (`tutorial1_hosts.cfg`) has to be changed accordingly.

After DDS is installed the tutorial can be found in `$DDS_LOCATION/tutorials/tutorial1`

The source code of tasks is located in `<DDS_SRC_DIR>/dds-tutorials/dds-tutorial1`

### Tutorial 1 - Files of the tutorial

* **task-type-one**: executable of the task TaskTypeOne.

* **task-type-two**: executable of the task TaskTypeTwo.

* **tutorial1_topo.xml**: a topology file.

* **tutorial1_hosts.cfg**: a configuration file for DDS SSH plug-in.

### Tutorial 1 - Usage example

Before running the tutorial make sure that:

1. Default working directory `~/tmp/dds_wn_test` must exist before running the tutorial. The directory can be changed in `tutorial1_hosts.cfg`.
1. An SSH passwordless access to the localhost is required.

```shell
cd $DDS_LOCATION/tutorials/tutorial1
dds-session start
dds-submit -r ssh -c tutorial1_hosts.cfg
dds-topology --activate tutorial1_topo.xml
```

### Tutorial 1 - Result

To check the result, change to `~/tmp/dds_wn_test`. If the default setup was used, then there will be WN directories located: wn, wn_1, wn_2, wn_3, wn_4, wn_5.

DDS catches output of tasks and saves it in log files under names `[task_name]_[date_time]_out|err.log`.  
For example: `TaskTypeOne_2015-07-16-11-44-42_6255430612052815609_out.log`

## Tutorial 2

### Tutorial 2 - Usage example

Before running the tutorial make sure that:

1. Default working directory `~/tmp/dds_wn_test` must exist before running the tutorial. The directory can be changed in `tutorial1_hosts.cfg`.
1. An SSH passwordless access to the localhost is required.

```shell
cd $DDS_LOCATION/tutorials/tutorial2
dds-session start
dds-submit -r ssh -c tutorial2_hosts.cfg
dds-topology --activate tutorial2_topo.xml
ui-custom-cmd
```

### Tutorial 2 - Result

To check the result, change to `~/tmp/dds_wn_test`. If the default setup was used, then there will be WN directories located: wn, wn_1, wn_2, wn_3, wn_4, wn_5.

DDS catches output of tasks and saves it in log files under names `[task_name]_[date_time]_out|err.log`.  
For example: `TaskTypeOne_2015-07-16-11-44-42_6255430612052815609_out.log`

After executing ui-custom-command there will be an output to the console with receiving and sending custom commands. Also check output files of tasks.
