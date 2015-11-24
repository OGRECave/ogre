# Configure Samples build

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Common/include)
include_directories(${CMAKE_SOURCE_DIR}/Components/Overlay/include)

if (OGRE_BUILD_SAMPLES2 AND NOT OIS_FOUND)
    message(STATUS "Could not find dependency: SDL2")
    message(STATUS "Skipping samples build")
    set(OGRE_BUILD_SAMPLES2 0)
else()
    include_directories(${SDL2_INCLUDE_DIR})
endif()

if( OGRE_BUILD_SAMPLES2 )
    add_subdirectory(Common)

	set( OGRE_SAMPLES_LIBRARIES "OgreSamplesCommon" "OgreOverlay" )

	add_subdirectory(ApiUsage/CustomRenderable)
	add_subdirectory(ApiUsage/DynamicGeometry)
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
	add_subdirectory(Tutorials/Tutorial_ReconstructPosFromDepth)
	add_subdirectory(Tutorials/TutorialSky_Postprocess)
	add_subdirectory(Tutorials/TutorialUav01_Setup)
	add_subdirectory(Tutorials/TutorialUav02_Setup)
endif()
