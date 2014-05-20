#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure MeshUpgrader

macro( add_recursive dir retVal )
	file( GLOB_RECURSE ${retVal} ${dir}/*.h ${dir}/*.cpp ${dir}/*.c )
endmacro()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_recursive( ./src SOURCE_FILES )
add_recursive( ./include HEADER_FILES )

ogre_add_executable(OgreHlms ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(OgreHlms ${OGRE_LIBRARIES} ${OGRE_Hlms_LIBRARIES})
if (APPLE)
    set_target_properties(OgreHlms PROPERTIES
        LINK_FLAGS "-framework Carbon -framework Cocoa")
endif ()

ogre_config_tool(OgreHlms)
