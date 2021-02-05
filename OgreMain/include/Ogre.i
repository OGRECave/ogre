 %module(package="Ogre", directors="1") Ogre
 %{
 /* Includes the header in the wrapper code */
#include "Ogre.h"
#include "OgreArchiveFactory.h"
#include "OgreRectangle2D.h"
#include "OgreWireBoundingBox.h"
#include "OgreVertexBoneAssignment.h"
#include "OgreParticleSystemRenderer.h"
#include "OgreParticleEmitterFactory.h"
#include "OgreParticleAffectorFactory.h"
#include "OgreUnifiedHighLevelGpuProgram.h"
#include "OgreScriptCompiler.h"
#include "OgreConfigDialog.h"
#include "OgrePredefinedControllers.h"
#include "OgrePixelCountLodStrategy.h"
#include "OgreDefaultDebugDrawer.h"
%}

#ifdef SWIGPYTHON
%include std_multimap.i
%include std_list.i
#endif
 
%import "Core.i"
 
/* Parse the header file to generate wrappers */
%feature("director") *::Listener;
#ifdef SWIGPYTHON
// should be turned on globally if all renames are in place
%feature("flatnested") Ogre::MaterialManager::Listener;
%feature("flatnested") Ogre::SceneManager::Listener;
#endif

#ifdef __ANDROID__
%{
#include <android/native_window_jni.h>
static JavaVM *cached_jvm = 0;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved) {
  cached_jvm = jvm;
  return JNI_VERSION_1_2;
}

JNIEnv* OgreJNIGetEnv() {
  JNIEnv *env;
  jint rc = cached_jvm->GetEnv((void **)&env, JNI_VERSION_1_2);
  if (rc == JNI_EDETACHED)
    throw std::runtime_error("current thread not attached");
  if (rc == JNI_EVERSION)
    throw std::runtime_error("jni version not supported");
  return env;
}
%}
#endif


#ifdef SWIGCSHARP
%ignore Ogre::HardwareBuffer::UsageEnum;
%ignore Ogre::TextureUsage;
%ignore Ogre::GpuConstantType;
%ignore Ogre::Capabilities;
%typemap(csbase) Ogre::SceneManager::QueryTypeMask "uint";
%csmethodmodifiers *::ToString "public override";
// wrong "override" because of multiple inheritance
%csmethodmodifiers *::getMaterial "public";
%csmethodmodifiers *::getSquaredViewDepth "public";
%csmethodmodifiers *::getWorldTransforms "public";
%csmethodmodifiers *::getRenderOperation "public";
%csmethodmodifiers *::getLights "public";
%csmethodmodifiers *::queryResult "public";
%csmethodmodifiers *::createInstance "public";
%csmethodmodifiers *::loadingComplete "public";
%csmethodmodifiers *::getBoundingBox "public";
%csmethodmodifiers *::getBoundingRadius "public";
%csmethodmodifiers *::getMovableType "public";
%csmethodmodifiers *::visitRenderables "public";
%csmethodmodifiers *::loadResource "public";
%csmethodmodifiers *::createAnimation "public";
%csmethodmodifiers *::getAnimation "public";
%csmethodmodifiers *::hasAnimation "public";
%csmethodmodifiers *::removeAnimation "public";
%csmethodmodifiers *::getNumAnimations "public";
%csmethodmodifiers *::getAnimation "public";
%csmethodmodifiers *::_notifyCurrentCamera "public";
%csmethodmodifiers *::_updateRenderQueue "public";
%csmethodmodifiers *::_notifyAttached "public";
%csmethodmodifiers *::setRenderQueueGroup "public";
%csmethodmodifiers *::setRenderQueueGroupAndPriority "public";
%csmethodmodifiers *::getTypeFlags "public";
%csmethodmodifiers *::viewportDestroyed "public";
%csmethodmodifiers *::viewportDimensionsChanged "public";
%csmethodmodifiers *::viewportCameraChanged "public";
%csmethodmodifiers Ogre::BillboardChain::preRender "public";
#endif

