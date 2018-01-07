 %module(directors="1") Ogre
 %{
 /* Includes the header in the wrapper code */
#include "Ogre.h"
#include "OgreArchiveFactory.h"
#include "OgreRectangle2D.h"
#include "OgreWireBoundingBox.h"
#include "OgreVertexBoneAssignment.h"
#include "OgreCodec.h"
#include "OgreZip.h"
#include "OgreParticleIterator.h"
#include "OgreParticleEmitterFactory.h"
#include "OgreParticleAffectorFactory.h"
#include "OgreUnifiedHighLevelGpuProgram.h"
#include "OgreScriptCompiler.h"
#include "OgreConfigDialog.h"
%}

%include std_shared_ptr.i
%include std_string.i
%include std_pair.i
%include std_map.i
#ifdef SWIGPYTHON
%include std_multimap.i
#endif
%include std_vector.i
%include exception.i
%include typemaps.i
 
// so swig correctly resolves "using std::*" declarations
%inline %{
using namespace std;
%}
 
/* Parse the header file to generate wrappers */
#define _OgreExport
#define _OgrePrivate
#define OGRE_DEPRECATED
#define OGRE_NORETURN
#define OGRE_STATIC_MUTEX(arg)
#define OGRE_MUTEX(arg)

%feature("autodoc", "1");
%feature("director") *Listener;
%feature("director") *::Listener;
// should be turned on globally if all renames are in place
%feature("flatnested") Ogre::MaterialManager::Listener;

%ignore *::operator=;  // needs rename to wrap
%ignore *::setUserAny; // deprecated
%ignore *::getUserAny; // deprecated
%ignore *::getSingletonPtr; // only expose the non ptr variant
%ignore Ogre::Exception::getNumber; // deprecated
%ignore Ogre::ExceptionFactory::throwExceptionEx; // deprecated
%rename(OgreException) Ogre::Exception; // confilcts with Python Exception

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

#ifdef SWIG_DIRECTORS
%feature("director:except") {
    if ($error != NULL) {
        throw Swig::DirectorMethodException();
    }
}
#endif

// convert c++ exceptions to language native exceptions
%exception {
    try {
        $action
    }
#ifdef SWIGPYTHON
    catch (Swig::DirectorException &e) { 
        SWIG_fail;
    }
#endif
    catch (const std::exception& e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
    }
}

// connect operator<< to tp_repr
%ignore ::operator<<;
%feature("python:slot", "tp_repr", functype="reprfunc") *::__repr__;

#ifdef SWIGJAVA
#define REPRFUNC toString
#else
#define REPRFUNC __repr__
#endif

%define ADD_REPR(classname)
%extend Ogre::classname {
    const std::string REPRFUNC() {
        std::ostringstream out;
        out << *$self;
        return out.str();
    }
}
%enddef

#ifdef SWIGJAVA
// conflicts with SWIG method
%ignore *::getType;
#endif

// connect operator[] to __getitem__
%feature("python:slot", "sq_item", functype="ssizeargfunc") *::operator[];
%rename(__getitem__) *::operator[];
%ignore Ogre::Matrix3::operator[];
%ignore Ogre::Matrix4::operator[];
%ignore Ogre::ColourValue::operator[];

// stringinterface internal
%rename("$ignore", regextarget=1) "^Cmd+";

/* these are ordered by dependancy */
%include "OgreBuildSettings.h"
%include "OgrePrerequisites.h"
%include "OgrePlatform.h"
%include "OgreConfig.h"
%ignore Ogre::AllocPolicy;
%import "OgreMemoryAllocatorConfig.h"
%include "OgreCommon.h"
%template() Ogre::map<Ogre::String, Ogre::String>;
%template(NameValuePairList) std::map<Ogre::String, Ogre::String>;
%ignore Ogre::findCommandLineOpts; // not needed in python

