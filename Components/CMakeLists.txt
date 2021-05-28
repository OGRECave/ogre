#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure Components build

# Use same build type for components as main build
set(OGRE_COMP_LIB_TYPE ${OGRE_LIB_TYPE})

if (OGRE_BUILD_COMPONENT_BITES)
    add_subdirectory(Bites)
endif()

if (OGRE_BUILD_COMPONENT_PAGING)
  add_subdirectory(Paging)
endif ()

if (OGRE_BUILD_COMPONENT_MESHLODGENERATOR)
  add_subdirectory(MeshLodGenerator)
endif ()

if (OGRE_BUILD_COMPONENT_PROPERTY)
  add_subdirectory(Property)
endif ()

if (OGRE_BUILD_COMPONENT_TERRAIN)
  add_subdirectory(Terrain)
endif ()

if (OGRE_BUILD_COMPONENT_RTSHADERSYSTEM)
	add_subdirectory(RTShaderSystem)
endif ()

if (OGRE_BUILD_COMPONENT_VOLUME)
	add_subdirectory(Volume)
endif ()

if (OGRE_BUILD_COMPONENT_OVERLAY)
	add_subdirectory(Overlay)
endif ()

if (OGRE_BUILD_COMPONENT_PYTHON)
    add_subdirectory(Python)
endif()

if (OGRE_BUILD_COMPONENT_CSHARP)
    add_subdirectory(Csharp)
endif()

if (OGRE_BUILD_COMPONENT_JAVA)
  add_subdirectory(Java)
endif()