#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

###############################################################
# Install documentation
# Build API docs, if doxygen is available
###############################################################

# Set docs target directory
if (WIN32 OR APPLE)
  set(OGRE_DOCS_PATH "Docs")
elseif (UNIX)
  set(OGRE_DOCS_PATH "share/OGRE/docs")
endif ()
  
# Build and install API documentation if doxygen is available
if(DOXYGEN_FOUND)
    if (OGRE_INSTALL_DOCS)
      install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
	DESTINATION ${OGRE_DOCS_PATH}
	PATTERN ".hg" EXCLUDE
	PATTERN "src" EXCLUDE
      )
    endif ()

	configure_file(${OGRE_TEMPLATES_DIR}/html.cfg.in ${OGRE_BINARY_DIR}/html.cfg)
	add_custom_target(OgreDoc COMMAND 
    ${DOXYGEN_EXECUTABLE} ${OGRE_BINARY_DIR}/html.cfg 
    WORKING_DIRECTORY ${OGRE_SOURCE_DIR}/Docs/src/ 
    COMMENT "Building documentation" VERBATIM
  )
	file(WRITE ${OGRE_BINARY_DIR}/api/temp.txt "This file should be deleted before installation. It is only here to force the creation of the api folder")
	file(REMOVE ${OGRE_BINARY_DIR}/api/temp.txt)
	if (OGRE_INSTALL_DOCS)
		install(DIRECTORY ${OGRE_BINARY_DIR}/api DESTINATION ${OGRE_DOCS_PATH})
	endif()
endif()
