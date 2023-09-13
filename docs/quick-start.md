# Quick Start

1. [Download DDS source tarball](./download.md).
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

1. Use dds-info to find out a number of agents, which are online.

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
