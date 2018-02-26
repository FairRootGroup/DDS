#!/bin/bash

# Task name
#SBATCH -J dds_property_test

# Run time limit
#SBATCH --time=01:00:00

# Working directory on shared storage
#SBATCH -D /lustre/nyx/rz/andrey/wn_dds

# Standard and error output in different files
#SBATCH -o %j_%A_%a_%N.out.log
#SBATCH -e %j_%A_%a_%N.err.log

# Execute

env

# Set explicitly $HOME to /lustre/nyx
export HOME="/lustre/nyx/rz/andrey/"

dds_install_dir="/lustre/nyx/rz/andrey/DDS"

"${dds_install_dir}/tests/run_test_property.sh" "${dds_install_dir}"
