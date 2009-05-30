#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure Tools build
if (APPLE)
  include_directories("${CMAKE_SOURCE_DIR}/Mac/Ogre/include")
endif()

add_subdirectory(XMLConverter)
add_subdirectory(MeshUpgrader)

