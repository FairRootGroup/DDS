# Quick Start

This guide gets you up and running with DDS in just a few steps. You'll deploy your first DDS agents and activate a sample topology.

## Prerequisites

* DDS successfully installed (see [Installation Guide](./install.md))
* Terminal/shell access
* For this example: localhost access (no additional setup required)

## Steps

1. [Download DDS source tarball](./download.md) or clone from GitHub.

1. [Install DDS from source](./install.md).

1. Enable DDS environment.

   ```shell
   cd <DDS INSTALLATION location>
   source DDS_env.sh
   ```

1. Start DDS commander server.

   ```shell
   dds-session start
   ```

1. Deploy 1 DDS agent with 50 task slots on the localhost.

   ```shell
   dds-submit --rms localhost --slots 50
   ```

1. Use dds-info to find out the number of agents which are online.

   ```shell
   dds-info -n
   ```

1. Use dds-info to check detailed information about agents.

   ```shell
   dds-info -l 
   ```

1. Set and activate the topology.

   ```shell
   dds-topology --activate $DDS_LOCATION/tutorials/tutorial1/tutorial1_topo.xml
   ```

## What's Next?

* Learn more with [detailed tutorials](./tutorials.md)
* Explore [configuration options](./user-defaults-configuration.md)  
* Set up [resource management plugins](../plugins/README.md) for batch systems
* Read the complete [usage guide](./how-to-start.md)
