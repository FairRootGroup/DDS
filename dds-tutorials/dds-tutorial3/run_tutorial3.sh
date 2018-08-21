#! /usr/bin/env bash

# FIXME: Path to DDS installation directory
location="path_to_dds_installation_directory"
# FIXME: Path to the topology file
topologyFile="path_to_topology_file"
# FIXME: Required number of agent for the topology
requiredNofAgents=10

# Source DDS environment
echo "DDS from this location is used ${location}"
source ${location}/DDS_env.sh

# Start DDS commander server
echo "Starting DDS server..."
startOutput=$(dds-session start --local)
echo "${startOutput}"

# Extract session ID from "dds-session start --local" output
sessionID=$(echo -e "${startOutput}" | head -1 | awk '{split($0,a,":"); print a[2]}' |  tr -d '[:space:]')
echo "DDS session ID: ${sessionID}"

# Submit agents
echo "Submiting agents..."
dds-submit -r localhost -n ${requiredNofAgents} --session ${sessionID}

# Waiting for DDS agents.
# Give DDS agents 20 seconds to start.
counter=0
nofAgents=$(dds-info -n --session ${sessionID})
while [ ${nofAgents} -lt ${requiredNofAgents} ]; do
	nofAgents=$(dds-info -n --session ${sessionID})
    let counter=counter+1
    if [ ${counter} -gt 20 ]; then
      echo "Error: not enough agents"
      exit 1
    fi
    sleep 1
done

# Activate topology
echo "Activating topology..."
dds-topology --disable-validation --session ${sessionID} --activate ${topologyFile}

# FIXME: Implement proper waiting
sleep 60

# FIXME: Implement proper check of the results

# Stop DDS commander
echo "Stoping server..."
dds-session stop ${sessionID}

exit 0
