#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure VRMLConverter

file(GLOB HEADER_FILES vrmllib/*.h)
file(GLOB SOURCE_FILES src/VRML2mesh.cpp vrmllib/src/*.cpp)

add_executable(VRMLConverter ${HEADER_FILES} ${SOURCE_FILES})
target_include_directories(VRMLConverter PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/vrmllib/include)
target_link_libraries(VRMLConverter OgreMain)

if (OGRE_PROJECT_FOLDERS)
	set_property(TARGET VRMLConverter PROPERTY FOLDER Tools)
endif ()
ogre_config_tool(VRMLConverter)
