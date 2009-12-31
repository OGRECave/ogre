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
set(OGRE_COMP_STATIC ${OGRE_STATIC})

if (OGRE_COMP_STATIC)
	add_definitions(-DOGRE_COMPONENT_STATIC)
endif()

if (OGRE_BUILD_COMPONENT_PAGING)
  add_subdirectory(Paging)
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
