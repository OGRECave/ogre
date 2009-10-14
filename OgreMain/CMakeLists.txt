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
  include/OgreBorderPanelOverlayElement.h
  include/OgreCamera.h
  include/OgreCodec.h
  include/OgreColourValue.h
  include/OgreCommon.h
  include/OgreCompiler2Pass.h
  include/OgreCompositionPass.h
  include/OgreCompositionTargetPass.h
  include/OgreCompositionTechnique.h
  include/OgreCompositor.h
  include/OgreCompositorChain.h
  include/OgreCompositorLogic.h
  include/OgreCompositorInstance.h
  include/OgreCompositorManager.h
  include/OgreCompositorScriptCompiler.h
  include/OgreCompositorScriptCompiler2.h
  include/OgreCompositorSerializer.h
  include/OgreConfigDialog.h
  include/OgreConfigFile.h
  include/OgreConfigOptionMap.h
  include/OgreController.h
  include/OgreControllerManager.h
  include/OgreConvexBody.h
  include/OgreCustomCompositionPass.h
  include/OgreDataStream.h
  include/OgreDDSCodec.h
  include/OgreDefaultHardwareBufferManager.h
  include/OgreDistanceLodStrategy.h
  include/OgreDynLib.h
  include/OgreDynLibManager.h
  include/OgreEdgeListBuilder.h
  include/OgreEntity.h
  include/OgreErrorDialog.h
  include/OgreException.h
  include/OgreExternalTextureSource.h
  include/OgreExternalTextureSourceManager.h
  include/OgreFactoryObj.h
  include/OgreFileSystem.h
  include/OgreFont.h
  include/OgreFontManager.h
  include/OgreFrameListener.h
  include/OgreFreeImageCodec.h
  include/OgreFrustum.h
  include/OgreGpuProgram.h
  include/OgreGpuProgramManager.h
  include/OgreGpuProgramParams.h
  include/OgreGpuProgramUsage.h
  include/OgreHardwareBuffer.h
  include/OgreHardwareBufferManager.h
  include/OgreHardwareIndexBuffer.h
  include/OgreHardwareOcclusionQuery.h
  include/OgreHardwarePixelBuffer.h
  include/OgreHardwareVertexBuffer.h
  include/OgreHighLevelGpuProgram.h
  include/OgreHighLevelGpuProgramManager.h
  include/OgreImage.h
  include/OgreImageCodec.h
  include/OgreInstancedGeometry.h
  include/OgreIteratorRange.h
  include/OgreIteratorWrapper.h
  include/OgreIteratorWrappers.h
  include/OgreKeyFrame.h
  include/OgreLight.h
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
  include/OgreMemoryStdAlloc.h
  include/OgreMemorySTLAllocator.h
  include/OgreMemoryTracker.h
  include/OgreMesh.h
  include/OgreMeshFileFormat.h
  include/OgreMeshManager.h
  include/OgreMeshSerializer.h
  include/OgreMeshSerializerImpl.h
  include/OgreMovableObject.h
  include/OgreMovablePlane.h
  include/OgreNode.h
  include/OgreNumerics.h
  include/OgreOptimisedUtil.h
  include/OgreOverlay.h
  include/OgreOverlayContainer.h
  include/OgreOverlayElement.h
  include/OgreOverlayElementCommands.h
  include/OgreOverlayElementFactory.h
  include/OgreOverlayManager.h
  include/OgrePanelOverlayElement.h
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
  include/OgreProgressiveMesh.h
  include/OgreQuaternion.h
  include/OgreRadixSort.h
  include/OgreRay.h
  include/OgreRectangle.h
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
  include/OgreShadowCaster.h
  include/OgreShadowTextureManager.h
  include/OgreShadowVolumeExtrudeProgram.h
  include/OgreSharedPtr.h
  include/OgreSimpleRenderable.h
  include/OgreSimpleSpline.h
  include/OgreSingleton.h
  include/OgreSkeleton.h
  include/OgreSkeletonFileFormat.h
  include/OgreSkeletonInstance.h
  include/OgreSkeletonManager.h
  include/OgreSkeletonSerializer.h
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
  include/OgreTextAreaOverlayElement.h
  include/OgreTexture.h
  include/OgreTextureManager.h
  include/OgreTextureUnitState.h
  include/OgreTimer.h
  include/OgreUnifiedHighLevelGpuProgram.h
  include/OgreUserDefinedObject.h
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
  include/OgreZip.h
)

