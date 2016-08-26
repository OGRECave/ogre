#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure MeshTool

macro( add_recursive dir retVal )
	file( GLOB_RECURSE ${retVal} ${dir}/*.h ${dir}/*.cpp ${dir}/*.c )
endmacro()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_recursive( ./ SOURCE_FILES )

ogre_add_executable(OgreMeshTool ${SOURCE_FILES})
ogre_add_component_include_dir(MeshLodGenerator)

if(OGRE_STATIC)
	include_directories("${OGRE_SOURCE_DIR}/RenderSystems/NULL/include")
endif ()

target_link_libraries(OgreMeshTool ${OGRE_LIBRARIES} ${OGRE_MeshLodGenerator_LIBRARIES})

if(OGRE_STATIC)
	target_link_libraries(OgreMeshTool RenderSystem_NULL)
endif ()

if (APPLE)
    set_target_properties(OgreMeshTool PROPERTIES
        LINK_FLAGS "-framework Carbon -framework Cocoa")
endif ()

set(TINYXML_INCLUDE_DIR "")
set(TINYXML_LIBRARIES "")
add_definitions(-DTIXML_USE_STL)

ogre_config_tool(OgreMeshTool)
