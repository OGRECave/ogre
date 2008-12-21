INCLUDES = $(STLPORT_CFLAGS) $(FT2_CFLAGS) -I$(top_srcdir)/OgreMain/include -DOGRE_NONCLIENT_BUILD $(PLATFORM_CFLAGS) $(ZZIPLIB_CFLAGS) $(DEVIL_CFLAGS) $(FREEIMAGE_CFLAGS) $(GTK_CFLAGS) $(CFLAGS_VISIBILITY) -I$(top_srcdir)/OgreMain/src/nedmalloc

EXTRA_LTLIBRARIES = libSSEsupport.la
libSSEsupport_la_SOURCES = OgreOptimisedUtilSSE.cpp
libSSEsupport_la_CPPFLAGS = -msse

lib_LTLIBRARIES=libOgreMain.la

libOgreMain_la_SOURCES = OgreAlignedAllocator.cpp \
                         OgreAnimation.cpp \
			 OgreAnimable.cpp \
                         OgreAnimationState.cpp \
                         OgreAnimationTrack.cpp \
                         OgreArchiveManager.cpp \
			 OgreAutoParamDataSource.cpp \
			 OgreAxisAlignedBox.cpp \
                         OgreBillboard.cpp \
			 OgreBillboardChain.cpp \
                         OgreBillboardParticleRenderer.cpp \
                         OgreBillboardSet.cpp \
                         OgreBone.cpp \
                         OgreBorderPanelOverlayElement.cpp \
                         OgreCamera.cpp \
                         OgreCodec.cpp \
                         OgreColourValue.cpp \
                         OgreCommon.cpp \
                         OgreConfigFile.cpp \
                         OgreControllerManager.cpp \
						 OgreConvexBody.cpp \
                         OgreDataStream.cpp \
                         OgreDDSCodec.cpp \
                         OgreDefaultHardwareBufferManager.cpp \
			 OgreDefaultSceneQueries.cpp \
			 OgreDistanceLodStrategy.cpp \
                         OgreDynLib.cpp \
                         OgreDynLibManager.cpp \
                         OgreEdgeListBuilder.cpp \
                         OgreEntity.cpp \
                         OgreException.cpp \
			 OgreExternalTextureSource.cpp \
			 OgreExternalTextureSourceManager.cpp \
                         OgreFileSystem.cpp \
                         OgreFont.cpp \
                         OgreFontManager.cpp \
			 OgreFrustum.cpp \
                         OgreGpuProgram.cpp \
                         OgreGpuProgramManager.cpp \
                         OgreGpuProgramUsage.cpp \
			 OgreHardwareBufferManager.cpp \
			 OgreHardwareIndexBuffer.cpp \
			 OgreHardwareOcclusionQuery.cpp \
			 OgreHardwareVertexBuffer.cpp \
			 OgreHardwarePixelBuffer.cpp \
			 OgreHighLevelGpuProgram.cpp \
			 OgreHighLevelGpuProgramManager.cpp \
                         OgreImage.cpp \
						 OgreInstancedGeometry.cpp \
                         OgreKeyFrame.cpp \
                         OgreLight.cpp \
                         OgreLodStrategy.cpp \
                         OgreLodStrategyManager.cpp \
                         OgreLog.cpp \
                         OgreLogManager.cpp \
			 OgreManualObject.cpp \
                         OgreMaterial.cpp \
                         OgreMaterialManager.cpp \
                         OgreMaterialSerializer.cpp \
                         OgreMath.cpp \
                         OgreMatrix3.cpp \
                         OgreMatrix4.cpp \
                         OgreMemoryAllocatedObject.cpp \
						 OgreMemoryNedAlloc.cpp \
						 OgreMemoryTracker.cpp \
                         OgreMesh.cpp \
                         OgreMeshManager.cpp \
                         OgreMeshSerializer.cpp \
			 OgreMeshSerializerImpl.cpp \
                         OgreMovableObject.cpp \
			 OgreMovablePlane.cpp \
                         OgreNode.cpp \
						 OgreNumerics.cpp \
                         OgreOptimisedUtil.cpp \
                         OgreOptimisedUtilGeneral.cpp \
                         OgreOverlay.cpp \
                         OgreOverlayContainer.cpp \
			 OgreOverlayElement.cpp \
                         OgreOverlayElementCommands.cpp \
                         OgreOverlayManager.cpp \
                         OgrePixelFormat.cpp \
                         OgrePanelOverlayElement.cpp \
                         OgreParticle.cpp \
                         OgreParticleEmitter.cpp \
                         OgreParticleEmitterCommands.cpp \
                         OgreParticleIterator.cpp \
                         OgreParticleSystem.cpp \
                         OgreParticleSystemManager.cpp \
                         OgrePass.cpp \
			 OgrePatchMesh.cpp \
                         OgrePatchSurface.cpp \
                         OgrePixelCountLodStrategy.cpp \
                         OgrePlane.cpp \
                         OgrePlatformInformation.cpp \
						 OgrePolygon.cpp \
			 OgrePose.cpp \
                         OgrePredefinedControllers.cpp \
						 OgrePrefabFactory.cpp \
			 OgreProfiler.cpp \
                         OgreProgressiveMesh.cpp \
                         OgreQuaternion.cpp \
			 OgreRectangle2D.cpp \
                         OgreRenderQueue.cpp \
			 OgreRenderQueueInvocation.cpp \
			 OgreRenderQueueSortingGrouping.cpp \
                         OgreRenderSystem.cpp \
                         OgreRenderSystemCapabilities.cpp \
                         OgreRenderSystemCapabilitiesManager.cpp \
                         OgreRenderSystemCapabilitiesSerializer.cpp \
                         OgreRenderTarget.cpp \
                         OgreRenderTexture.cpp \
                         OgreRenderToVertexBuffer.cpp \
                         OgreRenderWindow.cpp \
                         OgreResource.cpp \
                         OgreResourceBackgroundQueue.cpp \
                         OgreResourceGroupManager.cpp \
                         OgreResourceManager.cpp \
			 OgreRibbonTrail.cpp \
                         OgreRoot.cpp \
                         OgreRotationSpline.cpp \
                         OgreSceneManager.cpp \
                         OgreSceneManagerEnumerator.cpp \
                         OgreSceneNode.cpp \
                         OgreSceneQuery.cpp \
                         OgreScriptCompiler.cpp \
                         OgreScriptLexer.cpp \
                         OgreScriptParser.cpp \
                         OgreScriptTranslator.cpp \
                         OgreSearchOps.cpp \
                         OgreSerializer.cpp \
						 OgreShadowCameraSetup.cpp \
						 OgreShadowCameraSetupFocused.cpp \
						 OgreShadowCameraSetupLiSPSM.cpp \
						 OgreShadowCameraSetupPlaneOptimal.cpp \
						 OgreShadowCameraSetupPSSM.cpp \
			 OgreShadowCaster.cpp \
			 OgreShadowTextureManager.cpp \
			 OgreShadowVolumeExtrudeProgram.cpp \
                         OgreSimpleRenderable.cpp \
                         OgreSimpleSpline.cpp \
                         OgreSkeleton.cpp \
                         OgreSkeletonInstance.cpp \
                         OgreSkeletonManager.cpp \
                         OgreSkeletonSerializer.cpp \
			 OgreStaticGeometry.cpp \
                         OgreString.cpp \
                         OgreStringConverter.cpp \
                         OgreStringInterface.cpp \
                         OgreSubEntity.cpp \
                         OgreSubMesh.cpp \
                         OgreTagPoint.cpp \
						 OgreTangentSpaceCalc.cpp \
                         OgreTechnique.cpp \
                         OgreTextAreaOverlayElement.cpp \
                         OgreTexture.cpp \
                         OgreTextureManager.cpp \
                         OgreTextureUnitState.cpp \
						 OgreUnifiedHighLevelGpuProgram.cpp \
                         OgreUserDefinedObject.cpp \
                         OgreVector2.cpp \
                         OgreVector3.cpp \
                         OgreVector4.cpp \
                         OgreVertexIndexData.cpp \
                         OgreViewport.cpp \
                         OgreWireBoundingBox.cpp \
                         OgreZip.cpp \
                         OgreCompositionPass.cpp \
                         OgreCompositionTargetPass.cpp \
                         OgreCompositionTechnique.cpp \
                         OgreCompositor.cpp \
                         OgreCompositorChain.cpp \
                         OgreCompositorInstance.cpp \
                         OgreCompositorManager.cpp \
                         OgreCompositorSerializer.cpp \
                         OgreCompositorScriptCompiler.cpp \
                         OgreCompiler2Pass.cpp \
                         $(OGRE_PLATFORM)/OgreTimer.cpp \
                         $(OGRE_GUI)/OgreConfigDialog.cpp \
                         $(OGRE_GUI)/OgreErrorDialog.cpp \
                         OgreWindowEventUtilities.cpp

if USE_FREEIMAGE
libOgreMain_la_SOURCES += OgreFreeImageCodec.cpp

endif

if USE_DEVIL
libOgreMain_la_SOURCES +=OgreILImageCodec.cpp \
                         OgreILCodecs.cpp \
                         OgreILUtil.cpp
endif

platformdir = $(libdir)
libOgreMain_la_LIBADD= $(STLPORT_LIBS) $(FT2_LIBS) $(ZZIPLIB_LIBS) $(GTK_LIBS) $(OGRE_THREAD_LIBS) $(PLATFORM_LIBS)
libOgreMain_la_LDFLAGS = $(SHARED_FLAGS) -release @PACKAGE_VERSION@ -Wl,-rpath,$(platformdir)
#-version-info @OGREMAIN_VERSION_INFO@ -Wl,-rpath,$(platformdir)
#-Wl,--version-script=$(top_srcdir)/OgreMain/src/ogremain.map

if OGRE_BUILD_SSE
libOgreMain_la_LIBADD += libSSEsupport.la
endif
