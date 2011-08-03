#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure Visual Test build

# Setup CTest
#ENABLE_TESTING()

# add Context directory
add_subdirectory(Context)

# add the PlayPen test plugin's directory
add_subdirectory(PlayPen)

# add VTests plugin directory
add_subdirectory(VTests)

# now add our tests

# Since CTest expects one command per test, but the Visual Testing setup runs all at once
# (since unloading and reloading everything for each test wouldn't be practical) we simply
# check an output file for passage of each test using CTest's regex checking

# What we'll use to output the file (cat in Unix, type in Windows)
set(TESTING_READ_FILE cat)

# define a macro for these tests:
macro (VisualTest test_name render_system)
	add_test("${test_name}_${render_system}" ${TESTING_READ_FILE} 
		"${CMAKE_BINARY_DIR}/Tests/TestResults_${render_system}.txt")
	set_tests_properties("${test_name}_${render_system}" 
		PROPERTIES FAIL_REGULAR_EXPRESSION 
		"${test_name}=Failed")
endmacro (VisualTest)

# and the list of tests itself:
VisualTest("PlayPen_16Textures" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_2Spotlights" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_AlphaToCoverage" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_AttachObjectsToBones" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_BasicPlane" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_BillboardAccurateFacing" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_BillboardChain" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_BillboardOrigins" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_BillboardTextureCoords" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_BlendDiffuseColour" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_BlitSubTextures" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_Bsp" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_BuildTangentOnAnimatedMesh" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_CameraSetDirection" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_ClearScene" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_CompositorTechniqueSwitch" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_CompositorTextureShadows" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_CubeDDS" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_CustomProjectionMatrix" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_DepthBias" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_DepthShadowMap" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_Distortion" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_Dxt1" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_Dxt1Alpha" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_Dxt1FromMemory" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_Dxt3" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_Dxt3FromMemory" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_Dxt5" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_FarFromOrigin" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_GeometryShaders" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_ImageCombine" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_InfiniteAAB" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_IntersectionSceneQuery" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_LiSPSM" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_LightClipPlanes" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_LightClipPlanesMoreLights" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_LightScissoring" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_LotsAndLotsOfEntities" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_ManualBlending" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_ManualBoneMovement" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_ManualIlluminationStage" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_ManualLOD" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_ManualLOD_File" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_ManualObject2D" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_ManualObjectIndexed" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_ManualObjectIndexedUpdateLarger" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_ManualObjectIndexedUpdateSmaller" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_ManualObjectNonIndexed" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_ManualObjectNonIndexedUpdateLarger" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_ManualObjectNonIndexedUpdateSmaller" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_MaterialSchemes" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_MaterialSchemesListener" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_MaterialSchemesWithLOD" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_MaterialSchemesWithMismatchedLOD" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_MorphAnimNormals" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_MorphAnimNoNormals" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_MultiSceneManagersSimple" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_MultiViewports" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_NegativeScale" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_NormalMapMirroredUVs" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_Ortho" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_PointSprites" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_PoseAnimNormals" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_PoseAnimNoNormals" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_ProjectSphere" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_Projection" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_RaySceneQuery" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_ReflectedBillboards" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_ReinitialiseEntityAlteredMesh" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_ReloadResources" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_RibbonTrail" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_SRGBtexture" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_SceneNodeTracking" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_SerialisedColour" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_ShadowLod" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_SkeletalAnimation" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_SkeletonAnimationOptimise" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_SpotlightViewProj" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_StaticGeometry" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_StaticGeometryWithLOD" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_StencilGlow" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_StencilShadows" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_StencilShadowsMixedOpSubMeshes" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_SubEntityVisibility" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_SuppressedShadows" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_TextureShadows" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_TextureShadowsCustomCasterMat" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_TextureShadowsCustomReceiverMat" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_TextureShadowsIntegrated" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_TextureShadowsIntegratedPSSM" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_TextureShadowsTransparentCaster" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_TransparencyMipMaps" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_VertexTexture" "OpenGLRenderingSubsystem")
VisualTest("PlayPen_ViewportNoShadows" "OpenGLRenderingSubsystem")

VisualTest("VTests_StencilShadows" "OpenGLRenderingSubsystem")
VisualTest("VTests_Transparency" "OpenGLRenderingSubsystem")
VisualTest("VTests_TextureEffects" "OpenGLRenderingSubsystem")
VisualTest("VTests_Particles" "OpenGLRenderingSubsystem")
VisualTest("VTests_CubeMapping" "OpenGLRenderingSubsystem")
