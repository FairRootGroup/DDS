#! /usr/bin/env bash


clean_up()
{
 popd
 exit 1
}

trap clean_up SIGHUP SIGINT SIGTERM


exec_test() {

if [ -e $1 ]; then
	echo ">>> Processing $1"
	./$1 $2 || clean_up
	echo
else
	echo "WARNING: can't find $1"
fi

}

# TODO: pass POD_LOCATION as an argument to the script
eval POD_LOCATION=../$1
if [ -z "${LD_LIBRARY_PATH}" ]; then
   LD_LIBRARY_PATH=$POD_LOCATION/lib; export LD_LIBRARY_PATH
else
   LD_LIBRARY_PATH=$POD_LOCATION/lib:$LD_LIBRARY_PATH; export LD_LIBRARY_PATH
fi

# Mac OS X
if [ -z "${DYLD_LIBRARY_PATH}" ]; then
   DYLD_LIBRARY_PATH=$POD_LOCATION/lib; export DYLD_LIBRARY_PATH   # Mac OS X
else
   DYLD_LIBRARY_PATH=$POD_LOCATION/lib:$DYLD_LIBRARY_PATH; export DYLD_LIBRARY_PATH
fi


pushd $(pwd)
cd $1

echo "Working directory: $(pwd)"
echo

echo "----------------------"
echo "MiscCommon UNIT-TESTs"
echo "----------------------"
exec_test "MiscCommon_test_MiscUtils"
#  we use --catch_system_errors=no here to prevent BOOST catching SIGCHLD
exec_test "MiscCommon_test_Process" "--catch_system_errors=no"
exec_test "MiscCommon_test_SysHelper"
exec_test "MiscCommon_test_FindCfgFile"

echo "----------------------"
echo "pod-agent UNIT-TESTs"
echo "----------------------"
exec_test "pod-agent_test_ProtocolCommands"
exec_test "pod-agent_test_Protocol"

# prepare dummy files
# TODO: implement a function for the following
ADMIN_PATH_SRV="PoDServer/.xproofd.22222/activesessions"
ADMIN_PATH=".xproofd.22222/activesessions"
mkdir -p $ADMIN_PATH_SRV
mkdir -p $ADMIN_PATH
echo "2" > $ADMIN_PATH_SRV/manafov.default.1234.status
echo "0" > $ADMIN_PATH/manafov.default.1234.status
echo "1" > $ADMIN_PATH/manafov.default.5678.status
echo "3" > $ADMIN_PATH/manafov.default.9.status
exec_test "pod-agent_test_ProofStatusFile"


echo "----------------------"
echo "pod-ssh UNIT-TESTs"
echo "----------------------"
exec_test "pod-ssh_test_config"
exec_test "pod-ssh_test_threadpool"

popd

exit 0
