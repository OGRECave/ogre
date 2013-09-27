#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

############################################################
# OgreMain core library
############################################################

PROJECT(OgreMain)

include(PrecompiledHeader)

# define header and source files for the library
set (HEADER_FILES
  include/asm_math.h
  include/Hash/MurmurHash3.h
  include/Ogre.h
  include/OgreAlignedAllocator.h
  include/OgreAnimable.h
  include/OgreAnimation.h
  include/OgreAnimationState.h
  include/OgreAnimationTrack.h
  include/OgreAny.h
  include/OgreArchive.h
  include/OgreArchiveFactory.h
  include/OgreArchiveManager.h
  include/Compositor/Pass/OgreCompositorPass.h
  include/Compositor/Pass/OgreCompositorPassDef.h
  include/Compositor/Pass/PassClear/OgreCompositorPassClear.h
  include/Compositor/Pass/PassClear/OgreCompositorPassClearDef.h
  include/Compositor/Pass/PassQuad/OgreCompositorPassQuad.h
  include/Compositor/Pass/PassQuad/OgreCompositorPassQuadDef.h
  include/Compositor/Pass/PassScene/OgreCompositorPassScene.h
  include/Compositor/Pass/PassScene/OgreCompositorPassSceneDef.h
  include/Compositor/OgreCompositorChannel.h
  include/Compositor/OgreCompositorCommon.h
  include/Compositor/OgreCompositorManager2.h
  include/Compositor/OgreCompositorNode.h
  include/Compositor/OgreCompositorNodeDef.h
  include/Compositor/OgreCompositorShadowNode.h
  include/Compositor/OgreCompositorShadowNodeDef.h
  include/Compositor/OgreCompositorWorkspace.h
  include/Compositor/OgreCompositorWorkspaceDef.h
  include/Compositor/OgreTextureDefinition.h
  include/Math/Array/OgreArrayAabb.h
  include/Math/Array/OgreArrayConfig.h
  include/Math/Array/OgreArrayMatrix4.h
  include/Math/Array/OgreArrayMemoryManager.h
  include/Math/Array/OgreArraySphere.h
  include/Math/Array/OgreArrayQuaternion.h
  include/Math/Array/OgreArrayVector3.h
  include/Math/Array/OgreBooleanMask.h
  include/Math/Array/OgreMathlib.h
  include/Math/Array/OgreNodeMemoryManager.h
  include/Math/Array/OgreObjectData.h
  include/Math/Array/OgreObjectMemoryManager.h
  include/Math/Array/OgreTransform.h
  include/Math/Simple/OgreAabb.h
  include/OgreAtomicWrappers.h
  include/OgreAutoParamDataSource.h
  include/OgreAxisAlignedBox.h
  include/OgreBillboard.h
  include/OgreBillboardChain.h
  include/OgreBillboardParticleRenderer.h
  include/OgreBillboardSet.h
  include/OgreBitwise.h
  include/OgreBlendMode.h
  include/OgreBone.h
  ${OGRE_BINARY_DIR}/include/OgreBuildSettings.h
  include/OgreCamera.h
  include/OgreCodec.h
  include/OgreColourValue.h
  include/OgreCommon.h
  include/OgreCompositionPass.h
  include/OgreCompositionTargetPass.h
  include/OgreCompositionTechnique.h
  include/OgreCompositor.h
  include/OgreCompositorChain.h
  include/OgreCompositorLogic.h
  include/OgreCompositorInstance.h
  include/OgreCompositorManager.h
  include/OgreConfig.h
  include/OgreConfigDialog.h
  include/OgreConfigFile.h
  include/OgreConfigOptionMap.h
  include/OgreController.h
  include/OgreControllerManager.h
  include/OgreConvexBody.h
  include/OgreCustomCompositionPass.h
  include/OgreDataStream.h
  include/OgreDefaultHardwareBufferManager.h
  include/OgreDeflate.h
  include/OgreDepthBuffer.h
  include/OgreDistanceLodStrategy.h
  include/OgreDualQuaternion.h
  include/OgreDynLib.h
  include/OgreDynLibManager.h
  include/OgreEdgeListBuilder.h
  include/OgreEntity.h
  include/OgreErrorDialog.h
  include/OgreException.h
  include/OgreExternalTextureSource.h
  include/OgreExternalTextureSourceManager.h
  include/OgreFactoryObj.h
  include/OgreFastArray.h
  include/OgreFileSystem.h
  include/OgreFileSystemLayer.h
  include/OgreFrameListener.h
  include/OgreFrameStats.h
  include/OgreFrustum.h
  include/OgreGpuProgram.h
  include/OgreGpuProgramManager.h
  include/OgreGpuProgramParams.h
  include/OgreGpuProgramUsage.h
  include/OgreHardwareBuffer.h
  include/OgreHardwareBufferManager.h
  include/OgreHardwareCounterBuffer.h
  include/OgreHardwareIndexBuffer.h
  include/OgreHardwareOcclusionQuery.h
  include/OgreHardwarePixelBuffer.h
  include/OgreHardwareUniformBuffer.h
  include/OgreHardwareVertexBuffer.h
  include/OgreHeaderPrefix.h
  include/OgreHeaderSuffix.h
  include/OgreHighLevelGpuProgram.h
  include/OgreHighLevelGpuProgramManager.h
  include/OgreId.h
  include/OgreIdString.h
  include/OgreImage.h
  include/OgreImageCodec.h
  src/OgreImageResampler.h
  include/OgreInstanceBatch.h
  include/OgreInstanceBatchHW.h
  include/OgreInstanceBatchHW_VTF.h
  include/OgreInstanceBatchShader.h
  include/OgreInstanceBatchVTF.h
  include/OgreInstancedEntity.h
  include/OgreInstancedEntity.inl
  include/OgreInstanceManager.h
  include/OgreIteratorRange.h
  include/OgreIteratorWrapper.h
  include/OgreIteratorWrappers.h
  include/OgreKeyFrame.h
  include/OgreLight.h
  include/OgreLodConfig.h
  include/OgreLodListener.h
  include/OgreLodStrategy.h
  include/OgreLodStrategyManager.h
  include/OgreLog.h
  include/OgreLogManager.h
  include/OgreManualObject.h
  include/OgreMaterial.h
  include/OgreMaterialManager.h
  include/OgreMaterialSerializer.h
  include/OgreMath.h
  include/OgreMatrix3.h
  include/OgreMatrix4.h
  include/OgreMemoryAllocatedObject.h
  include/OgreMemoryAllocatorConfig.h
  include/OgreMemoryNedAlloc.h
  include/OgreMemoryNedPooling.h
  include/OgreMemoryStdAlloc.h
  include/OgreMemorySTLAllocator.h
  include/OgreMemoryTracker.h
  include/OgreMesh.h
  include/OgreMeshFileFormat.h
  include/OgreMeshManager.h
  include/OgreMeshSerializer.h
  include/OgreMeshSerializerImpl.h
  include/OgreMovableObject.h
  include/OgreMovableObject.inl
  include/OgreMovablePlane.h
  include/OgreNode.h
  include/OgreNumerics.h
  include/OgreOldNode.h
  include/OgreOptimisedUtil.h
  include/OgreParticle.h
  include/OgreParticleAffector.h
  include/OgreParticleAffectorFactory.h
  include/OgreParticleEmitter.h
  include/OgreParticleEmitterCommands.h
  include/OgreParticleEmitterFactory.h
  include/OgreParticleIterator.h
  include/OgreParticleScriptCompiler.h
  include/OgreParticleSystem.h
  include/OgreParticleSystemManager.h
  include/OgreParticleSystemRenderer.h
  include/OgrePass.h
  include/OgrePatchMesh.h
  include/OgrePatchSurface.h
  src/OgrePixelConversions.h
  include/OgrePixelCountLodStrategy.h
  include/OgrePixelFormat.h
  include/OgrePlane.h
  include/OgrePlaneBoundedVolume.h
  include/OgrePlatform.h
  include/OgrePlatformInformation.h
  include/OgrePlugin.h
  include/OgrePolygon.h
  include/OgrePose.h
  include/OgrePredefinedControllers.h
  include/OgrePrefabFactory.h
  include/OgrePrerequisites.h
  include/OgreProfiler.h
  include/OgreProgressiveMeshGenerator.h
  include/OgreQuaternion.h
  include/OgreQueuedProgressiveMeshGenerator.h
  include/OgreRadixSort.h
  include/OgreRay.h
  include/OgreRectangle2D.h
  include/OgreRenderable.h
  include/OgreRenderObjectListener.h
  include/OgreRenderOperation.h
  include/OgreRenderQueue.h
  include/OgreRenderQueueInvocation.h
  include/OgreRenderQueueListener.h
  include/OgreRenderQueueSortingGrouping.h
  include/OgreRenderSystem.h
  include/OgreRenderSystemCapabilities.h
  include/OgreRenderSystemCapabilitiesManager.h
  include/OgreRenderSystemCapabilitiesSerializer.h
  include/OgreRenderTarget.h
  include/OgreRenderTargetListener.h
  include/OgreRenderTexture.h
  include/OgreRenderToVertexBuffer.h
  include/OgreRenderWindow.h
  include/OgreResource.h
  include/OgreResourceBackgroundQueue.h
  include/OgreResourceGroupManager.h
  include/OgreResourceManager.h
  include/OgreRibbonTrail.h
  include/OgreRoot.h
  include/OgreRotationalSpline.h
  include/OgreSceneManager.h
  include/OgreSceneManagerEnumerator.h
  include/OgreSceneNode.h
  include/OgreSceneQuery.h
  include/OgreScriptCompiler.h
  include/OgreScriptLexer.h
  include/OgreScriptLoader.h
  include/OgreScriptParser.h
  include/OgreScriptTranslator.h
  include/OgreSearchOps.h
  include/OgreSerializer.h
  include/OgreShadowCameraSetup.h
  include/OgreShadowCameraSetupFocused.h
  include/OgreShadowCameraSetupLiSPSM.h
  include/OgreShadowCameraSetupPlaneOptimal.h
  include/OgreShadowCameraSetupPSSM.h
  include/OgreShadowTextureManager.h
  include/OgreSharedPtr.h
  src/OgreSIMDHelper.h
  include/OgreSimpleRenderable.h
  include/OgreSimpleSpline.h
  include/OgreSingleton.h
  include/OgreSkeleton.h
  include/OgreSkeletonFileFormat.h
  include/OgreSkeletonInstance.h
  include/OgreSkeletonManager.h
  include/OgreSkeletonSerializer.h
  include/OgreSmallVector.h
  include/OgreSphere.h
  include/OgreSpotShadowFadePng.h
  include/OgreStableHeaders.h
  include/OgreStaticFaceGroup.h
  include/OgreStaticGeometry.h
  include/OgreStdHeaders.h
  include/OgreStreamSerialiser.h
  include/OgreString.h
  include/OgreStringConverter.h
  include/OgreStringInterface.h
  include/OgreStringVector.h
  include/OgreSubEntity.h
  include/OgreSubMesh.h
  include/OgreTagPoint.h
  include/OgreTangentSpaceCalc.h
  include/OgreTechnique.h
  include/OgreTexture.h
  include/OgreTextureManager.h
  include/OgreTextureUnitState.h
  include/OgreTimer.h
  include/OgreUnifiedHighLevelGpuProgram.h
  include/OgreUserObjectBindings.h
  include/OgreUTFString.h
  include/OgreVector2.h
  include/OgreVector3.h
  include/OgreVector4.h
  include/OgreVertexBoneAssignment.h
  include/OgreVertexIndexData.h
  include/OgreViewport.h
  include/OgreWindowEventUtilities.h
  include/OgreWireBoundingBox.h
  include/OgreWorkQueue.h
)

