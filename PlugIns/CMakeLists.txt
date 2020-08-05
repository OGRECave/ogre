#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure Plugins build

if (OGRE_BUILD_PLUGIN_OCTREE)
  add_subdirectory(OctreeSceneManager)
endif (OGRE_BUILD_PLUGIN_OCTREE)

if (OGRE_BUILD_PLUGIN_BSP)
  add_subdirectory(BSPSceneManager)
endif (OGRE_BUILD_PLUGIN_BSP)

if (OGRE_BUILD_PLUGIN_CG)
  if (NOT Cg_FOUND)
    message(STATUS "Could not find dependency: Cg")
    message(STATUS "Skipping Plugin_CgProgramManager")
    set(OGRE_BUILD_PLUGIN_CG 0)
  else (NOT Cg_FOUND)
    add_subdirectory(CgProgramManager)
  endif (NOT Cg_FOUND)
endif (OGRE_BUILD_PLUGIN_CG)

if (OGRE_BUILD_PLUGIN_EXRCODEC)
  add_subdirectory(EXRCodec)
endif (OGRE_BUILD_PLUGIN_EXRCODEC)

if(OGRE_BUILD_PLUGIN_STBI)
  add_subdirectory(STBICodec)
endif()

if(OGRE_BUILD_PLUGIN_FREEIMAGE)
  add_subdirectory(FreeImageCodec)
endif()

if (OGRE_BUILD_PLUGIN_PFX)
  add_subdirectory(ParticleFX)
endif (OGRE_BUILD_PLUGIN_PFX)

if (OGRE_BUILD_PLUGIN_PCZ)
  add_subdirectory(PCZSceneManager)
  add_subdirectory(OctreeZone)
endif (OGRE_BUILD_PLUGIN_PCZ)

if (OGRE_BUILD_PLUGIN_DOT_SCENE)
  add_subdirectory(DotScene)
endif ()

if (OGRE_BUILD_PLUGIN_ASSIMP)
  add_subdirectory(Assimp)
endif ()