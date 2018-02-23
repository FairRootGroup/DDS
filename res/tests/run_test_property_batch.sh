#! /usr/bin/env bash

location=$1
echo "DDS from this location is used ${location}"
source ${location}/DDS_env.sh

topologyFile=${DDS_LOCATION}/tests/property_test.xml
requiredNofAgents=10

echo "Starting DDS server..."
startOutput=$(dds-server start -s)

echo "${startOutput}"

sessionID=$(echo -e "${startOutput}" | head -1 | awk '{split($0,a,":"); print a[2]}' |  tr -d '[:space:]')

echo "SESSION ID: ${sessionID}"

echo "Submiting agents..."
dds-submit -r localhost -n 10 --session ${sessionID}

counter=0
nofAgents=$(dds-info -n --session ${sessionID})
while [ ${nofAgents} -lt ${requiredNofAgents} ]; do
	nofAgents=$(dds-info -n --session ${sessionID})
    counter=counter+1
    if [ ${counter} -gt 20 ]; then
      echo "Not enough agents"
      exit 1
    fi
    sleep 1
done

echo "Activating topology..."
dds-topology --disable-validation --session ${sessionID} --activate ${topologyFile}

# Give 60 seconds for tasks to finish
echo "Waiting 60 seconds..."
sleep 60

echo "Getting logs..."
dds-agent-cmd getlog -a --session ${sessionID}

wrkDir=$(dds-user-defaults --session ${sessionID} -V --key server.work_dir)
logDir=$(eval echo "${wrkDir}/log/agents")
echo "Search for logs in: ${logDir}"

for file in $(find "${logDir}" -name "*.tar.gz"); do tar -xf ${file} -C "${logDir}" ; done

nofGoodResults=$(grep -r --include "*.log" "User task exited with status 0" "${logDir}" | wc -l)

if [ "${nofGoodResults}" -eq "${requiredNofAgents}" ]
then
  echo "Test passed."
else
  echo "Test failed: number of good results is ${nofGoodResults}, required number of good results is ${requiredNofAgents}"
fi

echo "Stoping server..."
dds-server stop ${sessionID}

exit 0
