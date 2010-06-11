LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := libzzip
LOCAL_SRC_FILES := zzip/dir.c zzip/err.c zzip/fetch.c zzip/file.c zzip/fseeko.c zzip/info.c zzip/memdisk.c zzip/mmapped.c zzip/plugin.c zzip/stat.c zzip/write.c zzip/zip.c

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS) 

LOCAL_MODULE		:= libfreeimage
LOCAL_C_INCLUDES	:= $(LOCAL_PATH)/freeimage $(LOCAL_PATH)/freeimage/Metadata $(LOCAL_PATH)/freeimage/DeprecationManager
LOCAL_CFLAGS		+= -DFREEIMAGE_LIB=1 -DPNG_STATIC=1
LOCAL_SRC_FILES		:= \
	freeimage/LibJPEG/ckconfig.c\
	freeimage/LibJPEG/jaricom.c\
	freeimage/LibJPEG/jcapimin.c\
	freeimage/LibJPEG/jcapistd.c\
	freeimage/LibJPEG/jcarith.c\
	freeimage/LibJPEG/jccoefct.c\
	freeimage/LibJPEG/jccolor.c\
	freeimage/LibJPEG/jcdctmgr.c\
	freeimage/LibJPEG/jchuff.c\
	freeimage/LibJPEG/jcinit.c\
	freeimage/LibJPEG/jcmainct.c\
	freeimage/LibJPEG/jcmarker.c\
	freeimage/LibJPEG/jcmaster.c\
	freeimage/LibJPEG/jcomapi.c\
	freeimage/LibJPEG/jcparam.c\
	freeimage/LibJPEG/jcprepct.c\
	freeimage/LibJPEG/jcsample.c\
	freeimage/LibJPEG/jctrans.c\
	freeimage/LibJPEG/jdapimin.c\
	freeimage/LibJPEG/jdapistd.c\
	freeimage/LibJPEG/jdarith.c\
	freeimage/LibJPEG/jdatadst.c\
	freeimage/LibJPEG/jdatasrc.c\
	freeimage/LibJPEG/jdcoefct.c\
	freeimage/LibJPEG/jdcolor.c\
	freeimage/LibJPEG/jddctmgr.c\
	freeimage/LibJPEG/jdhuff.c\
	freeimage/LibJPEG/jdinput.c\
	freeimage/LibJPEG/jdmainct.c\
	freeimage/LibJPEG/jdmarker.c\
	freeimage/LibJPEG/jdmaster.c\
	freeimage/LibJPEG/jdmerge.c\
	freeimage/LibJPEG/jdpostct.c\
	freeimage/LibJPEG/jdsample.c\
	freeimage/LibJPEG/jdtrans.c\
	freeimage/LibJPEG/jerror.c\
	freeimage/LibJPEG/jfdctflt.c\
	freeimage/LibJPEG/jfdctfst.c\
	freeimage/LibJPEG/jfdctint.c\
	freeimage/LibJPEG/jidctflt.c\
	freeimage/LibJPEG/jidctfst.c\
	freeimage/LibJPEG/jidctint.c\
	freeimage/LibJPEG/jmemansi.c\
	freeimage/LibJPEG/jmemmgr.c\
	freeimage/LibJPEG/jmemname.c\
	freeimage/LibJPEG/jmemnobs.c\
	freeimage/LibJPEG/jquant1.c\
	freeimage/LibJPEG/jquant2.c\
	freeimage/LibJPEG/jutils.c\
	freeimage/LibJPEG/rdbmp.c\
	freeimage/LibJPEG/rdcolmap.c\
	freeimage/LibJPEG/rdgif.c\
	freeimage/LibJPEG/rdppm.c\
	freeimage/LibJPEG/rdrle.c\
	freeimage/LibJPEG/rdswitch.c\
	freeimage/LibJPEG/rdtarga.c\
	freeimage/LibJPEG/transupp.c\
	freeimage/LibJPEG/wrbmp.c\
	freeimage/LibJPEG/wrgif.c\
	freeimage/LibJPEG/wrppm.c\
	freeimage/LibJPEG/wrrle.c\
	freeimage/LibJPEG/wrtarga.c\
	freeimage/LibPNG/png.c\
	freeimage/LibPNG/pngerror.c\
	freeimage/LibPNG/pnggccrd.c\
	freeimage/LibPNG/pngget.c\
	freeimage/LibPNG/pngmem.c\
	freeimage/LibPNG/pngpread.c\
	freeimage/LibPNG/pngread.c\
	freeimage/LibPNG/pngrio.c\
	freeimage/LibPNG/pngrtran.c\
	freeimage/LibPNG/pngrutil.c\
	freeimage/LibPNG/pngset.c\
	freeimage/LibPNG/pngtrans.c\
	freeimage/LibPNG/pngvcrd.c\
	freeimage/LibPNG/pngwio.c\
	freeimage/LibPNG/pngwrite.c\
	freeimage/LibPNG/pngwtran.c\
	freeimage/LibPNG/pngwutil.c\
	freeimage/FreeImage/BitmapAccess.cpp\
	freeimage/FreeImage/CacheFile.cpp\
	freeimage/FreeImage/ColorLookup.cpp\
	freeimage/FreeImage/Conversion.cpp\
	freeimage/FreeImage/Conversion4.cpp\
	freeimage/FreeImage/Conversion8.cpp\
	freeimage/FreeImage/Conversion16_555.cpp\
	freeimage/FreeImage/Conversion16_565.cpp\
	freeimage/FreeImage/Conversion24.cpp\
	freeimage/FreeImage/Conversion32.cpp\
	freeimage/FreeImage/ConversionRGBF.cpp\
	freeimage/FreeImage/ConversionType.cpp\
	freeimage/FreeImage/FreeImage.cpp\
	freeimage/FreeImage/FreeImageIO.cpp\
	freeimage/FreeImage/GetType.cpp\
	freeimage/FreeImage/Halftoning.cpp\
	freeimage/FreeImage/J2KHelper.cpp\
	freeimage/FreeImage/MemoryIO.cpp\
	freeimage/FreeImage/MultiPage.cpp\
	freeimage/FreeImage/NNQuantizer.cpp\
	freeimage/FreeImage/PixelAccess.cpp\
	freeimage/FreeImage/Plugin.cpp\
	freeimage/FreeImage/PluginGIF.cpp\
	freeimage/FreeImage/PluginJPEG.cpp\
	freeimage/FreeImage/PluginPNG.cpp\
	freeimage/FreeImage/ToneMapping.cpp\
	freeimage/FreeImage/WuQuantizer.cpp\
	freeimage/FreeImage/ZLibInterface.cpp\
	freeimage/DeprecationManager/Deprecated.cpp\
	freeimage/DeprecationManager/DeprecationMgr.cpp\
	freeimage/Metadata/Exif.cpp\
	freeimage/Metadata/FIRational.cpp\
	freeimage/Metadata/FreeImageTag.cpp\
	freeimage/Metadata/IPTC.cpp\
	freeimage/Metadata/TagConversion.cpp\
	freeimage/Metadata/TagLib.cpp\
	freeimage/Metadata/XTIFF.cpp\
	freeimage/FreeImageToolkit/Background.cpp\
	freeimage/FreeImageToolkit/BSplineRotate.cpp\
	freeimage/FreeImageToolkit/Channels.cpp\
	freeimage/FreeImageToolkit/ClassicRotate.cpp\
	freeimage/FreeImageToolkit/Colors.cpp\
	freeimage/FreeImageToolkit/CopyPaste.cpp\
	freeimage/FreeImageToolkit/Display.cpp\
	freeimage/FreeImageToolkit/Flip.cpp\
	freeimage/FreeImageToolkit/JPEGTransform.cpp\
	freeimage/FreeImageToolkit/MultigridPoissonSolver.cpp\
	freeimage/FreeImageToolkit/Rescale.cpp\
	freeimage/FreeImageToolkit/Resize.cpp

