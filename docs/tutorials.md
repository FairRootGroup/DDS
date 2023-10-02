# Tutorials

## Tutorial 1

This tutorial demonstrates how to deploy a simple topology of 2 types of tasks (TaskTypeOne and TaskTypeTwo).  
By default, there will be deployed one instance of TaskTypeTwo and 5 instances of TaskTypeOne. Additionally TaskTypeTwo subscribes on key-value property from TaskTypeOne, which name is TaskIndexProperty.  
Once TaskTypeTwo receives values of TaskIndexProperty from all TaskTypeOne, it will set the ReplyProperty property.  
Number of instances can be changed in the topology file (`tutorial1_topo.xml`) using the `--instances` option of TaskTypeOne. Please note that number of worker nodes in the SSH-plugin configuration file (`tutorial1_hosts.cfg`) has to be changed accordingly.

After DDS is installed the tutorial can be found in `$DDS_LOCATION/tutorials/tutorial1`

The source code of tasks is located in `<DDS_SRC_DIR>/dds-tutorials/dds-tutorial1`

### Files of the tutorial

* **task-type-one**: executable of the task TaskTypeOne.

* **task-type-two**: executable of the task TaskTypeTwo.

* **tutorial1_topo.xml**: a topology file.

* **tutorial1_hosts.cfg**: a configuration file for DDS SSH plug-in.

### Usage example

Before running the tutorial make sure that:

1. Default working directory `~/tmp/dds_wn_test` must exist before running the tutorial. The directory can be changed in `tutorial1_hosts.cfg`.
1. An SSH passwordless access to the localhost is required.

```shell
cd $DDS_LOCATION/tutorials/tutorial1
dds-session start --local
dds-submit -r ssh -c tutorial1_hosts.cfg
dds-topology --activate tutorial1_topo.xml
```

### Result

To check the result, change to `~/tmp/dds_wn_test`. If the default setup was used, then there will be WN directories located: wn, wn_1, wn_2, wn_3, wn_4, wn_5.

DDS catches output of tasks and saves it in log files under names `[task_name]_[date_time]_out|err.log`.  
For example: `TaskTypeOne_2015-07-16-11-44-42_6255430612052815609_out.log`

## Tutorial 2

### Usage example

Before running the tutorial make sure that:

1. Default working directory `~/tmp/dds_wn_test` must exist before running the tutorial. The directory can be changed in `tutorial1_hosts.cfg`.
1. An SSH passwordless access to the localhost is required.

```shell
cd $DDS_LOCATION/tutorials/tutorial2
dds-session start --local
dds-submit -r ssh -c tutorial2_hosts.cfg
dds-topology --activate tutorial2_topo.xml
ui-custom-command
```

### Result

To check the result, change to `~/tmp/dds_wn_test`. If the default setup was used, then there will be WN directories located: wn, wn_1, wn_2, wn_3, wn_4, wn_5.

DDS catches output of tasks and saves it in log files under names `[task_name]_[date_time]_out|err.log`.  
For example: `TaskTypeOne_2015-07-16-11-44-42_6255430612052815609_out.log`

After executing ui-custom-command there will be an output to the console with receiving and sending custom commands. Also check output files of tasks.
