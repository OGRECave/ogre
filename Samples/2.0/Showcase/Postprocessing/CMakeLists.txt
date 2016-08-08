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

include_directories(${CMAKE_SOURCE_DIR}/Components/Hlms/Common/include)
ogre_add_component_include_dir(Hlms/Pbs)

add_recursive( ./ SOURCE_FILES )

set( POSTPROCESSING_RESOURCES ${SAMPLE_COMMON_RESOURCES} )
if( APPLE )
	set( POSTPROCESSING_RESOURCES ${POSTPROCESSING_RESOURCES}
		 ${CMAKE_SOURCE_DIR}/Samples/Media/packs/cubemapsJS.zip )
endif()

ogre_add_executable(Sample_Postprocessing WIN32 MACOSX_BUNDLE ${SOURCE_FILES} ${POSTPROCESSING_RESOURCES})

target_link_libraries(Sample_Postprocessing ${OGRE_LIBRARIES} ${OGRE_SAMPLES_LIBRARIES})
ogre_config_sample_lib(Sample_Postprocessing)

if( APPLE )
	set_target_properties( Sample_Postprocessing PROPERTIES RESOURCE "${POSTPROCESSING_RESOURCES}" )
	set_target_properties( Sample_Postprocessing PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${SAMPLES_PLIST} )
endif()