#ifdef SWIGPYTHON
    #define XSTR(x) #x
    #define STR(x) XSTR(x)
    #define __version__ STR(OGRE_VERSION_MAJOR) "." STR(OGRE_VERSION_MINOR) "." STR(OGRE_VERSION_PATCH)
    #undef STR
    #undef XSTR
#endif

// stringinterface internal
%rename("$ignore", regextarget=1) "^Cmd+";

/* these are ordered by dependency */
// Configfile
%include "OgreConfigOptionMap.h"
%template(ConfigOptionMap) std::map<Ogre::String, Ogre::ConfigOption>;
%ignore Ogre::ConfigFile::load; // conflicting overloads
%ignore Ogre::ConfigFile::getSettingsIterator; // deprecated
%ignore Ogre::ConfigFile::getSectionIterator;
%template(SettingsBySection) std::map<Ogre::String, std::multimap< Ogre::String, Ogre::String> >;
#ifdef SWIGPYTHON
%template(SettingsMultiMap) std::multimap<Ogre::String, Ogre::String>;
#endif
%include "OgreConfigFile.h"

%include "OgreSerializer.h"
%include "OgreScriptLoader.h"
// Listeners
%feature("director") Ogre::FrameListener;
%include "OgreFrameListener.h"
%feature("director") Ogre::LodListener;
%include "OgreLodListener.h"
%feature("director") Ogre::RenderObjectListener;
%include "OgreRenderObjectListener.h"
%feature("director") Ogre::RenderQueueListener;
%include "OgreRenderQueueListener.h"
%feature("director") Ogre::RenderTargetListener;
%include "OgreRenderTargetListener.h"
%feature("director") Ogre::MeshSerializerListener;
%feature("director") Ogre::ResourceLoadingListener;
// More Data Types
%include "OgreSceneQuery.h"
%template(RaySceneQueryResult) std::vector<Ogre::RaySceneQueryResultEntry>;

%include "OgreNameGenerator.h"
%include "OgreController.h"

%ignore Ogre::GpuProgramParameters::hasPassIterationNumber; // deprecated
%ignore Ogre::GpuProgramParameters::getPassIterationNumberIndex; // deprecated
%ignore Ogre::GpuProgramParameters::getConstantDefinitionIterator;
%ignore Ogre::GpuSharedParameters::getConstantDefinitionIterator;
%include "OgreGpuProgramParams.h"
%include "OgreBillboard.h"
%include "OgreParticle.h"
%include "OgreHardwareOcclusionQuery.h"
SHARED_PTR(HardwareBuffer);
%include "OgreHardwareBuffer.h"

SHARED_PTR(ParticleAffector);
%include "OgreParticleAffector.h"
    %include "OgreParticleAffectorFactory.h"
//    SHARED_PTR(ParticleEmitter);
%include "OgreParticleEmitter.h"
    %include "OgreParticleEmitterFactory.h"
SHARED_PTR(Resource);
%include "OgreResource.h"
    SHARED_PTR(Texture);
    %include "OgreTexture.h"
    SHARED_PTR(GpuProgram);
    %include "OgreGpuProgram.h"
        SHARED_PTR(HighLevelGpuProgram);
        %include "OgreHighLevelGpuProgram.h"
