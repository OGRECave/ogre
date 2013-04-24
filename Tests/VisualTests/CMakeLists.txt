#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure Visual Test build

# add Context directory
add_subdirectory(Context)

# add the PlayPen test plugin's directory
add_subdirectory(PlayPen)

# add VTests plugin directory
add_subdirectory(VTests)

# setup cmake scripts
configure_file(RunVTests.cmake ${CMAKE_BINARY_DIR}/Tests/VisualTests/RunVTests.cmake COPYONLY)
configure_file(CheckResult.cmake ${CMAKE_BINARY_DIR}/Tests/VisualTests/CheckResult.cmake COPYONLY)
configure_file(PostTest.cmake ${CMAKE_BINARY_DIR}/Tests/VisualTests/PostTest.cmake COPYONLY)

# CTest configuration (this seems to need to be placed in the root of the build dir to be found)
configure_file(CTestCustom.cmake ${CMAKE_BINARY_DIR}/CTestCustom.cmake COPYONLY)

# build a list of render systems to test
set(TEST_RENDER_SYSTEMS "")

if (OGRE_BUILD_RENDERSYSTEM_GL)
  list(APPEND TEST_RENDER_SYSTEMS "OpenGL Rendering Subsystem")
endif ()
if (OGRE_BUILD_RENDERSYSTEM_GLPLUS)
  list(APPEND TEST_RENDER_SYSTEMS "OpenGL 3+ Rendering Subsystem")
endif ()
if (OGRE_BUILD_RENDERSYSTEM_D3D9)
  list(APPEND TEST_RENDER_SYSTEMS "Direct3D9 Rendering Subsystem")
endif ()
if (OGRE_BUILD_RENDERSYSTEM_D3D11)
  list(APPEND TEST_RENDER_SYSTEMS "Direct3D11 Rendering Subsystem")
endif ()
if (OGRE_BUILD_RENDERSYSTEM_GLES)
  list(APPEND TEST_RENDER_SYSTEMS "OpenGL ES 1.x Rendering Subsystem")
endif ()
if (OGRE_BUILD_RENDERSYSTEM_GLES2)
  list(APPEND TEST_RENDER_SYSTEMS "OpenGL ES 2.x Rendering Subsystem")
endif ()

# Figure out the home directory for this system (this is where the context outputs result data for now)
if (UNIX)
  set(USER_HOME_DIRECTORY $ENV{HOME})
elseif (WIN32)
  string(REPLACE "\\" "/" USER_HOME_DIRECTORY "$ENV{HOMEDRIVE}$ENV{HOMEPATH}")
# other platforms?
endif ()

# define a list of the visual tests
set (VISUAL_TEST_LIST
  PlayPen_16Textures
  PlayPen_2Spotlights
  PlayPen_AlphaToCoverage
  PlayPen_AttachObjectsToBones
  PlayPen_BasicPlane
  PlayPen_BillboardAccurateFacing
  PlayPen_BillboardChain
  PlayPen_BillboardOrigins
  PlayPen_BillboardTextureCoords
  PlayPen_BlendDiffuseColour
  PlayPen_BlitSubTextures
  PlayPen_Bsp
  PlayPen_BuildTangentOnAnimatedMesh
  PlayPen_CameraSetDirection
  PlayPen_ClearScene
  PlayPen_CompositorTechniqueSwitch
  PlayPen_CompositorTextureShadows
  PlayPen_CubeDDS
  PlayPen_CustomProjectionMatrix
  PlayPen_DepthBias
  PlayPen_DepthShadowMap
  PlayPen_Distortion
  PlayPen_Dxt1
  PlayPen_Dxt1Alpha
  PlayPen_Dxt1FromMemory
  PlayPen_Dxt3
  PlayPen_Dxt3FromMemory
  PlayPen_Dxt5
  PlayPen_FarFromOrigin
  PlayPen_GeometryShaders
  PlayPen_ImageCombine
  PlayPen_InfiniteAAB
  PlayPen_IntersectionSceneQuery
  PlayPen_LiSPSM
  PlayPen_LightClipPlanes
  PlayPen_LightClipPlanesMoreLights
  PlayPen_LightScissoring
  PlayPen_LotsAndLotsOfEntities
  PlayPen_ManualBlending
  PlayPen_ManualBoneMovement
  PlayPen_ManualIlluminationStage
  PlayPen_ManualLOD
  PlayPen_ManualLOD_File
  PlayPen_ManualObject2D
  PlayPen_ManualObjectIndexed
  PlayPen_ManualObjectIndexedUpdateLarger
  PlayPen_ManualObjectIndexedUpdateSmaller
  PlayPen_ManualObjectNonIndexed
  PlayPen_ManualObjectNonIndexedUpdateLarger
  PlayPen_ManualObjectNonIndexedUpdateSmaller
  PlayPen_MaterialSchemes
  PlayPen_MaterialSchemesListener
  PlayPen_MaterialSchemesWithLOD
  PlayPen_MaterialSchemesWithMismatchedLOD
  PlayPen_MorphAnimNormals
  PlayPen_MorphAnimNoNormals
  PlayPen_MultiSceneManagersSimple
  PlayPen_MultiViewports
  PlayPen_NegativeScale
  PlayPen_NonUniqueResourceNames
  PlayPen_NormalMapMirroredUVs
  PlayPen_Ortho
  PlayPen_PointSprites
  PlayPen_PoseAnimNormals
  PlayPen_PoseAnimNoNormals
  PlayPen_ProjectSphere
  PlayPen_Projection
  PlayPen_RaySceneQuery
  PlayPen_ReflectedBillboards
  PlayPen_ReinitialiseEntityAlteredMesh
  PlayPen_ReloadResources
  PlayPen_RibbonTrail
  PlayPen_SRGBtexture
  PlayPen_SceneNodeTracking
  PlayPen_SerialisedColour
  PlayPen_ShadowLod
  PlayPen_SkeletalAnimation
  PlayPen_SkeletonAnimationOptimise
  PlayPen_SpotlightViewProj
  PlayPen_StaticGeometry
  PlayPen_StaticGeometryWithLOD
  PlayPen_StencilGlow
  PlayPen_StencilShadows
  PlayPen_StencilShadowsMixedOpSubMeshes
  PlayPen_SubEntityVisibility
  PlayPen_SuppressedShadows
  PlayPen_TextureShadows
  PlayPen_TextureShadowsCustomCasterMat
  PlayPen_TextureShadowsCustomReceiverMat
  PlayPen_TextureShadowsIntegrated
  PlayPen_TextureShadowsIntegratedPSSM
  PlayPen_TextureShadowsTransparentCaster
  PlayPen_TransparencyMipMaps
  PlayPen_VertexTexture
  PlayPen_ViewportNoShadows
  VTests_StencilShadows
  VTests_Transparency
  VTests_TextureEffects
  VTests_Particles
  VTests_CubeMapping)