set (SOURCE_FILES
  src/Hash/MurmurHash3.cpp
  src/OgreAlignedAllocator.cpp
  src/OgreAnimable.cpp
  src/OgreAnimation.cpp
  src/OgreAnimationState.cpp
  src/OgreAnimationTrack.cpp
  src/OgreArchiveManager.cpp
  src/Compositor/Pass/OgreCompositorPass.cpp
  src/Compositor/Pass/OgreCompositorPassDef.cpp
  src/Compositor/Pass/PassClear/OgreCompositorPassClear.cpp
  src/Compositor/Pass/PassQuad/OgreCompositorPassQuad.cpp
  src/Compositor/Pass/PassScene/OgreCompositorPassScene.cpp
  src/Compositor/OgreCompositorManager2.cpp
  src/Compositor/OgreCompositorNode.cpp
  src/Compositor/OgreCompositorNodeDef.cpp
  src/Compositor/OgreCompositorShadowNode.cpp
  src/Compositor/OgreCompositorShadowNodeDef.cpp
  src/Compositor/OgreCompositorWorkspace.cpp
  src/Compositor/OgreCompositorWorkspaceDef.cpp
  src/Compositor/OgreTextureDefinition.cpp
  src/Math/Array/SSE2/Single/OgreArrayAabb.cpp
  src/Math/Array/SSE2/Single/OgreArrayMatrix4.cpp
  src/Math/Array/SSE2/Single/OgreArrayQuaternion.cpp
  src/Math/Array/SSE2/Single/OgreArrayVector3.cpp
  src/Math/Array/SSE2/Single/OgreMathlibSSE2.cpp
  src/Math/Array/OgreArrayMemoryManager.cpp
  src/Math/Array/OgreNodeArrayMemoryManager.cpp
  src/Math/Array/OgreNodeMemoryManager.cpp
  src/Math/Array/OgreObjectDataArrayMemoryManager.cpp
  src/Math/Array/OgreObjectMemoryManager.cpp
  src/Math/Simple/C/OgreAabb.cpp
  src/OgreAutoParamDataSource.cpp
  src/OgreAxisAlignedBox.cpp
  src/OgreBillboard.cpp
  src/OgreBillboardChain.cpp
  src/OgreBillboardParticleRenderer.cpp
  src/OgreBillboardSet.cpp
  src/OgreBone.cpp
  src/OgreCamera.cpp
  src/OgreCodec.cpp
  src/OgreColourValue.cpp
  src/OgreCommon.cpp
  src/OgreCompositionPass.cpp
  src/OgreCompositionTargetPass.cpp
  src/OgreCompositionTechnique.cpp
  src/OgreCompositor.cpp
  src/OgreCompositorChain.cpp
  src/OgreCompositorInstance.cpp
  src/OgreCompositorManager.cpp
  src/OgreConfigFile.cpp
  src/OgreControllerManager.cpp
  src/OgreConvexBody.cpp
  src/OgreDataStream.cpp
  src/OgreDefaultHardwareBufferManager.cpp
  src/OgreDefaultSceneQueries.cpp
  src/OgreDeflate.cpp
  src/OgreDepthBuffer.cpp
  src/OgreDistanceLodStrategy.cpp
  src/OgreDualQuaternion.cpp
  src/OgreDynLib.cpp
  src/OgreDynLibManager.cpp
  src/OgreEdgeListBuilder.cpp
  src/OgreEntity.cpp
  src/OgreException.cpp
  src/OgreExternalTextureSource.cpp
  src/OgreExternalTextureSourceManager.cpp
  src/OgreFileSystem.cpp
  src/OgreFrustum.cpp
  src/OgreGpuProgram.cpp
  src/OgreGpuProgramManager.cpp
  src/OgreGpuProgramParams.cpp
  src/OgreGpuProgramUsage.cpp
  src/OgreHardwareBufferManager.cpp
  src/OgreHardwareCounterBuffer.cpp
  src/OgreHardwareIndexBuffer.cpp
  src/OgreHardwareOcclusionQuery.cpp
  src/OgreHardwarePixelBuffer.cpp
  src/OgreHardwareUniformBuffer.cpp
  src/OgreHardwareVertexBuffer.cpp
  src/OgreHighLevelGpuProgram.cpp
  src/OgreHighLevelGpuProgramManager.cpp
  src/OgreImage.cpp
  src/OgreInstanceBatch.cpp
  src/OgreInstanceBatchHW.cpp
  src/OgreInstanceBatchHW_VTF.cpp
  src/OgreInstanceBatchShader.cpp
  src/OgreInstanceBatchVTF.cpp
  src/OgreInstancedEntity.cpp
  src/OgreInstanceManager.cpp
  src/OgreKeyFrame.cpp
  src/OgreLight.cpp
  src/OgreLodStrategy.cpp
  src/OgreLodStrategyManager.cpp
  src/OgreLog.cpp
  src/OgreLogManager.cpp
  src/ogremain.map
  src/OgreManualObject.cpp
  src/OgreMaterial.cpp
  src/OgreMaterialManager.cpp
  src/OgreMaterialSerializer.cpp
  src/OgreMath.cpp
  src/OgreMatrix3.cpp
  src/OgreMatrix4.cpp
  src/OgreMemoryAllocatedObject.cpp
  src/OgreMemoryNedAlloc.cpp
  src/OgreMemoryNedPooling.cpp
  src/OgreMemoryTracker.cpp
  src/OgreMesh.cpp
  src/OgreMeshManager.cpp
  src/OgreMeshSerializer.cpp
  src/OgreMeshSerializerImpl.cpp
  src/OgreMovableObject.cpp
  src/OgreMovablePlane.cpp
  src/OgreNode.cpp
  src/OgreNumerics.cpp
  src/OgreOldNode.cpp
  src/OgreOptimisedUtil.cpp
  src/OgreOptimisedUtilGeneral.cpp
#  src/OgreOptimisedUtilNEON.cpp
  src/OgreOptimisedUtilSSE.cpp
#  src/OgreOptimisedUtilVFP.cpp
  src/OgreParticle.cpp
  src/OgreParticleEmitter.cpp
  src/OgreParticleEmitterCommands.cpp
  src/OgreParticleIterator.cpp
  src/OgreParticleSystem.cpp
  src/OgreParticleSystemManager.cpp
  src/OgrePass.cpp
  src/OgrePatchMesh.cpp
  src/OgrePatchSurface.cpp
  src/OgrePixelCountLodStrategy.cpp
  src/OgrePixelFormat.cpp
  src/OgrePlane.cpp
  src/OgrePlatformInformation.cpp
  src/OgrePolygon.cpp
  src/OgrePose.cpp
  src/OgrePrecompiledHeaders.cpp
  src/OgrePredefinedControllers.cpp
  src/OgrePrefabFactory.cpp
  src/OgreProfiler.cpp
  src/OgreProgressiveMeshGenerator.cpp
  src/OgreQuaternion.cpp
  src/OgreQueuedProgressiveMeshGenerator.cpp
  src/OgreRectangle2D.cpp
  src/OgreRenderQueue.cpp
  src/OgreRenderQueueInvocation.cpp
  src/OgreRenderQueueSortingGrouping.cpp
  src/OgreRenderSystem.cpp
  src/OgreRenderSystemCapabilities.cpp
  src/OgreRenderSystemCapabilitiesManager.cpp
  src/OgreRenderSystemCapabilitiesSerializer.cpp
  src/OgreRenderTarget.cpp
  src/OgreRenderTexture.cpp
  src/OgreRenderToVertexBuffer.cpp
  src/OgreRenderWindow.cpp
  src/OgreResource.cpp
  src/OgreResourceBackgroundQueue.cpp
  src/OgreResourceGroupManager.cpp
  src/OgreResourceManager.cpp
  src/OgreRibbonTrail.cpp
  src/OgreRoot.cpp
  src/OgreRotationSpline.cpp
  src/OgreSceneManager.cpp
  src/OgreSceneManagerEnumerator.cpp
  src/OgreSceneNode.cpp
  src/OgreSceneQuery.cpp
  src/OgreScriptCompiler.cpp
  src/OgreScriptLexer.cpp
  src/OgreScriptParser.cpp
  src/OgreScriptTranslator.cpp
  src/OgreSerializer.cpp
  src/OgreShadowCameraSetup.cpp
  src/OgreShadowCameraSetupFocused.cpp
  src/OgreShadowCameraSetupLiSPSM.cpp
  src/OgreShadowCameraSetupPlaneOptimal.cpp
  src/OgreShadowCameraSetupPSSM.cpp
  src/OgreShadowTextureManager.cpp
  src/OgreSimpleRenderable.cpp
  src/OgreSimpleSpline.cpp
  src/OgreSkeleton.cpp
  src/OgreSkeletonInstance.cpp
  src/OgreSkeletonManager.cpp
  src/OgreSkeletonSerializer.cpp
  src/OgreSmallVector.cpp
  src/OgreStaticGeometry.cpp
  src/OgreStreamSerialiser.cpp
  src/OgreString.cpp
  src/OgreStringConverter.cpp
  src/OgreStringInterface.cpp
  src/OgreSubEntity.cpp
  src/OgreSubMesh.cpp
  src/OgreTagPoint.cpp
  src/OgreTangentSpaceCalc.cpp
  src/OgreTechnique.cpp
  src/OgreTexture.cpp
  src/OgreTextureManager.cpp
  src/OgreTextureUnitState.cpp
  src/OgreUnifiedHighLevelGpuProgram.cpp
  src/OgreUserObjectBindings.cpp
  src/OgreUTFString.cpp
  src/OgreVector2.cpp
  src/OgreVector3.cpp
  src/OgreVector4.cpp
  src/OgreVertexIndexData.cpp
  src/OgreViewport.cpp
  src/OgreWindowEventUtilities.cpp
  src/OgreWireBoundingBox.cpp
  src/OgreWorkQueue.cpp
)