set (SOURCE_FILES
  src/OgreAlignedAllocator.cpp
  src/OgreAnimable.cpp
  src/OgreAnimation.cpp
  src/OgreAnimationState.cpp
  src/OgreAnimationTrack.cpp
  src/OgreArchiveManager.cpp
  src/OgreAutoParamDataSource.cpp
  src/OgreAxisAlignedBox.cpp
  src/OgreBillboard.cpp
  src/OgreBillboardChain.cpp
  src/OgreBillboardParticleRenderer.cpp
  src/OgreBillboardSet.cpp
  src/OgreBone.cpp
  src/OgreBorderPanelOverlayElement.cpp
  src/OgreCamera.cpp
  src/OgreCodec.cpp
  src/OgreColourValue.cpp
  src/OgreCommon.cpp
  src/OgreCompiler2Pass.cpp
  src/OgreCompositionPass.cpp
  src/OgreCompositionTargetPass.cpp
  src/OgreCompositionTechnique.cpp
  src/OgreCompositor.cpp
  src/OgreCompositorChain.cpp
  src/OgreCompositorInstance.cpp
  src/OgreCompositorManager.cpp
  src/OgreCompositorScriptCompiler.cpp
  src/OgreCompositorSerializer.cpp
  src/OgreConfigFile.cpp
  src/OgreControllerManager.cpp
  src/OgreConvexBody.cpp
  src/OgreDataStream.cpp
  src/OgreDDSCodec.cpp
  src/OgreDefaultHardwareBufferManager.cpp
  src/OgreDefaultSceneQueries.cpp
  src/OgreDistanceLodStrategy.cpp
  src/OgreDynLib.cpp
  src/OgreDynLibManager.cpp
  src/OgreEdgeListBuilder.cpp
  src/OgreEntity.cpp
  src/OgreException.cpp
  src/OgreExternalTextureSource.cpp
  src/OgreExternalTextureSourceManager.cpp
  src/OgreFileSystem.cpp
  src/OgreFont.cpp
  src/OgreFontManager.cpp
  src/OgreFreeImageCodec.cpp
  src/OgreFrustum.cpp
  src/OgreGpuProgram.cpp
  src/OgreGpuProgramManager.cpp
  src/OgreGpuProgramParams.cpp
  src/OgreGpuProgramUsage.cpp
  src/OgreHardwareBufferManager.cpp
  src/OgreHardwareIndexBuffer.cpp
  src/OgreHardwareOcclusionQuery.cpp
  src/OgreHardwarePixelBuffer.cpp
  src/OgreHardwareVertexBuffer.cpp
  src/OgreHighLevelGpuProgram.cpp
  src/OgreHighLevelGpuProgramManager.cpp
  src/OgreImage.cpp
  src/OgreImageResampler.h
  src/OgreInstancedGeometry.cpp
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
  src/OgreMemoryTracker.cpp
  src/OgreMesh.cpp
  src/OgreMeshManager.cpp
  src/OgreMeshSerializer.cpp
  src/OgreMeshSerializerImpl.cpp
  src/OgreMovableObject.cpp
  src/OgreMovablePlane.cpp
  src/OgreNode.cpp
  src/OgreNumerics.cpp
  src/OgreOptimisedUtil.cpp
  src/OgreOptimisedUtilGeneral.cpp
#  src/OgreOptimisedUtilNEON.cpp
  src/OgreOptimisedUtilSSE.cpp
#  src/OgreOptimisedUtilVFP.cpp
  src/OgreOverlay.cpp
  src/OgreOverlayContainer.cpp
  src/OgreOverlayElement.cpp
  src/OgreOverlayElementCommands.cpp
  src/OgreOverlayManager.cpp
  src/OgrePanelOverlayElement.cpp
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
  src/OgrePixelConversions.h
  src/OgrePixelFormat.cpp
  src/OgrePlane.cpp
  src/OgrePlatformInformation.cpp
  src/OgrePolygon.cpp
  src/OgrePose.cpp
  src/OgrePrecompiledHeaders.cpp
  src/OgrePredefinedControllers.cpp
  src/OgrePrefabFactory.cpp
  src/OgreProfiler.cpp
  src/OgreProgressiveMesh.cpp
  src/OgreQuaternion.cpp
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
  src/OgreShadowCaster.cpp
  src/OgreShadowTextureManager.cpp
  src/OgreShadowVolumeExtrudeProgram.cpp
  src/OgreSIMDHelper.h
  src/OgreSimpleRenderable.cpp
  src/OgreSimpleSpline.cpp
  src/OgreSkeleton.cpp
  src/OgreSkeletonInstance.cpp
  src/OgreSkeletonManager.cpp
  src/OgreSkeletonSerializer.cpp
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
  src/OgreTextAreaOverlayElement.cpp
  src/OgreTexture.cpp
  src/OgreTextureManager.cpp
  src/OgreTextureUnitState.cpp
  src/OgreUnifiedHighLevelGpuProgram.cpp
  src/OgreUserDefinedObject.cpp
  src/OgreVector2.cpp
  src/OgreVector3.cpp
  src/OgreVector4.cpp
  src/OgreVertexIndexData.cpp
  src/OgreViewport.cpp
  src/OgreWindowEventUtilities.cpp
  src/OgreWireBoundingBox.cpp
  src/OgreWorkQueue.cpp
  src/OgreZip.cpp
)

