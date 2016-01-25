#! /usr/bin/env bash

location=$1
echo "DDS from this location is used ${location}"
source ${location}/DDS_env.sh

sshFileTemplate=${DDS_LOCATION}/tests/index_test_hosts.cfg
sshFile=${DDS_LOCATION}/tests/index_test_hosts_real.cfg
topologyFile=${DDS_LOCATION}/tests/index_test.xml
resultFile=${DDS_LOCATION}/tests/index_test_result.txt
tmpResultFile=${DDS_LOCATION}/tests/index_test_result_tmp.txt
wrkDir=${DDS_LOCATION}/tests/tmp/wn_test_dir/
requiredNofAgents=8

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
sleep 65

echo "Getting property values..."
dds-info --prop-values | sort > ${tmpResultFile}

difference=$(diff ${resultFile} ${tmpResultFile})

if [ -z "${difference}" ]
then
  echo "Test passed."
else
  echo "Test failed"
fi

echo "Stoping server..."
dds-server stop

rm ${tmpResultFile}
rm ${sshFile}
rm -rf ${wrkDir}

exit 0