// Basic Data Types
%include "OgreException.h"
%ignore Ogre::Any::getType; // deprecated
%ignore Ogre::Any::destroy; // deprecated
%include "OgreAny.h"
%include "OgreIteratorWrapper.h"
%include "OgreMath.h"
ADD_REPR(Degree)
ADD_REPR(Radian)
%include "OgreStringVector.h"
%template() Ogre::vector<Ogre::String>; // instantiate vector<T>::type
%template(StringVector) std::vector<Ogre::String>;  // actual vector<T>
%template(StringVectorPtr) Ogre::SharedPtr<std::vector<Ogre::String> >;
// Linear Algebra
%include "OgreVector2.h"
ADD_REPR(Vector2)
%include "OgreVector3.h"
ADD_REPR(Vector3)
%include "OgreVector4.h"
ADD_REPR(Vector4)
%include "OgreMatrix3.h"
ADD_REPR(Matrix3)
%include "OgreMatrix4.h"
ADD_REPR(Matrix4)
%include "OgreQuaternion.h"
ADD_REPR(Quaternion)
%include "OgreSimpleSpline.h"
%include "OgreRotationalSpline.h"
// Geometric Primitives
%include "OgreAxisAlignedBox.h"
ADD_REPR(AxisAlignedBox)
%include "OgreSphere.h"
%include "OgrePlane.h"
ADD_REPR(Plane)
%include "OgrePlaneBoundedVolume.h"
// I/O
%include "OgreConfigOptionMap.h"
%ignore Ogre::ConfigFile::load; // conflicting overloads
%ignore Ogre::ConfigFile::getSettingsIterator; // deprecated
%ignore Ogre::ConfigFile::getSectionIterator;
%template() Ogre::map<Ogre::String, std::multimap< Ogre::String, Ogre::String> >;
%template(SettingsBySection) std::map<Ogre::String, std::multimap< Ogre::String, Ogre::String> >;
%template() Ogre::multimap<Ogre::String, Ogre::String>;
#ifdef SWIGPYTHON
%template(SettingsMultiMap) std::multimap<Ogre::String, Ogre::String>;
#endif
%include "OgreConfigFile.h"
%feature("valuewrapper") Ogre::Log::Stream;
%include "OgreLog.h"
%include "OgreLogManager.h"
#ifdef SWIGJAVA
// conflicts with SWIG interal func
%ignore Ogre::MemoryDataStream::MemoryDataStream(size_t, bool);
%ignore Ogre::AtomAbstractNode::getValue;
#endif
%shared_ptr(Ogre::DataStream);
%ignore Ogre::MemoryDataStream::MemoryDataStream(DataStream&, bool = true, bool = false);
%ignore Ogre::MemoryDataStream::MemoryDataStream(const String&, DataStream&, bool = true, bool = false);
%shared_ptr(Ogre::MemoryDataStream);
%shared_ptr(Ogre::FileStreamDataStream);
%shared_ptr(Ogre::FileHandleDataStream);
%include "OgreDataStream.h"
%include "OgreArchive.h"
%include "OgreFactoryObj.h"
// %template(FactoryObjArchive) Ogre::FactoryObj<Ogre::Archive>;
%include "OgreArchiveFactory.h"
%ignore Ogre::ZipArchiveFactory; // private
%ignore Ogre::ZipDataStream; // private
%include "OgreZip.h"
%include "OgreArchiveManager.h"
%include "OgreCodec.h"
%include "OgreSerializer.h"
%include "OgreScriptLoader.h"
// Listeners 
%include "OgreFrameListener.h"
%include "OgreLodListener.h"
%include "OgreRenderObjectListener.h"
%include "OgreRenderQueueListener.h"
%include "OgreRenderTargetListener.h"
// More Data Types
%include "OgreColourValue.h"
ADD_REPR(ColourValue)
%ignore Ogre::PixelUtil::isValidExtent; // deprecated
%include "OgrePixelFormat.h"
%include "OgreBlendMode.h"
%include "OgreRay.h"
%include "OgreSceneQuery.h"
%include "OgreNameGenerator.h"
%include "OgreController.h"
%ignore Ogre::RenderSystemCapabilities::calculateSize; // deprecated
%include "OgreRenderSystemCapabilities.h"
%ignore Ogre::GpuProgramParameters::getAutoConstantIterator; // deprecated
%include "OgreGpuProgramParams.h"
%ignore Ogre::Image::loadDynamicImage(uchar*, uint32, uint32, PixelFormat); // deprecated
%ignore Ogre::Image::loadRawData(DataStreamPtr&, uint32, uint32, PixelFormat); // deprecated
%include "OgreImage.h"
%include "OgreBillboard.h"
%include "OgreParticle.h"
%include "OgreHardwareOcclusionQuery.h"
%shared_ptr(Ogre::HardwareBuffer);
%include "OgreHardwareBuffer.h"
%include "OgreParticleIterator.h"

