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

set( RESTART_RESOURCES ${SAMPLE_COMMON_RESOURCES} )
if( OGRE_BUILD_PLATFORM_APPLE_IOS )
	set( RESTART_RESOURCES ${RESTART_RESOURCES}
		 ${CMAKE_CURRENT_SOURCE_DIR}/iOS/Restart.storyboard )
	set( SOURCE_FILES ${SOURCE_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/iOS/RestartViewController.mm )
endif()

if( OGRE_BUILD_SAMPLES_AS_BUNDLES )
	ogre_add_executable(Test_Restart WIN32 MACOSX_BUNDLE ${SOURCE_FILES} ${RESTART_RESOURCES} )
else()
	ogre_add_executable(Test_Restart WIN32 ${SOURCE_FILES} ${RESTART_RESOURCES} )
endif()

target_link_libraries(Test_Restart ${OGRE_LIBRARIES} ${OGRE_SAMPLES_LIBRARIES})
ogre_config_sample_lib(Test_Restart)

if( APPLE )
	set_target_properties( Test_Restart PROPERTIES RESOURCE "${RESTART_RESOURCES}" )
	set_target_properties( Test_Restart PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/iOS/Info.plist )
endif()
