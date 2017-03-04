# Copyright 2017 GSI, Inc. All rights reserved.
#
#
include(GetPrerequisites)

# WORKAROUND: if "macro" is used it doesn't outoup "message" messages.
#macro(DDS_CollectPrerequisites)
	
	###################################################
	## Collect prerequisites for WN PKG
	###################################################
	message( STATUS "Using BOOST Library dir: " ${DDS_BOOST_LIB_DIR})
	string(REPLACE "::" ";" PREREQ_DIRS_LIST ${PREREQ_DIRS})
	message( STATUS "prerequisite dirs: " "${PREREQ_DIRS_LIST}")
	
	# WORKAROUND: the list comes broken into the macro, we need to rebuild it
	# if we don't do that, the  get_prerequisites doesn't use all avaliable directories.
	# I didn't find anyother way, but rebuilt the list.
	set(PREREQ_DIRS_LIST_REBUILT "")
	foreach(dir ${PREREQ_DIRS_LIST})
	     # message(STATU "DEBUG: " ${dir})
		 LIST(APPEND PREREQ_DIRS_LIST_REBUILT ${dir})
	 endforeach()

	get_prerequisites(${DDS_AGENT_BIN_PATH} DEPENDENCIES 1 0 "" "${PREREQ_DIRS_LIST_REBUILT}")

	
	foreach(DEPENDENCY_FILE ${DEPENDENCIES})
		# get file name to be able to resolve files with @rpath on macOS
		get_filename_component(PREREQNAME "${DEPENDENCY_FILE}"  NAME)
	  	gp_resolve_item(${DDS_AGENT_BIN_PATH} "${PREREQNAME}" "" "${PREREQ_DIRS_LIST_REBUILT}" resolved_file)
	 	message("WN PKG prerequisite='${resolved_file}'")
		file(COPY ${resolved_file} DESTINATION ${WN_PKG_DIR})
	endforeach()
	###################################################

#endmacro(DDS_CollectPrerequisites)