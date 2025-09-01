# The Dynamic Deployment System (DDS)

![DDS CI](https://github.com/FairRootGroup/DDS/actions/workflows/master.yaml/badge.svg)
[![License: LGPL v3](https://img.shields.io/badge/License-LGPL_v3-blue.svg)](https://www.gnu.org/licenses/lgpl-3.0)
[![fair-software.eu](https://img.shields.io/badge/fair--software.eu-%E2%97%8F%20%20%E2%97%8F%20%20%E2%97%8B%20%20%E2%97%8F%20%20%E2%97%8B-orange)](https://fair-software.eu)
[![DOI](https://zenodo.org/badge/16586922.svg)](https://zenodo.org/badge/latestdoi/16586922)

DDS is a tool-set that automates and significantly simplifies the deployment of user-defined processes and their dependencies on any resource management system using a given topology.

## What is DDS?

DDS is a tool-set that automates and significantly simplifies the deployment of user-defined processes and their dependencies on any resource management system using a given topology.

**Key Features:**

* Works with any batch system (SLURM, LSF, PBS) or standalone via SSH
* No pre-installation required on worker nodes
* Supports distributed task communication via key-value properties
* Provides isolated sandboxes for each deployment
* Handles complex task dependencies and execution rules

## Quick Start

1. **Install DDS**: [Installation Guide](./docs/install.md)
2. **First Run**: [Quick Start Guide](./docs/quick-start.md)
3. **Learn by Example**: [Tutorials](./docs/tutorials.md)

## Documentation

### Getting Started

* [Requirements](./docs/requirements.md) - System and network requirements
* [Download](./docs/download.md) - Get DDS releases
* [Installation](./docs/install.md) - Build and install from source
* [Quick Start](./docs/quick-start.md) - Run your first DDS deployment
* [How to Start](./docs/how-to-start.md) - Detailed usage guide

### Configuration and Usage

* [User Defaults Configuration](./docs/user-defaults-configuration.md) - Customize DDS behavior
* [Topology description](./dds-topology-lib/README.md) - Define your task workflows  
* [Tutorials](./docs/tutorials.md) - Step-by-step examples

### Command-Line Tools

* [dds-session](dds-session/README.md) - Session management
* [dds-commander](dds-commander/README.md) - Core server component
* [dds-user-defaults](dds-user-defaults/README.md) - Configuration management
* [dds-submit](dds-submit/README.md) - Agent deployment
* [dds-info](dds-info/README.md) - Status and monitoring
* [dds-test](dds-test/README.md) - Testing utilities
* [dds-topology](dds-topology/README.md) - Topology management
* [dds-agent-cmd](dds-agent-cmd/README.md) - Agent commands

### Development APIs

* [dds-tools](./dds-tools-lib/README.md) - C++ API for DDS integration

### Resource Management Plugins

* [localhost](./plugins/dds-submit-localhost/README.md) - Local machine deployment
* [ssh](./plugins/dds-submit-ssh/README.md) - SSH-based deployment
* [slurm](./plugins/dds-submit-slurm/README.md) - SLURM integration
* [lsf](./plugins/dds-submit-lsf/README.md) - LSF integration
* [pbs](./plugins/dds-submit-pbs/README.md) - PBS integration
* [For Plug-in developers](./plugins/README.md#for-plug-in-developers) - Creating custom plugins

### Additional Documentation

* [Build 3rd-party dependencies](./docs/3rd-party.md) - Development setup

## Links

* [Latest Release](https://github.com/FairRootGroup/DDS/releases/latest)
* [User's Manual](https://github.com/FairRootGroup/DDS/blob/master/docs/)
* [All Releases](https://github.com/FairRootGroup/DDS/releases)
