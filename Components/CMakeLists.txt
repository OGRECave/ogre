#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure Components build

set(OGRE_COMP_LIB_TYPE STATIC)
if (UNIX AND NOT APPLE)
  # Building components statically in an otherwise dynamic build causes
  # problems on Linux, so don't do that.
	set(OGRE_COMP_LIB_TYPE ${OGRE_LIB_TYPE})
endif ()

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