%include "OgreScriptCompiler.h"
%ignore Ogre::TextureUnitState::setCubicTexture;
%ignore Ogre::TextureUnitState::setCubicTextureName;
%ignore Ogre::TextureUnitState::isCubic;
%ignore Ogre::TextureUnitState::is3D;
%include "OgreTextureUnitState.h"
%template(ControllerReal) Ogre::Controller<Ogre::Real>;
%template(ControllerValueRealPtr) Ogre::SharedPtr<Ogre::ControllerValue<Ogre::Real> >;
%template(ControllerFunctionPtr) Ogre::SharedPtr<Ogre::ControllerFunction<Ogre::Real> >;
%include "OgreControllerManager.h"
%include "OgrePredefinedControllers.h"
SHARED_PTR(Compositor);
%ignore Ogre::Compositor::getTechniqueIterator;
%ignore Ogre::Compositor::getSupportedTechniqueIterator;
%include "OgreCompositor.h"
%ignore Ogre::CompositionTechnique::getNumTextureDefinitions;
%ignore Ogre::CompositionTechnique::getTextureDefinitionIterator;
%ignore Ogre::CompositionTechnique::getTargetPass;
%ignore Ogre::CompositionTechnique::getNumTargetPasses;
%ignore Ogre::CompositionTechnique::getTargetPassIterator;
%include "OgreCompositionTechnique.h"
%ignore Ogre::CompositionTargetPass::getPass;
%ignore Ogre::CompositionTargetPass::getNumPasses;
%ignore Ogre::CompositionTargetPass::getPassIterator;
%include "OgreCompositionTargetPass.h"
%include "OgreResourceBackgroundQueue.h"
SHARED_PTR(HardwareVertexBuffer);
#ifdef SWIGPYTHON
%template(VertexElementList) std::list<Ogre::VertexElement>;
#endif
%include "OgreHardwareVertexBuffer.h"
SHARED_PTR(HardwareIndexBuffer);
%include "OgreHardwareIndexBuffer.h"
SHARED_PTR(HardwarePixelBuffer);
#ifdef SWIGJAVA
%ignore Ogre::HardwarePixelBuffer::lock;  // duplicate definition
#endif
%include "OgreHardwarePixelBuffer.h"
SHARED_PTR(HardwareCounterBuffer);
%include "OgreHardwareCounterBuffer.h"
SHARED_PTR(HardwareUniformBuffer);
%include "OgreHardwareUniformBuffer.h"
%ignore Ogre::HardwareBufferManagerBase::_forceReleaseBufferCopies(HardwareVertexBuffer* sourceBuffer);
%include "OgreHardwareBufferManager.h"
%include "OgreVertexIndexData.h"
// managers
%ignore Ogre::ResourceGroupManager::openResource(const String&, const String&, bool, Resource*) const;
%ignore Ogre::ResourceGroupManager::openResource(const String&, const String&, bool) const;
%ignore Ogre::ResourceGroupManager::getResourceManagerIterator;
%include "OgreResourceGroupManager.h"
#if SWIG_VERSION < 0x030012 || defined(SWIGJAVA)
// conflicts with overloaded versions (GpuProgram, Texture)
%ignore Ogre::ResourceManager::getResourceByName;
%ignore Ogre::ResourceManager::createOrRetrieve;
%ignore Ogre::ResourceManager::prepare;
%ignore Ogre::ResourceManager::load;
#endif
%ignore Ogre::ResourceManager::getResourceIterator;
%include "OgreResourceManager.h"
    // ambiguity as enums are just ints
    %ignore Ogre::TextureManager::createManual(const String&, const String&,TextureType,uint,uint,int,PixelFormat,int);
    %ignore Ogre::TextureManager::createManual(const String&, const String&,TextureType,uint,uint,int,PixelFormat);
    %include "OgreTextureManager.h"
    %include "OgreGpuProgramManager.h"
    %include "OgreHighLevelGpuProgramManager.h"
SHARED_PTR(UnifiedHighLevelGpuProgram);
%include "OgreUnifiedHighLevelGpuProgram.h"
// animations
%ignore Ogre::VertexPoseKeyFrame::getPoseReferenceIterator;
%include "OgreKeyFrame.h"
%ignore Ogre::Pose::getNormalsIterator;
%ignore Ogre::Pose::getVertexOffsetIterator;
%include "OgrePose.h"
%include "OgreAnimationTrack.h"
%ignore Ogre::AnimationStateSet::getAnimationStateIterator;
%ignore Ogre::AnimationStateSet::getEnabledAnimationStateIterator;
%include "OgreAnimationState.h"