# Add platform specific files
if (WIN32)
  include_directories("include/WIN32")
  set(PLATFORM_HEADER_FILES
    include/WIN32/OgreConfigDialogImp.h
    include/WIN32/OgreErrorDialogImp.h
    include/WIN32/OgreTimerImp.h
  )
  set(PLATFORM_SOURCE_FILES
    src/WIN32/resource.h
    src/WIN32/OgreConfigDialog.cpp
    src/WIN32/OgreErrorDialog.cpp
    src/WIN32/OgreTimer.cpp
    src/WIN32/OgreWin32Resources.rc
    src/WIN32/winres.h
  )  
  set(RESOURCE_FILES
    src/WIN32/resource.h
    src/WIN32/OgreWin32Resources.rc
    src/WIN32/winres.h
  )
  source_group(Resources FILES ${RESOURCE_FILES})
  set(PLATFORM_HEADER_INSTALL "win32")

elseif (APPLE)
  if (OGRE_BUILD_PLATFORM_IPHONE)
    include_directories("include/iPhone")
    set(PLATFORM_HEADER_FILES
      include/iPhone/OgreConfigDialogImp.h
      include/iPhone/OgreErrorDialogImp.h
      include/iPhone/OgreTimerImp.h
      include/iPhone/macUtils.h
    )
    set(PLATFORM_SOURCE_FILES
      src/iPhone/OgreConfigDialog.cpp
      src/iPhone/OgreErrorDialog.cpp
      src/iPhone/OgreTimer.cpp
      src/iPhone/macUtils.mm
      src/OgreSearchOps.cpp
    )
    set(PLATFORM_LIBS "")
    set(PLATFORM_HEADER_INSTALL "iPhone")
  else ()
    include_directories("include/OSX")
    set(PLATFORM_HEADER_FILES
      # Hack around a CMake bug.  The first item has its role set to Project instead of public.
      # including it twice works around the issue
      # In addition, include OgreZip.h because of a similar issue
	  include/OSX/config.h
	  include/OSX/config.h
      include/OSX/macUtils.h
      include/OSX/OgreConfigDialogImp.h
      include/OSX/OgreErrorDialogImp.h
      include/OSX/OgreTimerImp.h
      include/OSX/OgreOSXCocoaView.h
	  include/OgreConfig.h
	  include/OgreZip.h
	  ${OGRE_BINARY_DIR}/include/buildsettings.h
    )
    set(PLATFORM_SOURCE_FILES
      src/OSX/OgreConfigDialog.mm
      src/OSX/OgreErrorDialog.mm
      src/OSX/OgreTimer.cpp
      src/OSX/macUtils.cpp
      src/OSX/OgreOSXCocoaView.mm
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
elseif (UNIX)
  include_directories(include/GLX ${X11_INCLUDE_DIR})
  set(PLATFORM_HEADER_FILES
    include/GLX/OgreConfigDialogImp.h
    include/GLX/OgreErrorDialogImp.h
    include/GLX/OgreTimerImp.h
  )
  set(PLATFORM_SOURCE_FILES
    src/GLX/GLX_backdrop.h
    src/GLX/OgreConfigDialog.cpp
    src/GLX/OgreErrorDialog.cpp
    src/GLX/OgreTimer.cpp
    src/OgreSearchOps.cpp
  )
  set(PLATFORM_LIBS ${X11_LIBRARIES} ${X11_Xt_LIB} ${XAW_LIBRARY} pthread)
  set(PLATFORM_HEADER_INSTALL "GLX")

endif(WIN32)


# Configure threading files
set(THREAD_HEADER_FILES
	include/Threading/OgreThreadDefines.h
	include/Threading/OgreThreadHeaders.h
	include/Threading/OgreDefaultWorkQueue.h
)
if (OGRE_THREAD_PROVIDER EQUAL 0)
	list(APPEND THREAD_HEADER_FILES
		include/Threading/OgreThreadDefinesNone.h
		include/Threading/OgreDefaultWorkQueueStandard.h
	)
	set(THREAD_SOURCE_FILES
		src/Threading/OgreDefaultWorkQueueStandard.cpp
	)
elseif (OGRE_THREAD_PROVIDER EQUAL 1)
	list(APPEND THREAD_HEADER_FILES
		include/Threading/OgreThreadDefinesBoost.h
		include/Threading/OgreThreadHeadersBoost.h
		include/Threading/OgreDefaultWorkQueueStandard.h
	)
	set(THREAD_SOURCE_FILES
		src/Threading/OgreDefaultWorkQueueStandard.cpp
	)
elseif (OGRE_THREAD_PROVIDER EQUAL 2)
	list(APPEND THREAD_HEADER_FILES
		include/Threading/OgreThreadDefinesPoco.h
		include/Threading/OgreThreadHeadersPoco.h
		include/Threading/OgreDefaultWorkQueueStandard.h
	)
	set(THREAD_SOURCE_FILES
		src/Threading/OgreDefaultWorkQueueStandard.cpp
	)
elseif (OGRE_THREAD_PROVIDER EQUAL 3)
	list(APPEND THREAD_HEADER_FILES
		include/Threading/OgreThreadDefinesTBB.h
		include/Threading/OgreThreadHeadersTBB.h
		include/Threading/OgreDefaultWorkQueueTBB.h
	)
	set(THREAD_SOURCE_FILES
		src/Threading/OgreDefaultWorkQueueTBB.cpp
	)
endif ()


# Add needed definitions and nedmalloc include dir
add_definitions(-DOGRE_NONCLIENT_BUILD -DFREEIMAGE_LIB -D_MT -D_USRDLL)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/nedmalloc)