#ifndef SWIGJAVA
%template() Ogre::vector<Ogre::ParameterDef>;
%ignore std::vector<Ogre::ParameterDef>::resize; // non default constructible
%ignore std::vector<Ogre::ParameterDef>::vector;
%template(ParameterList) std::vector<Ogre::ParameterDef>;
#endif
%shared_ptr(Ogre::StringInterface);
%include "OgreStringInterface.h"
    %shared_ptr(Ogre::ParticleAffector);
    %include "OgreParticleAffector.h"
        %include "OgreParticleAffectorFactory.h"
    %include "OgreParticleEmitterCommands.h"
    %shared_ptr(Ogre::ParticleEmitter);
    %include "OgreParticleEmitter.h"
        %include "OgreParticleEmitterFactory.h"
    %shared_ptr(Ogre::Resource);
    %ignore Ogre::Resource::setManuallyLoaded;
    %ignore Ogre::Resource::setToLoaded;
    %include "OgreResource.h"
        %shared_ptr(Ogre::Texture);
        %include "OgreTexture.h"
        %shared_ptr(Ogre::GpuProgram);
        %include "OgreGpuProgram.h"
            %shared_ptr(Ogre::HighLevelGpuProgram);
            %include "OgreHighLevelGpuProgram.h"
%include "OgreScriptCompiler.h"
%include "OgreTextureUnitState.h"
%include "OgreControllerManager.h"
%shared_ptr(Ogre::Compositor);
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
%shared_ptr(Ogre::HardwareVertexBuffer);
%include "OgreHardwareVertexBuffer.h"
%shared_ptr(Ogre::HardwareIndexBuffer);
%include "OgreHardwareIndexBuffer.h"
%shared_ptr(Ogre::HardwarePixelBuffer);
#ifdef SWIGJAVA
%ignore Ogre::HardwarePixelBuffer::lock;  // duplicate definition
#endif
%include "OgreHardwarePixelBuffer.h"
%shared_ptr(Ogre::HardwareCounterBuffer);
%include "OgreHardwareCounterBuffer.h"
%shared_ptr(Ogre::HardwareUniformBuffer);
%include "OgreHardwareUniformBuffer.h"
%ignore Ogre::HardwareBufferManagerBase::_forceReleaseBufferCopies(HardwareVertexBuffer* sourceBuffer);
%include "OgreHardwareBufferManager.h"
%include "OgreVertexIndexData.h"
// managers
%ignore Ogre::ResourceGroupManager::openResource(const String&, const String&, bool, Resource*) const;
%ignore Ogre::ResourceGroupManager::openResource(const String&, const String&, bool) const;
%include "OgreResourceGroupManager.h"
// overloaded by specfic managers (GpuProgram, Texture)
%ignore Ogre::ResourceManager::getResourceByName;
%ignore Ogre::ResourceManager::createOrRetrieve;
%ignore Ogre::ResourceManager::prepare;
%ignore Ogre::ResourceManager::load;
%include "OgreResourceManager.h"
    %include "OgreTextureManager.h"
    %include "OgreGpuProgramManager.h"
    %include "OgreHighLevelGpuProgramManager.h"
