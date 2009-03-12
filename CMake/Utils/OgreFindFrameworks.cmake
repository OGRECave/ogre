# - helper module to find OSX frameworks
# Adapted from standard CMake version, but added dependencies
# Standard finder does not look in any variable locations such as 
# CMAKE_FRAMEWORK_PATH (not sure why not)

IF(NOT OGRE_FIND_FRAMEWORKS_INCLUDED)
  SET(OGRE_FIND_FRAMEWORKS_INCLUDED 1)
  MACRO(OGRE_FIND_FRAMEWORKS fwk)
    IF(APPLE)
      SET(${fwk}_FRAMEWORKS)
      SET(OGRE_FRAMEWORK_PATH
		${OGRE_DEPENDENCIES_DIR}
    	~/Library/Frameworks
    	/Library/Frameworks
    	/System/Library/Frameworks
    	/Network/Library/Frameworks
	  )
	  FOREACH(dir ${OGRE_FRAMEWORK_PATH})
	    SET(fwkpath ${dir}/${fwk}.framework)
	    IF(EXISTS ${fwkpath})
          SET(${fwk}_FRAMEWORKS ${${fwk}_FRAMEWORKS} ${fwkpath})
        ENDIF(EXISTS ${fwkpath})
      ENDFOREACH(dir)
    ENDIF(APPLE)
  ENDMACRO(OGRE_FIND_FRAMEWORKS)
ENDIF(NOT OGRE_FIND_FRAMEWORKS_INCLUDED)
