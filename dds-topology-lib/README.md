# Topology description

The definition of the topology by the user has to be simple and powerful at the same time. Therefore a simple and powerful so called topology language has been developed.

The basic building block of the system is a [**task**](#decltask). Namely, a task is a user defined executable or a shell script, which will be deployed and executed by DDS on a given Resource Management System.

In order to describe dependencies between tasks in a topology we use [**properties**](#properties). At run-time properties will be turned into simple key-value pairs. DDS uses its key-value propagation engine to make sure, that once property is set by one task, it will be propagated to other depended tasks. DDS treats values of properties as simple strings and doesn't do any special treatment/preprocessing on them. So, basically tasks can write anything into the values of properties (256 char max). Any of depended tasks can set properties. Anytime property is set it will be propagated to other depended tasks. (see for details TODO:"key-value propagation").

> [!NOTE]  
> For example, if one task needs to connect with another task they can have the same property. A "server" task can store its TCP/IP port and host in the property. Once the property set, DDS will notice that and propagate it to other tasks.

Tasks can be grouped into [**collections**](#declcollection) and [**groups**](#group). Both collections and groups can be used to group several tasks.  
The main difference between collections and groups is that a collection requests from DDS to execute its tasks on the same physical machine, if resource allow that. This is useful if tasks suppose to communicate a lot or they want to access the same shared memory. A set of tasks and task collections can be also grouped into task groups. Another difference between groups and collection is that only groups can define multiplication factor for all its child elements.

Main group defines the entry point for task execution. Only main group can contain other groups.

## Topology file

At the moment we use an XML based file to store topologies. XML is chosen because it can be validated against XSD schema. DDS's XSD schema file can be found in [$DDS_LOCATION/share/topology.xsd](../res/topology.xsd).

```xml
<topology name="myTopology">
   [... Definition of tasks, properties, and collections ...]
   <main name="main">
      [... Definition of the topology itself, where also groups can be defined ...]
   </main>
</topology>
```

The file is basically divided on two parts: declaration and main part.

All properties, tasks and collections should be defined in the declaration part of the file. Users can define any number of topology entities in that block, even some, which are not going to be used in the main block.

In the main block the topology itself is defined. Groups and multiplication factors are also defined in main block.

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

DDS allows to define variables which later can be used inside the topology file. During the preprocessing all variable are replaced with their values. Variables are defined using the var tag which has two attributes name and value. Inside the file variable can be used as follows `${variable_name}`. In the above example we define two variables `${appNameVar}` and `${nofGroups}`.

When a particular task or collection is multiplied, sometimes it is necessary for the user to get the index of the task or collection instance. This can be done in two different ways. In the definition of the executable path one can use special tags `%taskIndex%` and `%collectionIndex%` to get the task and collection index respectively. Before the task execution these tags are replaced with real values. The second possibility is to get task and collection index from environment. Two environment variables are defined for each task `$DDS_TASK_INDEX` and `$DDS_COLLECTION_INDEX`.

For each user task a set of environment variables is populated.

**Populated environment variables**

* `$DDS_TASK_PATH` - full path to the user task, for example, `main/group1/collection_12/task_3`

* `$DDS_GROUP_NAME` - ID of the parent group.

* `$DDS_COLLECTION_NAME` - ID of the parent collection if any.

* `$DDS_TASK_NAME` - ID of the task.

* `$DDS_TASK_INDEX` - index of the task.

* `$DDS_COLLECTION_INDEX` - index of the collection.

* `$DDS_SESSION_ID` - DDS session this task belongs to.

In the example above we define 2 properties - `property1` and `property2`. As you can see the property tag is used to define properties. The `name` attribute is required and has to be unique for all properties.

Requirements is a way to tell DDS that a task or a collection has to be deployed on a particular computing node. As of now only host name or worker node name which is defined in the SSH configuration file are supported. Requirements are defined using the [declrequirement](#declrequirement) tag which has a number of attributes. All attributes are required. name attribute is an identifier and has to be unique for all requirements. type attribute is a type of the requirement. value attribute is a string value of the requirement. In order to define the pattern of the host name use either hostname or wnname values for the type attribute. value attribute for these requirement types can be either a full host name or a regular expression which matches the required host name. Use hostname if the requirement is defined based on the host name or wnname if the requirement is defined based on the SSH worker node name.

Task trigger defines a certain action which has to be performed whenever a specified condition is triggered. For example, if task crashed DDS will try to restart the task multiple times. For the moment only predefined conditions and actions are supported. Triggers are defined using the [decltrigger](#decltrigger) tag which has a number of attributes. All attributes are required. name attribute is an identifier and has to be unique for all triggers. condition attribute is a predefined condition. Has to be one of the following: TaskCrashed. action attribute is a predefined action. Has to be one of the following: RestartTask. arg is an argument for the action, for example, it can specify the number of attempts to restart the task.

In the next block we define tasks. For this the [decltask](#decltask) tag is used. A task must also have the name attribute which is required and has to be unique for all declared tasks. The requirements element is optional and specifies the list of the already declared requirements for the task. The triggers element is optional and defines the list of task triggers. The exe element defines path to executable. The path can include program options, even options with quotes. DDS will automatically parse the path and extract program options in runtime. The exe tag has an optional attribute reachable, which defines whether executable is available on worker nodes. If it is not available, then DDS will take care of delivering it to an assigned worker in run-time.

In case when there is a script, that, for example sets environment, has to be executed prior to main executable one can specify it using the env element. The env tag also have reachable attribute.

If a task depends on some properties this can de specified using the properties tag together with a list of name elements which specify ID of already declared properties. Each property has an optional access attribute which defines whether user task will read (read), write (write) or both read and write (readwrite) a property. Default is readwrite.

Collections are declared using the [declcollection](#declcollection) tag. It contains a list of task tags with IDs which specified already declared tasks. Task has to be declared before it can be used in the collection. As for the task collection has an optional requirements element which is used to specify a list of the requirements for the collection. If the requirement defined for both task and collection than collection requirement has higher priority and is used for deployment.

The main tag declares the topology itself. In the example our main block consists of one task (task1), one collection (collection1) and two groups (group1 and group2).

A group is declared using the group tag. It has a required attribute name, which is used to uniquely identify the group and optional attribute n, which defines multiplication factor for the group. In the example group1 consists of one task (task1) and two collections (collection1 and collection2). group2 consists of one collection (collection1).

## Topology XML references

### Topology XML tags

[topology](#topology), [var](#var), [property](#property), [declrequirement](#declrequirement), [decltrigger](#decltrigger), [decltask](#decltask), [declcollection](#declcollection), [task](#task), [collection](#collection), [group](#group), [main](#main), [exe](#exe), [env](#env), [requirements](#requirements), [properties](#properties), [name](#name)

#### topology

| Parents | Children | Attributes | Description |
| --- | --- | --- | --- |
| no | [property](#property), [task](#task), [collection](#collection), [main](#main), [var](#var) | [name](#attribute-name) | Declares a topology.|

Example:

```xml
<topology name="myTopology">
  [... Definition of tasks, 
  properties, collections and 
  groups ...]
</topology>
```

#### var

| Parents | Children | Attributes | Description |
| --- | --- | --- | --- |
| [topology](#topology) | no | [name](#attribute-name), value | Declares a variable which can be used inside the topology file as `${variable_name}`. |

Example:

```xml
<var name="var1" value="value1"/>
<var name="var2" value="value2"/>
```

#### property

| Parents | Children | Attributes | Description |
| --- | --- | --- | --- |
| [topology](#topology) | no | name | Declares a property. |

```xml
<property name="property1"/>
<property name="property2"/>
```

#### declrequirement

| Parents | Children | Attributes | Description |
| --- | --- | --- | --- |
| [topology](#topology) | no | [name](#attribute-name), [type](#attribute-type), value | Declares a requirement for tasks and collections. |

```xml
<declrequirement name="requirement1" type="hostname" value="+.gsi.de"/>
```

#### decltrigger

| Parents | Children | Attributes | Description |
| --- | --- | --- | --- |
| [topology](#topology) | no | [name](#attribute-name), [condition](#attribute-condition), [action](#attribute-action), arg | Declares a task trigger. |

```xml
<decltrigger name="trigger1" condition="TaskCrashed" action="RestartTask" arg="5"/>
```

#### decltask

| Parents | Children | Attributes | Description |
| --- | --- | --- | --- |
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

| Parents | Children | Attributes | Description |
| --- | --- | --- | --- |
| [topology](#topology) | [task](#task) | [name](#attribute-name) | Declares a collection. |

```xml
<declcollection name="collection1">
    <task>task1</task>
    <task>task1</task>
</declcollection>
```

#### task

| Parents | Children | Attributes | Description |
| --- | --- | --- | --- |
| [collection](#collection), [group](#group) | no | no | Specifies the unique ID of the already defined task. |

```xml
<task>task1</task>
```

#### collection

| Parents | Children | Attributes | Description |
| --- | --- | --- | --- |
| [group](#group) | no | no | Specifies the unique ID of the already defined collection. |

```xml
<collection>collection1</collection>
```

#### group

| Parents | Children | Attributes | Description |
| --- | --- | --- | --- |
| [main](#main) | [task](#task), [collection](#collection) | [name](#attribute-name), [n](#attribute-n) | Declares a group. |

```xml
<group name="group1" n="10">
    <task>task1</task>
    <collection>collection1</collection>
    <collection>collection2</collection>
</group>
```

#### main

| Parents | Children | Attributes | Description |
| --- | --- | --- | --- |
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

| Parents | Children | Attributes | Description |
| --- | --- | --- | --- |
| [decltask](#decltask) | no | [reachable](#attribute-reachable) | Defines path to the executable or script for the task. |

```xml
<exe reachable="true">app1 -l -n</exe>
```

#### env

> [!NOTE]  
> Optional.

| Parents | Children | Attributes | Description |
| --- | --- | --- | --- |
| [decltask](#decltask) | no | [reachable](#attribute-reachable) | Defines the path to script that has to be executed prior to main executable. |

```xml
<env reachable="false">setEnv.sh</env>
```

#### requirements

> [!NOTE]  
> Optional.

| Parents | Children | Attributes | Description |
| --- | --- | --- | --- |
| [decltask](#decltask), [declcollection](#declcollection) | [name](#name) | no | Defines a list of requirements. |

```xml
<requirements>
   <name>requirement1</name>
   <name>requirement2</name>
</requirements>
```

#### properties

> [!NOTE]  
> Optional.

| Parents | Children | Attributes | Description |
| --- | --- | --- | --- |
| [decltask](#decltask) | [name](#name) | no | Defines a list of dependent properties. |

```xml
<properties>
    <name>property1</name>
    <name>property2</name>
</properties>
```

#### name

> [!NOTE]  
> Required.

| Parents | Children | Attributes | Description |
| --- | --- | --- | --- |
| [properties](#properties) | no | [access](#attribute-access) | Defines an ID of the already declared property. |

```xml
<name>property1</name>
```

### Topology XML attributes

[name](#attribute-name), [reachable](#attribute-reachable), [n](#attribute-n), [access](#attribute-access), [type](#attribute-type), [condition](#attribute-condition), [action](#attribute-action)

#### attribute: name

| Required | Default | Tags | Restrictions | Description |
| --- | --- | --- | --- | --- |
| yes | - | [topology](#topology), [property](#properties), [declrequirement](#declrequirement), [decltask](#decltask), [declcollection](#declcollection), [group](#group), [main](#main) | A string with a minimum length of 1 character. | Defines identifier (ID) for topology, property, requirement, task, collection and group. ID has to be unique within its scope, i.e. ID for tasks has to be unique only for tasks. |

```xml
<topology name="myTopology">
```

#### attribute: reachable

| Required | Default | Tags | Restrictions | Description |
| --- | --- | --- | --- | --- |
| no | true | [exe](#exe), [env](#env) |  `true` \| `false` | Defines if executable or script is available on the worker node. |

```xml
<exe reachable="true">app -l</exe>
<env>env1</env>
```

#### attribute: n

| Required | Default | Tags | Restrictions | Description |
| --- | --- | --- | --- | --- |
| no | 1 | [group](#group) |  An unsigned integer 32-bit value. Min is 1 | Defines multiplication factor for group. |

```xml
<group name="group1" n="10">
    <task>task1</task>
    <collection>collection1</collection>
    <collection>collection2</collection>
</group>
```

#### attribute: access

| Required | Default | Tags | Restrictions | Description |
| --- | --- | --- | --- | --- |
| no | `readwrite` | [name](#name) |  `read`\|`write`\|`readwrite` | Defines access type from user task to properties. |

```xml
<name access="read">property1</name>
```

#### attribute: type

| Required | Default | Tags | Restrictions | Description |
| --- | --- | --- | --- | --- |
| yes | - | [declrequirement](#declrequirement) |  `hostname`|`wnname` | Defines the type of the requirement. |

```xml
<declrequirement name="requirement1" type="wnname" value="wn2"/>
```

#### attribute: condition

| Required | Default | Tags | Restrictions | Description |
| --- | --- | --- | --- | --- |
| yes | - | [decltrigger](#decltrigger) |  `TaskCrashed` | Defines trigger condition. |

```xml
<decltrigger name="trigger1" condition="TaskCrashed" action="RestartTask" arg="5"/>
```

#### attribute: action

| Required | Default | Tags | Restrictions | Description |
| --- | --- | --- | --- | --- |
| yes | - | [decltrigger](#decltrigger) |  `RestartTask` | Defines trigger action. |

```xml
<decltrigger name="trigger1" condition="TaskCrashed" action="RestartTask" arg="5"/>
```
