# Configure Samples build

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Common/include)
include_directories(${CMAKE_SOURCE_DIR}/Components/Overlay/include)

set( SAMPLE_COMMON_BASIC_RESOURCES "" )
set( SAMPLE_COMMON_RESOURCES "" )
set( SAMPLES_PLIST "" )
set( SAMPLE_COMMON_STORYBOARD_RESOURCE "" )

set (OGRE_BUILD_SAMPLES2_SKIP 0 )

if( NOT OGRE_BUILD_PLATFORM_APPLE_IOS )
	if( OGRE_BUILD_SAMPLES2 AND NOT SDL2_FOUND )
		message( STATUS "Could not find dependency for samples: SDL2" )
		set( OGRE_BUILD_SAMPLES2_SKIP 1 )
	else()
		include_directories(${SDL2_INCLUDE_DIR})
	endif()
endif()

if( NOT OGRE_BUILD_COMPONENT_OVERLAY )
	message( STATUS "Samples require Overlays (OGRE_BUILD_COMPONENT_OVERLAY) to be set." )
	set( OGRE_BUILD_SAMPLES2_SKIP 1 )
endif()

if( OGRE_BUILD_SAMPLES2_SKIP )
	message(STATUS "Skipping samples & tests build")
endif()

if( APPLE )
        if( OGRE_BUILD_PLATFORM_APPLE_IOS )
            # This needs to be first in the list in order show up in the Copy Bundle Resources build phase.
            set( SAMPLE_COMMON_STORYBOARD_RESOURCE ${CMAKE_CURRENT_SOURCE_DIR}/Common/src/System/iOS/Resources/Main.storyboard )
        endif()

	set( SAMPLE_COMMON_BASIC_RESOURCES ${SAMPLE_COMMON_BASIC_RESOURCES}
                 ${SAMPLE_COMMON_STORYBOARD_RESOURCE}
		 ${CMAKE_BINARY_DIR}/bin/resources2.cfg
                 ${CMAKE_BINARY_DIR}/bin/plugins.cfg
		 ${CMAKE_SOURCE_DIR}/Samples/Media/2.0
		 ${CMAKE_SOURCE_DIR}/Samples/Media/Hlms
		 ${CMAKE_SOURCE_DIR}/Samples/Media/packs/DebugPack.zip )

	set( SAMPLE_COMMON_RESOURCES ${SAMPLE_COMMON_BASIC_RESOURCES}
		 ${CMAKE_SOURCE_DIR}/Samples/Media/models
		 ${CMAKE_SOURCE_DIR}/Samples/Media/materials/textures )

        if( OGRE_BUILD_PLATFORM_APPLE_IOS )
            set( SAMPLES_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Common/src/System/iOS/Resources/Info.plist )
        else()
            set( SAMPLES_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Common/src/System/OSX/Resources/Info.plist )
        endif()
endif()

if( OGRE_BUILD_SAMPLES2 AND NOT OGRE_BUILD_SAMPLES2_SKIP )
    add_subdirectory(Common)
endif()

if( OGRE_BUILD_SAMPLES2 AND NOT OGRE_BUILD_SAMPLES2_SKIP )
	set( OGRE_SAMPLES_LIBRARIES "OgreSamplesCommon" "OgreOverlay" )

	if( OGRE_STATIC )
		if( OGRE_BUILD_RENDERSYSTEM_GL3PLUS )
			set( OGRE_SAMPLES_LIBRARIES ${OGRE_SAMPLES_LIBRARIES} "RenderSystem_GL3Plus" )
		endif()
		if( OGRE_BUILD_RENDERSYSTEM_D3D11 )
			set( OGRE_SAMPLES_LIBRARIES ${OGRE_SAMPLES_LIBRARIES} "RenderSystem_D3D11" )
		endif()
		if( OGRE_BUILD_RENDERSYSTEM_METAL )
			set( OGRE_SAMPLES_LIBRARIES "RenderSystem_Metal" ${OGRE_SAMPLES_LIBRARIES} )
		endif()
	endif()

	add_subdirectory(ApiUsage/AnimationTagPoint)
	add_subdirectory(ApiUsage/AreaApproxLights)
	add_subdirectory(ApiUsage/ImportAnimationsShareSkeletonInstance)
	add_subdirectory(ApiUsage/CustomRenderable)
	add_subdirectory(ApiUsage/Decals)
	add_subdirectory(ApiUsage/DynamicGeometry)
	add_subdirectory(ApiUsage/InstantRadiosity)
	add_subdirectory(ApiUsage/LocalCubemaps)
	add_subdirectory(ApiUsage/LocalCubemapsManualProbes)
	add_subdirectory(ApiUsage/V1Interfaces)
	add_subdirectory(ApiUsage/V2Mesh)
	add_subdirectory(ApiUsage/V2ManualObject)
	if( OGRE_BUILD_COMPONENT_PLANAR_REFLECTIONS )
		add_subdirectory(ApiUsage/PlanarReflections)
	else()
		message(STATUS "Skipping PlanarReflections sample (OGRE_BUILD_COMPONENT_PLANAR_REFLECTIONS not set)")
	endif()
	if( OGRE_BUILD_COMPONENT_SCENE_FORMAT )
		add_subdirectory(ApiUsage/SceneFormat)
	else()
		message(STATUS "Skipping SceneFormat sample (OGRE_BUILD_COMPONENT_SCENE_FORMAT not set)")
	endif()
	add_subdirectory(ApiUsage/ScreenSpaceReflections)
	add_subdirectory(ApiUsage/StaticShadowMaps)
	add_subdirectory(ApiUsage/StencilTest)
	add_subdirectory(ApiUsage/StereoRendering)
	add_subdirectory(ApiUsage/ShadowMapDebugging)
	add_subdirectory(ApiUsage/ShadowMapFromCode)

	add_subdirectory(Showcase/Forward3D)
	add_subdirectory(Showcase/Hdr)
	add_subdirectory(Showcase/HdrSmaa)
	add_subdirectory(Showcase/PbsMaterials)
	add_subdirectory(Showcase/Postprocessing)

	add_subdirectory(Tutorials/Tutorial01_Initialization)
	add_subdirectory(Tutorials/Tutorial02_VariableFramerate)
	add_subdirectory(Tutorials/Tutorial03_DeterministicLoop)
	if( NOT APPLE )
		# Not ported yet to Metal
		add_subdirectory(Tutorials/Tutorial04_InterpolationLoop)
		add_subdirectory(Tutorials/Tutorial05_MultithreadingBasics)
		add_subdirectory(Tutorials/Tutorial06_Multithreading)
	endif()
	add_subdirectory(Tutorials/TutorialCompute01_UavTexture)
	add_subdirectory(Tutorials/TutorialCompute02_UavBuffer)
	add_subdirectory(Tutorials/Tutorial_Distortion)
	add_subdirectory(Tutorials/Tutorial_DynamicCubemap)
	add_subdirectory(Tutorials/Tutorial_ReconstructPosFromDepth)
	add_subdirectory(Tutorials/TutorialSky_Postprocess)
	add_subdirectory(Tutorials/Tutorial_SSAO)
	add_subdirectory(Tutorials/Tutorial_SMAA)
	add_subdirectory(Tutorials/Tutorial_Terrain)
	add_subdirectory(Tutorials/TutorialUav01_Setup)
	add_subdirectory(Tutorials/TutorialUav02_Setup)
endif()

if( OGRE_BUILD_TESTS )
	add_subdirectory(Tests/Restart)
endif()
