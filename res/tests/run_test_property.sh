#! /usr/bin/env bash

eval location=$DDS_LOCATION
echo "DDS from this location is used ${location}"
source ${location}/DDS_env.sh

topologyFile=${DDS_LOCATION}/tests/property_test.xml
requiredNofAgents=10

echo "Starting DDS server..."
dds-session start

sessionID=$(dds-user-defaults --default-session-id)

echo "SESSION ID: ${sessionID}"

echo "Submiting agents..."
dds-submit -r localhost -n 10

dds-info --wait-for-idle-agents ${requiredNofAgents}

echo "Activating topology..."
dds-topology --disable-validation --activate ${topologyFile}

dds-info --wait-for-idle-agents ${requiredNofAgents}

echo "Getting logs..."
dds-agent-cmd getlog -a

wrkDir=$(dds-user-defaults -V --key server.work_dir)
logDir=$(eval echo "${wrkDir}/log/agents")
echo "Search for logs in: ${logDir}"

for file in $(find "${logDir}" -name "*.tar.gz"); do tar -xf ${file} -C "${logDir}" ; done

nofGoodResults=$(grep -r --include "*.log" "User task exited with status 0" "${logDir}" | wc -l)

if [ "${nofGoodResults}" -eq "${requiredNofAgents}" ]
then
  echo "-------------------------"
  echo "---=== Test passed ===---"
  echo "-------------------------"
else
  echo "Test failed: number of good results is ${nofGoodResults}, required number of good results is ${requiredNofAgents}"
fi

echo "Stoping server..."
dds-session stop ${sessionID}

exit 0
