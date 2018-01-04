/*-------------------------------------------------------------------------
This source file is a part of OGRE
(Object-oriented Graphics Rendering Engine)

For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE
-------------------------------------------------------------------------*/
#ifndef __OgrePrerequisites_H__
#define __OgrePrerequisites_H__

// Platform-specific stuff
#include "OgrePlatform.h"

#include <string>
#include <memory>

namespace Ogre {
    // Define ogre version
    #define OGRE_VERSION_MAJOR 1
    #define OGRE_VERSION_MINOR 11
    #define OGRE_VERSION_PATCH 0
    #define OGRE_VERSION_SUFFIX "dev"
    #define OGRE_VERSION_NAME "Rhagorthua"

    #define OGRE_VERSION    ((OGRE_VERSION_MAJOR << 16) | (OGRE_VERSION_MINOR << 8) | OGRE_VERSION_PATCH)

    // define the real number values to be used
    // default to use 'float' unless precompiler option set
    #if OGRE_DOUBLE_PRECISION == 1
        /** Software floating point type.
        @note Not valid as a pointer to GPU buffers / parameters
        */
        typedef double Real;
    #else
        /** Software floating point type.
        @note Not valid as a pointer to GPU buffers / parameters
        */
        typedef float Real;
    #endif

    #define OGRE_HashMap ::std::unordered_map
    #define OGRE_HashMultiMap ::std::unordered_multimap
    #define OGRE_HashSet ::std::unordered_set
    #define OGRE_HashMultiSet ::std::unordered_multiset


    /** In order to avoid finger-aches :)
    */
    typedef unsigned char uchar;
    typedef unsigned short ushort;
    typedef unsigned int uint;
    typedef unsigned long ulong;

    #if __cplusplus >= 201103L
    #define register
    #endif
// Pre-declare classes
// Allows use of pointers in header files without including individual .h
// so decreases dependencies between files
    class Angle;
    class AnimableValue;
    class Animation;
    class AnimationState;
    class AnimationStateSet;
    class AnimationTrack;
    class Archive;
    class ArchiveFactory;
    class ArchiveManager;
    class AutoParamDataSource;
    class AxisAlignedBox;
    class AxisAlignedBoxSceneQuery;
    class Billboard;
    class BillboardChain;
    class BillboardSet;
    class Bone;
    class Camera;
    class Codec;
    class ColourValue;
    class ConfigDialog;
    template <typename T> class Controller;
    template <typename T> class ControllerFunction;
    class ControllerManager;
    template <typename T> class ControllerValue;
    class DataStream;
    class DefaultWorkQueue;
    class Degree;
    class DepthBuffer;
    class DynLib;
    class DynLibManager;
    class EdgeData;
    class EdgeListBuilder;
    class Entity;
    class ExternalTextureSourceManager;
    class Factory;
    struct FrameEvent;
    class FrameListener;
    class Frustum;
    struct GpuLogicalBufferStruct;
    struct GpuNamedConstants;
    class GpuProgramParameters;
    class GpuSharedParameters;
    class GpuProgram;
    class GpuProgramManager;
    class GpuProgramUsage;
    class HardwareCounterBuffer;
    class HardwareIndexBuffer;
    class HardwareOcclusionQuery;
    class HardwareUniformBuffer;
    class HardwareVertexBuffer;
    class HardwarePixelBuffer;
    class HighLevelGpuProgram;
    class HighLevelGpuProgramManager;
    class HighLevelGpuProgramFactory;
    class IndexData;
    class InstanceBatch;
    class InstanceBatchHW;
    class InstanceBatchHW_VTF;
    class InstanceBatchShader;
    class InstanceBatchVTF;
    class InstanceManager;
    class InstancedEntity;
    class IntersectionSceneQuery;
    class IntersectionSceneQueryListener;
    class Image;
    class KeyFrame;
    class Light;
    class Log;
    class LogManager;
    class LodStrategy;
    class LodStrategyManager;
    class ManualResourceLoader;
    class ManualObject;
    class Material;
    class MaterialManager;
    class Math;
    class Matrix3;
    class Matrix4;
    class MemoryDataStream;
    class MemoryManager;
    class Mesh;
    class MeshSerializer;
    class MeshSerializerImpl;
    class MeshManager;
    class MovableObject;
    class MovablePlane;
    class Node;
    class NodeAnimationTrack;
    class NodeKeyFrame;
    class NumericAnimationTrack;
    class NumericKeyFrame;
    class Particle;
    class ParticleAffector;
    class ParticleAffectorFactory;
    class ParticleEmitter;
    class ParticleEmitterFactory;
    class ParticleSystem;
    class ParticleSystemManager;
    class ParticleSystemRenderer;
    class ParticleSystemRendererFactory;
    class ParticleVisualData;
    class Pass;
    class PatchMesh;
    class PixelBox;
    class Plane;
    class PlaneBoundedVolume;
    class Plugin;
    class Pose;
    class Profile;
    class Profiler;
    class Quaternion;
    class Radian;
    class Ray;
    class RaySceneQuery;
    class RaySceneQueryListener;
    class Renderable;
    class RenderPriorityGroup;
    class RenderQueue;
    class RenderQueueGroup;
    class RenderQueueInvocation;
    class RenderQueueInvocationSequence;
    class RenderQueueListener;
    class RenderObjectListener;
    class RenderSystem;
    class RenderSystemCapabilities;
    class RenderSystemCapabilitiesManager;
    class RenderSystemCapabilitiesSerializer;
    class RenderTarget;
    class RenderTargetListener;
    class RenderTexture;
    class RenderToVertexBuffer;
    class MultiRenderTarget;
    class RenderWindow;
    class RenderOperation;
    class Resource;
    class ResourceBackgroundQueue;
    class ResourceGroupManager;
    class ResourceManager;
    class RibbonTrail;
    class Root;
    class SceneManager;
    class SceneManagerEnumerator;
    class SceneNode;
    class SceneQuery;
    class SceneQueryListener;
    class ScriptCompiler;
    class ScriptCompilerManager;
    class ScriptLoader;
    class Serializer;
    class ShadowCameraSetup;
    class ShadowCaster;
    class ShadowRenderable;
    class ShadowTextureManager;
    class SimpleRenderable;
    class SimpleSpline;
    class Skeleton;
    class SkeletonInstance;
    class SkeletonManager;
    class Sphere;
    class SphereSceneQuery;
    class StaticGeometry;
    class StreamSerialiser;
    class StringConverter;
    class StringInterface;
    class SubEntity;
    class SubMesh;
    class TagPoint;
    class Technique;
    class TempBlendedBufferInfo;
    class ExternalTextureSource;
    class TextureUnitState;
    class Texture;
    class TextureManager;
    class TransformKeyFrame;
    class Timer;
    class UserObjectBindings;
    class Vector2;
    class Vector3;
    class Vector4;
    class Viewport;
    class VertexAnimationTrack;
    class VertexBufferBinding;
    class VertexData;
    class VertexDeclaration;
    class VertexMorphKeyFrame;
    class WireBoundingBox;
    class WorkQueue;
    class Compositor;
    class CompositorManager;
    class CompositorChain;
    class CompositorInstance;
    class CompositorLogic;
    class CompositionTechnique;
    class CompositionPass;
    class CompositionTargetPass;
    class CustomCompositionPass;

