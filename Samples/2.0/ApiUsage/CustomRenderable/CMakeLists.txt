#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

macro( add_recursive dir retVal )
	file( GLOB_RECURSE ${retVal} ${dir}/*.h ${dir}/*.cpp ${dir}/*.c )
endmacro()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_recursive( ./ SOURCE_FILES )

ogre_add_executable(Sample_CustomRenderable WIN32 ${SOURCE_FILES} ${SAMPLE_COMMON_RESOURCES})

target_link_libraries(Sample_CustomRenderable ${OGRE_LIBRARIES} ${OGRE_SAMPLES_LIBRARIES})
ogre_config_sample_lib(Sample_CustomRenderable)

if( APPLE )
	set_target_properties( Sample_CustomRenderable PROPERTIES RESOURCE "${SAMPLE_COMMON_RESOURCES}" )
	set_target_properties( Sample_CustomRenderable PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${SAMPLES_PLIST} )
endif()