include $(BUILD_STATIC_LIBRARY) 

include $(CLEAR_VARS)

LOCAL_MODULE		:= libogre
LOCAL_C_INCLUDES	:= $(LOCAL_PATH)/ogre $(LOCAL_PATH)/ogre/OgreMain/include $(LOCAL_PATH)/ogre/dependencies $(LOCAL_PATH)/ogre/dependencies/freeimage $(LOCAL_PATH)/ogre/Components/RTShaderSystem/include $(LOCAL_PATH)/ogre/RenderSystems/GLES2/include $(LOCAL_PATH)/ogre/RenderSystems/GLES2/include $(LOCAL_PATH)/ogre/RenderSystems/GLES2/src/GLSLES/include $(LOCAL_PATH)/ogre/RenderSystems/GLES2/include/Android $(LOCAL_PATH)/ogre/Plugins/ParticleFX/include
LOCAL_CFLAGS		:= -DFREEIMAGE_LIB=1 -DOGRE_NONCLIENT_BUILD=1
LOCAL_LDLIBS		:= -ldl -llog -lz -lGLESv2
LOCAL_SRC_FILES		:= ogre/OgreMain/src/OgreAlignedAllocator.cpp\
	ogre/OgreMain/src/OgreAnimable.cpp\
	ogre/OgreMain/src/OgreAnimation.cpp\
	ogre/OgreMain/src/OgreAnimationState.cpp\
	ogre/OgreMain/src/OgreAnimationTrack.cpp\
	ogre/OgreMain/src/OgreArchiveManager.cpp\
	ogre/OgreMain/src/OgreAutoParamDataSource.cpp\
	ogre/OgreMain/src/OgreAxisAlignedBox.cpp\
	ogre/OgreMain/src/OgreBillboard.cpp\
	ogre/OgreMain/src/OgreBillboardChain.cpp\
	ogre/OgreMain/src/OgreBillboardParticleRenderer.cpp\
	ogre/OgreMain/src/OgreBillboardSet.cpp\
	ogre/OgreMain/src/OgreBone.cpp\
	ogre/OgreMain/src/OgreBorderPanelOverlayElement.cpp\
	ogre/OgreMain/src/OgreCamera.cpp\
	ogre/OgreMain/src/OgreCodec.cpp\
	ogre/OgreMain/src/OgreColourValue.cpp\
	ogre/OgreMain/src/OgreCommon.cpp\
	ogre/OgreMain/src/OgreCompositionPass.cpp\
	ogre/OgreMain/src/OgreCompositionTargetPass.cpp\
	ogre/OgreMain/src/OgreCompositionTechnique.cpp\
	ogre/OgreMain/src/OgreCompositor.cpp\
	ogre/OgreMain/src/OgreCompositorChain.cpp\
	ogre/OgreMain/src/OgreCompositorInstance.cpp\
	ogre/OgreMain/src/OgreCompositorManager.cpp\
	ogre/OgreMain/src/OgreConfigFile.cpp\
	ogre/OgreMain/src/OgreControllerManager.cpp\
	ogre/OgreMain/src/OgreConvexBody.cpp\
	ogre/OgreMain/src/OgreDataStream.cpp\
	ogre/OgreMain/src/OgreDDSCodec.cpp\
	ogre/OgreMain/src/OgreDefaultHardwareBufferManager.cpp\
	ogre/OgreMain/src/OgreDefaultSceneQueries.cpp\
	ogre/OgreMain/src/OgreDepthBuffer.cpp\
	ogre/OgreMain/src/OgreDistanceLodStrategy.cpp\
	ogre/OgreMain/src/OgreDynLib.cpp\
	ogre/OgreMain/src/OgreDynLibManager.cpp\
	ogre/OgreMain/src/OgreEdgeListBuilder.cpp\
	ogre/OgreMain/src/OgreEntity.cpp\
	ogre/OgreMain/src/OgreException.cpp\
	ogre/OgreMain/src/OgreExternalTextureSource.cpp\
	ogre/OgreMain/src/OgreExternalTextureSourceManager.cpp\
	ogre/OgreMain/src/OgreFileSystem.cpp\
	ogre/OgreMain/src/OgreFont.cpp\
	ogre/OgreMain/src/OgreFontManager.cpp\
	ogre/OgreMain/src/OgreFreeImageCodec.cpp\
	ogre/OgreMain/src/OgreFrustum.cpp\
	ogre/OgreMain/src/OgreGpuProgram.cpp\
	ogre/OgreMain/src/OgreGpuProgramManager.cpp\
	ogre/OgreMain/src/OgreGpuProgramParams.cpp\
	ogre/OgreMain/src/OgreGpuProgramUsage.cpp\
	ogre/OgreMain/src/OgreHardwareBufferManager.cpp\
	ogre/OgreMain/src/OgreHardwareIndexBuffer.cpp\
	ogre/OgreMain/src/OgreHardwareOcclusionQuery.cpp\
	ogre/OgreMain/src/OgreHardwarePixelBuffer.cpp\
	ogre/OgreMain/src/OgreHardwareVertexBuffer.cpp\
	ogre/OgreMain/src/OgreHighLevelGpuProgram.cpp\
	ogre/OgreMain/src/OgreHighLevelGpuProgramManager.cpp\
	ogre/OgreMain/src/OgreImage.cpp\
	ogre/OgreMain/src/OgreInstancedGeometry.cpp\
	ogre/OgreMain/src/OgreKeyFrame.cpp\
	ogre/OgreMain/src/OgreLight.cpp\
	ogre/OgreMain/src/OgreLodStrategy.cpp\
	ogre/OgreMain/src/OgreLodStrategyManager.cpp\
	ogre/OgreMain/src/OgreLog.cpp\
	ogre/OgreMain/src/OgreLogManager.cpp\
	ogre/OgreMain/src/OgreManualObject.cpp\
	ogre/OgreMain/src/OgreMaterial.cpp\
	ogre/OgreMain/src/OgreMaterialManager.cpp\
	ogre/OgreMain/src/OgreMaterialSerializer.cpp\
	ogre/OgreMain/src/OgreMath.cpp\
	ogre/OgreMain/src/OgreMatrix3.cpp\
	ogre/OgreMain/src/OgreMatrix4.cpp\
	ogre/OgreMain/src/OgreMemoryAllocatedObject.cpp\
	ogre/OgreMain/src/OgreMemoryNedAlloc.cpp\
	ogre/OgreMain/src/OgreMemoryNedPooling.cpp\
	ogre/OgreMain/src/OgreMemoryTracker.cpp\
	ogre/OgreMain/src/OgreMesh.cpp\
	ogre/OgreMain/src/OgreMeshManager.cpp\
	ogre/OgreMain/src/OgreMeshSerializer.cpp\
	ogre/OgreMain/src/OgreMeshSerializerImpl.cpp\
	ogre/OgreMain/src/OgreMovableObject.cpp\
	ogre/OgreMain/src/OgreMovablePlane.cpp\
	ogre/OgreMain/src/OgreNode.cpp\
	ogre/OgreMain/src/OgreNumerics.cpp\
	ogre/OgreMain/src/OgreOptimisedUtil.cpp\
	ogre/OgreMain/src/OgreOptimisedUtilGeneral.cpp\
	ogre/OgreMain/src/OgreOptimisedUtilSSE.cpp\
	ogre/OgreMain/src/OgreOverlay.cpp\
	ogre/OgreMain/src/OgreOverlayContainer.cpp\
	ogre/OgreMain/src/OgreOverlayElement.cpp\
	ogre/OgreMain/src/OgreOverlayElementCommands.cpp\
	ogre/OgreMain/src/OgreOverlayManager.cpp\
	ogre/OgreMain/src/OgrePanelOverlayElement.cpp\
	ogre/OgreMain/src/OgreParticle.cpp\
	ogre/OgreMain/src/OgreParticleEmitter.cpp\
	ogre/OgreMain/src/OgreParticleEmitterCommands.cpp\
	ogre/OgreMain/src/OgreParticleIterator.cpp\
	ogre/OgreMain/src/OgreParticleSystem.cpp\
	ogre/OgreMain/src/OgreParticleSystemManager.cpp\
	ogre/OgreMain/src/OgrePass.cpp\
	ogre/OgreMain/src/OgrePatchMesh.cpp\
	ogre/OgreMain/src/OgrePatchSurface.cpp\
	ogre/OgreMain/src/OgrePixelCountLodStrategy.cpp\
	ogre/OgreMain/src/OgrePixelFormat.cpp\
	ogre/OgreMain/src/OgrePlane.cpp\
	ogre/OgreMain/src/OgrePlatformInformation.cpp\
	ogre/OgreMain/src/OgrePolygon.cpp\
	ogre/OgreMain/src/OgrePose.cpp\
	ogre/OgreMain/src/OgrePrecompiledHeaders.cpp\
	ogre/OgreMain/src/OgrePredefinedControllers.cpp\
	ogre/OgreMain/src/OgrePrefabFactory.cpp\
	ogre/OgreMain/src/OgreProfiler.cpp\
	ogre/OgreMain/src/OgreProgressiveMesh.cpp\
	ogre/OgreMain/src/OgrePVRTCCodec.cpp\
	ogre/OgreMain/src/OgreQuaternion.cpp\
	ogre/OgreMain/src/OgreRectangle2D.cpp\
	ogre/OgreMain/src/OgreRenderQueue.cpp\
	ogre/OgreMain/src/OgreRenderQueueInvocation.cpp\
	ogre/OgreMain/src/OgreRenderQueueSortingGrouping.cpp\
	ogre/OgreMain/src/OgreRenderSystem.cpp\
	ogre/OgreMain/src/OgreRenderSystemCapabilities.cpp\
	ogre/OgreMain/src/OgreRenderSystemCapabilitiesManager.cpp\
	ogre/OgreMain/src/OgreRenderSystemCapabilitiesSerializer.cpp\
	ogre/OgreMain/src/OgreRenderTarget.cpp\
	ogre/OgreMain/src/OgreRenderTexture.cpp\
	ogre/OgreMain/src/OgreRenderToVertexBuffer.cpp\
	ogre/OgreMain/src/OgreRenderWindow.cpp\
	ogre/OgreMain/src/OgreResource.cpp\
	ogre/OgreMain/src/OgreResourceBackgroundQueue.cpp\
	ogre/OgreMain/src/OgreResourceGroupManager.cpp\
	ogre/OgreMain/src/OgreResourceManager.cpp\
	ogre/OgreMain/src/OgreRibbonTrail.cpp\
	ogre/OgreMain/src/OgreRoot.cpp\
	ogre/OgreMain/src/OgreRotationSpline.cpp\
	ogre/OgreMain/src/OgreSceneManager.cpp\
	ogre/OgreMain/src/OgreSceneManagerEnumerator.cpp\
	ogre/OgreMain/src/OgreSceneNode.cpp\
	ogre/OgreMain/src/OgreSceneQuery.cpp\
	ogre/OgreMain/src/OgreScriptCompiler.cpp\
	ogre/OgreMain/src/OgreScriptLexer.cpp\
	ogre/OgreMain/src/OgreScriptParser.cpp\
	ogre/OgreMain/src/OgreScriptTranslator.cpp\
	ogre/OgreMain/src/OgreSearchOps.cpp\
	ogre/OgreMain/src/OgreSerializer.cpp\
	ogre/OgreMain/src/OgreShadowCameraSetup.cpp\
	ogre/OgreMain/src/OgreShadowCameraSetupFocused.cpp\
	ogre/OgreMain/src/OgreShadowCameraSetupLiSPSM.cpp\
	ogre/OgreMain/src/OgreShadowCameraSetupPlaneOptimal.cpp\
	ogre/OgreMain/src/OgreShadowCameraSetupPSSM.cpp\
	ogre/OgreMain/src/OgreShadowCaster.cpp\
	ogre/OgreMain/src/OgreShadowTextureManager.cpp\
	ogre/OgreMain/src/OgreShadowVolumeExtrudeProgram.cpp\
	ogre/OgreMain/src/OgreSimpleRenderable.cpp\
	ogre/OgreMain/src/OgreSimpleSpline.cpp\
	ogre/OgreMain/src/OgreSkeleton.cpp\
	ogre/OgreMain/src/OgreSkeletonInstance.cpp\
	ogre/OgreMain/src/OgreSkeletonManager.cpp\
	ogre/OgreMain/src/OgreSkeletonSerializer.cpp\
	ogre/OgreMain/src/OgreStaticGeometry.cpp\
	ogre/OgreMain/src/OgreStreamSerialiser.cpp\
	ogre/OgreMain/src/OgreString.cpp\
	ogre/OgreMain/src/OgreStringConverter.cpp\
	ogre/OgreMain/src/OgreStringInterface.cpp\
	ogre/OgreMain/src/OgreSubEntity.cpp\
	ogre/OgreMain/src/OgreSubMesh.cpp\
	ogre/OgreMain/src/OgreTagPoint.cpp\
	ogre/OgreMain/src/OgreTangentSpaceCalc.cpp\
	ogre/OgreMain/src/OgreTechnique.cpp\
	ogre/OgreMain/src/OgreTextAreaOverlayElement.cpp\
	ogre/OgreMain/src/OgreTexture.cpp\
	ogre/OgreMain/src/OgreTextureManager.cpp\
	ogre/OgreMain/src/OgreTextureUnitState.cpp\
	ogre/OgreMain/src/OgreUnifiedHighLevelGpuProgram.cpp\
	ogre/OgreMain/src/OgreUserObjectBindings.cpp\
	ogre/OgreMain/src/OgreUTFString.cpp\
	ogre/OgreMain/src/OgreVector2.cpp\
	ogre/OgreMain/src/OgreVector3.cpp\
	ogre/OgreMain/src/OgreVector4.cpp\
	ogre/OgreMain/src/OgreVertexIndexData.cpp\
	ogre/OgreMain/src/OgreViewport.cpp\
	ogre/OgreMain/src/OgreWindowEventUtilities.cpp\
	ogre/OgreMain/src/OgreWireBoundingBox.cpp\
	ogre/OgreMain/src/OgreWorkQueue.cpp\
	ogre/OgreMain/src/OgreStringSerialiser.cpp\
	ogre/OgreMain/src/Android/OgreTimer.cpp\
	ogre/OgreMain/src/Android/OgreConfigDialog.cpp\
	ogre/OgreMain/src/Android/OgreErrorDialog.cpp\
	ogre/OgreMain/src/Threading/OgreDefaultWorkQueueStandard.cpp\
	ogre/Components/RTShaderSystem/src/OgreShaderExIntegratedPSSM3.cpp\
	ogre/Components/RTShaderSystem/src/OgreShaderExLayeredBlending.cpp\
	ogre/Components/RTShaderSystem/src/OgreShaderExNormalMapLighting.cpp\
	ogre/Components/RTShaderSystem/src/OgreShaderExPerPixelLighting.cpp\
	ogre/Components/RTShaderSystem/src/OgreShaderExHardwareSkinning.cpp\
	ogre/Components/RTShaderSystem/src/OgreShaderFFPColour.cpp\
	ogre/Components/RTShaderSystem/src/OgreShaderFFPFog.cpp\
	ogre/Components/RTShaderSystem/src/OgreShaderFFPLighting.cpp\
	ogre/Components/RTShaderSystem/src/OgreShaderFFPRenderStateBuilder.cpp\
	ogre/Components/RTShaderSystem/src/OgreShaderFFPTexturing.cpp\
	ogre/Components/RTShaderSystem/src/OgreShaderFFPTransform.cpp\
	ogre/Components/RTShaderSystem/src/OgreShaderFunction.cpp\
	ogre/Components/RTShaderSystem/src/OgreShaderFunctionAtom.cpp\
	ogre/Components/RTShaderSystem/src/OgreShaderGenerator.cpp\
	ogre/Components/RTShaderSystem/src/OgreShaderGLSLESProgramProcessor.cpp\
	ogre/Components/RTShaderSystem/src/OgreShaderGLSLESProgramWriter.cpp\
	ogre/Components/RTShaderSystem/src/OgreShaderMaterialSerializerListener.cpp\
	ogre/Components/RTShaderSystem/src/OgreShaderParameter.cpp\
	ogre/Components/RTShaderSystem/src/OgreShaderProgram.cpp\
	ogre/Components/RTShaderSystem/src/OgreShaderProgramManager.cpp\
	ogre/Components/RTShaderSystem/src/OgreShaderProgramProcessor.cpp\
	ogre/Components/RTShaderSystem/src/OgreShaderProgramSet.cpp\
	ogre/Components/RTShaderSystem/src/OgreShaderProgramWriter.cpp\
	ogre/Components/RTShaderSystem/src/OgreShaderProgramWriterManager.cpp\
	ogre/Components/RTShaderSystem/src/OgreShaderRenderState.cpp\
	ogre/Components/RTShaderSystem/src/OgreShaderScriptTranslator.cpp\
	ogre/Components/RTShaderSystem/src/OgreShaderSubRenderState.cpp\
	ogre/RenderSystems/GLES2/src/OgreGLES2Context.cpp\
	ogre/RenderSystems/GLES2/src/OgreGLES2DefaultHardwareBufferManager.cpp\
	ogre/RenderSystems/GLES2/src/OgreGLES2DepthBuffer.cpp\
	ogre/RenderSystems/GLES2/src/OgreGLES2EngineDLL.cpp\
	ogre/RenderSystems/GLES2/src/OgreGLES2FBOMultiRenderTarget.cpp\
	ogre/RenderSystems/GLES2/src/OgreGLES2FBORenderTexture.cpp\
	ogre/RenderSystems/GLES2/src/OgreGLES2FrameBufferObject.cpp\
	ogre/RenderSystems/GLES2/src/OgreGLES2GpuProgram.cpp\
	ogre/RenderSystems/GLES2/src/OgreGLES2GpuProgramManager.cpp\
	ogre/RenderSystems/GLES2/src/OgreGLES2HardwareBufferManager.cpp\
	ogre/RenderSystems/GLES2/src/OgreGLES2HardwareIndexBuffer.cpp\
	ogre/RenderSystems/GLES2/src/OgreGLES2HardwarePixelBuffer.cpp\
	ogre/RenderSystems/GLES2/src/OgreGLES2HardwareVertexBuffer.cpp\
	ogre/RenderSystems/GLES2/src/OgreGLES2PixelFormat.cpp\
	ogre/RenderSystems/GLES2/src/OgreGLES2Plugin.cpp\
	ogre/RenderSystems/GLES2/src/OgreGLES2RenderSystem.cpp\
	ogre/RenderSystems/GLES2/src/OgreGLES2RenderTexture.cpp\
	ogre/RenderSystems/GLES2/src/OgreGLES2Support.cpp\
	ogre/RenderSystems/GLES2/src/OgreGLES2Texture.cpp\
	ogre/RenderSystems/GLES2/src/OgreGLES2TextureManager.cpp\
	ogre/RenderSystems/GLES2/src/GLSLES/src/OgreGLSLESExtSupport.cpp\
	ogre/RenderSystems/GLES2/src/GLSLES/src/OgreGLSLESGpuProgram.cpp\
	ogre/RenderSystems/GLES2/src/GLSLES/src/OgreGLSLESLinkProgram.cpp\
	ogre/RenderSystems/GLES2/src/GLSLES/src/OgreGLSLESLinkProgramManager.cpp\
	ogre/RenderSystems/GLES2/src/GLSLES/src/OgreGLSLESPreprocessor.cpp\
	ogre/RenderSystems/GLES2/src/GLSLES/src/OgreGLSLESProgram.cpp\
	ogre/RenderSystems/GLES2/src/GLSLES/src/OgreGLSLESProgramFactory.cpp\
	ogre/RenderSystems/GLES2/src/Android/OgreAndroidGLContext.cpp\
	ogre/RenderSystems/GLES2/src/Android/OgreAndroidGLSupport.cpp\
	ogre/RenderSystems/GLES2/src/Android/OgreAndroidWindow.cpp\
	ogre/PlugIns/ParticleFX/src/OgreAreaEmitter.cpp\
	ogre/PlugIns/ParticleFX/src/OgreBoxEmitter.cpp\
	ogre/PlugIns/ParticleFX/src/OgreColourFaderAffector.cpp\
	ogre/PlugIns/ParticleFX/src/OgreColourFaderAffector2.cpp\
	ogre/PlugIns/ParticleFX/src/OgreColourImageAffector.cpp\
	ogre/PlugIns/ParticleFX/src/OgreColourInterpolatorAffector.cpp\
	ogre/PlugIns/ParticleFX/src/OgreCylinderEmitter.cpp\
	ogre/PlugIns/ParticleFX/src/OgreDeflectorPlaneAffector.cpp\
	ogre/PlugIns/ParticleFX/src/OgreDirectionRandomiserAffector.cpp\
	ogre/PlugIns/ParticleFX/src/OgreEllipsoidEmitter.cpp\
	ogre/PlugIns/ParticleFX/src/OgreHollowEllipsoidEmitter.cpp\
	ogre/PlugIns/ParticleFX/src/OgreLinearForceAffector.cpp\
	ogre/PlugIns/ParticleFX/src/OgreParticleFXPlugin.cpp\
	ogre/PlugIns/ParticleFX/src/OgrePointEmitter.cpp\
	ogre/PlugIns/ParticleFX/src/OgreRingEmitter.cpp\
	ogre/PlugIns/ParticleFX/src/OgreRotationAffector.cpp\
	ogre/PlugIns/ParticleFX/src/OgreScaleAffector.cpp

