#! /usr/bin/env bash

location=$1
echo "DDS from this location is used ${location}"
source ${location}/DDS_env.sh

topologyFile=${DDS_LOCATION}/tests/property_test.xml
requiredNofAgents=10

echo "Starting DDS server..."
dds-server start -s

echo "Submiting agents..."
dds-submit -r localhost -n 10

sleep 5

nofAgents=$(dds-info -n)
while [  ${nofAgents} -lt ${requiredNofAgents} ]; do
	nofAgents=$(dds-info -n)
done

echo "Activating topology..."
dds-topology --activate ${topologyFile}

# DDS commander updates property list each 60 seconds
echo "Waiting 60 seconds..."
sleep 60

echo "Getting logs..."
dds-agent-cmd getlog -a

wrkDir=$(dds-user-defaults -V --key server.work_dir)
logDir=$(eval echo "${wrkDir}/log/agents")
echo "Search for logs in: ${logDir}"

for file in $(find "${logDir}" -name "*.tar.gz"); do tar -xf ${file} -C "${logDir}" ; done

nofGoodResults=$(grep -r --include "*.log" "Task successfully done" "${logDir}" | wc -l)

if [ "${nofGoodResults}" -eq "${requiredNofAgents}" ]
then
  echo "Test passed."
else
  echo "Test failed: number of good results is ${nofGoodResults}, required number of good results is ${requiredNofAgents}"
fi

echo "Stoping server..."
dds-server stop

exit 0
