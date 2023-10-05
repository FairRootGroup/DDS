# Copyright 2017 GSI, Inc. All rights reserved.
#
#
include(GetPrerequisites)

# WORKAROUND: if "macro" is used it doesn't output "message" messages.
#macro(DDS_CollectPrerequisites)
	
	###################################################
	## Collect prerequisites for WN PKG
	###################################################
	message( STATUS "Using BOOST Library dir: " ${DDS_BOOST_LIB_DIR})
	string(REPLACE "::" ";" PREREQ_DIRS_LIST ${PREREQ_DIRS})
	message( STATUS "prerequisite dirs: " "${PREREQ_DIRS_LIST}")
	
	# WORKAROUND: the list comes broken into the macro, we need to rebuild it
	# if we don't do that, GET_RUNTIME_DEPENDENCIES doesn't use all avaliable directories.
	# I didn't find anyother way, but rebuilt the list.
	set(PREREQ_DIRS_LIST_REBUILT "")
	foreach(dir ${PREREQ_DIRS_LIST})
	     # message(STATU "DEBUG: " ${dir})
		 LIST(APPEND PREREQ_DIRS_LIST_REBUILT ${dir})
	 endforeach()

    file(GET_RUNTIME_DEPENDENCIES
        EXECUTABLES ${DDS_AGENT_BIN_PATH}
        RESOLVED_DEPENDENCIES_VAR _r_deps
        UNRESOLVED_DEPENDENCIES_VAR _u_deps
        DIRECTORIES ${PREREQ_DIRS_LIST_REBUILT}
        )
	
    foreach(_file ${_r_deps})
        message("WN PKG prerequisite='${_file}'")
        file(COPY "${_file}" DESTINATION ${PREREQ_DESTINATION})
    endforeach()
    
    list(LENGTH _u_deps _u_length)
    if("${_u_length}" GREATER 0)
        message(WARNING "Unresolved dependencies detected!")
    endif()
	###################################################

#endmacro(DDS_CollectPrerequisites)