# Since CTest expects one command per test, but the Visual Testing setup runs all at once
# (since unloading and reloading everything for each test wouldn't be practical) we
# run the context first, outputting a file of results, then the tests themselves just
# cat (or 'type' in windows) the file out and CTest's regex checking does the rest

# define a macro for tests
macro (VisualTest test_name render_system)

  if (UNIX)

    # In Unix we can just cat the file
    add_test("${test_name}_${render_system}" cat ${USER_HOME_DIRECTORY}/TestResults_${render_system}.txt)

  else (UNIX)

    # In Windows we'll have to call a separate script (the cat equivalent is 'type', but
    # this seems to require a 'cmd /c type [file]', which doesn't seem to get called
    # properly with add_test, so CheckResult.cmake just does this via exec_program)
    add_test("${test_name}_${render_system}" cmake
      "-D" "TEST_RESULT_FILE=${USER_HOME_DIRECTORY}/TestResults_${render_system}.txt" 
      "-P" "CheckResult.cmake")

  endif (UNIX)

  # look for failure message (either [test]=Failed, or a file not found message from cat/type)
  set_tests_properties("${test_name}_${render_system}" 
    PROPERTIES FAIL_REGULAR_EXPRESSION 
    "${test_name}=Failed;cannot find the file;No such file")

  # this test must be run after the appropriate test context has run
  set_tests_properties("${test_name}_${render_system}" 
    PROPERTIES DEPENDS
    "TestContext_${render_system}")

  # an extra measure to be sure the context runs first
  set_tests_properties("${test_name}_${render_system}" 
    PROPERTIES COST 1)

endmacro (VisualTest)

# function for running visual tests for a given render system
function (TestRenderSystem render_system)

  # strip spaces from rendersystem name
  string (REPLACE " " "" render_system_nospace ${render_system})

  # Run a CMake script that handles the test context
  add_test(NAME "TestContext_${render_system_nospace}"
    COMMAND "cmake" 
    "-D" # executable name
      "TEST_CONTEXT_EXECUTABLE=$<TARGET_FILE_NAME:TestContext>"
    "-D" # path to executable (this one's tricker than it seems, since it'll vary based on build config)
      "TEST_CONTEXT_PATH=$<TARGET_FILE_DIR:TestContext>"
    "-D" # the render system to use
      "TEST_CONTEXT_RENDER_SYSTEM=${render_system}"
    "-D" # the directory where the output file will go
      "TEST_CONTEXT_RESULT_DIR=${USER_HOME_DIRECTORY}"
    "-P" # the script that carries this out
      "RunVTests.cmake")

  # so we're extra sure this gets run first
  set_tests_properties("TestContext_${render_system_nospace}" 
    PROPERTIES COST 1000)

  # and add_test for each test
  foreach (t ${VISUAL_TEST_LIST})
    VisualTest(${t} ${render_system_nospace})
  endforeach ()

endfunction (TestRenderSystem)

# Run the tests once for each rendersystem
foreach (rs ${TEST_RENDER_SYSTEMS})
  TestRenderSystem(${rs})
endforeach ()

