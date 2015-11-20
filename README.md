# The Dynamic Deployment System (DDS)
DDS - is a tool-set that automates and significantly simplifies a deployment of user defined processes and their dependencies on any resource management system using a given topology.

## Basic concepts
DDS:
- implements a single-responsibility-principle command line tool-set and APIs,
- treats users’ tasks as black boxes,
- doesn’t depend on RMS (provides deployment via SSH, when no RMS is present),
- supports workers behind FireWalls (outgoing connection from WNs required),
- doesn’t require pre-installation on WNs,
- deploys private facilities on demand with isolated sandboxes,
- provides a key-value properties propagation service for tasks,
- provides a rules based execution of tasks.
