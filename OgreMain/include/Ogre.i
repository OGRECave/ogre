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
%}

%include std_string.i
%include std_pair.i
%include std_map.i
%include std_multimap.i
%include std_vector.i
%include exception.i 
 
/* Parse the header file to generate wrappers */
#define _OgreExport
#define _OgrePrivate
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
%rename(__getitem__) *::operator[];
%ignore Ogre::Matrix3::operator[];
%ignore Ogre::Matrix4::operator[];
%rename(OgreException) Ogre::Exception; // confilcts with Python Exception

// convert c++ exceptions to language native exceptions
%exception {
    try {
        $action
    } catch (const Ogre::Exception& e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
    }
}

%define ADD_REPR(classname)
%extend Ogre::classname {
    const std::string __repr__() {
        std::ostringstream out;
        out << *$self;
        return out.str();
    }
}
%enddef

/* these are ordered by dependancy */
%include "OgreBuildSettings.h"
%include "OgrePrerequisites.h"
%include "OgrePlatform.h"
%include "OgreConfig.h"
%include "OgreMemoryAllocatorConfig.h"
%include "OgreCommon.h"
// Basic Data Types
%include "OgreException.h"
%include "OgreAtomicScalar.h"
%include "OgreSharedPtr.h"
%include "OgreAny.h"
%include "OgreIteratorWrapper.h"
%include "OgreMath.h"
ADD_REPR(Degree)
ADD_REPR(Radian)
//%include "OgreStringVector.h"
// the original definitions confuses SWIG by typedeffing to std inside a struct
namespace Ogre {
typedef std::vector<std::string> StringVector;
typedef Ogre::SharedPtr<StringVector> StringVectorPtr;
}
%template(StringVector) std::vector<Ogre::String>;
%template(StringVectorPtr) Ogre::SharedPtr<std::vector<std::string> >;
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
%include "OgreConfigFile.h"
%feature("valuewrapper") Ogre::Log::Stream;
%include "OgreLog.h"
%include "OgreLogManager.h"
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
%include "OgrePixelFormat.h"
%include "OgreBlendMode.h"
%include "OgreRay.h"
%include "OgreSceneQuery.h"
%include "OgreNameGenerator.h"
%include "OgreController.h"
%include "OgreRenderSystemCapabilities.h"
%include "OgreGpuProgramParams.h"
%include "OgreImage.h"
%include "OgreBillboard.h"
%include "OgreParticle.h"
%include "OgreHardwareOcclusionQuery.h"
%include "OgreHardwareBuffer.h"
%include "OgreParticleIterator.h"
%include "OgreStringInterface.h"
    %include "OgreParticleAffector.h"
        %include "OgreParticleAffectorFactory.h"
    %include "OgreParticleEmitterCommands.h"
    %include "OgreParticleEmitter.h"
        %include "OgreParticleEmitterFactory.h"
    %include "OgreResource.h"
        %include "OgreTexture.h"
        %include "OgreGpuProgram.h"
            %include "OgreHighLevelGpuProgram.h"
%include "OgreScriptCompiler.h"
%include "OgreTextureUnitState.h"
%include "OgreControllerManager.h"
%include "OgreCompositor.h"
%include "OgreCompositionTechnique.h"
%include "OgreCompositionTargetPass.h"
%include "OgreResourceBackgroundQueue.h"
// instantiated in c++ code
// %template(HardwareVertexBufferPtr) Ogre::SharedPtr<Ogre::HardwareVertexBuffer>;
%include "OgreHardwareVertexBuffer.h"
// instantiated in c++ code
// %template(HardwareIndexBufferPtr) Ogre::SharedPtr<Ogre::HardwareIndexBuffer>;
%include "OgreHardwareIndexBuffer.h"
// instantiated in c++ code
// %template(HardwarePixelBufferPtr) Ogre::SharedPtr<Ogre::HardwarePixelBuffer>;
%include "OgreHardwarePixelBuffer.h"
// instantiated in c++ code
// %template(HardwareCounterBufferPtr) Ogre::SharedPtr<Ogre::HardwareCounterBuffer>;
%include "OgreHardwareCounterBuffer.h"
// instantiated in c++ code
// %template(HardwareUniformBufferPtr) Ogre::SharedPtr<Ogre::HardwareUniformBuffer>;
%include "OgreHardwareUniformBuffer.h"
%include "OgreHardwareBufferManager.h"
%include "OgreVertexIndexData.h"
// managers
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
%include "OgreUnifiedHighLevelGpuProgram.h"
// animations
%include "OgreKeyFrame.h"
%include "OgrePose.h"
%include "OgreAnimationTrack.h"
%include "OgreAnimationState.h"
%include "OgreAnimation.h"
%include "OgreSkeleton.h"
    %include "OgreSkeletonInstance.h"
%include "OgreSkeletonManager.h"
%include "OgreRenderQueue.h"
%include "OgreRenderOperation.h"
%include "OgreMaterial.h"
%include "OgreRenderSystem.h"
%include "OgreCompositorManager.h"
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
    %include "OgreLight.h"
    %include "OgreNode.h"
        %include "OgreBone.h"
        %include "OgreSceneNode.h"
    %include "OgreShadowCameraSetup.h"
        %include "OgreShadowCameraSetupFocused.h"
        %include "OgreShadowCameraSetupLiSPSM.h"
        %include "OgreShadowCameraSetupPlaneOptimal.h"
        %include "OgreShadowCameraSetupPSSM.h"
    %include "OgreFrustum.h"
        %include "OgreCamera.h"
        ADD_REPR(Camera)
    %include "OgreManualObject.h"
    %include "OgreEntity.h"
    %include "OgreSubEntity.h"
    %include "OgreParticleSystem.h"
    %include "OgreInstancedEntity.h"
    %include "OgreInstanceBatch.h"
    %include "OgreSimpleRenderable.h"
        %include "OgreRectangle2D.h"
        %include "OgreWireBoundingBox.h"
%include "OgreParticleSystemManager.h"
%include "OgreInstanceManager.h" 
%include "OgreVertexBoneAssignment.h"
%include "OgreMesh.h"
%include "OgreSubMesh.h"
%include "OgreStaticGeometry.h"
%include "OgrePatchSurface.h"
    %include "OgrePatchMesh.h"
%include "OgreMeshManager.h"
%include "OgrePass.h"
    %include "OgreTechnique.h"
%ignore Ogre::RenderTarget::copyContentsToMemory(const PixelBox&);
%ignore Ogre::RenderTarget::copyContentsToMemory(const PixelBox&, FrameBuffer); // deprecated
%include "OgreRenderTarget.h"
    %include "OgreRenderWindow.h"
    %include "OgreRenderTexture.h"
%include "OgreViewport.h"
%include "OgreCompositorChain.h"
%include "OgreShadowTextureManager.h"
%include "OgreRenderQueueSortingGrouping.h"
%include "OgreRenderQueueInvocation.h"
%include "OgreSceneManager.h"
%include "OgreSceneManagerEnumerator.h"
%include "OgreRoot.h"
// dont wrap: platform specific
// %include "OgreWindowEventUtilities.h"
// %include "OgreTimer.h"
// dont wrap: not useful in high level languages
// %include "OgreRadixSort.h"
// %include "OgreString.h"
// %include "OgreStringConverter.h"
// %include "OgreProfiler.h"