# Add platform specific files
if (OGRE_BUILD_PLATFORM_NACL)
  include_directories("include/NaCl")
  set(PLATFORM_HEADERS
    include/NaCl/OgreConfigDialogImp.h
    include/NaCl/OgreErrorDialogImp.h
    include/NaCl/OgreTimerImp.h
  )
  set(PLATFORM_SOURCE_FILES
    src/NaCl/OgreConfigDialog.cpp
    src/NaCl/OgreErrorDialog.cpp
    src/NaCl/OgreFileSystemLayer.cpp
    src/NaCl/OgreTimer.cpp
    src/OgreSearchOps.cpp
  )
  set(PLATFORM_LIBS nosys)
  set(PLATFORM_HEADER_INSTALL "NaCl")
elseif (WIN32)
  set(THREAD_SOURCE_FILES
      src/Threading/OgreBarrierWin.cpp
	  src/Threading/OgreLightweightMutexWin.cpp
      src/Threading/OgreThreadsWin.cpp
  )
  list(APPEND PLATFORM_SOURCE_FILES src/WIN32/OgreWin32Resources.rc)
  if (OGRE_BUILD_PLATFORM_WINRT OR OGRE_BUILD_PLATFORM_WINDOWS_PHONE)
    include_directories("include/WIN32")
    set(PLATFORM_HEADERS
      include/WIN32/OgreConfigDialogImpWinRT.h
      include/WIN32/OgreErrorDialogImpWinRT.h
      include/WIN32/OgreTimerImp.h
    )
    set(PLATFORM_SOURCE_FILES
      src/WIN32/OgreConfigDialogWinRT.cpp
      src/WIN32/OgreErrorDialogWinRT.cpp
      src/WIN32/OgreTimer.cpp
	  src/WIN32/OgreOptimisedUtilDirectXMath.cpp
    )  

    set(PLATFORM_HEADER_INSTALL "WIN32")
  else ()
  include_directories("include/WIN32")
  set(PLATFORM_HEADERS
    include/WIN32/OgreConfigDialogImp.h
    include/WIN32/OgreErrorDialogImp.h
    include/WIN32/OgreTimerImp.h
    include/WIN32/OgreMinGWSupport.h
  )
  set(PLATFORM_SOURCE_FILES
    src/WIN32/OgreConfigDialog.cpp
    src/WIN32/OgreErrorDialog.cpp
    src/WIN32/OgreFileSystemLayer.cpp
    src/WIN32/OgreTimer.cpp
    src/WIN32/resource.h
    src/WIN32/winres.h
  )  
  set(RESOURCE_FILES
    src/WIN32/resource.h
    src/WIN32/OgreWin32Resources.rc
    src/WIN32/winres.h
  )
  source_group(Resources FILES ${RESOURCE_FILES})
  set(PLATFORM_HEADER_INSTALL "WIN32")
 
  if (MINGW) 
    # Older CMake versions do not know how to deal with resource files on MinGW.
	# Newer ones do, but pass along invalid command options to windres.
    # This is a manual fix for the moment.
    get_filename_component(OGRE_MINGW_PATH ${CMAKE_CXX_COMPILER} PATH)
    #set(OGRE_MINGW_RC ${OGRE_MINGW_PATH}/windres.exe)
	file(GLOB OGRE_MINGW_RC_NAMES "${OGRE_MINGW_PATH}/*windres*")
	find_program(OGRE_MINGW_RC NAMES ${OGRE_MINGW_RC_NAMES} HINTS ${OGRE_MINGW_PATH} DOC "Path to MinGW resource compiler")
	if (NOT OGRE_MINGW_RC)
	  message(SEND_ERROR "Could not find MinGW resource compiler. Please specify path in OGRE_MINGW_RC.")
	endif ()
	mark_as_advanced(OGRE_MINGW_RC)
	add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/OgreWin32Resources.rc.obj
	  COMMAND ${OGRE_MINGW_RC} -I${CMAKE_CURRENT_SOURCE_DIR}/src/WIN32 
	                           -i${CMAKE_CURRENT_SOURCE_DIR}/src/WIN32/OgreWin32Resources.rc
							   -o${CMAKE_CURRENT_BINARY_DIR}/OgreWin32Resources.rc.obj
      DEPENDS ${RESOURCE_FILES})
	list(APPEND PLATFORM_SOURCE_FILES ${CMAKE_CURRENT_BINARY_DIR}/OgreWin32Resources.rc.obj)
  else ()
    list(APPEND PLATFORM_SOURCE_FILES src/WIN32/OgreWin32Resources.rc)
  endif ()
  endif ()

