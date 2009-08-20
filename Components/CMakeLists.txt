#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure Components build

if (OGRE_BUILD_COMPONENT_PAGING)
  add_subdirectory(Paging)
endif ()

if (OGRE_BUILD_COMPONENT_PROPERTY)
  add_subdirectory(Property)
endif ()

if (OGRE_BUILD_COMPONENT_TERRAIN)
  add_subdirectory(Terrain)
endif ()
