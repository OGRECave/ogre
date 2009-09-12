#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure CgProgramManager build

set (HEADER_FILES
  include/OgreCgFxScriptLoader.h
  include/OgreCgPlugin.h
  include/OgreCgPrerequisites.h
  include/OgreCgProgram.h
  include/OgreCgProgramFactory.h
)

set (SOURCE_FILES
  src/OgreCgFxScriptLoader.cpp
  src/OgreCgPlugin.cpp
  src/OgreCgProgram.cpp
  src/OgreCgProgramFactory.cpp
  src/OgreCgProgramManagerDll.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include ${Cg_INCLUDE_DIRS})
add_definitions(-D_USRDLL)

add_library(Plugin_CgProgramManager ${OGRE_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(Plugin_CgProgramManager OgreMain ${Cg_LIBRARIES})
if (NOT OGRE_STATIC)
  set_target_properties(Plugin_CgProgramManager PROPERTIES
    COMPILE_DEFINITIONS OGRE_CGPLUGIN_EXPORTS
  ) 
endif (NOT OGRE_STATIC)

if (APPLE AND NOT OGRE_BUILD_PLATFORM_IPHONE)
    # Set the INSTALL_PATH so that Plugins can be installed in the application package
    set_target_properties(Plugin_CgProgramManager
       PROPERTIES BUILD_WITH_INSTALL_RPATH 1
       INSTALL_NAME_DIR "@executable_path/../Plugins"
    )
endif()

ogre_config_plugin(Plugin_CgProgramManager)
if (OGRE_INSTALL_PLUGINS_HEADERS)
  install(FILES ${HEADER_FILES} DESTINATION include/OGRE/Plugins/CgProgramManager)
endif ()