%ignore Ogre::UnifiedHighLevelGpuProgram::setPrioriry;
%shared_ptr(Ogre::UnifiedHighLevelGpuProgram);
%include "OgreUnifiedHighLevelGpuProgram.h"
// animations
%ignore Ogre::VertexPoseKeyFrame::getPoseReferenceIterator;
%include "OgreKeyFrame.h"
%include "OgrePose.h"
%include "OgreAnimationTrack.h"
%ignore Ogre::AnimationStateSet::getAnimationStateIterator;
%ignore Ogre::AnimationStateSet::getEnabledAnimationStateIterator;
%include "OgreAnimationState.h"
%include "OgreAnimation.h"
%shared_ptr(Ogre::Skeleton);
// deprecated
%ignore Ogre::Skeleton::getRootBone;
%ignore Ogre::Skeleton::getRootBoneIterator;
%ignore Ogre::Skeleton::getBoneIterator;
%include "OgreSkeleton.h"
    %shared_ptr(Ogre::SkeletonInstance);
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
%shared_ptr(Ogre::Material);
%template() Ogre::vector<Ogre::Technique*>;
%template(Techniques) std::vector<Ogre::Technique*>;
%include "OgreMaterial.h"
%ignore Ogre::RenderSystem::addClipPlane(Real, Real, Real, Real);
%ignore Ogre::RenderSystem::getFixedPipelineEnabled;
%ignore Ogre::RenderSystem::setFixedPipelineEnabled;
%ignore Ogre::RenderSystem::setVertexDeclaration;
%ignore Ogre::RenderSystem::setVertexBufferBinding;
%ignore Ogre::RenderSystem::_setTexture(size_t, bool, const String&);
%include "OgreRenderSystem.h"
%include "OgreCompositorManager.h"
#ifdef SWIGJAVA
%ignore Ogre::CompositorInstance::Listener; // issue with converting shared_ptr<Material>
#endif
%include "OgreCompositorInstance.h"
%include "OgreCompositionPass.h"
%include "OgreMaterialSerializer.h"
%ignore Ogre::UserObjectBindings::getEmptyUserAny;
%include "OgreUserObjectBindings.h"
%rename(MaterialManager_Listener) Ogre::MaterialManager::Listener;
%include "OgreMaterialManager.h"
%include "OgreRenderable.h"
    %include "OgreBillboardChain.h"
        %include "OgreRibbonTrail.h"
    %include "OgreBillboardSet.h"
%include "OgreShadowCaster.h"
%include "OgreMovableObject.h"
    %include "OgreMovablePlane.h"
    %ignore Ogre::Light::setPosition;
    %ignore Ogre::Light::getPosition;
    %include "OgreLight.h"
    %include "OgreNode.h"
        %include "OgreBone.h"
        %include "OgreSceneNode.h"
    %shared_ptr(Ogre::ShadowCameraSetup);
    %shared_ptr(Ogre::DefaultShadowCameraSetup);
    %include "OgreShadowCameraSetup.h"
        %shared_ptr(Ogre::FocusedShadowCameraSetup);
        %include "OgreShadowCameraSetupFocused.h"
        %shared_ptr(Ogre::LiSPSMShadowCameraSetup);
        %include "OgreShadowCameraSetupLiSPSM.h"
        %shared_ptr(Ogre::PlaneOptimalShadowCameraSetup);
        %include "OgreShadowCameraSetupPlaneOptimal.h"
        %shared_ptr(Ogre::PSSMShadowCameraSetup);
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
    %template() Ogre::vector<Ogre::SubEntity*>;
    %template(SubEntityList) std::vector<Ogre::SubEntity*>;
    %include "OgreEntity.h"
    %include "OgreSubEntity.h"
    %shared_ptr(Ogre::ParticleSystem);
    %include "OgreParticleSystem.h"
    %include "OgreInstancedEntity.h"
    %include "OgreInstanceBatch.h"
    %ignore Ogre::SimpleRenderable::setMaterial(const String&);
    %include "OgreSimpleRenderable.h"
        %include "OgreRectangle2D.h"
        %include "OgreWireBoundingBox.h"
