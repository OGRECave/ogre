#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure Tools build

# Tools can't be run on the iOS so don't build them
if (NOT OGRE_BUILD_PLATFORM_APPLE_IOS AND NOT (WINDOWS_STORE OR WINDOWS_PHONE) AND OGRE_BUILD_COMPONENT_MESHLODGENERATOR)
  add_subdirectory(MeshTool)
endif (NOT OGRE_BUILD_PLATFORM_APPLE_IOS AND NOT (WINDOWS_STORE OR WINDOWS_PHONE) AND OGRE_BUILD_COMPONENT_MESHLODGENERATOR)