elseif (APPLE)
  if (OGRE_BUILD_PLATFORM_APPLE_IOS)
    include_directories("include/iOS")
    set(PLATFORM_HEADERS
      include/iOS/OgreConfigDialogImp.h
      include/iOS/OgreErrorDialogImp.h
      include/iOS/OgreTimerImp.h
      include/iOS/macUtils.h
    )
    set(PLATFORM_SOURCE_FILES
      src/iOS/OgreConfigDialog.cpp
      src/iOS/OgreErrorDialog.cpp
      src/iOS/OgreFileSystemLayer.mm
      src/iOS/OgreTimer.cpp
      src/iOS/macUtils.mm
      src/OgreSearchOps.cpp
    )
    set(PLATFORM_LIBS "")
    set(PLATFORM_HEADER_INSTALL "iOS")
  else ()
    include_directories("include/OSX")
    set(PLATFORM_HEADERS
      # Hack around a CMake bug.  The first item has its role set to Project instead of public.
      # including it twice works around the issue
      # In addition, include OgreZip.h because of a similar issue
      include/OSX/macUtils.h
      include/OSX/macUtils.h
      include/OSX/OgreConfigDialogImp.h
      include/OSX/OgreErrorDialogImp.h
      include/OSX/OgreTimerImp.h
	  include/OgreZip.h
    )
    set(PLATFORM_SOURCE_FILES
      src/OSX/OgreConfigDialog.mm
      src/OSX/OgreErrorDialog.mm
      src/OSX/OgreFileSystemLayer.cpp
      src/OSX/OgreTimer.cpp
      src/OSX/macUtils.mm
      src/OgreSearchOps.cpp
	  include/OSX/ogrelogo.png
    )

    set(RESOURCE_FILES
	  include/OSX/ogrelogo.png
    )
    source_group(Resources FILES ${RESOURCE_FILES})

    set(PLATFORM_LIBS ${Carbon_LIBRARY_FWK} ${Cocoa_LIBRARY_FWK})
    set(PLATFORM_HEADER_INSTALL "OSX")
  endif ()
