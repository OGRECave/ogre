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

    file(GLOB TUTORIALS ${OGRE_SOURCE_DIR}/Docs/src/tutorials/*.md)
    file(GLOB BASIC_TUTORIALS ${OGRE_SOURCE_DIR}/Docs/src/tutorials/basictutorials/*.md)

    set(CMAKE_DOXYGEN_INPUT_LIST
        ${OGRE_SOURCE_DIR}/OgreMain/include
        ${OGRE_SOURCE_DIR}/Components
        ${OGRE_SOURCE_DIR}/PlugIns
        ${OGRE_SOURCE_DIR}/PlugIns/PCZSceneManager/docs/readme.md
        ${OGRE_SOURCE_DIR}/RenderSystems
        ${OGRE_SOURCE_DIR}/Docs/src/apimainpage.md
        ${OGRE_SOURCE_DIR}/Docs/src/manual.md
        ${OGRE_SOURCE_DIR}/Docs/src/OgreShadows.md
        ${OGRE_SOURCE_DIR}/Docs/src/scripts.md
        ${OGRE_SOURCE_DIR}/Docs/src/material-scripts.md
        ${OGRE_SOURCE_DIR}/Docs/src/high-level-programs.md
        ${OGRE_SOURCE_DIR}/Docs/src/particle-scripts.md
        ${OGRE_SOURCE_DIR}/Docs/src/hardware-buffers.md
        ${OGRE_SOURCE_DIR}/Docs/src/hlms.md
        ${OGRE_SOURCE_DIR}/Docs/src/rtss.md
        ${OGRE_SOURCE_DIR}/BuildingOgre.md
        ${TUTORIALS}
        ${BASIC_TUTORIALS})
    string(REPLACE ";" " \\\n" CMAKE_DOXYGEN_INPUT_LIST "${CMAKE_DOXYGEN_INPUT_LIST}")
    configure_file(${OGRE_TEMPLATES_DIR}/html.cfg.in ${OGRE_BINARY_DIR}/html.cfg)
    add_custom_target(OgreDoc COMMAND 
        ${DOXYGEN_EXECUTABLE} ${OGRE_BINARY_DIR}/html.cfg
        DEPENDS ${OGRE_BINARY_DIR}/html.cfg
        WORKING_DIRECTORY ${OGRE_SOURCE_DIR}/Docs/src/
        COMMENT "Building documentation" VERBATIM)
    file(MAKE_DIRECTORY ${OGRE_BINARY_DIR}/api/)
    if (OGRE_INSTALL_DOCS)
        install(DIRECTORY ${OGRE_BINARY_DIR}/api DESTINATION ${OGRE_DOCS_PATH})
    endif()
endif()