%include "OgreParticleSystemManager.h"
%include "OgreInstanceManager.h" 
%include "OgreVertexBoneAssignment.h"
// deprecated
%ignore Ogre::Mesh::getSubMeshIterator;
%ignore Ogre::Mesh::getPoseCount;
%ignore Ogre::Mesh::getPose;
%ignore Ogre::Mesh::getPoseIterator;
%template() Ogre::vector<Ogre::Pose*>;
%template(PoseList) std::vector<Ogre::Pose*>;
%template() Ogre::vector<Ogre::SubMesh*>;
%template(SubMeshList) std::vector<Ogre::SubMesh*>;
%shared_ptr(Ogre::Mesh);
%include "OgreMesh.h"
%include "OgreSubMesh.h"
%include "OgreStaticGeometry.h"
%include "OgrePatchSurface.h"
    %shared_ptr(Ogre::PatchMesh);
    %include "OgrePatchMesh.h"
%include "OgreMeshManager.h"
%ignore Ogre::Pass::getTextureUnitStateIterator; // deprecated
%template() Ogre::vector<Ogre::TextureUnitState*>;
%template(TextureUnitStates) std::vector<Ogre::TextureUnitState*>;
%include "OgrePass.h"
    %ignore Ogre::Technique::getGPUVendorRuleIterator;
    %ignore Ogre::Technique::getGPUDeviceNameRuleIterator;
    %ignore Ogre::Technique::getIlluminationPassIterator;
    %ignore Ogre::Technique::getPassIterator();
    %template() Ogre::vector<Ogre::Pass*>;
    %template(Passes) std::vector<Ogre::Pass*>;
    %template() Ogre::vector<Ogre::IlluminationPass*>;
    %template(IlluminationPassList) std::vector<Ogre::IlluminationPass*>;
    %include "OgreTechnique.h"
%ignore Ogre::RenderTarget::copyContentsToMemory(const PixelBox&);
%ignore Ogre::RenderTarget::copyContentsToMemory(const PixelBox&, FrameBuffer); // deprecated
%ignore Ogre::RenderTarget::getStatistics(float&, float&, float&, float&) const;
%ignore Ogre::RenderTarget::getLastFPS;
%ignore Ogre::RenderTarget::getAverageFPS;
%ignore Ogre::RenderTarget::getBestFPS;
%ignore Ogre::RenderTarget::getWorstFPS;
%ignore Ogre::RenderTarget::getBestFrameTime;
%ignore Ogre::RenderTarget::getWorstFrameTime;
%ignore Ogre::RenderTarget::getTriangleCount;
%ignore Ogre::RenderTarget::getBatchCount;
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
%include "OgreCompositorChain.h"
%include "OgreShadowTextureManager.h"
%include "OgreRenderQueueSortingGrouping.h"
%include "OgreRenderQueueInvocation.h"
%ignore Ogre::SceneManager::getCameraIterator; // deprecated
%ignore Ogre::SceneManager::getAnimationIterator;
%ignore Ogre::SceneManager::getAnimationStateIterator;
%ignore Ogre::SceneManager::setShadowTextureCasterMaterial(const String&);
%ignore Ogre::SceneManager::setShadowTextureReceiverMaterial(const String&);
%include "OgreSceneManager.h"
%include "OgreSceneManagerEnumerator.h"
%include "OgreConfigDialog.h"
%template() Ogre::vector<Ogre::RenderSystem*>;
%template(RenderSystemList) std::vector<Ogre::RenderSystem*>;
%ignore Ogre::Root::showConfigDialog(); // deprecated
%ignore Ogre::Root::addResourceLocation; // deprecated
%ignore Ogre::Root::removeResourceLocation; // deprecated
%ignore Ogre::Root::getErrorDescription; // deprecated
%ignore Ogre::Root::getSceneManagerMetaDataIterator;
%ignore Ogre::Root::getSceneManagerIterator;
%ignore Ogre::Root::createSceneManager(SceneTypeMask);
%ignore Ogre::Root::createSceneManager(SceneTypeMask, const String&);
%ignore Ogre::Root::setFreqUpdatedBuffersUploadOption;
%ignore Ogre::Root::getFreqUpdatedBuffersUploadOption;
%include "OgreRoot.h"
// dont wrap: platform specific
// %include "OgreWindowEventUtilities.h"
// %include "OgreTimer.h"
// dont wrap: not useful in high level languages
// %include "OgreRadixSort.h"
// %include "OgreString.h"
// %include "OgreStringConverter.h"
// %include "OgreProfiler.h"