elseif(ANDROID)
  set(PLATFORM_HEADERS
    include/Android/OgreConfigDialogImp.h
    include/Android/OgreErrorDialogImp.h
    include/Android/OgreTimerImp.h
    include/Android/OgreAndroidLogListener.h
    include/Android/OgreAPKFileSystemArchive.h
  )
  set(PLATFORM_SOURCE_FILES
    src/Android/OgreConfigDialog.cpp
    src/Android/OgreErrorDialog.cpp
    src/Android/OgreTimer.cpp
    src/OgreSearchOps.cpp
    src/Android/OgreAndroidLogListener.cpp
    src/Android/OgreAPKFileSystemArchive.cpp
    src/Android/OgreFileSystemLayer.cpp
  )

  set(PLATFORM_HEADER_INSTALL "Android")
elseif(FLASHCC)
  set(PLATFORM_HEADERS
    include/FlashCC/OgreTimerImp.h
  )
  set(PLATFORM_SOURCE_FILES
    src/FlashCC/OgreTimer.cpp
  )

  set(PLATFORM_HEADER_INSTALL "FlashCC")
elseif (UNIX)
  include_directories(include/GLX ${X11_INCLUDE_DIR})
  set(PLATFORM_HEADERS
    include/GLX/OgreConfigDialogImp.h
    include/GLX/OgreErrorDialogImp.h
    include/GLX/OgreTimerImp.h
  )
  set(PLATFORM_SOURCE_FILES
    src/GLX/GLX_backdrop.h
    src/GLX/OgreConfigDialog.cpp
    src/GLX/OgreErrorDialog.cpp
    src/GLX/OgreFileSystemLayer.cpp
    src/GLX/OgreTimer.cpp
    src/OgreSearchOps.cpp
  )
  set(PLATFORM_LIBS ${X11_LIBRARIES} ${X11_Xt_LIB} ${XAW_LIBRARY} pthread)
  set(PLATFORM_HEADER_INSTALL "GLX")
  # some platforms require explicit linking to libdl, see if it's available
  find_library(DL_LIBRARY NAMES dl dlsym)
  mark_as_advanced(DL_LIBRARY)
  if (DL_LIBRARY)
    list(APPEND PLATFORM_LIBS dl)
  endif ()