set(LIBRARIES
  ${ZLIB_LIBRARIES}
  ${ZZip_LIBRARIES}
  ${FreeImage_LIBRARIES}
  ${FREETYPE_LIBRARIES}
  ${PLATFORM_LIBS}
)

if (OGRE_CONFIG_THREADS)
  list(APPEND LIBRARIES ${OGRE_THREAD_LIBRARIES})
endif ()

if (OGRE_CONFIG_DISABLE_FREEIMAGE)
  list(REMOVE_ITEM HEADER_FILES include/OgreFreeImageCodec.h)
  list(REMOVE_ITEM SOURCE_FILES src/OgreFreeImageCodec.cpp)
  list(REMOVE_ITEM LIBRARIES    "${FreeImage_LIBRARIES}")
endif()

if (OGRE_CONFIG_DISABLE_DDS)
  list(REMOVE_ITEM HEADER_FILES include/OgreDDSCodec.h)
  list(REMOVE_ITEM SOURCE_FILES src/OgreDDSCodec.cpp)
endif ()

if (OGRE_CONFIG_DISABLE_ZIP)
  list(REMOVE_ITEM HEADER_FILES include/OgreZip.h)
  list(REMOVE_ITEM SOURCE_FILES src/OgreZip.cpp)
  list(REMOVE_ITEM LIBRARIES "${ZZip_LIBRARIES}")
  list(REMOVE_ITEM LIBRARIES "${ZLIB_LIBRARIES}")
