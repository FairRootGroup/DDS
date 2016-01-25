#! /usr/bin/env bash

location=$1
echo "DDS from this location is used ${location}"
source ${location}/DDS_env.sh

sshFileTemplate=${DDS_LOCATION}/tests/functional_test_hosts.cfg
sshFile=${DDS_LOCATION}/tests/functional_test_hosts_real.cfg
topologyFile=${DDS_LOCATION}/tests/functional_test.xml
wrkDir=${DDS_LOCATION}/tests/tmp/wn_test_dir/
requiredNofAgents=20

mkdir -p ${wrkDir}

# Replace __WRK_DIR__ in SSH file with real directory
sedDir="${wrkDir//\//\\/}"
sed -e "s/__WRK_DIR__/${sedDir}/" ${sshFileTemplate} > ${sshFile}

echo "Starting DDS server..."
dds-server restart -s

echo "Submiting agents..."
dds-submit -r ssh --config ${sshFile}

nofAgents=$(dds-info -n)
while [  ${nofAgents} -lt ${requiredNofAgents} ]; do
	nofAgents=$(dds-info -n)
done

echo "Setting topology file..."
dds-topology -t ${topologyFile}

echo "Activating topology..."
dds-topology --activate

# DDS commander updates property list each 60 seconds
echo "Waiting 60 seconds..."
sleep 60

nofGoodResults=$(grep "User task exited with status 0" ${wrkDir}/wn_functional*/*.log | wc -l)

if [ "${nofGoodResults}" -eq "${requiredNofAgents}" ]
then
  echo "Test passed."
else
  echo "Test failed: number of good results is ${nofGoodResults}, required number of good results is ${requiredNofAgents}"
fi

echo "Stoping server..."
dds-server stop

rm ${sshFile}
rm -rf ${wrkDir}

exit 0