endif()

if (NOT WIN32)
  set(THREAD_SOURCE_FILES
      src/Threading/OgreBarrierPThreads.cpp
	  src/Threading/OgreLightweightMutexPThreads.cpp
      src/Threading/OgreThreadsPThreads.cpp
  )
endif()


# Configure threading files
set(THREAD_HEADER_FILES
	include/Threading/OgreBarrier.h
	include/Threading/OgreLightweightMutex.h
	include/Threading/OgreThreadDefines.h
	include/Threading/OgreThreadHeaders.h
	include/Threading/OgreThreads.h
	include/Threading/OgreDefaultWorkQueue.h
	include/Threading/OgreUniformScalableTask.h
)
if (OGRE_THREAD_PROVIDER EQUAL 0)
	list(APPEND THREAD_HEADER_FILES
		include/Threading/OgreThreadDefinesNone.h
		include/Threading/OgreDefaultWorkQueueStandard.h
	)
	list(APPEND THREAD_SOURCE_FILES
		src/Threading/OgreDefaultWorkQueueStandard.cpp
	)
elseif (OGRE_THREAD_PROVIDER EQUAL 1)
	list(APPEND THREAD_HEADER_FILES
		include/Threading/OgreThreadDefinesBoost.h
		include/Threading/OgreThreadHeadersBoost.h
		include/Threading/OgreDefaultWorkQueueStandard.h
	)
	list(APPEND THREAD_SOURCE_FILES
		src/Threading/OgreDefaultWorkQueueStandard.cpp
	)
