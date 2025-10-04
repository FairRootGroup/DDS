# DDS Topology

## Introduction

DDS topology is an XML file that describes how to distribute and execute your computational tasks across computing nodes in an HPC environment. It defines the structure, requirements, and relationships between your tasks, making it easy to scale from single-node development to large-scale distributed computing.

### Key Concepts

DDS topology provides a powerful yet simple language for describing distributed computing workflows:

- **[Tasks](#decltask)**: Individual executable processes that form the basic building blocks
- **[Properties](#properties)**: Communication channels between tasks using key-value pairs
- **[Collections](#declcollection)**: Groups of tasks deployed together on the same physical machine
- **[Groups](#group)**: Scalable containers that can multiply collections and tasks
- **[Requirements](#requirements)**: Constraints for task placement (specific nodes, GPUs, etc.)
- **[Assets](#assets)**: Files or data that tasks need access to
- **[Variables](#variables)**: Parameterization and reusable values
- **[Triggers](#triggers)**: Automatic actions based on task conditions

### How Tasks Communicate

Tasks communicate through **properties** - key-value pairs managed by DDS's propagation engine. When one task sets a property value, DDS automatically propagates it to all other tasks that depend on that property. This enables patterns like:

- Service discovery: A "server" task publishes its host and port
- Data passing: One task writes results that others consume  
- Coordination: Tasks signal completion or status changes

> [!NOTE]  
> Property values are treated as strings (256 chars max) and can contain any data your tasks need to exchange.

### Deployment Strategy

- **Collections** group tasks that should run on the same physical machine (for shared memory, high-bandwidth communication, or local file access)
- **Groups** provide scaling by multiplying their contents with a factor `n`
- The **main** group serves as the entry point and can contain other groups

## Quick Start Example

Here's a simple producer-consumer topology:

```xml
<topology name="myTopology">
   <!-- Variables for easy parameterization -->
   <var name="nWorkers" value="4" />
   <var name="appPath" value="$DDS_LOCATION/bin/my-app" />
   
   <!-- Communication properties -->
   <property name="dataChannel"/>
   <property name="resultChannel"/>
   
   <!-- Producer task -->
   <decltask name="producer">
      <exe reachable="true">${appPath} --mode=producer</exe>
      <properties>
         <name access="write">dataChannel</name>
      </properties>
   </decltask>
   
   <!-- Worker task -->
   <decltask name="worker"> 
      <exe reachable="true">${appPath} --mode=worker --id=%taskIndex%</exe>
      <properties>
         <name access="read">dataChannel</name>
         <name access="write">resultChannel</name>
      </properties>
   </decltask>
   
   <!-- Deploy producer and worker together -->
   <declcollection name="workUnit">
      <tasks>
         <name>producer</name>
         <name>worker</name>
      </tasks>
   </declcollection>
   
   <!-- Scale up with multiple work units -->
   <main name="main">
      <group name="workers" n="${nWorkers}">
         <collection>workUnit</collection>
      </group>
   </main>
</topology>
```

This creates 4 work units, each containing a producer and worker on the same node, for a total of 8 processes.

## Topology File Structure

Topology files are validated against the XSD schema at `$DDS_LOCATION/share/topology.xsd`.

## Topology file example

A topology example:

```xml
<topology name="myTopology">

   <var name="appNameVar" value="app1 -l -n --taskIndex %taskIndex% --collectionIndex %collectionIndex%" />
   <var name="nofGroups" value="10" />

   <property name="property1" />
   <property name="property2" />

   <declrequirement name="requirement1" type="hostname" value="+.gsi.de"/>

  <decltrigger name="trigger1" condition="TaskCrashed" action="RestartTask" arg="5"/>

   <decltask name="task1">
      <requirements>
         <name>requirement1</name>
      </requirements>
      <exe reachable="true">${appNameVar}</exe>
      <env reachable="false">env1</env>
      <properties>
         <name access="read">property1</name>
         <name access="readwrite">property2</name>
      </properties>
      <triggers>
         <name>trigger1</name>
      </triggers>
   </decltask>
   <decltask name="task2">
      <exe>app2</exe>
      <properties>
         <name access="write">property1</name>
      </properties>
   </decltask>

   <declcollection name="collection1">
      <requirements>
         <name>requirement1</name>
      </requirements>
      <tasks>
         <name>task1</name>
         <name>task2</name>
         <name>task2</name>
      </tasks>
   </declcollection>

   <declcollection name="collection2">
      <tasks>
         <name>task1</name>
         <name>task1</name>
      </tasks>
   </declcollection>

   <main name="main">
      <task>task1</task>
      <collection>collection1</collection>
      <group name="group1" n="${nofGroups}">
         <task>task1</task>
         <collection>collection1</collection>
         <collection>collection2</collection>
      </group>
      <group name="group2" n="15">
         <collection>collection1</collection>
      </group>
   </main>

</topology>
```

This example demonstrates a complete workflow with 62 total processes distributed across different groups.

## Variables

DDS supports powerful variable substitution for parameterization and reusability:

### User-Defined Variables

Variables are defined using the `<var>` tag and referenced with `${variable_name}` syntax:

```xml
<var name="nWorkers" value="8" />
<var name="appPath" value="$DDS_LOCATION/bin/my-app" />
<var name="logLevel" value="INFO" />

<decltask name="worker">
   <exe>${appPath} --workers=${nWorkers} --log=${logLevel}</exe>
</decltask>
```

Variables can reference:
- Environment variables: `$DDS_LOCATION`, `$HOME`, etc.
- Other variables: `${nWorkers}`
- Any string values for parameterization

### Built-in Index Variables

For scaled deployments, DDS provides special index tags that are replaced at runtime:

- `%taskIndex%` - Zero-based index of the task instance
- `%collectionIndex%` - Zero-based index of the collection instance

```xml
<decltask name="worker">
   <exe>my-app --id=%taskIndex% --group=%collectionIndex%</exe>
</decltask>
```

### Environment Variables

DDS automatically populates environment variables for each task:

- `$DDS_TASK_PATH` - Full path to the user task, e.g., `main/group1/collection_12/task_3`
- `$DDS_GROUP_NAME` - ID of the parent group
- `$DDS_COLLECTION_NAME` - ID of the parent collection (if any)
- `$DDS_TASK_NAME` - ID of the task
- `$DDS_TASK_INDEX` - Zero-based index of the task instance
- `$DDS_COLLECTION_INDEX` - Zero-based index of the collection instance
- `$DDS_SESSION_ID` - DDS session this task belongs to

These can be accessed from within your application code or shell scripts.

## Properties

Properties enable communication between tasks using DDS's key-value propagation engine. When one task sets a property, DDS automatically propagates it to all dependent tasks.

### Property Declaration

```xml
<property name="serviceEndpoint" scope="global"/>
<property name="workerStatus" scope="collection"/>
```

**Attributes:**
- `name` (required) - Unique identifier for the property
- `scope` (optional) - Either `global` (default) or `collection`
  - `global`: Property is shared across all tasks in the topology
  - `collection`: Property is scoped to tasks within the same collection instance

### Property Access

Tasks specify which properties they use and how:

```xml
<decltask name="server">
   <properties>
      <name access="write">serviceEndpoint</name>
   </properties>
</decltask>

<decltask name="client">
   <properties>
      <name access="read">serviceEndpoint</name>
      <name access="readwrite">workerStatus</name>
   </properties>
</decltask>
```

**Access types:**
- `read` - Task can only read the property value
- `write` - Task can only write the property value  
- `readwrite` (default) - Task can both read and write

Property values are strings (256 chars max) and can contain any data your tasks need to exchange.

## Requirements

Requirements specify constraints for task placement on computing nodes. They help you ensure tasks run on appropriate hardware or specific machines.

### Requirement Types

```xml
<declrequirement name="login_nodes" type="hostname" value="login[0-9]+\.cluster\.local"/>
<declrequirement name="worker_limit" type="maxinstances" value="4"/>
<declrequirement name="compute_group" type="groupname" value="gpu_partition"/>
<declrequirement name="special_node" type="wnname" value="node001"/>
<declrequirement name="custom_check" type="custom" value="has_infiniband"/>
```

**Requirement types:**

- `hostname` - Match compute node hostname (supports regex)
- `wnname` - Match worker node name from SSH configuration
- `maxinstances` - Limit number of task instances per host
- `groupname` - Target specific node groups/partitions  
- `custom` - Custom requirement evaluation (parsed but ignored by scheduler)

> **Note:** The `gpu` requirement type is defined in the topology schema but is not currently implemented in the DDS scheduler. GPU node selection must be handled through `hostname`, `wnname`, or `groupname` patterns that target GPU-enabled nodes. Custom requirements are parsed but currently ignored during scheduling - they are placeholders for future extensibility.

For `hostname` and `wnname`, the value can be a full name or regular expression.

### Agent Group Targeting Example

You can submit agents with specific group names and then target them in your topology, enabling fine-grained control over task placement across heterogeneous resources:

```bash
# Submit GPU-capable agents with a group tag
dds-submit -r slurm -n 10 --slots 8 --group-name="gpu_workers"

# Submit CPU-only agents with a different tag
dds-submit -r slurm -n 20 --slots 16 --group-name="cpu_workers"

# Submit high-memory agents
dds-submit -r slurm -n 5 --slots 32 --group-name="highmem_workers"
```

Then target specific agent groups in your topology:

```xml
<topology name="heterogeneous_workflow">
   <!-- Define requirements for different agent groups -->
   <declrequirement name="gpu_req" type="groupname" value="gpu_workers"/>
   <declrequirement name="cpu_req" type="groupname" value="cpu_workers"/>
   <declrequirement name="highmem_req" type="groupname" value="highmem_workers"/>
   
   <!-- GPU-intensive task -->
   <decltask name="gpu_task">
      <exe>cuda_app --device=gpu</exe>
      <requirements>
         <name>gpu_req</name>  <!-- Will run only on gpu_workers agents -->
      </requirements>
   </decltask>
   
   <!-- Standard CPU task -->
   <decltask name="cpu_task">
      <exe>standard_app</exe>
      <requirements>
         <name>cpu_req</name>  <!-- Will run only on cpu_workers agents -->
      </requirements>
   </decltask>
   
   <!-- Memory-intensive task -->
   <decltask name="memory_task">
      <exe>bigdata_app --memory=large</exe>
      <requirements>
         <name>highmem_req</name>  <!-- Will run only on highmem_workers agents -->
      </requirements>
   </decltask>
   
   <main name="main">
      <task>gpu_task</task>
      <group name="cpu_workers" n="10">
         <task>cpu_task</task>
      </group>
      <group name="memory_workers" n="3">
         <task>memory_task</task>
      </group>
   </main>
</topology>
```

This pattern is particularly useful for:

- **Heterogeneous clusters**: Mix GPU and CPU nodes in the same workflow
- **Resource optimization**: Ensure memory-intensive tasks get high-memory nodes
- **Cost management**: Separate expensive GPU resources from cheaper CPU resources
- **Multi-tenant environments**: Isolate different user groups or projects

### Using Requirements

Requirements can be applied to tasks or collections:

```xml
<decltask name="compute_task">
   <exe>cuda_app</exe>
   <requirements>
      <name>login_nodes</name>
      <name>worker_limit</name>
   </requirements>
</decltask>
```

Collection requirements override task requirements when both are specified.

## Triggers

Triggers define automatic actions based on task conditions for fault tolerance and automated recovery.

> **Important:** While triggers can be defined in topology files and are parsed correctly, **trigger functionality is not currently implemented in the DDS runtime**. Tasks that crash will not be automatically restarted regardless of trigger definitions. This feature exists only in the schema and parsing layer.

### Trigger Declaration

```xml
<decltrigger name="auto_restart" condition="TaskCrashed" action="RestartTask" arg="3"/>
```

**Attributes:**
- `name` (required) - Unique identifier
- `condition` (required) - Currently supports: `TaskCrashed`
- `action` (required) - Currently supports: `RestartTask`  
- `arg` (required) - Action parameter (e.g., number of restart attempts)

### Using Triggers

```xml
<decltask name="critical_service">
   <exe>my_service</exe>
   <triggers>
      <name>auto_restart</name>
   </triggers>
</decltask>
```

> **Note:** The trigger will be stored in the topology but will have no runtime effect. For fault tolerance, implement restart logic in your application or use external process supervisors.

## Assets

Assets allow tasks to access files or data, with DDS handling distribution to compute nodes.

### Asset Declaration

```xml
<asset name="config_file" type="inline" visibility="task" value="param1=value1&#10;param2=value2"/>
<asset name="shared_data" type="inline" visibility="global" value="#!/bin/bash&#10;export MYVAR=123"/>
```

**Attributes:**
- `name` (required) - Unique identifier
- `type` (required) - Currently supports: `inline`
- `visibility` (required) - `task` (per-task) or `global` (session-wide)
- `value` (required) - Asset content (use HTML entities for special characters)

### Using Assets

```xml
<decltask name="worker">
   <exe>my_app --config=asset1</exe>
   <assets>
      <name>config_file</name>
   </assets>
</decltask>
```

Tasks can access assets by name in their execution environment.

## Tasks

Tasks are the fundamental building blocks - individual executable processes that form your distributed application.

### Task Declaration

```xml
<decltask name="worker">
   <exe reachable="true">$DDS_LOCATION/bin/my-app --mode=worker</exe>
   <env reachable="false">setup_environment.sh</env>
   <properties>
      <name access="read">inputData</name>
      <name access="write">results</name>
   </properties>
   <requirements>
      <name>gpu_node</name>
   </requirements>
   <triggers>
      <name>auto_restart</name>
   </triggers>
   <assets>
      <name>config_file</name>
   </assets>
</decltask>
```

### Task Elements

- `<exe>` (required) - Path to executable with optional arguments
  - `reachable` attribute: `true` if executable exists on worker nodes, `false` if DDS should deploy it
- `<env>` (optional) - Environment setup script to run before the executable
  - `reachable` attribute: `true` if script exists on worker nodes, `false` if DDS should deploy it
- `<properties>` (optional) - List of properties this task uses
- `<requirements>` (optional) - List of placement requirements  
- `<triggers>` (optional) - List of fault tolerance triggers
- `<assets>` (optional) - List of files/data this task needs

### Executable Paths

The `<exe>` element supports:
- Full paths: `/usr/bin/python3 script.py`
- Variables: `${appPath} --config=${configFile}`
- Built-in indices: `my-app --id=%taskIndex%`
- Complex arguments with quotes: `bash -c "echo 'Hello World'"`

DDS automatically parses and handles program arguments at runtime.

## Collections

Collections group tasks that should be deployed together on the same physical machine. This is useful for:
- High-bandwidth communication between tasks
- Shared memory access
- Local file system dependencies
- Reducing network latency

### Collection Declaration

```xml
<declcollection name="worker_unit">
   <requirements>
      <name>gpu_nodes</name>
   </requirements>
   <tasks>
      <name>data_loader</name>
      <name>processor</name>
      <name>output_writer</name>
   </tasks>
</declcollection>
```

Collections can have their own requirements that override individual task requirements. This ensures the entire collection is placed appropriately.

### Task Ordering

Tasks within a collection are listed in the order they should be considered, but DDS handles the actual scheduling and execution coordination.

## Groups and Main

Groups provide scaling by multiplying their contents and serve as organizational containers.

### Group Declaration

```xml
<group name="workers" n="8">
   <task>standalone_task</task>
   <collection>worker_unit</collection>
   <group name="sub_workers" n="2">
      <task>nested_task</task>
   </group>
</group>
```

The `n` attribute specifies the multiplication factor. In this example:
- 8 instances of `standalone_task` 
- 8 instances of `worker_unit` collection
- 16 instances of `nested_task` (8 × 2)

### Main Group

The `main` group is the entry point for execution:

```xml
<main name="main">
   <task>init_task</task>
   <group name="workers" n="4">
      <collection>processing_unit</collection>
   </group>
   <task>cleanup_task</task>
</main>
```

Only the main group can contain other groups, providing a hierarchical structure for complex topologies.

## Topology XML references

### Topology XML tags

[topology](#topology), [var](#var), [property](#property), [declrequirement](#declrequirement), [decltrigger](#decltrigger), [decltask](#decltask), [declcollection](#declcollection), [task](#task), [collection](#collection), [group](#group), [main](#main), [exe](#exe), [env](#env), [requirements](#requirements), [properties](#properties), [name](#name)

#### topology

| Parents | Children                                                                                    | Attributes              | Description          |
| ------- | ------------------------------------------------------------------------------------------- | ----------------------- | -------------------- |
| no      | [property](#property), [task](#task), [collection](#collection), [main](#main), [var](#var) | [name](#attribute-name) | Declares a topology. |

Example:

```xml
<topology name="myTopology">
  [... Definition of tasks, 
  properties, collections and 
  groups ...]
</topology>
```

#### var

| Parents               | Children | Attributes                     | Description                                                                           |
| --------------------- | -------- | ------------------------------ | ------------------------------------------------------------------------------------- |
| [topology](#topology) | no       | [name](#attribute-name), value | Declares a variable which can be used inside the topology file as `${variable_name}`. |

Example:

```xml
<var name="var1" value="value1"/>
<var name="var2" value="value2"/>
```

#### property

| Parents               | Children | Attributes | Description          |
| --------------------- | -------- | ---------- | -------------------- |
| [topology](#topology) | no       | name       | Declares a property. |

```xml
<property name="property1"/>
<property name="property2"/>
```

#### declrequirement

| Parents               | Children | Attributes                                              | Description                                       |
| --------------------- | -------- | ------------------------------------------------------- | ------------------------------------------------- |
| [topology](#topology) | no       | [name](#attribute-name), [type](#attribute-type), value | Declares a requirement for tasks and collections. |

```xml
<declrequirement name="requirement1" type="hostname" value="+.gsi.de"/>
```

#### decltrigger

| Parents               | Children | Attributes                                                                                   | Description              |
| --------------------- | -------- | -------------------------------------------------------------------------------------------- | ------------------------ |
| [topology](#topology) | no       | [name](#attribute-name), [condition](#attribute-condition), [action](#attribute-action), arg | Declares a task trigger. |

```xml
<decltrigger name="trigger1" condition="TaskCrashed" action="RestartTask" arg="5"/>
```

#### decltask

| Parents               | Children                                                                                     | Attributes              | Description      |
| --------------------- | -------------------------------------------------------------------------------------------- | ----------------------- | ---------------- |
| [topology](#topology) | [exe](#exe), [env](#env), [requirements](#requirements), triggers, [properties](#properties) | [name](#attribute-name) | Declares a task. |

```xml
<decltask name="task1">
    <exe reachable="true">app1 -l -n</exe>
    <env reachable="false">env1</env>
    <requirements>
       <name>requirement1</name>
    </requirement>
  <triggers>
       <name>trigger1</name>
    </triggers>
    <properties>
        <name access="read">property1</name>
        <name access="readwrite">property2</name>
    </properties>
</decltask>
```

#### declcollection

| Parents               | Children      | Attributes              | Description            |
| --------------------- | ------------- | ----------------------- | ---------------------- |
| [topology](#topology) | [task](#task) | [name](#attribute-name) | Declares a collection. |

```xml
<declcollection name="collection1">
    <task>task1</task>
    <task>task1</task>
</declcollection>
```

#### task

| Parents                                    | Children | Attributes | Description                                          |
| ------------------------------------------ | -------- | ---------- | ---------------------------------------------------- |
| [collection](#collection), [group](#group) | no       | no         | Specifies the unique ID of the already defined task. |

```xml
<task>task1</task>
```

#### collection

| Parents         | Children | Attributes | Description                                                |
| --------------- | -------- | ---------- | ---------------------------------------------------------- |
| [group](#group) | no       | no         | Specifies the unique ID of the already defined collection. |

```xml
<collection>collection1</collection>
```

#### group

| Parents       | Children                                 | Attributes                                 | Description       |
| ------------- | ---------------------------------------- | ------------------------------------------ | ----------------- |
| [main](#main) | [task](#task), [collection](#collection) | [name](#attribute-name), [n](#attribute-n) | Declares a group. |

```xml
<group name="group1" n="10">
    <task>task1</task>
    <collection>collection1</collection>
    <collection>collection2</collection>
</group>
```

#### main

| Parents               | Children                                                  | Attributes              | Description              |
| --------------------- | --------------------------------------------------------- | ----------------------- | ------------------------ |
| [topology](#topology) | [task](#task), [collection](#collection), [group](#group) | [name](#attribute-name) | Declares the main group. |

```xml
<main name="main">
    <task>task1</task>
    <collection>collection1</collection>
    <group name="group1" n="10">
        <task>task1</task>
        <collection>collection1</collection>
        <collection>collection2</collection>
    </group>
</main>
```

#### exe

> [!NOTE]  
> Required.

| Parents               | Children | Attributes                        | Description                                            |
| --------------------- | -------- | --------------------------------- | ------------------------------------------------------ |
| [decltask](#decltask) | no       | [reachable](#attribute-reachable) | Defines path to the executable or script for the task. |

```xml
<exe reachable="true">app1 -l -n</exe>
```

#### env

> [!NOTE]  
> Optional.

| Parents               | Children | Attributes                        | Description                                                                  |
| --------------------- | -------- | --------------------------------- | ---------------------------------------------------------------------------- |
| [decltask](#decltask) | no       | [reachable](#attribute-reachable) | Defines the path to script that has to be executed prior to main executable. |

```xml
<env reachable="false">setEnv.sh</env>
```

#### requirements

> [!NOTE]  
> Optional.

| Parents                                                  | Children      | Attributes | Description                     |
| -------------------------------------------------------- | ------------- | ---------- | ------------------------------- |
| [decltask](#decltask), [declcollection](#declcollection) | [name](#name) | no         | Defines a list of requirements. |

```xml
<requirements>
   <name>requirement1</name>
   <name>requirement2</name>
</requirements>
```

#### properties

> [!NOTE]  
> Optional.

| Parents               | Children      | Attributes | Description                             |
| --------------------- | ------------- | ---------- | --------------------------------------- |
| [decltask](#decltask) | [name](#name) | no         | Defines a list of dependent properties. |

```xml
<properties>
    <name>property1</name>
    <name>property2</name>
</properties>
```

#### name

> [!NOTE]  
> Required.

| Parents                   | Children | Attributes                  | Description                                     |
| ------------------------- | -------- | --------------------------- | ----------------------------------------------- |
| [properties](#properties) | no       | [access](#attribute-access) | Defines an ID of the already declared property. |

```xml
<name>property1</name>
```

### Topology XML attributes

[name](#attribute-name), [reachable](#attribute-reachable), [n](#attribute-n), [access](#attribute-access), [type](#attribute-type), [condition](#attribute-condition), [action](#attribute-action)

#### attribute: name

| Required | Default | Tags                                                                                                                                                                          | Restrictions                                   | Description                                                                                                                                                                       |
| -------- | ------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| yes      | -       | [topology](#topology), [property](#properties), [declrequirement](#declrequirement), [decltask](#decltask), [declcollection](#declcollection), [group](#group), [main](#main) | A string with a minimum length of 1 character. | Defines identifier (ID) for topology, property, requirement, task, collection and group. ID has to be unique within its scope, i.e. ID for tasks has to be unique only for tasks. |

```xml
<topology name="myTopology">
```

#### attribute: reachable

| Required | Default | Tags                     | Restrictions      | Description                                                      |
| -------- | ------- | ------------------------ | ----------------- | ---------------------------------------------------------------- |
| no       | true    | [exe](#exe), [env](#env) | `true` \| `false` | Defines if executable or script is available on the worker node. |

```xml
<exe reachable="true">app -l</exe>
<env>env1</env>
```

#### attribute: n

| Required | Default | Tags            | Restrictions                               | Description                              |
| -------- | ------- | --------------- | ------------------------------------------ | ---------------------------------------- |
| no       | 1       | [group](#group) | An unsigned integer 32-bit value. Min is 1 | Defines multiplication factor for group. |

```xml
<group name="group1" n="10">
    <task>task1</task>
    <collection>collection1</collection>
    <collection>collection2</collection>
</group>
```

#### attribute: access

| Required | Default     | Tags          | Restrictions                 | Description                                       |
| -------- | ----------- | ------------- | ---------------------------- | ------------------------------------------------- |
| no       | `readwrite` | [name](#name) | `read`\|`write`\|`readwrite` | Defines access type from user task to properties. |

```xml
<name access="read">property1</name>
```

#### attribute: type

| Required | Default | Tags                                | Restrictions                                                | Description                                                                                                                                                      |
| -------- | ------- | ----------------------------------- | ----------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| yes      | -       | [declrequirement](#declrequirement) | `hostname`\|`wnname`\|`maxinstances`\|`groupname`\|`custom` | Defines the type of the requirement. Note: `gpu` type exists in schema but is not implemented in runtime; `custom` type is parsed but ignored during scheduling. |

```xml
<declrequirement name="host_req" type="hostname" value="login[0-9]+\.cluster\.local"/>
<declrequirement name="limit_req" type="maxinstances" value="4"/>
```

#### attribute: scope

| Required | Default  | Tags                  | Restrictions           | Description                                                                                    |
| -------- | -------- | --------------------- | ---------------------- | ---------------------------------------------------------------------------------------------- |
| no       | `global` | [property](#property) | `global`\|`collection` | Defines property scope: global (across all tasks) or collection (within collection instances). |

```xml
<property name="globalData" scope="global"/>
<property name="localState" scope="collection"/>
```

#### attribute: visibility

| Required | Default | Tags            | Restrictions     | Description                                                                  |
| -------- | ------- | --------------- | ---------------- | ---------------------------------------------------------------------------- |
| yes      | -       | [asset](#asset) | `task`\|`global` | Defines asset visibility: task (per-task instance) or global (session-wide). |

```xml
<asset name="config" type="inline" visibility="task" value="config_data"/>
```

#### attribute: condition

| Required | Default | Tags                        | Restrictions  | Description                |
| -------- | ------- | --------------------------- | ------------- | -------------------------- |
| yes      | -       | [decltrigger](#decltrigger) | `TaskCrashed` | Defines trigger condition. |

```xml
<decltrigger name="trigger1" condition="TaskCrashed" action="RestartTask" arg="5"/>
```

#### attribute: action

| Required | Default | Tags                        | Restrictions  | Description             |
| -------- | ------- | --------------------------- | ------------- | ----------------------- |
| yes      | -       | [decltrigger](#decltrigger) | `RestartTask` | Defines trigger action. |

```xml
<decltrigger name="trigger1" condition="TaskCrashed" action="RestartTask" arg="5"/>
```

## Best Practices for HPC Environments

### Resource Management

**Use Requirements Effectively:**

```xml
<!-- Target specific node patterns for GPU nodes -->
<declrequirement name="gpu_nodes" type="hostname" value="gpu[0-9]+\.cluster\.local"/>

<!-- Limit concurrent tasks per node to avoid resource contention -->
<declrequirement name="task_limit" type="maxinstances" value="2"/>

<!-- Target specific partitions or node groups -->
<declrequirement name="compute_partition" type="groupname" value="gpu_partition"/>
```

**Optimize for Network Topology:**
```xml
<!-- Group communicating tasks in collections for locality -->
<declcollection name="mpi_unit">
   <tasks>
      <name>mpi_coordinator</name>
      <name>mpi_worker_1</name>
      <name>mpi_worker_2</name>
   </tasks>
</declcollection>
```

### Scalability Patterns

**Parameter Sweep Pattern:**
```xml
<topology name="parameter_sweep">
   <var name="nJobs" value="100"/>
   
   <decltask name="sweep_job">
      <exe>./simulation --param=%taskIndex%</exe>
   </decltask>
   
   <main name="main">
      <group name="jobs" n="${nJobs}">
         <task>sweep_job</task>
      </group>
   </main>
</topology>
```

**Producer-Consumer Pattern:**
```xml
<topology name="pipeline">
   <property name="workQueue" scope="global"/>
   <property name="results" scope="global"/>
   
   <decltask name="producer">
      <exe>./producer</exe>
      <properties>
         <name access="write">workQueue</name>
      </properties>
   </decltask>
   
   <decltask name="consumer">
      <exe>./consumer --id=%taskIndex%</exe>
      <properties>
         <name access="read">workQueue</name>
         <name access="write">results</name>
      </properties>
   </decltask>
   
   <declcollection name="processing_unit">
      <tasks>
         <name>producer</name>
         <name>consumer</name>
      </tasks>
   </declcollection>
   
   <main name="main">
      <group name="workers" n="8">
         <collection>processing_unit</collection>
      </group>
   </main>
</topology>
```

### Fault Tolerance

**Current Status of DDS Triggers:**

DDS triggers are implemented at the topology library level (parsing, validation, storage) but are not yet supported by the DDS runtime system. The trigger infrastructure is in place and waiting for user demand to justify full implementation. If automatic task restart functionality is important for your use case, please consider contributing to the DDS project or expressing your interest to the development team.

**Implement Application-Level Recovery:**

```xml
<!-- Since DDS triggers aren't implemented, build resilience into your application -->
<decltask name="robust_service">
   <exe>./service --retry-on-failure --max-attempts=3</exe>
   <requirements>
      <name>gpu_nodes</name>
   </requirements>
</decltask>
```

**Alternative Approaches:**

- Use external process supervisors (systemd, supervisor, etc.)
- Implement retry logic within your applications
- Use container orchestrators (Kubernetes) for automatic restart
- Design applications to be crash-resilient

### Configuration Management

**Use Assets for Configuration:**
```xml
<asset name="app_config" type="inline" visibility="global" value="
# Application Configuration
max_workers=4
timeout=300
log_level=INFO
"/>

<decltask name="app">
   <exe>./app --config=app_config</exe>
   <assets>
      <name>app_config</name>
   </assets>
</decltask>
```

### Environment Setup

**Handle Dependencies:**
```xml
<decltask name="ml_training">
   <exe reachable="false">python3 train.py --epochs=100</exe>
   <env reachable="true">$HOME/setup_ml_env.sh</env>
   <requirements>
      <name>gpu_nodes</name>
   </requirements>
</decltask>
```

### Development vs Production

**Use Variables for Flexibility:**
```xml
<topology name="simulation">
   <!-- Development: small scale -->
   <!-- <var name="scale" value="2"/> -->
   
   <!-- Production: full scale -->
   <var name="scale" value="100"/>
   <var name="app_path" value="$DDS_LOCATION/bin/simulation"/>
   
   <main name="main">
      <group name="workers" n="${scale}">
         <task>worker</task>
      </group>
   </main>
</topology>
```

### Debugging Tips

1. **Start Small:** Begin with `n="1"` in groups and increase gradually
2. **Use reachable="false":** During development to avoid deployment issues
3. **Leverage Environment Variables:** Access `$DDS_TASK_INDEX` and `$DDS_TASK_PATH` for debugging
4. **Property Scope:** Use `scope="collection"` for isolated testing, `scope="global"` for coordination

### Performance Considerations

- **Collection Size:** Keep collections to 2-8 tasks for optimal placement
- **Property Updates:** Minimize frequent property updates as they trigger propagation
- **Resource Requirements:** Be specific to avoid suboptimal placement
- **Index Usage:** Prefer `%taskIndex%` in executables over environment variables for performance

This documentation reflects the current DDS topology library implementation and capabilities.
