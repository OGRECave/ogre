# Configure Samples build

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Common/include)
include_directories(${CMAKE_SOURCE_DIR}/Components/Overlay/include)

set( SAMPLE_COMMON_BASIC_RESOURCES "" )
set( SAMPLE_COMMON_RESOURCES "" )
set( SAMPLES_PLIST "" )

if( NOT OGRE_BUILD_PLATFORM_APPLE_IOS )
	if (OGRE_BUILD_SAMPLES2 AND NOT SDL2_FOUND)
		message(STATUS "Could not find dependency: SDL2")
		message(STATUS "Skipping samples & tests build")
		set(OGRE_BUILD_SAMPLES2 0)
	else()
		include_directories(${SDL2_INCLUDE_DIR})
	endif()
endif()

if( APPLE )
	set( SAMPLE_COMMON_BASIC_RESOURCES ${SAMPLE_COMMON_BASIC_RESOURCES}
		 ${CMAKE_CURRENT_SOURCE_DIR}/Common/src/System/iOS/Resources/Main.storyboard
		 ${CMAKE_BINARY_DIR}/bin/resources2.cfg
		 ${CMAKE_SOURCE_DIR}/Samples/Media/2.0
		 ${CMAKE_SOURCE_DIR}/Samples/Media/Hlms
		 ${CMAKE_SOURCE_DIR}/Samples/Media/packs/DebugPack.zip )

	set( SAMPLE_COMMON_RESOURCES ${SAMPLE_COMMON_BASIC_RESOURCES}
		 ${CMAKE_SOURCE_DIR}/Samples/Media/models
		 ${CMAKE_SOURCE_DIR}/Samples/Media/materials/textures )

	set( SAMPLES_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Common/src/System/iOS/Resources/Info.plist )
endif()

if( OGRE_BUILD_SAMPLES2 )
    add_subdirectory(Common)
endif()

if( OGRE_BUILD_SAMPLES2 )
	set( OGRE_SAMPLES_LIBRARIES "OgreSamplesCommon" "OgreOverlay" )

	if( OGRE_STATIC )
		if( OGRE_BUILD_RENDERSYSTEM_METAL )
			set( OGRE_SAMPLES_LIBRARIES "RenderSystem_Metal" ${OGRE_SAMPLES_LIBRARIES} )
		endif()
	endif()

	add_subdirectory(ApiUsage/AnimationTagPoint)
	add_subdirectory(ApiUsage/ImportAnimationsShareSkeletonInstance)
	add_subdirectory(ApiUsage/CustomRenderable)
	add_subdirectory(ApiUsage/DynamicGeometry)
	add_subdirectory(ApiUsage/LocalCubemaps)
	add_subdirectory(ApiUsage/V1Interfaces)
	add_subdirectory(ApiUsage/V2Mesh)
	add_subdirectory(ApiUsage/V2ManualObject)
	add_subdirectory(ApiUsage/StencilTest)
	add_subdirectory(ApiUsage/StereoRendering)
	add_subdirectory(ApiUsage/ShadowMapDebugging)

	add_subdirectory(Showcase/Forward3D)
	add_subdirectory(Showcase/Hdr)
	add_subdirectory(Showcase/PbsMaterials)
	add_subdirectory(Showcase/Postprocessing)

	add_subdirectory(Tutorials/Tutorial01_Initialization)
	add_subdirectory(Tutorials/Tutorial02_VariableFramerate)
	add_subdirectory(Tutorials/Tutorial03_DeterministicLoop)
	add_subdirectory(Tutorials/Tutorial04_InterpolationLoop)
	add_subdirectory(Tutorials/Tutorial05_MultithreadingBasics)
	add_subdirectory(Tutorials/Tutorial06_Multithreading)
	add_subdirectory(Tutorials/TutorialCompute01_UavTexture)
	add_subdirectory(Tutorials/TutorialCompute02_UavBuffer)
	add_subdirectory(Tutorials/Tutorial_Distortion)
	add_subdirectory(Tutorials/Tutorial_DynamicCubemap)
	add_subdirectory(Tutorials/Tutorial_ReconstructPosFromDepth)
	add_subdirectory(Tutorials/TutorialSky_Postprocess)
	add_subdirectory(Tutorials/Tutorial_SSAO)
	add_subdirectory(Tutorials/Tutorial_Terrain)
	add_subdirectory(Tutorials/TutorialUav01_Setup)
	add_subdirectory(Tutorials/TutorialUav02_Setup)
endif()

if( OGRE_BUILD_TESTS )
	add_subdirectory(Tests/Restart)
endif()
