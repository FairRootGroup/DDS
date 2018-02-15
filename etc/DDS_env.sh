#!/usr/bin/env bash
# Copyright 2014 GSI, Inc. All rights reserved.
#
#
#=============================================================================
# ***** create_dir  *****
#=============================================================================
create_dir()
{
   if [ ! -d "$1" ]; then
      mkdir -p "$1"
   fi
}
#=============================================================================
# ***** MAIN  *****
#=============================================================================
# DDS Variables
if [ "x${BASH_ARGV[0]}" = "x" ]; then
    if [ ! -f DDS_env.sh ]; then
        echo ERROR: must "cd where/DDS/is" before calling ". DDS_env.sh" for this version of bash!
        DDS_LOCATION=; export DDS_LOCATION
        return 1
    fi
    DDS_LOCATION="$PWD"; export DDS_LOCATION
else
    # get param to "."
    THIS=$(dirname ${BASH_ARGV[0]})
    DDS_LOCATION=$(cd ${THIS};pwd); export DDS_LOCATION
fi

export PATH=$DDS_LOCATION/bin:$PATH

if [ -z "${LD_LIBRARY_PATH}" ]; then
   LD_LIBRARY_PATH=$DDS_LOCATION/lib; export LD_LIBRARY_PATH
else
   LD_LIBRARY_PATH=$DDS_LOCATION/lib:$LD_LIBRARY_PATH; export LD_LIBRARY_PATH
fi

# Mac OS X
if [ -z "${DYLD_LIBRARY_PATH}" ]; then
   DYLD_LIBRARY_PATH=$DDS_LOCATION/lib; export DYLD_LIBRARY_PATH   # Mac OS X
else
   DYLD_LIBRARY_PATH=$DDS_LOCATION/lib:$DYLD_LIBRARY_PATH; export DYLD_LIBRARY_PATH
fi


# local folder
eval LOCAL_DDS="$HOME/.DDS"

# create local DDS directories
create_dir "$LOCAL_DDS"

## create a default configuration file if needed
DDS_CFG=$(dds-user-defaults -p)
if [ -z "$DDS_CFG" ]; then
   dds-user-defaults -d -c "$LOCAL_DDS/DDS.cfg"
fi

## set log directory for the server
eval DDS_LOG_LOCATION=$(dds-user-defaults --key server.log_dir); export DDS_LOG_LOCATION
#
## create working dir for custom locations
#eval WORK_DIR=$(pod-user-defaults -V --key server.work_dir)
#create_dir "$WORK_DIR"

