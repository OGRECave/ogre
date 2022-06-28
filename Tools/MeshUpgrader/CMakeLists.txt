#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure MeshUpgrader
add_executable(OgreMeshUpgrader src/main.cpp)
target_link_libraries(OgreMeshUpgrader OgreMain OgreMeshLodGenerator)
if (OGRE_PROJECT_FOLDERS)
	set_property(TARGET OgreMeshUpgrader PROPERTY FOLDER Tools)
endif ()
ogre_config_tool(OgreMeshUpgrader)