elseif (OGRE_THREAD_PROVIDER EQUAL 2)
	list(APPEND THREAD_HEADER_FILES
		include/Threading/OgreThreadDefinesPoco.h
		include/Threading/OgreThreadHeadersPoco.h
		include/Threading/OgreDefaultWorkQueueStandard.h
	)
	list(APPEND THREAD_SOURCE_FILES
		src/Threading/OgreDefaultWorkQueueStandard.cpp
	)
elseif (OGRE_THREAD_PROVIDER EQUAL 3)
	list(APPEND THREAD_HEADER_FILES
		include/Threading/OgreThreadDefinesTBB.h
		include/Threading/OgreThreadHeadersTBB.h
		include/Threading/OgreDefaultWorkQueueTBB.h
	)
	list(APPEND THREAD_SOURCE_FILES
		src/Threading/OgreDefaultWorkQueueTBB.cpp
	)
endif ()

list(APPEND HEADER_FILES ${THREAD_HEADER_FILES})

# Add needed definitions and nedmalloc include dir
add_definitions(-DOGRE_NONCLIENT_BUILD -DFREEIMAGE_LIB -D_MT -D_USRDLL)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/nedmalloc)

set(LIBRARIES
  ${PLATFORM_LIBS}
)

if (OGRE_CONFIG_THREADS)
  list(APPEND LIBRARIES ${OGRE_THREAD_LIBRARIES})
endif ()

if (OGRE_CONFIG_ENABLE_FREEIMAGE)
  list(APPEND HEADER_FILES include/OgreFreeImageCodec.h)
  list(APPEND SOURCE_FILES src/OgreFreeImageCodec.cpp)
  list(APPEND LIBRARIES    "${FreeImage_LIBRARIES}")
