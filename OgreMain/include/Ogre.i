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
#include "OgreFileSystemLayer.h"
#include "OgrePredefinedControllers.h"
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
#define OGRE_NODISCARD
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
#elif defined(SWIGCSHARP)
#define REPRFUNC ToString
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

%define SHARED_PTR(classname)
// %shared_ptr(type);
%template(classname ## Ptr) Ogre::SharedPtr<Ogre::classname >;
%enddef

#ifdef SWIGJAVA
// use proper (1.5+) enums
%include "enums.swg"
// conflicts with SWIG method
%ignore *::getType;
#endif

#ifdef SWIGCSHARP
%ignore Ogre::TextureUsage;
%ignore Ogre::GpuConstantType;
%ignore Ogre::Capabilities;
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

#ifdef SWIGPYTHON
    #define XSTR(x) #x
    #define STR(x) XSTR(x)
    #define __version__ STR(OGRE_VERSION_MAJOR) "." STR(OGRE_VERSION_MINOR) "." STR(OGRE_VERSION_PATCH)
    #undef STR
    #undef XSTR
#endif

%include "OgrePrerequisites.h"
%include "OgrePlatform.h"
%include "OgreConfig.h"
%ignore Ogre::AllocPolicy;
%import "OgreMemoryAllocatorConfig.h"
%include "OgreCommon.h"
%template(NameValuePairList) std::map<Ogre::String, Ogre::String>;
ADD_REPR(TRect)
%template(Rect) Ogre::TRect<long>;
%template(FloatRect) Ogre::TRect<float>;
%ignore Ogre::findCommandLineOpts; // not needed in python

// Basic Data Types
%include "OgreException.h"
%ignore Ogre::SharedPtr::useCount;
%ignore Ogre::SharedPtr::bind;
%ignore Ogre::SharedPtr::getPointer;
%ignore Ogre::SharedPtr::setNull;
%ignore Ogre::SharedPtr::isNull;
%ignore Ogre::SharedPtr::setUseCount;
%include "OgreSharedPtr.h"
%ignore Ogre::Any::getType; // deprecated
%ignore Ogre::Any::destroy; // deprecated
%ignore Ogre::Any::isEmpty; // deprecated
%include "OgreAny.h"
%include "OgreIteratorWrapper.h"
%include "OgreMath.h"
ADD_REPR(Degree)
ADD_REPR(Radian)
%include "OgreStringVector.h"
%template(StringVector) std::vector<Ogre::String>;  // actual vector<T>
%template(StringVectorPtr) Ogre::SharedPtr<std::vector<Ogre::String> >;
%include "OgreFileSystemLayer.h"
// Linear Algebra
%ignore Ogre::Vector<2, Ogre::Real>::Vector(float, float, float);
%ignore Ogre::Vector<2, Ogre::Real>::Vector(float, float, float, float);
%ignore Ogre::Vector<2, Ogre::Real>::xy;
%ignore Ogre::Vector<2, Ogre::Real>::xyz;
%ignore Ogre::Vector<3, Ogre::Real>::Vector(float, float);
%ignore Ogre::Vector<3, Ogre::Real>::Vector(float, float, float, float);
%ignore Ogre::Vector<3, Ogre::Real>::xyz;
%ignore Ogre::Vector<4, Ogre::Real>::Vector(float, float);
%ignore Ogre::Vector<4, Ogre::Real>::Vector(float, float, float);
%include "OgreVector.h"
ADD_REPR(Vector)
%template(Vector2) Ogre::Vector<2, Ogre::Real>;
%template(Vector3) Ogre::Vector<3, Ogre::Real>; 
%template(Vector4) Ogre::Vector<4, Ogre::Real>;  
%include "OgreMatrix3.h"
ADD_REPR(Matrix3)
%ignore Ogre::TransformBase::extract3x3Matrix; // deprecated
%ignore Ogre::TransformBase::extractQuaternion; // deprecated
%ignore Ogre::Matrix4::concatenate; // deprecated
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
%template(SettingsBySection) std::map<Ogre::String, std::multimap< Ogre::String, Ogre::String> >;
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
SHARED_PTR(DataStream);
%ignore Ogre::MemoryDataStream::MemoryDataStream(DataStream&, bool = true, bool = false);
%ignore Ogre::MemoryDataStream::MemoryDataStream(const String&, DataStream&, bool = true, bool = false);
SHARED_PTR(MemoryDataStream);
SHARED_PTR(FileStreamDataStream);
SHARED_PTR(FileHandleDataStream);
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
%ignore Ogre::PixelUtil::getBNFExpressionOfPixelFormats;
%include "OgrePixelFormat.h"
%include "OgreBlendMode.h"
%include "OgreRay.h"
%include "OgreSceneQuery.h"
%template(RaySceneQueryResult) std::vector<Ogre::RaySceneQueryResultEntry>;

%include "OgreNameGenerator.h"
%include "OgreController.h"
%include "OgreRenderSystemCapabilities.h"
%ignore Ogre::GpuProgramParameters::getAutoConstantIterator; // deprecated
%include "OgreGpuProgramParams.h"
%include "OgreImage.h"
%include "OgreBillboard.h"
%include "OgreParticle.h"
%include "OgreHardwareOcclusionQuery.h"
SHARED_PTR(HardwareBuffer);
%include "OgreHardwareBuffer.h"
%include "OgreParticleIterator.h"

#ifdef SWIGPYTHON
%ignore std::vector<Ogre::ParameterDef>::resize; // non default constructible
%ignore std::vector<Ogre::ParameterDef>::vector;
%template(ParameterList) std::vector<Ogre::ParameterDef>;
#endif
SHARED_PTR(StringInterface);
%include "OgreStringInterface.h"
    SHARED_PTR(ParticleAffector);
    %include "OgreParticleAffector.h"
        %include "OgreParticleAffectorFactory.h"
    %include "OgreParticleEmitterCommands.h"
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
%include "OgreTextureUnitState.h"
%template(ControllerReal) Ogre::Controller<Ogre::Real>;
%template(ControllerValueRealPtr) Ogre::SharedPtr<Ogre::ControllerValue<Ogre::Real> >;
%template(ControllerFunctionPtr) Ogre::SharedPtr<Ogre::ControllerFunction<Ogre::Real> >;
%include "OgreControllerManager.h"
%include "OgrePredefinedControllers.h"
SHARED_PTR(Compositor);
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
%include "OgreResourceGroupManager.h"
#if SWIG_VERSION < 0x030012 || defined(SWIGJAVA)
// conflicts with overloaded versions (GpuProgram, Texture)
%ignore Ogre::ResourceManager::getResourceByName;
%ignore Ogre::ResourceManager::createOrRetrieve;
%ignore Ogre::ResourceManager::prepare;
%ignore Ogre::ResourceManager::load;
#endif
%include "OgreResourceManager.h"
    %include "OgreTextureManager.h"
    %include "OgreGpuProgramManager.h"
    %include "OgreHighLevelGpuProgramManager.h"
SHARED_PTR(UnifiedHighLevelGpuProgram);
%include "OgreUnifiedHighLevelGpuProgram.h"
// animations
%ignore Ogre::VertexPoseKeyFrame::getPoseReferenceIterator;
%include "OgreKeyFrame.h"
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
%include "OgreAnimation.h"
SHARED_PTR(Skeleton);
// deprecated
%ignore Ogre::Skeleton::getRootBone;
%ignore Ogre::Skeleton::getRootBoneIterator;
%ignore Ogre::Skeleton::getBoneIterator;
%template(BoneList) std::vector<Ogre::Bone*>;
%include "OgreSkeleton.h"
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
    %include "OgreBillboardChain.h"
        %include "OgreRibbonTrail.h"
    %include "OgreBillboardSet.h"
%include "OgreShadowCaster.h"
%include "OgreMovableObject.h"
    %include "OgreMovablePlane.h"
    %ignore Ogre::Light::setPosition;
    %ignore Ogre::Light::getPosition;
    %ignore Ogre::Light::getDirection;
    %include "OgreLight.h"
    %include "OgreNode.h"
        %include "OgreBone.h"
        %ignore Ogre::SceneNode::getAttachedObjectIterator;
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
    %include "OgreEntity.h"
    %include "OgreSubEntity.h"
    SHARED_PTR(ParticleSystem);
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
%template(PoseList) std::vector<Ogre::Pose*>;
%template(SubMeshList) std::vector<Ogre::SubMesh*>;
SHARED_PTR(Mesh);
%include "OgreMesh.h"
%include "OgreSubMesh.h"
%include "OgreStaticGeometry.h"
%include "OgrePatchSurface.h"
    SHARED_PTR(PatchMesh);
    %include "OgrePatchMesh.h"
%include "OgreMeshManager.h"
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
%include "OgreShadowTextureManager.h"
%include "OgreRenderQueueSortingGrouping.h"
%include "OgreRenderQueueInvocation.h"
%ignore Ogre::SceneManager::getCameraIterator; // deprecated
%ignore Ogre::SceneManager::getAnimationIterator;
%ignore Ogre::SceneManager::getAnimationStateIterator;
%ignore Ogre::SceneManager::setShadowTextureCasterMaterial(const String&);
%ignore Ogre::SceneManager::setShadowTextureReceiverMaterial(const String&);
%include "OgreSceneManager.h"
%ignore Ogre::SceneManagerEnumerator::createSceneManager(SceneTypeMask);
%ignore Ogre::SceneManagerEnumerator::createSceneManager(SceneTypeMask, const String&);
%include "OgreSceneManagerEnumerator.h"
%include "OgreConfigDialog.h"
%template(RenderSystemList) std::vector<Ogre::RenderSystem*>;
%ignore Ogre::Root::getSceneManagerMetaDataIterator;
%ignore Ogre::Root::getSceneManagerIterator;
%ignore Ogre::Root::createSceneManager(SceneTypeMask);
%ignore Ogre::Root::createSceneManager(SceneTypeMask, const String&);
%include "OgreRoot.h"
// dont wrap: platform specific
// %include "OgreWindowEventUtilities.h"
// %include "OgreTimer.h"
// dont wrap: not useful in high level languages
// %include "OgreRadixSort.h"
// %include "OgreString.h"
// %include "OgreStringConverter.h"
// %include "OgreProfiler.h"
