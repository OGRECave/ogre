#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure Tools build

# Tools can't be run on the iPhone so don't build them
if (NOT OGRE_BUILD_PLATFORM_IPHONE)
  add_subdirectory(XMLConverter)
  add_subdirectory(MeshUpgrader)
endif (NOT OGRE_BUILD_PLATFORM_IPHONE)
