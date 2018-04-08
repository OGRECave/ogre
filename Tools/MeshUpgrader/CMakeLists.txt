#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure MeshUpgrader

set(SOURCE_FILES 
  src/main.cpp
)

add_executable(OgreMeshUpgrader ${SOURCE_FILES})
target_link_libraries(OgreMeshUpgrader ${OGRE_LIBRARIES} ${OGRE_MeshLodGenerator_LIBRARIES})
if (APPLE)
    set_target_properties(OgreMeshUpgrader PROPERTIES
        LINK_FLAGS "-framework Carbon -framework Cocoa")
endif ()
if (OGRE_PROJECT_FOLDERS)
	set_property(TARGET OgreMeshUpgrader PROPERTY FOLDER Tools)
endif ()
ogre_config_tool(OgreMeshUpgrader)
