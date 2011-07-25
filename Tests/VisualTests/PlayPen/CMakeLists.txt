#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

set(HEADER_FILES
    include/PlayPenTestPlugin.h
    include/PlayPenTests.h)

set(SOURCE_FILES
	src/PlayPenAttachObjectsToBones.cpp
	src/PlayPenMultiSceneManagersSimple.cpp
    src/PlayPen16Textures.cpp
    src/PlayPen2Spotlights.cpp
    src/PlayPenAlphaToCoverage.cpp
    src/PlayPenBasicPlane.cpp
    src/PlayPenBillboardAccurateFacing.cpp
    src/PlayPenBillboardChain.cpp
    src/PlayPenBillboardOrigins.cpp
    src/PlayPenBillboardTextureCoords.cpp
    src/PlayPenBlendDiffuseColour.cpp
    src/PlayPenBlitSubTextures.cpp
    src/PlayPenBuildTangentOnAnimatedMesh.cpp
    src/PlayPenCubeDDS.cpp
    src/PlayPenCustomProjectionMatrix.cpp
    src/PlayPenDepthBias.cpp
    src/PlayPenDistortion.cpp
    src/PlayPenDxt1.cpp
    src/PlayPenDxt1Alpha.cpp
    src/PlayPenDxt1FromMemory.cpp
    src/PlayPenDxt3.cpp
    src/PlayPenDxt3FromMemory.cpp
    src/PlayPenDxt5.cpp
    src/PlayPenFarFromOrigin.cpp
    src/PlayPenImageCombine.cpp
    src/PlayPenLightClipPlanes.cpp
    src/PlayPenLightClipPlanesMoreLights.cpp
    src/PlayPenLightScissoring.cpp
    src/PlayPenLotsAndLotsOfEntities.cpp
    src/PlayPenManualObjectIndexed.cpp
    src/PlayPenManualObjectIndexedUpdateLarger.cpp
    src/PlayPenManualObjectIndexedUpdateSmaller.cpp
    src/PlayPenManualObjectNonIndexed.cpp
    src/PlayPenManualObjectNonIndexedUpdateLarger.cpp
    src/PlayPenManualObjectNonIndexedUpdateSmaller.cpp
    src/PlayPenMaterialSchemes.cpp
    src/PlayPenMultiViewports.cpp
    src/PlayPenNegativeScale.cpp
    src/PlayPenOrtho.cpp
    src/PlayPenPointSprites.cpp
    src/PlayPenReflectedBillboards.cpp
    src/PlayPenRibbonTrail.cpp
    src/PlayPenSRGBtexture.cpp
    src/PlayPenStaticGeometry.cpp
    src/PlayPenStaticGeometryWithLOD.cpp
    src/PlayPenStencilGlow.cpp
    src/PlayPenStencilShadows.cpp
    src/PlayPenTests.cpp
    src/PlayPenTransparencyMipMaps.cpp
    src/PlayPenWindowedViewportMode.cpp
    src/PlayPenBsp.cpp
    src/PlayPenSkeletalAnimation.cpp
    src/PlayPenSkeletonAnimationOptimise.cpp
    src/PlayPenManualObject2D.cpp
    src/PlayPenSubEntityVisibility.cpp
    src/PlayPenTextureShadows.cpp

    #src/PlayPenMultiSceneManagersComplex.cpp

    #src/PlayPenGeneratedLOD.cpp
	#src/PlayPenAlpha.cpp
    #src/PlayPen2Windows.cpp
    #src/PlayPen2bppAlphaPVR.cpp
    #src/PlayPen2bppPVR.cpp
    #src/PlayPen4bppAlphaPVR.cpp
    #src/PlayPen4bppPVR.cpp
    #src/PlayPenAnimation.cpp
    #src/PlayPenAnimationBlend.cpp
    #src/PlayPenBackgroundLoadResourceGroup.cpp
    #src/PlayPenBug.cpp
    #src/PlayPenClearScene.cpp
    #src/PlayPenCompositorTechniqueSwitch.cpp
    #src/PlayPenCompositorTextureShadows.cpp
    #src/PlayPenCthNewBlending.cpp
    #src/PlayPenDepthShadowMap.cpp
    #src/PlayPenEdgeBuilderMultiIndexBufMultiVertexBuf.cpp
    #src/PlayPenEdgeBuilderMultiIndexBufSingleVertexBuf.cpp
    #src/PlayPenEdgeBuilderSingleIndexBufSingleVertexBuf.cpp
    #src/PlayPenExportPrecompiledAssemblerProgram.cpp
    #src/PlayPenFallbackResourceGroup.cpp
    #src/PlayPenFloat128DDS.cpp
    #src/PlayPenFloat16DDS.cpp
    #src/PlayPenFloat32DDS.cpp
    #src/PlayPenFloat64DDS.cpp
    #src/PlayPenGLSLTangent.cpp
    #src/PlayPenGeometryShaders.cpp
    #src/PlayPenGpuPrograms.cpp
    #src/PlayPenInfiniteAAB.cpp
    #src/PlayPenIntersectionSceneQuery.cpp
    #src/PlayPenLiSPSM.cpp
    #src/PlayPenLod.cpp
    #src/PlayPenMRT.cpp
    #src/PlayPenMRTCompositorScript.cpp
    #src/PlayPenManualBoneMovement.cpp
    #src/PlayPenManualIlluminationStage.cpp
    #src/PlayPenManualLoader.cpp
    #src/PlayPenMaterialSchemesListener.cpp
    #src/PlayPenMaterialSchemesWithLOD.cpp
    #src/PlayPenMaterialSchemesWithMismatchedLOD.cpp
    #src/PlayPenMaterialSerializer.cpp
    #src/PlayPenMatrices.cpp
    #src/PlayPenNewTerrain.cpp
    #src/PlayPenNormalMapMirroredUVs.cpp
    #src/PlayPenOverlayRelativeMode.cpp
    #src/PlayPenOverlayZOrder.cpp
    #src/PlayPenPoseAnimation2.cpp
    #src/PlayPenPrepareShadowVolume.cpp
    #src/PlayPenProjection.cpp
    #src/PlayPenRadixSort.cpp  Not Visual!
    #src/PlayPenRaySceneQuery.cpp
    #src/PlayPenReinitialiseEntityAlteredMesh.cpp
    #src/PlayPenReloadResources.cpp
    #src/PlayPenSerialisedColour.cpp
    #src/PlayPenShadowLod.cpp
    #src/PlayPenSharedGpuParameters.cpp
    #src/PlayPenSharedPtrBug.cpp
    #src/PlayPenSimpleMesh.cpp
    #src/PlayPenSplitPassesTooManyTexUnits.cpp
    #src/PlayPenSpotlightViewProj.cpp
    #src/PlayPenStencilShadowsMixedOpSubMeshes.cpp
    #src/PlayPenStringTokenising.cpp
    #src/PlayPenSuppressedShadows.cpp
    src/PlayPenTextureShadowsCustomCasterMat.cpp
    src/PlayPenTextureShadowsCustomReceiverMat.cpp
    src/PlayPenTextureShadowsIntegrated.cpp
    src/PlayPenTextureShadowsIntegratedPSSM.cpp
    #src/PlayPenTextureShadowsTransparentCaster.cpp
    #src/PlayPenTimeCreateDestroyObject.cpp
    #src/PlayPenTwoNewTerrains.cpp
    #src/PlayPenVertexTexture.cpp
    #src/PlayPenViewportNoShadows.cpp
	#src/PlayPenMaterial.cpp
    src/PlayPenTestPlugin.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${OGRE_SOURCE_DIR}/Samples/Common/include)
include_directories(${OGRE_SOURCE_DIR}/Samples/Browser/include)
include_directories(${OGRE_SOURCE_DIR}/Tests/VisualTests/Common/include)

ogre_add_component_include_dir(Terrain)
ogre_add_component_include_dir(Paging)
ogre_add_component_include_dir(RTShaderSystem)

add_library(PlayPenTests ${OGRE_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES} )
target_link_libraries(PlayPenTests ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
ogre_config_sample_lib(PlayPenTests)

if (APPLE AND NOT OGRE_BUILD_PLATFORM_APPLE_IOS)
    # Set the INSTALL_PATH so that Samples can be installed in the application package
    set_target_properties(PlayPenTests
       PROPERTIES BUILD_WITH_INSTALL_RPATH 1
       INSTALL_NAME_DIR "@executable_path/../Plugins"
    )
endif()
