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
    src/PlayPenTests.cpp
    src/PlayPenStencilGlow.cpp
    #src/PlayPenManualLoader.cpp
    #src/PlayPenMRTCompositorScript.cpp
    #src/PlayPenNormalMapMirroredUVs.cpp
    #src/PlayPenMaterialSerializer.cpp
    #src/PlayPenTransparencyMipMaps.cpp
    #src/PlayPenCthNewBlending.cpp
    #src/PlayPenMatrices.cpp
    #src/PlayPenBasicPlane.cpp
	##		src/PlayPenAlpha.cpp
    ##		src/PlayPenBsp.cpp
    #src/PlayPenAnimation.cpp
    #src/PlayPenAnimationBlend.cpp
    #src/PlayPenGpuPrograms.cpp
    #src/PlayPenProjection.cpp
    #src/PlayPenMultiViewports.cpp
    #src/PlayPenDistortion.cpp
    #src/PlayPenEdgeBuilderSingleIndexBufSingleVertexBuf.cpp
    #src/PlayPenEdgeBuilderMultiIndexBufSingleVertexBuf.cpp
    #src/PlayPenEdgeBuilderMultiIndexBufMultiVertexBuf.cpp
    #src/PlayPenSkeletalAnimation.cpp
    #src/PlayPenPrepareShadowVolume.cpp
    #src/PlayPenWindowedViewportMode.cpp
    #src/PlayPenSubEntityVisibility.cpp
    #src/PlayPenAttachObjectsToBones.cpp
    #src/PlayPenOrtho.cpp
    #src/PlayPenFallbackResourceGroup.cpp
    #src/PlayPenGeneratedLOD.cpp
    #src/PlayPenClearScene.cpp
    src/PlayPenStencilShadows.cpp
    #src/PlayPenStencilShadowsMixedOpSubMeshes.cpp
    #src/PlayPen2Spotlights.cpp
    #src/PlayPenTextureShadows.cpp
    #src/PlayPenTextureShadowsCustomCasterMat.cpp
    #src/PlayPenTextureShadowsCustomReceiverMat.cpp
    #src/PlayPenCompositorTextureShadows.cpp
    #src/PlayPenCompositorTechniqueSwitch.cpp
    #src/PlayPenOverlayZOrder.cpp
    #src/PlayPenIntersectionSceneQuery.cpp
    #src/PlayPenRaySceneQuery.cpp
    #src/PlayPenLotsAndLotsOfEntities.cpp
    #src/PlayPenSimpleMesh.cpp
    #src/PlayPenGLSLTangent.cpp
    #src/PlayPenInfiniteAAB.cpp
    #src/PlayPen2Windows.cpp
    src/PlayPenStaticGeometry.cpp
    src/PlayPenStaticGeometryWithLOD.cpp
    #src/PlayPenReloadResources.cpp
    #src/PlayPenBillboardTextureCoords.cpp
    #src/PlayPenRadixSort.cpp  Not Visual!
    #src/PlayPenPoseAnimation2.cpp
    src/PlayPenReflectedBillboards.cpp
    src/PlayPenManualObjectNonIndexed.cpp
    src/PlayPenManualObjectNonIndexedUpdateSmaller.cpp
    src/PlayPenManualObjectNonIndexedUpdateLarger.cpp
    src/PlayPenManualObjectIndexed.cpp
    src/PlayPenManualObjectIndexedUpdateSmaller.cpp
    src/PlayPenManualObjectIndexedUpdateLarger.cpp
    src/PlayPenBillboardChain.cpp
    #src/PlayPenCubeDDS.cpp
    #src/PlayPenDxt1.cpp
    #src/PlayPenDxt1FromMemory.cpp
    #src/PlayPenDxt1Alpha.cpp
    #src/PlayPenDxt3.cpp
    #src/PlayPenDxt3FromMemory.cpp
    #src/PlayPenDxt5.cpp
    #src/PlayPenFloat64DDS.cpp
    #src/PlayPenFloat128DDS.cpp
    #src/PlayPenFloat16DDS.cpp
    #src/PlayPenFloat32DDS.cpp
    #src/PlayPen4bppPVR.cpp
    #src/PlayPen4bppAlphaPVR.cpp
    #src/PlayPen2bppPVR.cpp
    #src/PlayPen2bppAlphaPVR.cpp
    #src/PlayPenRibbonTrail.cpp
    #src/PlayPenExportPrecompiledAssemblerProgram.cpp
#src/PlayPenMaterial.cpp
    src/PlayPenBlendDiffuseColour.cpp
    #src/PlayPenSplitPassesTooManyTexUnits.cpp
    #src/PlayPenCustomProjectionMatrix.cpp
    src/PlayPenPointSprites.cpp
    #src/PlayPenSuppressedShadows.cpp
    #src/PlayPenViewportNoShadows.cpp
    #src/PlayPenSerialisedColour.cpp
    src/PlayPenBillboardAccurateFacing.cpp
	src/PlayPenMultiSceneManagersSimple.cpp
    #src/PlayPenMultiSceneManagersComplex.cpp
    #src/PlayPenManualBoneMovement.cpp
    src/PlayPenMaterialSchemes.cpp
    #src/PlayPenMaterialSchemesListener.cpp
    #src/PlayPenMaterialSchemesWithLOD.cpp
    #src/PlayPenMaterialSchemesWithMismatchedLOD.cpp
    #src/PlayPenSkeletonAnimationOptimise.cpp
    #src/PlayPenBuildTangentOnAnimatedMesh.cpp
    #src/PlayPenVertexTexture.cpp
    #src/PlayPenBackgroundLoadResourceGroup.cpp
    #src/PlayPenOverlayRelativeMode.cpp
    src/PlayPenBillboardOrigins.cpp
    #src/PlayPenDepthBias.cpp
    #src/PlayPenStringTokenising.cpp
    #src/PlayPenTextureShadowsTransparentCaster.cpp
    #src/PlayPenTextureShadowsIntegrated.cpp
    #src/PlayPenTextureShadowsIntegratedPSSM.cpp
    #src/PlayPenTimeCreateDestroyObject.cpp
    #src/PlayPenLightScissoring.cpp
    #src/PlayPenLightClipPlanes.cpp
    #src/PlayPenLightClipPlanesMoreLights.cpp
    #src/PlayPenMRT.cpp
    #src/PlayPen16Textures.cpp
    #src/PlayPenSharedPtrBug.cpp
    #src/PlayPenSpotlightViewProj.cpp
    #src/PlayPenManualIlluminationStage.cpp
    #src/PlayPenReinitialiseEntityAlteredMesh.cpp
    src/PlayPenSRGBtexture.cpp
    src/PlayPenNegativeScale.cpp
    #src/PlayPenLod.cpp
    #src/PlayPenBug.cpp
    #src/PlayPenManualObject2D.cpp
    #src/PlayPenShadowLod.cpp
    #src/PlayPenFarFromOrigin.cpp
    #src/PlayPenGeometryShaders.cpp
    #src/PlayPenAlphaToCoverage.cpp
    #src/PlayPenLiSPSM.cpp
    #src/PlayPenBlitSubTextures.cpp
    #src/PlayPenDepthShadowMap.cpp
    #src/PlayPenSharedGpuParameters.cpp
    #src/PlayPenImageCombine.cpp
    #src/PlayPenNewTerrain.cpp
    #src/PlayPenTwoNewTerrains.cpp
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