#ifdef SWIGPYTHON
%{
    // this is a workaround for the following map with swig 3.0.12
    namespace swig {
    template<> struct traits<Ogre::AnimationState> {
        typedef pointer_category category;
        static const char* type_name() { return "Ogre::AnimationState"; }
    };
    }
%}
#endif
%template(AnimationStateMap) std::map<Ogre::String, Ogre::AnimationState*>;
%ignore Ogre::Animation::getVertexTrackIterator;
%ignore Ogre::Animation::getNodeTrackIterator;
%ignore Ogre::Animation::getNumericTrackIterator;
%include "OgreAnimation.h"
SHARED_PTR(Skeleton);
// deprecated
%ignore Ogre::Skeleton::getRootBone;
%ignore Ogre::Skeleton::getRootBoneIterator;
%ignore Ogre::Skeleton::getBoneIterator;
%ignore Ogre::Skeleton::getLinkedSkeletonAnimationSourceIterator;
%template(BoneList) std::vector<Ogre::Bone*>;
%include "OgreSkeleton.h"
    %ignore Ogre::SkeletonInstance::getLinkedSkeletonAnimationSourceIterator;
    SHARED_PTR(SkeletonInstance);
    %include "OgreSkeletonInstance.h"
%include "OgreSkeletonManager.h"
%include "OgreRenderQueue.h"
%include "OgreRenderOperation.h"
%ignore Ogre::Material::getLodValueIterator;
%ignore Ogre::Material::getUserLodValueIterator;
%ignore Ogre::Material::getSupportedTechniqueIterator;
%ignore Ogre::Material::getTechniqueIterator;
%ignore Ogre::Material::getSupportedTechnique;
%ignore Ogre::Material::getNumSupportedTechniques;
SHARED_PTR(Material);
%template(Techniques) std::vector<Ogre::Technique*>;
%include "OgreMaterial.h"
%ignore Ogre::RenderSystem::_setBindingType;
%ignore Ogre::RenderSystem::_setBindingType;
%ignore Ogre::RenderSystem::_setTextureUnitFiltering;
%ignore Ogre::RenderSystem::_setTextureAddressingMode;
%ignore Ogre::RenderSystem::_setSceneBlending;
%ignore Ogre::RenderSystem::_setFog;
%ignore Ogre::RenderSystem::_setWorldMatrix;
%ignore Ogre::RenderSystem::_setViewMatrix;
%ignore Ogre::RenderSystem::_setProjectionMatrix;
%ignore Ogre::RenderSystem::getRenderTargetIterator;
%include "OgreRenderSystem.h"
%include "OgreCompositorManager.h"
#ifdef SWIGJAVA
%ignore Ogre::CompositorInstance::Listener; // issue with converting shared_ptr<Material>
#endif
%include "OgreCompositorInstance.h"
%include "OgreCompositionPass.h"
%include "OgreMaterialSerializer.h"
%include "OgreUserObjectBindings.h"
%rename(MaterialManager_Listener) Ogre::MaterialManager::Listener;
%include "OgreMaterialManager.h"
%include "OgreRenderable.h"
%ignore Ogre::ShadowCaster::getShadowVolumeRenderableIterator;
%include "OgreShadowCaster.h"
%extend Ogre::MovableObject {
  Entity* castEntity()
  {
    return dynamic_cast<Ogre::Entity*>($self);
  }
}
%include "OgreMovableObject.h"
    %include "OgreBillboardChain.h"
        %ignore Ogre::RibbonTrail::getNodeIterator;
        %include "OgreRibbonTrail.h"
    %include "OgreBillboardSet.h"
    %include "OgreMovablePlane.h"
    %ignore Ogre::Light::setPosition;
    %ignore Ogre::Light::getPosition;
    %ignore Ogre::Light::getDirection;
    %include "OgreLight.h"
    %ignore Ogre::Node::getChildIterator;
    %include "OgreNode.h"
        %include "OgreBone.h"
        %ignore Ogre::SceneNode::getAttachedObjectIterator;
        %template(NodeObjectMap) std::vector<Ogre::MovableObject*>;
        %include "OgreSceneNode.h"
    SHARED_PTR(ShadowCameraSetup);
    SHARED_PTR(DefaultShadowCameraSetup);
    %include "OgreShadowCameraSetup.h"
        SHARED_PTR(FocusedShadowCameraSetup);
        %include "OgreShadowCameraSetupFocused.h"
        SHARED_PTR(LiSPSMShadowCameraSetup);
        %include "OgreShadowCameraSetupLiSPSM.h"
        SHARED_PTR(PlaneOptimalShadowCameraSetup);
        %include "OgreShadowCameraSetupPlaneOptimal.h"
        SHARED_PTR(PSSMShadowCameraSetup);
        %include "OgreShadowCameraSetupPSSM.h"  
    %ignore Ogre::Frustum::getFrustumExtents(Real&, Real& ,Real& ,Real&) const;
    %include "OgreFrustum.h"
        %ignore Ogre::Camera::setPosition;
        %ignore Ogre::Camera::getPosition;
        %ignore Ogre::Camera::setDirection;
        %ignore Ogre::Camera::getDirection;
        %ignore Ogre::Camera::setOrientation;
        %ignore Ogre::Camera::getOrientation;
        %ignore Ogre::Camera::rotate;
        %ignore Ogre::Camera::getUp;
        %ignore Ogre::Camera::getRight;
        %ignore Ogre::Camera::lookAt;
        %ignore Ogre::Camera::roll;
        %ignore Ogre::Camera::yaw;
        %ignore Ogre::Camera::pitch;
        %ignore Ogre::Camera::setFixedYawAxis;
        %ignore Ogre::Camera::setAutoTracking;
        %ignore Ogre::Camera::move;
        %ignore Ogre::Camera::moveRelative;
        %include "OgreCamera.h"
        ADD_REPR(Camera)
    %include "OgreManualObject.h"
    %template(SubEntityList) std::vector<Ogre::SubEntity*>;
    %ignore Ogre::Entity::getAttachedObjectIterator;
    %include "OgreEntity.h"
    %include "OgreSubEntity.h"
    %include "OgreParticleSystemRenderer.h"
    SHARED_PTR(ParticleSystem);
    %ignore Ogre::ParticleSystem::_getIterator;
    %include "OgreParticleSystem.h"
    %include "OgreInstancedEntity.h"
    %include "OgreInstanceBatch.h"
    %ignore Ogre::SimpleRenderable::setMaterial(const String&);
    %include "OgreSimpleRenderable.h"
        %include "OgreRectangle2D.h"
        %include "OgreWireBoundingBox.h"
