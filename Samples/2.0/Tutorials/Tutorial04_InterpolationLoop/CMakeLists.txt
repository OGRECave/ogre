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

set( TUTORIAL_RESOURCES ${SAMPLE_COMMON_BASIC_RESOURCES} )
if( OGRE_BUILD_PLATFORM_APPLE_IOS )
	set( TUTORIAL_RESOURCES ${SAMPLE_COMMON_BASIC_RESOURCES}
		 ${CMAKE_CURRENT_SOURCE_DIR}/iOS/Tutorial.storyboard )
	set( SOURCE_FILES ${SOURCE_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/iOS/TutorialViewController.mm )
endif()

if( OGRE_BUILD_SAMPLES_AS_BUNDLES )
    ogre_add_executable(Sample_Tutorial04_InterpolationLoop WIN32 MACOSX_BUNDLE ${SOURCE_FILES} ${TUTORIAL_RESOURCES} )
else()
    ogre_add_executable(Sample_Tutorial04_InterpolationLoop WIN32 ${SOURCE_FILES} ${TUTORIAL_RESOURCES} )
endif( OGRE_BUILD_SAMPLES_AS_BUNDLES )

target_link_libraries(Sample_Tutorial04_InterpolationLoop ${OGRE_LIBRARIES} ${OGRE_SAMPLES_LIBRARIES})
ogre_config_sample_lib(Sample_Tutorial04_InterpolationLoop)

if( APPLE )
	set_target_properties( Sample_Tutorial04_InterpolationLoop PROPERTIES RESOURCE "${TUTORIAL_RESOURCES}" )
	set_target_properties( Sample_Tutorial04_InterpolationLoop PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/iOS/Info.plist )
endif()
