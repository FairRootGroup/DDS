# Copyright 2017-2021 GSI, Inc. All rights reserved.
#
#

###################################################
## Collect WN PKG files
###################################################

# Collect non-system dependencies
file(GET_RUNTIME_DEPENDENCIES
  RESOLVED_DEPENDENCIES_VAR deps
  EXECUTABLES ${EXECUTABLES} )

# Generate regex that excludes system prefixes
list(JOIN EXCLUDED_SYSTEM_PREFIXES "|" excluded_system_prefix_regex)
set(excluded_system_prefix_regex "^(${excluded_system_prefix_regex})/")

# Copy dependencies to wn pkg staging directory
foreach(file IN LISTS EXECUTABLES deps)
  if(NOT file MATCHES ${excluded_system_prefix_regex})
    message("WN PKG prerequisite='${file}'")
    file(COPY ${file} DESTINATION ${DESTINATION})
  endif()
endforeach()