endif ()

# setup OgreMain target
add_library(OgreMain ${OGRE_LIB_TYPE} ${PREC_HEADER} ${HEADER_FILES} ${SOURCE_FILES} ${PLATFORM_HEADER_FILES} ${PLATFORM_SOURCE_FILES} ${THREAD_HEADER_FILES} ${THREAD_SOURCE_FILES})
set_target_properties(OgreMain PROPERTIES VERSION ${OGRE_VERSION} SOVERSION ${OGRE_VERSION_MAJOR})
if (APPLE)
  if (NOT OGRE_BUILD_PLATFORM_IPHONE)
    set_target_properties(OgreMain PROPERTIES FRAMEWORK TRUE)
    # Set the INSTALL_PATH so that Frameworks can be local
    set_target_properties(OgreMain
       PROPERTIES BUILD_WITH_INSTALL_RPATH 1
       INSTALL_NAME_DIR "@executable_path/../Frameworks"
    )
    set_target_properties(OgreMain PROPERTIES PUBLIC_HEADER "${HEADER_FILES} ${PLATFORM_HEADER_FILES} ${THREAD_HEADER_FILES}")
	set_target_properties(OgreMain PROPERTIES RESOURCE "${RESOURCE_FILES}")
	set_source_files_properties("${RESOURCE_FILES}" PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  else ()
    set_target_properties(OgreMain PROPERTIES INSTALL_NAME_DIR "OGRE")
    set_target_properties(OgreMain PROPERTIES XCODE_ATTRIBUTE_GCC_THUMB_SUPPORT "NO")
    set_target_properties(OgreMain PROPERTIES XCODE_ATTRIBUTE_GCC_UNROLL_LOOPS "YES")
    set_target_properties(OgreMain PROPERTIES XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer")
    set_target_properties(OgreMain PROPERTIES XCODE_ATTRIBUTE_GCC_PRECOMPILE_PREFIX_HEADER "YES")
    set_target_properties(OgreMain PROPERTIES XCODE_ATTRIBUTE_GCC_PREFIX_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/include/OgreStableHeaders.h")
  endif ()

  # Framework is called 'Ogre'
  set_target_properties(OgreMain PROPERTIES	OUTPUT_NAME Ogre)
endif ()
target_link_libraries(OgreMain ${LIBRARIES})

# specify a precompiled header to use
use_precompiled_header(OgreMain 
  "${CMAKE_CURRENT_SOURCE_DIR}/include/OgreStableHeaders.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/OgrePrecompiledHeaders.cpp"
)

# install OgreMain
ogre_config_lib(OgreMain)
install(FILES ${HEADER_FILES}
  DESTINATION include/OGRE
)
install(FILES ${PLATFORM_HEADER_FILES}
  DESTINATION include/OGRE/${PLATFORM_HEADER_INSTALL}
)
install(FILES ${THREAD_HEADER_FILES}
	DESTINATION include/OGRE/Threading
)
