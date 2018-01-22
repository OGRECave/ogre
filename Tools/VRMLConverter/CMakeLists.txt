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

if(UNIX)
    # add_definitions(-Wno-shadow) # inside tinyxml sources
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/vrmllib/include)

add_executable(VRMLConverter ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(VRMLConverter ${OGRE_LIBRARIES})

if (APPLE)
    set_target_properties(VRMLConverter PROPERTIES
        LINK_FLAGS "-framework Carbon -framework Cocoa")
endif ()
if (OGRE_PROJECT_FOLDERS)
	set_property(TARGET VRMLConverter PROPERTY FOLDER Tools)
endif ()
ogre_config_tool(VRMLConverter)