endif()

if (OGRE_CONFIG_ENABLE_DDS)
  list(APPEND HEADER_FILES include/OgreDDSCodec.h)
  list(APPEND SOURCE_FILES src/OgreDDSCodec.cpp)
endif ()

if (OGRE_CONFIG_ENABLE_PVRTC)
  list(APPEND HEADER_FILES include/OgrePVRTCCodec.h)
  list(APPEND SOURCE_FILES src/OgrePVRTCCodec.cpp)
endif ()

if (OGRE_CONFIG_ENABLE_ETC1)
  list(APPEND HEADER_FILES include/OgreETC1Codec.h)
  list(APPEND SOURCE_FILES src/OgreETC1Codec.cpp)
endif ()

if (OGRE_CONFIG_ENABLE_ZIP)
  list(APPEND HEADER_FILES include/OgreZip.h)
  list(APPEND SOURCE_FILES src/OgreZip.cpp)

  if(ANDROID)
    ADD_DEFINITIONS(-DZZIP_OMIT_CONFIG_H)
    list(APPEND PLATFORM_HEADERS include/Android/OgreAPKZipArchive.h)
    list(APPEND PLATFORM_SOURCE_FILES src/Android/OgreAPKZipArchive.cpp)
  endif()

  list(APPEND LIBRARIES "${ZZip_LIBRARIES}")
  list(APPEND LIBRARIES "${ZLIB_LIBRARIES}")
endif ()

if (OGRE_CONFIG_ENABLE_GLES2_GLSL_OPTIMISER)
  list(APPEND LIBRARIES "${GLSL_Optimizer_LIBRARIES}")
endif ()

if (OGRE_CONFIG_ENABLE_GLES2_CG_SUPPORT)
  list(APPEND LIBRARIES "${HLSL2GLSL_LIBRARIES}")
endif ()

set (TARGET_LINK_FLAGS "")

# setup OgreMain target
if (OGRE_BUILD_PLATFORM_WINRT OR OGRE_BUILD_PLATFORM_WINDOWS_PHONE)
	# exclude OgreTimer.cpp from unity builds; causes problem
	ogre_add_library(OgreMain ${OGRE_LIB_TYPE} ${PREC_HEADER} ${HEADER_FILES} ${SOURCE_FILES} ${PLATFORM_HEADERS} ${PLATFORM_SOURCE_FILES} ${THREAD_HEADER_FILES} ${THREAD_SOURCE_FILES} SEPARATE "src/WIN32/OgreTimer.cpp")
else ()
	# exclude OgreAlignedAllocator.cpp from unity builds; causes problems on Linux
	ogre_add_library(OgreMain ${OGRE_LIB_TYPE} ${PREC_HEADER} ${HEADER_FILES} ${SOURCE_FILES} ${PLATFORM_HEADERS} ${PLATFORM_SOURCE_FILES} ${THREAD_HEADER_FILES} ${THREAD_SOURCE_FILES} SEPARATE "src/OgreAlignedAllocator.cpp")
endif ()
# In visual studio 2010 - 64 bit we get this error: "LINK : fatal error LNK1210: exceeded internal ILK size limit; link with /INCREMENTAL:NO"
if(WIN32 AND MSVC10 AND CMAKE_CL_64)
  set_target_properties(OgreMain PROPERTIES 
                        VERSION ${OGRE_SOVERSION}
                        LINK_FLAGS "/INCREMENTAL:NO"
                      )
else()
  set_target_properties(OgreMain PROPERTIES VERSION ${OGRE_SOVERSION} SOVERSION ${OGRE_SOVERSION})
endif()

if (APPLE)
  if (OGRE_BUILD_PLATFORM_APPLE_IOS)
    set_target_properties(OgreMain PROPERTIES INSTALL_NAME_DIR "OGRE")
  else ()
    ogre_config_framework(OgreMain)
  endif ()

  # Framework is called 'Ogre'
  set_target_properties(OgreMain PROPERTIES	OUTPUT_NAME Ogre)
endif ()
target_link_libraries(OgreMain ${LIBRARIES})
if (MINGW)
  # may need winsock htons functions for FreeImage
  target_link_libraries(OgreMain ws2_32)
endif ()

# specify a precompiled header to use
use_precompiled_header(OgreMain 
  "${CMAKE_CURRENT_SOURCE_DIR}/include/OgreStableHeaders.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/OgrePrecompiledHeaders.cpp"
)

# install OgreMain
ogre_config_lib(OgreMain TRUE)
install(FILES ${HEADER_FILES}
  DESTINATION include/OGRE
)
install(FILES ${PLATFORM_HEADERS}
  DESTINATION include/OGRE/${PLATFORM_HEADER_INSTALL}
)
install(FILES ${THREAD_HEADER_FILES}
	DESTINATION include/OGRE/Threading
)
