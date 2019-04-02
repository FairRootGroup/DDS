#! /usr/bin/env bash

location=$1

current_dir=$(pwd)
build_dir="${current_dir}/buildtest"
echo "DDS from this location is used ${location}"
source ${location}/DDS_env.sh

echo "Start building the project in ${build_dir}..."
mkdir -p ${build_dir}
cd ${build_dir}
cmake ../
make

echo "Starting test-exec ..."
exeFile="${build_dir}/test-exec"
${exeFile}

echo "Removing build directory ${build_dir}"
rm -rf ${build_dir}

echo "Test done"
