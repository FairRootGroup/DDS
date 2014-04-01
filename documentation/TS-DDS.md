# DDS - Technical Specification

---
# Introduction
This document describes design and technical specifications of the DDS (Dynamic distribution system).

DDS - is an independent set of utilities and interfaces, which provide a dynamic distribution of different user processes by any given topology 
on any RMS.

## System Summary

### Design goals
* deploy any task or a set of tasks,
* utilize any RMS,
* support workers behind FireWalls,
* secure execution of tasks (watchdog),
* support different topologies and task dependencies,
* provide an isolated execution,
* provide a central log engine.

## Assumptions & Prerequisites

## System Dependencies

* a C++11 compiler
* CMake 2.6.2
* BOOST 1.55

## Technical Requirements
N/A

---
# Architecture

## ERD Model

## Topology design details
### Topo components
* Task
* Containers
  * Group
  * Collection

#### Task
* A task is a single entity of the system.
* A task can be an executable or a script.
* A task is defined by a user with a set of props and rules.
* Each task will have a dedicated DDS watchdog process.

### Topo File

~~~~~~~~~~~~~
<topology name="myTopology">

[… Definition of tasks, properties, and collections …]

	<main name=“main">

[… Definition of the topology itself, where also groups can be defined …]
	  
	</main>

</topology>
~~~~~~~~~~~~~

---

### Path
Each topology element is referenced by a path. At the moment the path is a list the element and all its parents separated by slash. For example to refer to a task T1, which is a part of the collection C2 and a group G3, we will use: "main/G3/C2/T1".

A path is mostly used by other modules of DDS to have a persistent access to the topology elements.

# Developer Testing


---
# Known Issues