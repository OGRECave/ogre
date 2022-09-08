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
if (NOT OGRE_DOCS_PATH)
    if (WIN32 OR APPLE)
        set(OGRE_DOCS_PATH "Docs")
    elseif (UNIX)
        set(OGRE_DOCS_PATH "share/doc/OGRE")
    endif ()
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

    file(GLOB TUTORIALS ${PROJECT_SOURCE_DIR}/Docs/src/tutorials/*.md)
    file(GLOB BASIC_TUTORIALS ${PROJECT_SOURCE_DIR}/Docs/src/tutorials/basictutorials/*.md)
    file(GLOB COMPONENTS ${PROJECT_SOURCE_DIR}/Components/*/include)
    file(GLOB PLUGINS ${PROJECT_SOURCE_DIR}/PlugIns/*/include)

    set(CMAKE_DOXYGEN_INPUT_LIST
        ${PROJECT_SOURCE_DIR}/OgreMain/include
        ${COMPONENTS}
        ${PLUGINS}
        ${PROJECT_SOURCE_DIR}/PlugIns/PCZSceneManager/docs/readme.md
        ${PROJECT_SOURCE_DIR}/PlugIns/DotScene/README.md
        ${PROJECT_SOURCE_DIR}/RenderSystems
        ${BASIC_TUTORIALS}
        ${PROJECT_SOURCE_DIR}/Docs/src/apimainpage.md
        ${PROJECT_SOURCE_DIR}/Docs/src/manual.md
        ${PROJECT_SOURCE_DIR}/Docs/src/ogre-shadows.md
        ${PROJECT_SOURCE_DIR}/Docs/src/scripts.md
        ${PROJECT_SOURCE_DIR}/Docs/src/material-scripts.md
        ${PROJECT_SOURCE_DIR}/Docs/src/high-level-programs.md
        ${PROJECT_SOURCE_DIR}/Docs/src/particle-scripts.md
        ${PROJECT_SOURCE_DIR}/Docs/src/hardware-buffers.md
        ${PROJECT_SOURCE_DIR}/Docs/src/rtss.md
        ${PROJECT_SOURCE_DIR}/Docs/src/instancing.md
        ${PROJECT_SOURCE_DIR}/Docs/src/resourcemanagement.md
        ${PROJECT_SOURCE_DIR}/BuildingOgre.md
		${TUTORIALS})
    string(REPLACE ";" " \\\n" CMAKE_DOXYGEN_INPUT_LIST "${CMAKE_DOXYGEN_INPUT_LIST}")
    configure_file(${OGRE_TEMPLATES_DIR}/html.cfg.in ${PROJECT_BINARY_DIR}/html.cfg)
    add_custom_target(OgreDoc COMMAND 
        ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/html.cfg
        DEPENDS ${PROJECT_BINARY_DIR}/html.cfg
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/Docs/src/
        COMMENT "Building documentation" VERBATIM)
    file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/api/)
    if (OGRE_INSTALL_DOCS)
        install(DIRECTORY ${PROJECT_BINARY_DIR}/api DESTINATION ${OGRE_DOCS_PATH})
    endif()
endif()
