set(OGRE_SAMPLES_COMMON_LIBRARY "OgreSamplesCommon")

# Sources
macro( add_recursive dir retVal )
	file( GLOB_RECURSE ${retVal} ${dir}/*.h ${dir}/*.cpp ${dir}/*.c )
endmacro()

include_directories(  )

file( GLOB SOURCE_FILES
		"${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/src/System/*.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/src/Threading/*.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/src/Utils/*.cpp" )

if( OGRE_BUILD_PLATFORM_APPLE_IOS )
	file( GLOB IOS_SOURCE_FILES
		"${CMAKE_CURRENT_SOURCE_DIR}/src/System/iOS/*.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/src/System/iOS/*.mm" )
	set( SOURCE_FILES ${SOURCE_FILES} ${IOS_SOURCE_FILES} )
else()
	file( GLOB DESKTOP_SOURCE_FILES
		"${CMAKE_CURRENT_SOURCE_DIR}/src/System/Desktop/*.cpp" )
	set( SOURCE_FILES ${SOURCE_FILES} ${DESKTOP_SOURCE_FILES} )
endif()

add_recursive( ./include HEADER_FILES )

ogre_add_component_include_dir(Hlms/Pbs)
ogre_add_component_include_dir(Hlms/Unlit)
ogre_add_component_include_dir(Hlms/Common)

if( OGRE_STATIC )
	# Static linking means we need to directly use plugins
	#include_directories(${OGRE_SOURCE_DIR}/PlugIns/ParticleFX/include)
	include_directories( ${OGRE_SOURCE_DIR}/RenderSystems/Direct3D11/include )
	include_directories( ${OGRE_SOURCE_DIR}/RenderSystems/GL3Plus/include )
	include_directories( ${OGRE_SOURCE_DIR}/RenderSystems/Metal/include )
endif()

add_library(${OGRE_SAMPLES_COMMON_LIBRARY} STATIC ${SOURCE_FILES} ${HEADER_FILES})

link_directories(${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(${OGRE_SAMPLES_COMMON_LIBRARY} ${SDL2_LIBRARY} OgreHlmsPbs OgreHlmsUnlit)

if( NOT APPLE )
	include_directories( BEFORE SYSTEM "/home/matias/OSX_Shared/Platforms/" )
	include_directories( BEFORE SYSTEM "/home/matias/OSX_Shared/Platforms/iOS" )
	include_directories( BEFORE SYSTEM "/home/matias/OSX_Shared/Platforms/iOS/include" )
	include_directories( BEFORE SYSTEM "/home/matias/OSX_Shared/Platforms/iOS/include/c++/4.2.1" )
	include_directories( BEFORE SYSTEM "/home/matias/OSX_Shared/Platforms/iOS/include/objc" )
	include_directories( BEFORE SYSTEM "/home/matias/OSX_Shared/Platforms/iOS/include++" )
	add_definitions( -D__arm64__ -DTARGET_CPU_ARM64 -D__APPLE_CPP__ -D__APPLE__ -D__APPLE_CC__ -D__IPHONE_OS_VERSION_MIN_REQUIRED=90000 "-include /home/matias/OSX_Shared/Platforms/__OSX_Fixes.h" -fblocks -fobjc-abi-version=3 -fobjc-arc -Wno-unknown-attributes -Wno-deprecated-register )
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11" )
endif()
