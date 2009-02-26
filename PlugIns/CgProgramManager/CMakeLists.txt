# Configure Octree SceneManager build

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
ogre_config_build(Plugin_CgProgramManager)
target_link_libraries(Plugin_CgProgramManager OgreMain ${Cg_LIBRARIES})

if (NOT OGRE_STATIC)
  set_target_properties(Plugin_CgProgramManager PROPERTIES
    COMPILE_DEFINITIONS OGRE_CGPLUGIN_EXPORTS
  ) 
endif (NOT OGRE_STATIC)
set_target_properties(Plugin_CgProgramManager PROPERTIES PREFIX "")

ogre_install_plugin(Plugin_CgProgramManager)