%ignore Ogre::ParticleSystemManager::getTemplateIterator;
%ignore Ogre::ParticleSystemManager::getEmitterFactoryIterator;
%ignore Ogre::ParticleSystemManager::getAffectorFactoryIterator;
%ignore Ogre::ParticleSystemManager::getRendererFactoryIterator;
%include "OgreParticleSystemManager.h"
%ignore Ogre::InstanceManager::getInstanceBatchIterator;
%ignore Ogre::InstanceManager::getInstanceBatchMapIterator;
%include "OgreInstanceManager.h" 
%include "OgreVertexBoneAssignment.h"
// deprecated
%ignore Ogre::Mesh::getSubMeshIterator;
%ignore Ogre::Mesh::getPoseCount;
%ignore Ogre::Mesh::getPose;
%ignore Ogre::Mesh::getPoseIterator;
%ignore Ogre::Mesh::getBoneAssignmentIterator;
%template(PoseList) std::vector<Ogre::Pose*>;
%template(SubMeshList) std::vector<Ogre::SubMesh*>;
SHARED_PTR(Mesh);
%include "OgreMesh.h"
%ignore Ogre::SubMesh::getBoneAssignmentIterator;
%ignore Ogre::SubMesh::getAliasTextureIterator;
%include "OgreSubMesh.h"
%ignore Ogre::StaticGeometry::getRegionIterator;
%ignore Ogre::StaticGeometry::Region::getLODIterator;
%ignore Ogre::StaticGeometry::MaterialBucket::getGeometryIterator;
%ignore Ogre::StaticGeometry::LODBucket::getMaterialIterator;
%include "OgreStaticGeometry.h"
%include "OgrePatchSurface.h"
    SHARED_PTR(PatchMesh);
    %include "OgrePatchMesh.h"
