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

    #define OGRE_MIN_VERSION(MAJOR, MINOR, PATCH) OGRE_VERSION >= ((MAJOR << 16) | (MINOR << 8) | PATCH)

// OSX needs this correctly export typeinfo, however on MSVC there is huge fallout with it. Linux is fine either way.
#if OGRE_COMPILER == OGRE_COMPILER_MSVC
    #define _OgreMaybeExport
#else
    #define _OgreMaybeExport _OgreExport
#endif

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
    typedef Controller<float> ControllerFloat;
    typedef Controller<float> ControllerReal;
    template <typename T> class ControllerFunction;
    class ControllerManager;
    template <typename T> class ControllerValue;
    class DataStream;
    class DebugDrawer;
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
    class GpuProgramFactory;
    typedef GpuProgramFactory HighLevelGpuProgramFactory; //!< @deprecated
    class GpuProgramManager;
    typedef GpuProgramManager HighLevelGpuProgramManager; //!< @deprecated
    class GpuProgramUsage;
    class HardwareBuffer;
    class HardwareIndexBuffer;
    class HardwareOcclusionQuery;
    class HardwareVertexBuffer;
    class HardwarePixelBuffer;
    class HighLevelGpuProgram;
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
    template<typename T> class FactoryObj;
    typedef FactoryObj<ParticleSystemRenderer> ParticleSystemRendererFactory;
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
    class ExternalTextureSource;
    class TextureUnitState;
    class Texture;
    class TextureManager;
    class TransformKeyFrame;
    class Timer;
    class UserObjectBindings;
    template <int dims, typename T> class _OgreMaybeExport Vector;
    typedef Vector<2, Real> Vector2;
    typedef Vector<2, float> Vector2f;
    typedef Vector<2, int> Vector2i;
    typedef Vector<3, Real> Vector3;
    typedef Vector<3, float> Vector3f;
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
    typedef SharedPtr<HardwareBuffer> HardwareBufferPtr;
    typedef SharedPtr<HardwareIndexBuffer> HardwareIndexBufferPtr;
    typedef SharedPtr<HardwarePixelBuffer> HardwarePixelBufferPtr;
    typedef SharedPtr<HardwareVertexBuffer> HardwareVertexBufferPtr;
    typedef SharedPtr<Material> MaterialPtr;
    typedef SharedPtr<MemoryDataStream> MemoryDataStreamPtr;
    typedef SharedPtr<Mesh> MeshPtr;
    typedef SharedPtr<PatchMesh> PatchMeshPtr;
    typedef SharedPtr<RenderToVertexBuffer> RenderToVertexBufferPtr;
    typedef SharedPtr<Resource> ResourcePtr;
    typedef SharedPtr<ShadowCameraSetup> ShadowCameraSetupPtr;
    typedef SharedPtr<Skeleton> SkeletonPtr;
    typedef SharedPtr<Texture> TexturePtr;

    typedef RenderToVertexBufferPtr RenderToVertexBufferSharedPtr; //!< @deprecated
    typedef HardwareIndexBufferPtr HardwareIndexBufferSharedPtr; //!< @deprecated
    typedef HardwarePixelBufferPtr HardwarePixelBufferSharedPtr; //!< @deprecated
    typedef HardwareVertexBufferPtr HardwareVertexBufferSharedPtr; //!< @deprecated
    typedef GpuProgramPtr HighLevelGpuProgramPtr; //!< @deprecated
    typedef HardwareBufferPtr HardwareUniformBufferSharedPtr; //!< @deprecated
    typedef HardwareBufferPtr HardwareCounterBufferSharedPtr; //!< @deprecated
    typedef GpuProgramParametersPtr GpuProgramParametersSharedPtr; //!< @deprecated
}

/* Include all the standard header *after* all the configuration
settings have been made.
*/
#include "OgreStdHeaders.h"
#include "OgreMemoryAllocatorConfig.h"


namespace Ogre
{
    typedef std::string String;
    typedef std::stringstream StringStream;

    template <typename T, size_t Alignment = OGRE_SIMD_ALIGNMENT>
    using aligned_vector = std::vector<T, AlignedAllocator<T, Alignment>>;

    _OgreExport extern const String MOT_ENTITY;
    _OgreExport extern const String MOT_LIGHT;
    _OgreExport extern const String MOT_MANUAL_OBJECT;
    _OgreExport extern const String MOT_PARTICLE_SYSTEM;
    _OgreExport extern const String MOT_BILLBOARD_SET;
    _OgreExport extern const String MOT_BILLBOARD_CHAIN;
    _OgreExport extern const String MOT_RIBBON_TRAIL;
    _OgreExport extern const String MOT_RECTANGLE2D;
    _OgreExport extern const String MOT_STATIC_GEOMETRY;
    _OgreExport extern const String MOT_CAMERA;
    _OgreExport extern const String MOT_FRUSTRUM;
    _OgreExport extern const String MOT_MOVABLE_PLANE;
    _OgreExport extern const String MOT_INSTANCE_BATCH;
    _OgreExport extern const String MOT_INSTANCED_ENTITY;
    _OgreExport extern const String MOT_SIMPLE_RENDERABLE;
}

#endif // __OgrePrerequisites_H__