LOCAL_STATIC_LIBRARIES := libfreeimage libft2 libzzip
	
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE    	:= libois
LOCAL_C_INCLUDES	:= $(LOCAL_PATH)/OIS/includes
LOCAL_SRC_FILES 	:= OIS/src/OISEffect.cpp OIS/src/OISException.cpp OIS/src/OISForceFeedback.cpp OIS/src/OISInputManager OIS/src/OISJoystick.cpp OIS/src/OISKeyboard.cpp OIS/src/OISObject.cpp

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE    	:= libogresamplebrowser
LOCAL_C_INCLUDES	:= $(LOCAL_PATH)/ogre $(LOCAL_PATH)/ogre/OgreMain/include $(LOCAL_PATH)/freeimage $(LOCAL_PATH)/freetype/freetype/include $(LOCAL_PATH)/ogre/RenderSystems/GLES2/include $(LOCAL_PATH)/ogre/Components/RTShaderSystem/include $(LOCAL_PATH)/ogre/RenderSystems/GLES2/include/Android $(LOCAL_PATH)/ogre/PlugIns/ParticleFX/include $(LOCAL_PATH)/ogre/Samples/Common/include $(LOCAL_PATH)/OIS/includes $(LOCAL_PATH)/ogre/Samples/Browser/include $(LOCAL_PATH)/ogre/Samples/BezierPatch/include $(LOCAL_PATH)/ogre/Samples/CameraTrack/include $(LOCAL_PATH)/ogre/Samples/CelShading/include $(LOCAL_PATH)/ogre/Samples/Character/include $(LOCAL_PATH)/ogre/Samples/Compositor/include $(LOCAL_PATH)/ogre/Samples/CubeMapping/include $(LOCAL_PATH)/ogre/Samples/Dot3Bump/include $(LOCAL_PATH)/ogre/Samples/DynTex/include $(LOCAL_PATH)/ogre/Samples/FacialAnimation/include $(LOCAL_PATH)/ogre/Samples/Fresnel/include $(LOCAL_PATH)/ogre/Samples/Grass/include $(LOCAL_PATH)/ogre/Samples/Lighting/include $(LOCAL_PATH)/ogre/Samples/OceanDemo/include $(LOCAL_PATH)/ogre/Samples/ParticleFX/include $(LOCAL_PATH)/ogre/Samples/ShaderSystem/include $(LOCAL_PATH)/ogre/Samples/SkyBox/include $(LOCAL_PATH)/ogre/Samples/SkyDome/include $(LOCAL_PATH)/ogre/Samples/SkyPlane/include $(LOCAL_PATH)/ogre/Samples/Smoke/include $(LOCAL_PATH)/ogre/Samples/SphereMapping/include $(LOCAL_PATH)/ogre/Samples/TextureFX/include $(LOCAL_PATH)/ogre/Samples/Transparency/include $(LOCAL_PATH)/ogre/Samples/Water/include $(LOCAL_PATH)/nv_util
LOCAL_CFLAGS		+= -DFREEIMAGE_LIB=1 -DPNG_STATIC=1 -DOGRE_STATIC_LIB=1 -DUSE_RTSHADER_SYSTEM=1
LOCAL_LDLIBS		:= -llog -lz
LOCAL_SRC_FILES 	:= acpwrapper.cpp ACPChunk.cpp ACPFile.cpp AndroidLogListener.cpp AndroidArchive.cpp ogrewrapper.cpp ogresamplebrowser.cpp  ogre/Samples/Browser/src/FileSystemLayerImpl_Android.cpp ogre/Samples/BezierPatch/src/BezierPatch.cpp ogre/Samples/CameraTrack/src/CameraTrack.cpp ogre/Samples/CelShading/src/CelShading.cpp ogre/Samples/Character/src/CharacterSample.cpp ogre/Samples/Character/src/Sinbad.cpp ogre/Samples/CubeMapping/src/CubeMapping.cpp ogre/Samples/Dot3Bump/src/Dot3Bump.cpp ogre/Samples/DynTex/src/DynTex.cpp ogre/Samples/FacialAnimation/src/FacialAnimation.cpp ogre/Samples/Fresnel/src/Fresnel.cpp ogre/Samples/Grass/src/Grass.cpp ogre/Samples/Lighting/src/Lighting.cpp ogre/Samples/OceanDemo/src/OceanDemo.cpp ogre/Samples/OceanDemo/src/MaterialControls.cpp  ogre/Samples/ParticleFX/src/ParticleFX.cpp ogre/Samples/ShaderSystem/src/ShaderSystem.cpp ogre/Samples/ShaderSystem/src/ShaderExReflectionMap.cpp  ogre/Samples/SkyBox/src/SkyBox.cpp ogre/Samples/SkyDome/src/SkyDome.cpp ogre/Samples/SkyPlane/src/SkyPlane.cpp ogre/Samples/Smoke/src/Smoke.cpp ogre/Samples/SphereMapping/src/SphereMapping.cpp ogre/Samples/TextureFX/src/TextureFX.cpp ogre/Samples/Transparency/src/Transparency.cpp ogre/Samples/Water/src/Water.cpp ogre/Samples/Water/src/WaterMesh.cpp nv_util/nv_util.cpp

LOCAL_SHARED_LIBRARIES := libogre libois

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
include $(LOCAL_PATH)/freetype/freetype/Android.mk