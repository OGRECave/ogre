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
    #define OGRE_TOKEN_PASTE_INNER(x, y) x ## y
    #define OGRE_TOKEN_PASTE(x, y) OGRE_TOKEN_PASTE_INNER(x, y)

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

    /// @deprecated
    #define OGRE_HashMap ::std::unordered_map
    /// @deprecated
    #define OGRE_HashMultiMap ::std::unordered_multimap
    /// @deprecated
    #define OGRE_HashSet ::std::unordered_set
    /// @deprecated
    #define OGRE_HashMultiSet ::std::unordered_multiset

    /** In order to avoid finger-aches :)
    */
    typedef unsigned char uchar;
    typedef unsigned short ushort;
    typedef unsigned int uint;
    typedef unsigned long ulong;

// Pre-declare classes
// Allows use of pointers in header files without including individual .h
// so decreases dependencies between files
    class Affine3;
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
    class HardwareBuffer;
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
    class SceneLoaderManager;
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
    template <int dims, typename T> class Vector;
    typedef Vector<2, Real> Vector2;
    typedef Vector<2, int> Vector2i;
    typedef Vector<3, Real> Vector3;
    typedef Vector<3, int> Vector3i;
    typedef Vector<4, Real> Vector4;
    typedef Vector<4, float> Vector4f;
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

    typedef SharedPtr<AnimableValue> AnimableValuePtr;
    typedef SharedPtr<Compositor> CompositorPtr;
    typedef SharedPtr<DataStream> DataStreamPtr;
    typedef SharedPtr<GpuProgram> GpuProgramPtr;
    typedef SharedPtr<GpuNamedConstants> GpuNamedConstantsPtr;
    typedef SharedPtr<GpuLogicalBufferStruct> GpuLogicalBufferStructPtr;
    typedef SharedPtr<GpuSharedParameters> GpuSharedParametersPtr;
    typedef SharedPtr<GpuProgramParameters> GpuProgramParametersPtr;
    typedef GpuProgramParametersPtr GpuProgramParametersSharedPtr; //!< @deprecated
    typedef SharedPtr<HardwareBuffer> HardwareBufferPtr;
    typedef SharedPtr<HardwareIndexBuffer> HardwareIndexBufferSharedPtr;
    typedef SharedPtr<HardwarePixelBuffer> HardwarePixelBufferSharedPtr;
    typedef SharedPtr<HardwareUniformBuffer> HardwareUniformBufferSharedPtr;
    typedef HardwareUniformBufferSharedPtr HardwareCounterBufferSharedPtr;
    typedef SharedPtr<HardwareVertexBuffer> HardwareVertexBufferSharedPtr;
    typedef SharedPtr<HighLevelGpuProgram> HighLevelGpuProgramPtr;
    typedef SharedPtr<Material> MaterialPtr;
    typedef SharedPtr<MemoryDataStream> MemoryDataStreamPtr;
    typedef SharedPtr<Mesh> MeshPtr;
    typedef SharedPtr<PatchMesh> PatchMeshPtr;
    typedef SharedPtr<RenderToVertexBuffer> RenderToVertexBufferSharedPtr;
    typedef SharedPtr<Resource> ResourcePtr;
    typedef SharedPtr<ShadowCameraSetup> ShadowCameraSetupPtr;
    typedef SharedPtr<Skeleton> SkeletonPtr;
    typedef SharedPtr<Texture> TexturePtr;
}

/* Include all the standard header *after* all the configuration
settings have been made.
*/
#include "OgreStdHeaders.h"
#include "OgreMemoryAllocatorConfig.h"


namespace Ogre
{
    /// @deprecated use std::atomic
    template<class T> using AtomicScalar = std::atomic<T>;

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
    struct OGRE_DEPRECATED deque
    { 
        typedef typename std::deque<T> type;
        typedef typename std::deque<T>::iterator iterator;
        typedef typename std::deque<T>::const_iterator const_iterator;
    };

    template <typename T>
    struct OGRE_DEPRECATED vector
    { 
        typedef typename std::vector<T> type;
        typedef typename std::vector<T>::iterator iterator;
        typedef typename std::vector<T>::const_iterator const_iterator;
    };

    template <typename T, size_t Alignment = OGRE_SIMD_ALIGNMENT>
    using aligned_vector = std::vector<T, AlignedAllocator<T, Alignment>>;

    template <typename T>
    struct OGRE_DEPRECATED list
    { 
        typedef typename std::list<T> type;
        typedef typename std::list<T>::iterator iterator;
        typedef typename std::list<T>::const_iterator const_iterator;
    };

    template <typename T, typename P = std::less<T> >
    struct OGRE_DEPRECATED set
    { 
        typedef typename std::set<T, P> type;
        typedef typename std::set<T, P>::iterator iterator;
        typedef typename std::set<T, P>::const_iterator const_iterator;
    };

    template <typename K, typename V, typename P = std::less<K> >
    struct OGRE_DEPRECATED map
    { 
        typedef typename std::map<K, V, P> type;
        typedef typename std::map<K, V, P>::iterator iterator;
        typedef typename std::map<K, V, P>::const_iterator const_iterator;
    };

    template <typename K, typename V, typename P = std::less<K> >
    struct OGRE_DEPRECATED multimap
    { 
        typedef typename std::multimap<K, V, P> type;
        typedef typename std::multimap<K, V, P>::iterator iterator;
        typedef typename std::multimap<K, V, P>::const_iterator const_iterator;
    };
} // Ogre

#endif // __OgrePrerequisites_H__