    using std::shared_ptr;
    using std::unique_ptr;
    template<typename T> class SharedPtr;

    typedef shared_ptr<AnimableValue> AnimableValuePtr;
    typedef shared_ptr<Compositor> CompositorPtr;
    typedef shared_ptr<DataStream> DataStreamPtr;
    typedef shared_ptr<GpuProgram> GpuProgramPtr;
    typedef shared_ptr<GpuNamedConstants> GpuNamedConstantsPtr;
    typedef shared_ptr<GpuLogicalBufferStruct> GpuLogicalBufferStructPtr;
    typedef shared_ptr<GpuSharedParameters> GpuSharedParametersPtr;
    typedef shared_ptr<GpuProgramParameters> GpuProgramParametersSharedPtr;
    typedef shared_ptr<HardwareCounterBuffer> HardwareCounterBufferSharedPtr;
    typedef shared_ptr<HardwareIndexBuffer> HardwareIndexBufferSharedPtr;
    typedef shared_ptr<HardwarePixelBuffer> HardwarePixelBufferSharedPtr;
    typedef shared_ptr<HardwareUniformBuffer> HardwareUniformBufferSharedPtr;
    typedef shared_ptr<HardwareVertexBuffer> HardwareVertexBufferSharedPtr;
    typedef shared_ptr<HighLevelGpuProgram> HighLevelGpuProgramPtr;
    typedef shared_ptr<Material> MaterialPtr;
    typedef shared_ptr<MemoryDataStream> MemoryDataStreamPtr;
    typedef shared_ptr<Mesh> MeshPtr;
    typedef shared_ptr<PatchMesh> PatchMeshPtr;
    typedef shared_ptr<RenderToVertexBuffer> RenderToVertexBufferSharedPtr;
    typedef shared_ptr<Resource> ResourcePtr;
    typedef shared_ptr<ShadowCameraSetup> ShadowCameraSetupPtr;
    typedef shared_ptr<Skeleton> SkeletonPtr;
    typedef shared_ptr<Texture> TexturePtr;
}

/* Include all the standard header *after* all the configuration
settings have been made.
*/
#include "OgreStdHeaders.h"
#include "OgreMemoryAllocatorConfig.h"


namespace Ogre
{
    typedef std::string _StringBase;
    typedef std::basic_stringstream<char,std::char_traits<char>,std::allocator<char> > _StringStreamBase;

    typedef _StringBase String;
    typedef _StringStreamBase StringStream;
    typedef StringStream stringstream;
}

//for stl container
namespace Ogre
{
    template <typename T>
    struct deque 
    { 
        typedef typename std::deque<T> type;
        typedef typename std::deque<T>::iterator iterator;
        typedef typename std::deque<T>::const_iterator const_iterator;
    };

    template <typename T>
    struct vector 
    { 
        typedef typename std::vector<T> type;
        typedef typename std::vector<T>::iterator iterator;
        typedef typename std::vector<T>::const_iterator const_iterator;
    };

    template <typename T>
    struct list 
    { 
        typedef typename std::list<T> type;
        typedef typename std::list<T>::iterator iterator;
        typedef typename std::list<T>::const_iterator const_iterator;
    };

    template <typename T, typename P = std::less<T> >
    struct set 
    { 
        typedef typename std::set<T, P> type;
        typedef typename std::set<T, P>::iterator iterator;
        typedef typename std::set<T, P>::const_iterator const_iterator;
    };

    template <typename K, typename V, typename P = std::less<K> >
    struct map 
    { 
        typedef typename std::map<K, V, P> type;
        typedef typename std::map<K, V, P>::iterator iterator;
        typedef typename std::map<K, V, P>::const_iterator const_iterator;
    };

    template <typename K, typename V, typename P = std::less<K> >
    struct multimap 
    { 
        typedef typename std::multimap<K, V, P> type;
        typedef typename std::multimap<K, V, P>::iterator iterator;
        typedef typename std::multimap<K, V, P>::const_iterator const_iterator;
    };
} // Ogre

#endif // __OgrePrerequisites_H__


