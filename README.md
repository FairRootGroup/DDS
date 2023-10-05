# The Dynamic Deployment System (DDS)

![DDS CI](https://github.com/FairRootGroup/DDS/actions/workflows/master.yaml/badge.svg)
[![license](https://alfa-ci.gsi.de/shields/badge/license-LGPL--3.0-orange.svg)](COPYRIGHT)
[![fair-software.eu](https://img.shields.io/badge/fair--software.eu-%E2%97%8F%20%20%E2%97%8F%20%20%E2%97%8B%20%20%E2%97%8F%20%20%E2%97%8B-orange)](https://fair-software.eu)
[![DOI](https://zenodo.org/badge/16586922.svg)](https://zenodo.org/badge/latestdoi/16586922)

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

## Documentation

- [Requirements](./docs/requirements.md)
- [Quick Start](./docs/quick-start.md)
- [Download](./docs/download.md)
- [Installation](./docs/install.md)
- [How to Start](./docs/how-to-start.md)
- [Topology description](./dds-topology-lib/README.md)
- [Tutorials](./docs/tutorials.md)
- Command-line interface
  - [dds-session](dds-session/README.md)
  - [dds-commander](dds-commander/README.md)
  - [dds-user-defaults](dds-user-defaults/README.md)
  - [dds-submit](dds-submit/README.md)
  - [dds-info](dds-info/README.md)
  - [dds-test](dds-test/README.md)
  - [dds-topology](dds-topology/README.md)
  - [dds-agent-cmd](dds-agent-cmd/README.md)
- RMS plug-ins
  - [localhost](./plugins/dds-submit-localhost/README.md)
  - [ssh](./plugins/dds-submit-ssh/README.md)
  - [slurm](./plugins/dds-submit-slurm/README.md)
  - [lsf](./plugins/dds-submit-lsf/README.md)
  - [pbs](./plugins/dds-submit-pbs/README.md)
  - [For Plug-in developers](./plugins/README.md#for-plug-in-developers)
- Additional docs
  - [Build 3rd-party dependencies](./docs/3rd-party.md)

## Links

- [User's manual](http://dds.gsi.de/documentation.html)