%include "OgreMeshSerializer.h"
%include "OgreMeshManager.h"
%include "OgreLodStrategy.h"
%include "OgrePixelCountLodStrategy.h"
%ignore Ogre::Pass::getTextureUnitStateIterator; // deprecated
%ignore Ogre::Pass::hasSeparateSceneBlending;
%ignore Ogre::Pass::hasSeparateSceneBlendingOperations;
%template(TextureUnitStates) std::vector<Ogre::TextureUnitState*>;
%include "OgrePass.h"
    %ignore Ogre::Technique::getGPUVendorRuleIterator;
    %ignore Ogre::Technique::getGPUDeviceNameRuleIterator;
    %ignore Ogre::Technique::getIlluminationPassIterator;
    %ignore Ogre::Technique::getPassIterator();
    %template(Passes) std::vector<Ogre::Pass*>;
    %template(IlluminationPassList) std::vector<Ogre::IlluminationPass*>;
    %include "OgreTechnique.h"
%ignore Ogre::RenderTarget::copyContentsToMemory(const PixelBox&);
%ignore Ogre::RenderTarget::copyContentsToMemory(const PixelBox&, FrameBuffer); // deprecated
%feature("flatnested") Ogre::RenderTarget::FrameStats;
%include "OgreRenderTarget.h"
#ifdef __ANDROID__
    %ignore Ogre::RenderWindow::_notifySurfaceCreated(void*);
    %ignore Ogre::RenderWindow::_notifySurfaceCreated(void*, void*);
    %extend Ogre::RenderWindow {
        void _notifySurfaceCreated(jobject surface) {
            ANativeWindow* nativeWnd = ANativeWindow_fromSurface(OgreJNIGetEnv(), surface);
            $self->_notifySurfaceCreated(nativeWnd, NULL);
        }
    }
#endif
    %include "OgreRenderWindow.h"
    %include "OgreRenderTexture.h"
%include "OgreViewport.h"
%ignore Ogre::CompositorChain::getNumCompositors;
%ignore Ogre::CompositorChain::getCompositor;
%ignore Ogre::CompositorChain::getCompositors;
%include "OgreCompositorChain.h"
%ignore Ogre::RenderQueueGroup::getIterator;
%include "OgreRenderQueueSortingGrouping.h"
%ignore Ogre::RenderQueueInvocationSequence::iterator;
%include "OgreRenderQueueInvocation.h"
%ignore Ogre::SceneManager::getCameraIterator; // deprecated
%ignore Ogre::SceneManager::getAnimationIterator;
%ignore Ogre::SceneManager::getAnimationStateIterator;
%ignore Ogre::SceneManager::getMovableObjectIterator;
%ignore Ogre::SceneManager::getShadowTextureCount;
%ignore Ogre::SceneManager::getShadowTextureConfigIterator;
%newobject Ogre::SceneManager::createRayQuery(const Ray&, uint32 mask);
%newobject Ogre::SceneManager::createRayQuery(const Ray&);
%rename(SceneManager_Listener) Ogre::SceneManager::Listener;
%include "OgreSceneManager.h"
%ignore Ogre::SceneManagerEnumerator::createSceneManager(SceneTypeMask);
%ignore Ogre::SceneManagerEnumerator::createSceneManager(SceneTypeMask, const String&);
%ignore Ogre::SceneManagerEnumerator::getSceneManagerIterator;
%ignore Ogre::SceneManagerEnumerator::getMetaDataIterator;
%include "OgreDefaultDebugDrawer.h"
%include "OgreSceneManagerEnumerator.h"
%include "OgreConfigDialog.h"
%template(RenderSystemList) std::vector<Ogre::RenderSystem*>;
%ignore Ogre::Root::getSceneManagerMetaDataIterator;
%ignore Ogre::Root::getSceneManagerIterator;
%ignore Ogre::Root::createSceneManager(SceneTypeMask);
%ignore Ogre::Root::createSceneManager(SceneTypeMask, const String&);
%ignore Ogre::Root::getMovableObjectFactoryIterator;
%include "OgreRoot.h"
// dont wrap: platform specific
// %include "OgreWindowEventUtilities.h"
// %include "OgreTimer.h"
// dont wrap: not useful in high level languages
// %include "OgreRadixSort.h"
// %include "OgreString.h"
// %include "OgreStringConverter.h"
// %include "OgreProfiler.h"
