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

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-------------------------------------------------------------------------*/
#ifndef __SceneManager_H__
#define __SceneManager_H__

// Precompiler options
#include "OgrePrerequisites.h"

#include "OgrePlane.h"
#include "OgreQuaternion.h"
#include "OgreColourValue.h"
#include "OgreCommon.h"
#include "OgreSceneQuery.h"
#include "OgreAutoParamDataSource.h"
#include "OgreAnimationState.h"
#include "OgreRenderQueue.h"
#include "OgreResourceGroupManager.h"
#include "OgreInstanceManager.h"
#include "OgreRenderSystem.h"
#include "OgreLodListener.h"
#include "OgreManualObject2.h"
#include "OgreRawPtr.h"
#include "Math/Array/OgreNodeMemoryManager.h"
#include "Math/Array/OgreObjectMemoryManager.h"
#include "Animation/OgreSkeletonAnimManager.h"
#include "Compositor/Pass/OgreCompositorPass.h"
#include "Threading/OgreThreads.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Scene
    *  @{
    */

    /** Structure for holding a position & orientation pair. */
    struct ViewPoint
    {
        Vector3 position;
        Quaternion orientation;
    };

    /** There are two Instancing techniques that perform culling of their own:
            * HW Basic
            * HW VTF
        Frustum culling is highly parallelizable & scalable. However, we first cull
        InstanceBatches & regular entities, then ask the culled InstanceBatches to
        perform their culling to the InstancedEntities they own.
        This results performance boost for skipping large amounts of instanced entities
        when the whole batch isn't visible.
        However, this also means threading frustum culling of instanced entities got harder.
    @par
        There are four approaches:
            * Ask all existing batches to frustum cull. Then use only the ones we want. Sheer
              brute force. Scales very well with cores, but sacrifices performance unnecessary
              when only a few batches are visible. This approach is not taken by Ogre.

            * Sync every time an InstanceBatchHW or InstanceBatchHW_VTF tries to frustum cull to
              delegate the job on worker threads. Considering there could be hundreds of
              InstanceBatches, this would cause a huge amount of thread synchronization overhead &
              context switches. This approach is not taken by Ogre.

            * Each thread after having culled all InstancedBatches & Entities, will parse the
              culled list to ask all MovableObjects to perform culling of their own. Entities
              will ignore this call (however they add to a small overhead for traversing them
              and calling a virtual function) while InstanceBatchHW & InstanceBatchHW_VTF will
              perform their own culling from within the multiple threads. This approach scales
              well with cores and only visible batches. However load balancing may be an issue
              for certain scenes:
              eg. an InstanceBatch with 5000 InstancedEntities in one thread, while the other
              three threads get one InstanceBatch each with 50 InstancedEntities. The first
              thread will have considerably more work to do than the other three.
              This approach is a good balance when compared to the first two. This is the
              approach taken by Ogre when INSTANCING_CULLING_THREADED is on

            * Don't multithread instanced entitites' frustum culling. Only the InstanceBatch &
              Entity's frustum culling will be threaded. This is what happens when
              INSTANCING_CULLING_SINGLE is on.

        Whether INSTANCING_CULLING_THREADED improves or degrades performance depends highly on your
        scene.
    @par
        <b>When to use INSTANCING_CULLING_SINGLETHREAD?</b>
        If your scene doesn't use HW Basic or HW VTF instancing techniques, or you have very few
        Instanced entities compared to the amount of regular Entities.
        Turning threading on, you'll be wasting your time traversing the list from multiple threads
        in search of InstanceBatchHW & InstanceBatchHW_VTF

        <b>When to use INSTANCING_CULLING_THREADED?</b>
        If your scene makes intensive use of HW Basic and/or HW VTF instancing techniques. Note
        that threaded culling is performed in SCENE_STATIC instances too.
        The most advantage is seen when the instances per batch is very high and when doing many
        PASS_SCENE, which require frustum culling multiple times per frame (eg. pssm shadows,
        multiple light sources with shadows, very advanced compositing, etc)

        Note that you can switch between methods at any time at runtime.
    */
    enum InstancingThreadedCullingMethod
    {
        INSTANCING_CULLING_SINGLETHREAD,
        INSTANCING_CULLING_THREADED,
    };

    typedef FastArray<MovableObject::MovableObjectArray> VisibleObjectsPerRq;
    typedef FastArray<VisibleObjectsPerRq> VisibleObjectsPerThreadArray;

    // Forward declarations
    class DefaultIntersectionSceneQuery;
    class DefaultRaySceneQuery;
    class DefaultSphereSceneQuery;
    class DefaultAxisAlignedBoxSceneQuery;
    class LodListener;
    struct MovableObjectLodChangedEvent;
    struct EntityMeshLodChangedEvent;
    struct EntityMaterialLodChangedEvent;
    class CompositorShadowNode;
    class UniformScalableTask;

    namespace v1
    {
        class Rectangle2D;
    }

    /// All variables are read-only for the worker threads.
    struct CullFrustumRequest
    {
        typedef vector<ObjectMemoryManager*>::type ObjectMemoryManagerVec;
        /// First RenderQueue ID to render (inclusive)
        uint8                           firstRq;
        /// Last RenderQueue ID to render (exclusive)
        uint8                           lastRq;
        /// Whether this is a shadow mapping pass
        bool                            casterPass;
        /// Whether we should immediately add to render queue v2 objects
        bool                            addToRenderQueue;
        bool                            cullingLights;
        /** Memory manager of the objects to cull. Could contain all Lights, all Entity, etc.
            Could be more than one depending on the high level cull system (i.e. tree-based sys)
            Must be const (it is read only for all threads).
        */
        ObjectMemoryManagerVec const    *objectMemManager;
        /// Camera whose frustum we're to cull against. Must be const (read only for all threads).
        Camera const                    *camera;
        /// Camera whose frustum we're to cull against. Must be const (read only for all threads).
        Camera const                    *lodCamera;

        CullFrustumRequest() :
            firstRq( 0 ), lastRq( 0 ), casterPass( false ), addToRenderQueue( true ),
            cullingLights( false ), objectMemManager( 0 ), camera( 0 ), lodCamera( 0 )
        {
        }
        CullFrustumRequest( uint8 _firstRq, uint8 _lastRq, bool _casterPass,
                            bool _addToRenderQueue, bool _cullingLights,
                            const ObjectMemoryManagerVec *_objectMemManager,
                            const Camera *_camera, const Camera *_lodCamera ) :
            firstRq( _firstRq ), lastRq( _lastRq ), casterPass( _casterPass ),
            addToRenderQueue( _addToRenderQueue ), cullingLights( _cullingLights ),
            objectMemManager( _objectMemManager ), camera( _camera ), lodCamera( _lodCamera )
        {
        }
    };

    struct UpdateLodRequest : public CullFrustumRequest
    {
        Real    lodBias;

        UpdateLodRequest() :
            CullFrustumRequest(), lodBias( 0 )
        {
        }
        UpdateLodRequest( uint8 _firstRq, uint8 _lastRq,
                            const ObjectMemoryManagerVec *_objectMemManager,
                            const Camera *_camera, const Camera *_lodCamera, Real _lodBias ) :
            CullFrustumRequest( _firstRq, _lastRq, false, false, false,
                                _objectMemManager, _camera, _lodCamera ),
            lodBias( _lodBias )
        {
        }
    };

    struct UpdateTransformRequest
    {
        Transform t;
        /// Number of nodes to process for each thread. Must be multiple of ARRAY_PACKED_REALS
        size_t numNodesPerThread;
        size_t numTotalNodes;

        UpdateTransformRequest() :
            numNodesPerThread( 0 ), numTotalNodes( 0 ) {}

        UpdateTransformRequest( const Transform &_t, size_t _numNodesPerThread, size_t _numTotalNodes ) :
            t( _t ), numNodesPerThread( _numNodesPerThread ), numTotalNodes( _numTotalNodes )
        {
        }
    };

    struct InstanceBatchCullRequest
    {
        Frustum const   *frustum;
        Camera const    *lodCamera;
        uint32          combinedVisibilityFlags;
        InstanceBatchCullRequest() : frustum( 0 ), lodCamera( 0 ), combinedVisibilityFlags( 0 ) {}
        InstanceBatchCullRequest( const Frustum *_frustum, const Camera *_lodCamera,
                                  uint32 _combinedVisibilityFlags ) :
                        frustum( _frustum ), lodCamera( _lodCamera ),
                        combinedVisibilityFlags( _combinedVisibilityFlags )
        {
        }
    };

    struct BuildLightListRequest
    {
        size_t startLightIdx;

        BuildLightListRequest() :
            startLightIdx( 0 ) {}
        BuildLightListRequest( size_t _startLightIdx ) :
            startLightIdx( _startLightIdx )
        {
        }
    };

    /** Manages the organisation and rendering of a 'scene' i.e. a collection 
        of objects and potentially world geometry.
    @remarks
        This class defines the interface and the basic behaviour of a 
        'Scene Manager'. A SceneManager organises the culling and rendering of
        the scene, in conjunction with the RenderQueue. This class is designed 
        to be extended through subclassing in order to provide more specialised
        scene organisation structures for particular needs. The default 
        SceneManager culls based on a hierarchy of node bounding boxes, other
        implementations can use an octree (@see OctreeSceneManager), a BSP
        tree (@see BspSceneManager), and many other options. New SceneManager
        implementations can be added at runtime by plugins, see 
        SceneManagerEnumerator for the interfaces for adding new SceneManager
        types.
    @par
        There is a distinction between 'objects' (which subclass MovableObject, 
        and are movable, discrete objects in the world), and 'world geometry',
        which is large, generally static geometry. World geometry tends to 
        influence the SceneManager organisational structure (e.g. lots of indoor
        static geometry might result in a spatial tree structure) and as such
        world geometry is generally tied to a given SceneManager implementation,
        whilst MovableObject instances can be used with any SceneManager.
        Subclasses are free to define world geometry however they please.
    @par
        Multiple SceneManager instances can exist at one time, each one with 
        a distinct scene. Which SceneManager is used to render a scene is
        dependent on the Camera, which will always call back the SceneManager
        which created it to render the scene. 
     */
    class _OgreExport SceneManager : public SceneMgtAlignedAlloc
    {
    public:
        /// Default query mask for entities @see SceneQuery
        static uint32 QUERY_ENTITY_DEFAULT_MASK;
        /// Default query mask for effects like billboardsets / particle systems @see SceneQuery
        static uint32 QUERY_FX_DEFAULT_MASK;
        /// Default query mask for StaticGeometry  @see SceneQuery
        static uint32 QUERY_STATICGEOMETRY_DEFAULT_MASK;
        /// Default query mask for lights  @see SceneQuery
        static uint32 QUERY_LIGHT_DEFAULT_MASK;
        /// Default query mask for frusta and cameras @see SceneQuery
        static uint32 QUERY_FRUSTUM_DEFAULT_MASK;

        /// Describes the stage of rendering when performing complex illumination
        enum IlluminationRenderStage
        {
            /// No special illumination stage
            IRS_NONE,
            /// Render to texture stage, used for texture based shadows
            IRS_RENDER_TO_TEXTURE
        };

        struct SkyDomeGenParameters
        {
            Real skyDomeCurvature;
            Real skyDomeTiling;
            Real skyDomeDistance;
            int skyDomeXSegments; 
            int skyDomeYSegments;
            int skyDomeYSegments_keep;
        };

        struct SkyPlaneGenParameters
        {
            Real skyPlaneScale;
            Real skyPlaneTiling; 
            Real skyPlaneBow; 
            int skyPlaneXSegments; 
            int skyPlaneYSegments; 
        };

        struct SkyBoxGenParameters
        {
            Real skyBoxDistance;
        };

        typedef vector<SceneNode*>::type SceneNodeList;
        typedef vector<MovableObject*>::type MovableObjectVec;

        /** Class that allows listening in on the various stages of SceneManager
            processing, so that custom behaviour can be implemented from outside.
        */
        class Listener
        {
        public:
            Listener() {}
            virtual ~Listener() {}

            /** Called prior to searching for visible objects in this SceneManager.
            @remarks
                Note that the render queue at this stage will be full of the last
                render's contents and will be cleared after this method is called.
            @param source The SceneManager instance raising this event.
            @param irs The stage of illumination being dealt with. IRS_NONE for 
                a regular render, IRS_RENDER_TO_TEXTURE for a shadow caster render.
            @param v The viewport being updated. You can get the camera from here.
            */
            virtual void preFindVisibleObjects(SceneManager* source, 
                IlluminationRenderStage irs, Viewport* v)
                        { (void)source; (void)irs; (void)v; }

            /** Called after searching for visible objects in this SceneManager.
            @remarks
                Note that the render queue at this stage will be full of the current
                scenes contents, ready for rendering. You may manually add renderables
                to this queue if you wish.
            @param source The SceneManager instance raising this event.
            @param irs The stage of illumination being dealt with. IRS_NONE for 
                a regular render, IRS_RENDER_TO_TEXTURE for a shadow caster render.
            @param v The viewport being updated. You can get the camera from here.
            */
            virtual void postFindVisibleObjects(SceneManager* source, 
                IlluminationRenderStage irs, Viewport* v)
                        { (void)source; (void)irs; (void)v; }

            /** Event raised after all shadow textures have been rendered into for 
                all queues / targets but before any other geometry has been rendered
                (including main scene geometry). 
            @remarks
                This callback is useful for those that wish to perform some 
                additional processing on shadow textures before they are used to 
                render shadows. For example you could perform some filtering by 
                rendering the existing shadow textures into another alternative 
                shadow texture with a shader.]
            @note
                This event will only be fired when texture shadows are in use.
            @param numberOfShadowTextures The number of shadow textures in use
            */
            virtual void shadowTexturesUpdated(size_t numberOfShadowTextures)
                        { (void)numberOfShadowTextures; }

            /** This event occurs just before the view & projection matrices are
                set for rendering into a shadow texture.
            @remarks
                You can use this event hook to perform some custom processing,
                such as altering the camera being used for rendering the light's
                view, including setting custom view & projection matrices if you
                want to perform an advanced shadow technique.
            @note
                This event will only be fired when texture shadows are in use.
            @param light Pointer to the light for which shadows are being rendered
            @param camera Pointer to the camera being used to render
            @param iteration For lights that use multiple shadow textures, the iteration number
            */
            virtual void shadowTextureCasterPreViewProj( const Light* light, 
                Camera* camera, size_t iteration)
                        { (void)light; (void)camera; (void)iteration; }

            /** Hook to allow the listener to override the ordering of lights for
                the entire frustum.
            @remarks
                Whilst ordinarily lights are sorted per rendered object 
                (@see MovableObject::queryLights), texture shadows adds another issue
                in that, given there is a finite number of shadow textures, we must
                choose which lights to render texture shadows from based on the entire
                frustum. These lights should always be listed first in every objects
                own list, followed by any other lights which will not cast texture 
                shadows (either because they have shadow casting off, or there aren't
                enough shadow textures to service them).
            @par
                This hook allows you to override the detailed ordering of the lights
                per frustum. The default ordering is shadow casters first (which you 
                must also respect if you override this method), and ordered
                by distance from the camera within those 2 groups. Obviously the closest
                lights with shadow casting enabled will be listed first. Only lights 
                within the range of the frustum will be in the list.
            @param lightList The list of lights within range of the frustum which you
                may sort.
            @return true if you sorted the list, false otherwise.
            */
            virtual bool sortLightsAffectingFrustum(LightList& lightList)
                        { (void)lightList; return false; }

            /** Event notifying the listener of the SceneManager's destruction. */
            virtual void sceneManagerDestroyed(SceneManager* source)
                        { (void)source; }
        };

    protected:
        /// Subclasses can override this to ensure their specialised SceneNode is used.
        virtual SceneNode* createSceneNodeImpl( SceneNode *parent,
                                                NodeMemoryManager *nodeMemoryManager );
        virtual TagPoint* createTagPointImpl( SceneNode *parent,
                                              NodeMemoryManager *nodeMemoryManager );

        typedef vector<NodeMemoryManager*>::type NodeMemoryManagerVec;
        typedef vector<ObjectMemoryManager*>::type ObjectMemoryManagerVec;
        typedef vector<SkeletonAnimManager*>::type SkeletonAnimManagerVec;

        /** These are the main memory managers. Note that some Scene Managers may have more than one
            memory manager (eg. one per Octant in an Octree implementation, one per Portal, etc)
            Those managers can, at the start of scene graph update, transfer/move the objects created
            in the main mem. managers into their localized versions.

            During @see highLevelCull, those scene managers will update mNodeMemoryManagerCulledList
            and co. to indicate which memory managers should be traversed for rendering.
        */
        NodeMemoryManager       mNodeMemoryManager[NUM_SCENE_MEMORY_MANAGER_TYPES];
        ObjectMemoryManager     mEntityMemoryManager[NUM_SCENE_MEMORY_MANAGER_TYPES];
        ObjectMemoryManager     mLightMemoryManager;
        SkeletonAnimManager     mSkeletonAnimationManager;
        NodeMemoryManager       mTagPointNodeMemoryManager;
        /// Filled and cleared every frame in HighLevelCull()
        NodeMemoryManagerVec    mNodeMemoryManagerUpdateList;
        NodeMemoryManagerVec    mTagPointNodeMemoryManagerUpdateList;
        ObjectMemoryManagerVec  mEntitiesMemoryManagerCulledList;
        ObjectMemoryManagerVec  mEntitiesMemoryManagerUpdateList;
        ObjectMemoryManagerVec  mLightsMemoryManagerCulledList;
        SkeletonAnimManagerVec  mSkeletonAnimManagerCulledList;

        /** Minimum depth level at which mNodeMemoryManager[SCENE_STATIC] is dirty.
        @remarks
            We do an optimization: We know for sure that if node at level N became dirty,
            we only need to update nodes starting from level N, N+1, N+2, ..., N+n
            No need to update between level 0, 1, 2, ..., N-1
        */
        uint16                  mStaticMinDepthLevelDirty;

        /** Whether mEntityMemoryManager[SCENE_STATIC] is dirty (assume all render queues,
            you shouldn't be doing this often anyway!)
        */
        bool                    mStaticEntitiesDirty;

        PrePassMode             mPrePassMode;
        TextureVec const        *mPrePassTextures;
        TextureVec const        *mPrePassDepthTexture;
        TextureVec const        *mSsrTexture;

        /// Instance name
        String mName;

        /// Queue of objects for rendering
        RenderQueue* mRenderQueue;

        ForwardPlusBase *mForwardPlusSystem;
        ForwardPlusBase *mForwardPlusImpl;

        /// Updated every frame, has enough memory to hold all lights.
        /// The order is not deterministic, it depends on the number
        /// of worker threads.
        LightListInfo mGlobalLightList;
        typedef FastArray<LightArray> LightArrayPerThread;
        typedef FastArray<BuildLightListRequest> BuildLightListRequestPerThread;
        LightArrayPerThread             mGlobalLightListPerThread;
        BuildLightListRequestPerThread  mBuildLightListRequestPerThread;

        /// Current ambient light.
        ColourValue mAmbientLight[2];
        Vector3     mAmbientLightHemisphereDir;

        /// The rendering system to send the scene to
        RenderSystem *mDestRenderSystem;

        typedef vector<Camera*>::type CameraList;
        typedef map<IdString, Camera*>::type CameraMap;

        /** Central list of cameras - for easy memory management and lookup.
        */
        CameraList  mCameras;
        CameraMap   mCamerasByName;
        FrustumVec  mVisibleCameras;
        FrustumVec  mCubeMapCameras;

        typedef vector<WireAabb*>::type WireAabbVec;
        WireAabbVec mTrackingWireAabbs;

        typedef map<String, v1::StaticGeometry* >::type StaticGeometryList;
        StaticGeometryList mStaticGeometryList;

        typedef vector<v1::InstanceManager*>::type  InstanceManagerVec;
        InstanceManagerVec  mInstanceManagers;

        /** Central list of SceneNodes - for easy memory management.
            @note
                Note that this list is used only for memory management; the structure of the scene
                is held using the hierarchy of SceneNodes starting with the root node. However you
                can look up nodes this way.
        */
        SceneNodeList   mSceneNodes;
        SceneNodeList   mSceneNodesWithListeners;

        /// Camera in progress
        Camera* mCameraInProgress;
        /// Current Viewport
        Viewport* mCurrentViewport;

        CompositorPass          *mCurrentPass;
        CompositorShadowNode    *mCurrentShadowNode;
        bool                    mShadowNodeIsReused;

        /// Root scene node
        SceneNode* mSceneRoot[NUM_SCENE_MEMORY_MANAGER_TYPES];
        SceneNode* mSceneDummy;

        /// Autotracking scene nodes
		struct AutoTrackingSceneNode
		{
			SceneNode *source;
			SceneNode *target;
			/// Tracking offset for fine tuning
			Vector3		offset;
			/// Local 'normal' direction vector
			Vector3		localDirection;

			AutoTrackingSceneNode( SceneNode *_source, SceneNode *_target,
								   const Vector3 &_offset, const Vector3 &_localDirection ) :
				source( _source ), target( _target ),
				offset( _offset ), localDirection( _localDirection )
			{
			}
		};

		typedef vector<AutoTrackingSceneNode>::type AutoTrackingSceneNodeVec;
		AutoTrackingSceneNodeVec mAutoTrackingSceneNodes;

        // Sky params
        // Sky plane
        v1::Entity* mSkyPlaneEntity;
        v1::Entity* mSkyDomeEntity[5];
        v1::ManualObject* mSkyBoxObj;

        SceneNode* mSkyPlaneNode;
        SceneNode* mSkyDomeNode;
        SceneNode* mSkyBoxNode;

        // Sky plane
        bool mSkyPlaneEnabled;
        Plane mSkyPlane;
        SkyPlaneGenParameters mSkyPlaneGenParameters;
        // Sky box
        bool mSkyBoxEnabled;
        Quaternion mSkyBoxOrientation;
        SkyBoxGenParameters mSkyBoxGenParameters;
        // Sky dome
        bool mSkyDomeEnabled;
        Quaternion mSkyDomeOrientation;
        SkyDomeGenParameters mSkyDomeGenParameters;

        // Fog
        FogMode mFogMode;
        ColourValue mFogColour;
        Real mFogStart;
        Real mFogEnd;
        Real mFogDensity;
        
        unsigned long mLastFrameNumber;
        OGRE_SIMD_ALIGNED_DECL( Matrix4, mTempXform[256] );
        bool mResetIdentityView;
        bool mResetIdentityProj;

        bool mFlipCullingOnNegativeScale;
        CullingMode mPassCullingMode;

    protected:
        /// Array defining shadow texture index in light list.
        vector<size_t>::type mShadowTextureIndexLightList;

        /// Cached light information, used to tracking light's changes
        struct _OgreExport LightInfo
        {
            Light* light;       /// Just a pointer for comparison, the light might destroyed for some reason
            int type;           /// Use int instead of Light::LightTypes to avoid header file dependence
            Real range;         /// Sets to zero if directional light
            Vector3 position;   /// Sets to zero if directional light
            uint32 lightMask;   /// Light mask

            bool operator== (const LightInfo& rhs) const
            {
                return light == rhs.light && type == rhs.type &&
                    range == rhs.range && position == rhs.position && lightMask == rhs.lightMask;
            }

            bool operator!= (const LightInfo& rhs) const
            {
                return !(*this == rhs);
            }
        };

        typedef vector<LightInfo>::type LightInfoList;

        /// Simple structure to hold MovableObject map and a mutex to go with it.
        struct MovableObjectCollection
        {
            MovableObjectVec movableObjects;
            OGRE_MUTEX(mutex);
        };
        typedef map<String, MovableObjectCollection*>::type MovableObjectCollectionMap;
        MovableObjectCollectionMap mMovableObjectCollectionMap;
        /** Gets the movable object collection for the given type name.
        @remarks
            This method create new collection if the collection does not exist.
        */
        MovableObjectCollection* getMovableObjectCollection(const String& typeName);
        /** Gets the movable object collection for the given type name.
        @remarks
            This method throw exception if the collection does not exist.
        */
        const MovableObjectCollection* getMovableObjectCollection(const String& typeName) const;
        /// Mutex over the collection of MovableObject types
        OGRE_MUTEX(mMovableObjectCollectionMapMutex);

        /// A pass designed to let us render shadow colour on white for texture shadows
        Pass* mShadowCasterPlainBlackPass;
        /** Internal method for turning a regular pass into a shadow caster pass.
        @remarks
            This is only used for texture shadows, basically we're trying to
            ensure that objects are rendered solid black.
            This method will usually return the standard solid black pass for
            all fixed function passes, but will merge in a vertex program
            and fudge the AutoParamDataSource to set black lighting for
            passes with vertex programs. 
        */
        virtual_l2 const Pass* deriveShadowCasterPass(const Pass* pass);
    
        /** Internal method to validate whether a Pass should be allowed to render.
        @remarks
            Called just before a pass is about to be used for rendering a group to
            allow the SceneManager to omit it if required. A return value of false
            skips this pass. 
        */
        virtual_l2 bool validatePassForRendering(const Pass* pass);

        enum BoxPlane
        {
            BP_FRONT = 0,
            BP_BACK = 1,
            BP_LEFT = 2,
            BP_RIGHT = 3,
            BP_UP = 4,
            BP_DOWN = 5
        };

        /* Internal utility method for creating the planes of a skybox.
        */
        virtual v1::MeshPtr createSkyboxPlane(
            BoxPlane bp,
            Real distance,
            const Quaternion& orientation,
            const String& groupName);

        /* Internal utility method for creating the planes of a skydome.
        */
        virtual v1::MeshPtr createSkydomePlane(
            BoxPlane bp,
            Real curvature, Real tiling, Real distance,
            const Quaternion& orientation,
            int xsegments, int ysegments, int ySegmentsToKeep, 
            const String& groupName);

        /// Flag indicating whether SceneNodes will be rendered as a set of 3 axes
        bool mDisplayNodes;

        /// Storage of animations, lookup by name
        typedef map<String, v1::Animation*>::type AnimationList;
        AnimationList mAnimationsList;
        OGRE_MUTEX(mAnimationsListMutex);
        v1::AnimationStateSet mAnimationStates;


        /** Internal method used by _renderSingleObject to deal with renderables
            which override the camera's own view / projection materices. */
        virtual void useRenderableViewProjMode(const Renderable* pRend, bool fixedFunction);
        
        /** Internal method used by _renderSingleObject to deal with renderables
            which override the camera's own view / projection matrices. */
        virtual void resetViewProjMode(bool fixedFunction);

        typedef vector<RenderQueueListener*>::type RenderQueueListenerList;
        RenderQueueListenerList mRenderQueueListeners;

        typedef vector<RenderObjectListener*>::type RenderObjectListenerList;
        RenderObjectListenerList mRenderObjectListeners;
        typedef vector<Listener*>::type ListenerList;
        ListenerList mListeners;
        /// Internal method for firing the queue start event
        virtual void firePreRenderQueues();
        /// Internal method for firing the queue end event
        virtual void firePostRenderQueues();
        /// Internal method for firing the queue start event, returns true if queue is to be skipped
        virtual bool fireRenderQueueStarted(uint8 id, const String& invocation);
        /// Internal method for firing the queue end event, returns true if queue is to be repeated
        virtual bool fireRenderQueueEnded(uint8 id, const String& invocation);
        /// Internal method for firing when rendering a single object.
        virtual void fireRenderSingleObject(Renderable* rend, const Pass* pass,
                                            const AutoParamDataSource* source,
                                            const LightList* pLightList,
                                            bool suppressRenderStateChanges);

        /// Internal method for firing the texture shadows updated event
        virtual void fireShadowTexturesUpdated(size_t numberOfShadowTextures);
        /// Internal method for firing the pre caster texture shadows event
        virtual void fireShadowTexturesPreCaster(const Light* light, Camera* camera, size_t iteration);
        /// Internal method for firing find visible objects event
        virtual void firePreFindVisibleObjects(Viewport* v);
        /// Internal method for firing find visible objects event
        virtual void firePostFindVisibleObjects(Viewport* v);
        /// Internal method for firing destruction event
        virtual void fireSceneManagerDestroyed();
        /** Internal method for setting the destination viewport for the next render. */
        virtual void setViewport(Viewport *vp);

        /** Flag that indicates if all of the scene node's bounding boxes should be shown as a wireframe. */
        bool mShowBoundingBoxes;      

        /** Internal method for preparing the render queue for use with each render. */
        virtual void prepareRenderQueue(void);


        /** Internal utility method for rendering a single object. 
        @remarks
            Assumes that the pass has already been set up.
        @param rend The renderable to issue to the pipeline
        @param pass The pass which is being used
        @param lightScissoringClipping If true, passes that have the getLightScissorEnabled
            and/or getLightClipPlanesEnabled flags will cause calculation and setting of 
            scissor rectangle and user clip planes. 
        @param doLightIteration If true, this method will issue the renderable to
            the pipeline possibly multiple times, if the pass indicates it should be
            done once per light
        @param manualLightList Only applicable if doLightIteration is false, this
            method allows you to pass in a previously determined set of lights
            which will be used for a single render of this object.
        */
        virtual_l1 void renderSingleObject(Renderable* rend, const Pass* pass, 
            bool lightScissoringClipping, bool doLightIteration);

        /** Internal method for creating the AutoParamDataSource instance. */
        virtual AutoParamDataSource* createAutoParamDataSource(void) const
        {
            return OGRE_NEW AutoParamDataSource();
        }

        /// Utility class for calculating automatic parameters for gpu programs
        AutoParamDataSource* mAutoParamDataSource;

        bool mLateMaterialResolving;

        ColourValue mShadowColour;
        v1::HardwareIndexBufferSharedPtr mShadowIndexBuffer;
        size_t mShadowIndexBufferUsedSize;
        v1::Rectangle2D* mFullScreenQuad;
        Real mShadowDirLightExtrudeDist;
        IlluminationRenderStage mIlluminationStage;
        /// Struct for caching light clipping information for re-use in a frame
        struct LightClippingInfo
        {
            RealRect scissorRect;
            PlaneList clipPlanes;
            bool scissorValid;
            unsigned long clipPlanesValid;
            LightClippingInfo() : scissorValid(false), clipPlanesValid(false) {}

        };
        typedef map<Light const *, LightClippingInfo>::type LightClippingInfoMap;
        LightClippingInfoMap mLightClippingInfoMap;
        unsigned long mLightClippingInfoMapFrameNumber;

        /** To be used with MovableObjects. Checks that the input pointer's mGlobalIndex
            is consistent with the container, otherwise there's a bug and mGlobalIndex
            is out of date (or could be caused by memory corruption, of course)
        @remarks
            It is a template since it's legal to pass a vector<Camera> since Camera
            is derived from MovableObject too; but would fail compilation.
        @param container
            The container that allegedly holds the mo.
        @param mo
            MovableObject whose integrity is going to be checked
        */
        template<typename T>
        void checkMovableObjectIntegrity( const typename vector<T*>::type &container,
                                            const T *mo ) const;

#ifdef OGRE_LEGACY_ANIMATIONS
        /// Updates all instance managers' animations
        void updateInstanceManagerAnimations(void);
#endif

        /** Updates all instance managers with dirty instance batches from multiple threads.
            @see updateInstanceManagers and @see InstanceBatch::_updateEntitiesBoundsThread */
        void updateInstanceManagersThread( size_t threadIdx );

        /** Updates all instance managers with dirty instance batches. @see _addDirtyInstanceManager */
        void updateInstanceManagers(void);

        /** Culls the scene in a high level fashion (i.e. Octree, Portal, etc.) by taking into account all
            registered cameras. Produces a list of culled Entities & SceneNodes that must follow a very
            strict set of rules:
                * Entities are separated by RenderQueue
                * Entities sharing the same skeleton need to be adjacent (TODO: Required? dark_sylinc)
                * SceneNodes must be separated by hierarchy depth and must be contiguous within the same
                  depth level. (@see mNodeMemoryManagerCulledList)
          @remarks
            The default implementation just returns all nodes in the scene. @See updateAllTransforms
            @see updateSceneGraph
        */
        virtual void highLevelCull();

        /** Permanently applies the relative origin change and propagates to children nodes
        @remarks
            Relative origins should happen at the root level. If both a parent & children apply
            the relative origin, we would be applying the offset twice.
            This algorithm works by analyzing if we can keep moving the change to children nodes.
            We can only do so if the parent node doesn't have attachments, or if their attachments
            are all Cameras.
        */
        void propagateRelativeOrigin( SceneNode *sceneNode, const Vector3 &relativeOrigin );
        
    public:

        //A render context, used to store internal data for pausing/resuming rendering
        struct RenderContext
        {
            RenderQueue* renderQueue;   
            Viewport* viewport;
            Camera* camera;
            RenderSystem::RenderSystemContext* rsContext;
        };

        /** Pause rendering of the frame. This has to be called when inside a renderScene call
            (Usually using a listener of some sort)
        */
        virtual RenderContext* _pauseRendering();
        /** Resume rendering of the frame. This has to be called after a _pauseRendering call
        @param context The rendring context, as returned by the _pauseRendering call
        */
        virtual void _resumeRendering(RenderContext* context);

    protected:
        Real mDefaultShadowFarDist;
        Real mDefaultShadowFarDistSquared;
        Real mShadowTextureOffset; /// Proportion of texture offset in view direction e.g. 0.4
        Real mShadowTextureFadeStart; /// As a proportion e.g. 0.6
        Real mShadowTextureFadeEnd; /// As a proportion e.g. 0.9
        Pass* mShadowTextureCustomCasterPass;
        String mShadowTextureCustomCasterVertexProgram;
        String mShadowTextureCustomCasterFragmentProgram;
        GpuProgramParametersSharedPtr mShadowTextureCustomCasterVPParams;
        GpuProgramParametersSharedPtr mShadowTextureCustomCasterFPParams;

        CompositorTextureVec        mCompositorTextures;
        CompositorTexture           mCompositorTarget;

        /// Visibility mask used to show / hide objects
        uint32 mVisibilityMask;
        bool mFindVisibleObjects;

        enum RequestType
        {
            CULL_FRUSTUM,
            UPDATE_ALL_ANIMATIONS,
            UPDATE_ALL_TRANSFORMS,
            UPDATE_ALL_BONE_TO_TAG_TRANSFORMS,
            UPDATE_ALL_TAG_ON_TAG_TRANSFORMS,
            UPDATE_ALL_BOUNDS,
            UPDATE_ALL_LODS,
            UPDATE_INSTANCE_MANAGERS,
            CULL_FRUSTUM_INSTANCEDENTS,
            BUILD_LIGHT_LIST01,
            BUILD_LIGHT_LIST02,
            USER_UNIFORM_SCALABLE_TASK,
            STOP_THREADS,
            NUM_REQUESTS
        };

        size_t mNumWorkerThreads;

        CullFrustumRequest              mCurrentCullFrustumRequest;
        UpdateLodRequest                mUpdateLodRequest;
        UpdateTransformRequest          mUpdateTransformRequest;
        ObjectMemoryManagerVec const    *mUpdateBoundsRequest;
        InstancingThreadedCullingMethod mInstancingThreadedCullingMethod;
        InstanceBatchCullRequest        mInstanceBatchCullRequest;
        UniformScalableTask *mUserTask;
        RequestType         mRequestType;
        Barrier             *mWorkerThreadsBarrier;
        ThreadHandleVec     mWorkerThreads;

        /** Contains MovableObjects to be visited and rendered.
        @rermarks
            Declared here to avoid allocating and deallocating every frame. Declared as array of
            arrays (vector of vectors) for multithreading purposes (put results in one array while
            another thread stores in the other array). @See cullFrustum
        */
        VisibleObjectsPerThreadArray mVisibleObjects;

        /** @See mVisibleObjects. This one is a variable used for temporary storage by (eg.) Instance
            Managers to cull their internal instanced entities from multiple threads. We do not
            guarantee that those who acquired our data retain sole ownership; thus extra care may
            be needed to ensure that no two separate systems request this variable at the same time
            Retrieve this buffer using @see _getTmpVisibleObjectsList
        */
        VisibleObjectsPerThreadArray mTmpVisibleObjects;

        /// Suppress render state changes?
        bool mSuppressRenderStateChanges;

        /// Set up a scissor rectangle from a group of lights
        virtual ClipResult buildAndSetScissor(const LightList& ll, const Camera* cam);
        /// Update a scissor rectangle from a single light
        virtual void buildScissor(const Light* l, const Camera* cam, RealRect& rect);
        /// Build a set of user clip planes from a single non-directional light
        virtual ClipResult buildAndSetLightClip(const LightList& ll);
        virtual void buildLightClip(const Light* l, PlaneList& planes);
        virtual void resetLightClip();
        virtual void checkCachedLightClippingInfo();

        /// Whether to use camera-relative rendering
        Matrix4 mCachedViewMatrix;
        Vector3 mRelativeOffset;

        /// Last light sets
        uint32 mLastLightHash;
        unsigned short mLastLightLimit;
        uint32 mLastLightHashGpuProgram;
        /// Gpu params that need rebinding (mask of GpuParamVariability)
        uint16 mGpuParamsDirty;

        virtual void useLights(const LightList& lights, unsigned short limit);
        virtual void setViewMatrix(const Matrix4& m);
        virtual void useLightsGpuProgram(const Pass* pass, const LightList* lights);
        virtual void updateGpuProgramParameters(const Pass* p);








        /// Set of registered LOD listeners
        typedef set<LodListener*>::type LodListenerSet;
        LodListenerSet mLodListeners;

        /// List of movable object LOD changed events
        typedef vector<MovableObjectLodChangedEvent>::type MovableObjectLodChangedEventList;
        MovableObjectLodChangedEventList mMovableObjectLodChangedEvents;

        /// List of entity mesh LOD changed events
        typedef vector<EntityMeshLodChangedEvent>::type EntityMeshLodChangedEventList;
        EntityMeshLodChangedEventList mEntityMeshLodChangedEvents;

        /// List of entity material LOD changed events
        typedef vector<EntityMaterialLodChangedEvent>::type EntityMaterialLodChangedEventList;
        EntityMaterialLodChangedEventList mEntityMaterialLodChangedEvents;

        /** Updates the Animations from the given request inside a thread. @See updateAllAnimations
        @param threadIdx
            Thread index so we know at which point we should start at.
            Must be unique for each worker thread
        */
        void updateAllAnimationsThread( size_t threadIdx );
        void updateAnimationTransforms( BySkeletonDef &bySkeletonDef, size_t threadIdx );

        /** Updates the Nodes from the given request inside a thread. @See updateAllTransforms
        @param request
            Fully setup request. @See UpdateTransformRequest.
        @param threadIdx
            Thread index so we know at which point we should start at.
            Must be unique for each worker thread
        */
        void updateAllTransformsThread( const UpdateTransformRequest &request, size_t threadIdx );

        /// @see TagPoint::updateAllTransformsBoneToTag
        void updateAllTransformsBoneToTagThread( const UpdateTransformRequest &request,
                                                 size_t threadIdx );

        /// @see TagPoint::updateAllTransformsTagOnTag
        void updateAllTransformsTagOnTagThread( const UpdateTransformRequest &request,
                                                size_t threadIdx );

        /** Updates the world aabbs from the given request inside a thread. @See updateAllTransforms
        @param threadIdx
            Thread index so we know at which point we should start at.
            Must be unique for each worker thread
        */
        void updateAllBoundsThread( const ObjectMemoryManagerVec &objectMemManager, size_t threadIdx );

        /**
        @param threadIdx
            Thread index so we know at which point we should start at.
            Must be unique for each worker thread
        */
        void updateAllLodsThread( const UpdateLodRequest &request, size_t threadIdx );

        /** Traverses mVisibleObjects[threadIdx] from each thread to call
            MovableObject::instanceBatchCullFrustumThreaded (which is supposed to cull objects)
        @param threadIdx
            Thread index so we know at which point we should start at.
            Must be unique for each worker thread
        */
        void instanceBatchCullFrustumThread( const InstanceBatchCullRequest &request, size_t threadIdx );

        /** Low level culling, culls all objects against the given frustum active cameras. This
            includes checking visibility flags (both scene and viewport's)
            @See MovableObject::cullFrustum
        @param request
            Fully setup request. @See CullFrustumRequest.
        @param threadIdx
            Index to mVisibleObjects so we know which array we should start at.
            Must be unique for each worker thread
        */
        void cullFrustum( const CullFrustumRequest &request, size_t threadIdx );

        /** Builds a list of all lights that are visible by all queued cameras (this should be fed by
            Compositor). Then calls MovableObject::buildLightList with that list so that each
            MovableObject gets it's own sorted list of the closest lights.
        @remarks
            @See MovableObject::buildLightList()
        */
        void buildLightList();

        void buildLightListThread01( const BuildLightListRequest &buildLightListRequest,
                                     size_t threadIdx );
        void buildLightListThread02( size_t threadIdx );

    public:
        /** Constructor.
        */
        SceneManager(const String& instanceName, size_t numWorkerThreads,
                    InstancingThreadedCullingMethod threadedCullingMethod);

        /** Default destructor.
        */
        virtual ~SceneManager();


        /** Mutex to protect the scene graph from simultaneous access from
            multiple threads.
        @remarks
            If you are updating the scene in a separate thread from the rendering
            thread, then you should lock this mutex before making any changes to 
            the scene graph - that means creating, modifying or deleting a
            scene node, or attaching / detaching objects. It is <b>your</b> 
            responsibility to take out this lock, the detail methods on the nodes
            will not do it for you (for the reasons discussed below).
        @par
            Note that locking this mutex will prevent the scene being rendered until 
            it is unlocked again. Therefore you should do this sparingly. Try
            to create any objects you need separately and fully prepare them
            before doing all your scene graph work in one go, thus keeping this
            lock for the shortest time possible.
        @note
            A single global lock is used rather than a per-node lock since 
            it keeps the number of locks required during rendering down to a 
            minimum. Obtaining a lock, even if there is no contention, is not free
            so for performance it is good to do it as little as possible. 
            Since modifying the scene in a separate thread is a fairly
            rare occurrence (relative to rendering), it is better to keep the 
            locking required during rendering lower than to make update locks
            more granular.
        */
        OGRE_MUTEX(sceneGraphMutex);

        /** Return the instance name of this SceneManager. */
        const String& getName(void) const { return mName; }

        /** Retrieve the type name of this scene manager.
        @remarks
            This method has to be implemented by subclasses. It should
            return the type name of this SceneManager which agrees with 
            the type name of the SceneManagerFactory which created it.
        */
        virtual const String& getTypeName(void) const = 0;

        size_t getNumWorkerThreads() const                          { return mNumWorkerThreads; }

        /// Finds all the movable objects with the type and name passed as parameters.
        virtual MovableObjectVec findMovableObjects( const String& type, const String& name );

        /** Creates a camera to be managed by this scene manager.
            @remarks
                This camera is automatically added to the scene by being attached to the Root
                Scene Node before returning. If you want to use your own SceneNode for this
                camera, you'll have to detach it first
                (can be done via camera->detachFromParent();)
            @param name
                Name to give the new camera. Must be unique.
            @param notShadowCaster
                True if this camera should be considered when creating the global light list
                of culled lights against all cameras. For example, cameras used for
                shadow mapping shouldn't be taken into account (set to false)
            @param forCubemapping
                True this camera will be used at least once in one of its passes as a cubemap
                (thus having to change the orientation but not position mid-rendering)
        */
        virtual Camera* createCamera(const String& name, bool notShadowCaster=true, bool forCubemapping=false);

        /** Finds the camera with the given name. Throws if not found.
        @param name
            Hash of the name of the camera to find
        @return
            Camera pointer
        */
        virtual Camera* findCamera( IdString name ) const;

        /** Finds the camera with the given name. Returns null pointer if not found.
        @param name
            Hash of the name of the camera to find
        @return
            Camera pointer. Null if not found.
        */
        virtual Camera* findCameraNoThrow( IdString name ) const;

        /** Removes a camera from the scene.
            @remarks
                This method removes a previously added camera from the scene.
                The camera is deleted so the caller must ensure no references
                to it's previous instance (e.g. in a SceneNode) are used.
            @param cam
                Pointer to the camera to remove
        */
        virtual void destroyCamera(Camera *cam);

        /** Removes (and destroys) all cameras from the scene.
            @remarks
                Some cameras are internal created to dealing with texture shadow,
                or compositor nodes. They aren't supposed to be destroyed outside.
                So, while you are using texture shadow, don't call this method.
        */
        virtual void destroyAllCameras(void);

        /// See Camera::setLightCullingVisibility
        void _setLightCullingVisibility( Camera *camera, bool collectLights, bool isCubemap );

        /** Creates a light for use in the scene.
            @remarks
                The direction and position of a light is managed via
                the scene node it is attached to. Make sure that you have
                attached the light to a scene node (SceneNode::attachObject)
                *before* calling Light::setDirection or Light::setPosition.
        */
        virtual Light* createLight();

        /// Clears temporary buffers and other data that needs to live every frame.
        void clearFrameData(void);

        const LightListInfo& getGlobalLightList(void) const { return mGlobalLightList; }

        /** Retrieve a set of clipping planes for a given light. 
        */
        virtual const PlaneList& getLightClippingPlanes(const Light* l);

        /** Retrieve a scissor rectangle for a given light and camera. 
        */
        virtual const RealRect& getLightScissorRect(const Light* l, const Camera* cam);

        /** Removes the light from the scene and destroys it based on a pointer.
            @remarks
                Any pointers held to this light after calling this method will be invalid.
        */
        virtual void destroyLight(Light* light);
        /** Removes and destroys all lights in the scene.
        */
        virtual void destroyAllLights(void);

        /// Don't call this function directly. @see TagPoint::createChildTagPoint
        virtual TagPoint* _createTagPoint( SceneNode *parent, NodeMemoryManager *nodeMemoryManager );

        /// Creates a TagPoint that can be used like a SceneNode, or be used to be
        /// attached to a Bone. @see Bone::addTagPoint
        virtual TagPoint* createTagPoint(void);

        /** @see createSceneNode. This functions exists to satisfy @see SceneNode::createChildImpl
            Don't call this function directly
            @par
                Parent to the scene node we're creating.
        */
        virtual SceneNode* _createSceneNode( SceneNode *parent, NodeMemoryManager *nodeMemoryManager );

        /** Creates an instance of a SceneNode.
            @remarks
                Note that this does not add the SceneNode to the scene hierarchy.
                This method is for convenience, since it allows an instance to
                be created for which the SceneManager is responsible for
                allocating and releasing memory, which is convenient in complex
                scenes.
            @par
                To include the returned SceneNode in the scene, use the addChild
                method of the SceneNode which is to be it's parent.
            @par
                Note that this method takes no parameters, and the node created is unnamed (it is
                actually given a generated name, which you can retrieve if you want).
                If you wish to create a node with a specific name, call the alternative method
                which takes a name parameter.
            @params sceneType
                Dynamic if this node is to be updated frequently. Static if you don't plan to be
                updating this node in a long time (performance optimization).
        */
        virtual SceneNode* createSceneNode( SceneMemoryMgrTypes sceneType = SCENE_DYNAMIC );

        /** Destroys a SceneNode.
        @remarks
            This allows you to physically delete an individual SceneNode if you want to.
            Note that this is not normally recommended, it's better to allow SceneManager
            to delete the nodes when the scene is cleared.
        */
        virtual void destroySceneNode(SceneNode* sn);

        /** Gets the SceneNode at the root of the scene hierarchy.
            @remarks
                The entire scene is held as a hierarchy of nodes, which
                allows things like relative transforms, general changes in
                rendering state etc (See the SceneNode class for more info).
                In this basic SceneManager class, the application using
                Ogre is free to structure this hierarchy however it likes,
                since it has no real significance apart from making transforms
                relative to each node (more specialised subclasses will
                provide utility methods for building specific node structures
                e.g. loading a BSP tree).
            @par
                However, in all cases there is only ever one root node of
                the hierarchy, and this method returns a pointer to it.
                There is actually an extra Root Node so that static
                objects can be attached to it. Note however, static nodes
                can be children of a dynamic root node.
        */
        SceneNode* getRootSceneNode( SceneMemoryMgrTypes sceneType = SCENE_DYNAMIC );

        /// Unlike mNodeMemoryManager->_getDummyNode(), this dummy node is fully allocated,
        /// which makes it possible to actually attach objects to this dummy, while
        /// we guarantee the dummy won't change its transform.
        SceneNode* getDummySceneNode(void) const        { return mSceneDummy; }

        /** Retrieves a SceneNode based on it's ID from the scene graph.
        @remarks
            @note Returns null if the ID does not exist
            @note It is a linear search O(N), retrieves the first node found
            with that name (it's not unique)
        */
        virtual_l1 SceneNode* getSceneNode( IdType id );
        virtual_l1 const SceneNode* getSceneNode( IdType id ) const;

        /// Finds all the scene nodes with the name passed as parameter.
        virtual_l1 SceneNodeList findSceneNodes( const String& name ) const;

        /** Node listeners need to be registered with us so that they can be successfully called
            when calling updateAllTransforms. @See updateAllTransforms
        */
        virtual void registerSceneNodeListener( SceneNode *sceneNode );

        /** Unregisters a registered node for listening. @See registerSceneNodeListener
        */
        virtual void unregisterSceneNodeListener( SceneNode *sceneNode );

        /// Returns the RenderQueue.
        RenderQueue* getRenderQueue(void) const             { return mRenderQueue; }

        /** Enables or disables the Forward3D implementation for using many non-shadowed
            lights in the scene.
            See the Forward3D sample for more information.
        @remarks
            The Hlms implementation must support this feature in order to work.
            Calling this function can recreate resources (even if called multiple
            times with the same exact paramters). Don't do it often.
        @param bEnable
            True to enable it. False to disable it.
        @param width
            The width of the view-space grid. Recommended value is 4 unless
            numSlices is very small.
        @param height
            The height of the view-space grid. Recommended value is 4 unless
            numSlices is very small.
        @param numSlices
            The number of slices. Each additional slice consumes much more memory.
            The width and height is doubled on each slice. It's like mipmapping
            but on reverse.
        @param lightsPerCell
            The maximum number of lights a cell in the grid can hold.
        @param minDistance
            Bias towards the camera for grid.
        @param maxDistance
            How far the grid array can go.
        */
        void setForward3D( bool bEnable, uint32 width, uint32 height, uint32 numSlices,
                           uint32 lightsPerCell, float minDistance, float maxDistance );

        void setForwardClustered( bool bEnable, uint32 width, uint32 height, uint32 numSlices,
                                  uint32 lightsPerCell, float minDistance, float maxDistance );

        ForwardPlusBase* getForwardPlus(void)                       { return mForwardPlusSystem; }
        ForwardPlusBase* _getActivePassForwardPlus(void)            { return mForwardPlusImpl; }

        /// For internal use.
        /// @see CompositorPassSceneDef::mEnableForwardPlus
        void _setForwardPlusEnabledInPass( bool bEnable );

        /// For internal use.
        /// @see CompositorPassSceneDef::mPrePassMode
        void _setPrePassMode( PrePassMode mode, const TextureVec *prepassTextures,
                              const TextureVec *prepassDepthTexture, const TextureVec *ssrTexture );
        PrePassMode getCurrentPrePassMode(void) const               { return mPrePassMode; }
        const TextureVec* getCurrentPrePassTextures(void) const     { return mPrePassTextures; }
        const TextureVec* getCurrentPrePassDepthTexture(void) const { return mPrePassDepthTexture; }
        const TextureVec* getCurrentSsrTexture(void) const          { return mSsrTexture; }

        NodeMemoryManager& _getNodeMemoryManager(SceneMemoryMgrTypes sceneType)
                                                                { return mNodeMemoryManager[sceneType]; }
        NodeMemoryManager& _getTagPointNodeMemoryManager(void)  { return mTagPointNodeMemoryManager; }

        /** Retrieves the main entity memory manager.
        @remarks
            Some Scene Managers may have more than one memory manager (e.g. one per octant in an
            Octree implementation). At end of scene graph update the scene manager may move
            the object created with the main memory manager into another another one.
        */
        ObjectMemoryManager& _getEntityMemoryManager(SceneMemoryMgrTypes sceneType)
                                                            { return mEntityMemoryManager[sceneType]; }
        ObjectMemoryManager& _getLightMemoryManager(void)
                                                            { return mLightMemoryManager; }

        /** Create an Item (instance of a discrete mesh).
            @param
                meshName The name of the Mesh it is to be based on (e.g. 'knot.oof'). The
                mesh will be loaded if it is not already.
        */
        virtual Item* createItem( const String& meshName,
                                  const String& groupName = ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
                                  SceneMemoryMgrTypes sceneType = SCENE_DYNAMIC );

        /** Create an Item (instance of a discrete mesh).
            @param
                pMesh The pointer to the Mesh it is to be based on.
        */
        virtual Item* createItem( const MeshPtr& pMesh,
                                  SceneMemoryMgrTypes sceneType = SCENE_DYNAMIC );

        /// Removes & destroys an Item from the SceneManager.
        virtual void destroyItem( Item *item );

        /// Removes & destroys all Items.
        virtual void destroyAllItems(void);

        /// Create an WireAabb
        virtual WireAabb* createWireAabb(void);

        /// Removes & destroys an WireAabb from the SceneManager.
        virtual void destroyWireAabb( WireAabb *wireAabb );

        /// Removes & destroys all WireAabbs.
        virtual void destroyAllWireAabbs(void);

        void _addWireAabb( WireAabb *wireAabb );
        void _removeWireAabb( WireAabb *wireAabb );

        /** Create an Entity (instance of a discrete mesh).
            @param
                meshName The name of the Mesh it is to be based on (e.g. 'knot.oof'). The
                mesh will be loaded if it is not already.
        */
        virtual v1::Entity* createEntity( const String& meshName,
                                          const String& groupName = ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
                                          SceneMemoryMgrTypes sceneType = SCENE_DYNAMIC );

        /** Create an Entity (instance of a discrete mesh).
            @param
                pMesh The pointer to the Mesh it is to be based on.
        */
        virtual v1::Entity* createEntity( const v1::MeshPtr& pMesh,
                                          SceneMemoryMgrTypes sceneType = SCENE_DYNAMIC );

        /** Prefab shapes available without loading a model.
            @note
                Minimal implementation at present.
            @todo
                Add more prefabs (teapots, teapots!!!)
        */
        enum PrefabType {
            PT_PLANE,
            PT_CUBE,
            PT_SPHERE
        };

        /** Create an Entity (instance of a discrete mesh) from a range of prefab shapes
            @param ptype The prefab type.
        */
        virtual v1::Entity* createEntity( PrefabType ptype,
                                          SceneMemoryMgrTypes sceneType = SCENE_DYNAMIC );

        /** Removes & destroys an Entity from the SceneManager.
            @warning
                Must only be done if the Entity is not attached
                to a SceneNode. It may be safer to wait to clear the whole
                scene if you are unsure use clearScene.
            @see
                SceneManager::clearScene
        */
        virtual void destroyEntity(v1::Entity* ent);

        /** Removes & destroys all Entities.
            @warning
                Again, use caution since no Entity must be referred to
                elsewhere e.g. attached to a SceneNode otherwise a crash
                is likely. Use clearScene if you are unsure (it clears SceneNode
                entries too.)
            @see
                SceneManager::clearScene
        */
        virtual void destroyAllEntities(void);

        /** Creates a 2D rectangle that can be displayed for screen space effects or
            showing a basic GUI.
            Notice that due to engine's requirements, you need to attach this object
            to a scene node in order to be rendered correctly.
        @par
            You can use the Root scene node. However if you're planning on also using
            setRelativeOrigin, beware that this would break the permanent setting and
            you should then use a dummy scene node for your rectangles.
        @remarks
            The Rectangle2D will request to use identity view and projection matrices,
            but only low level materials will honour that request. PBS shaders will
            ignore it (thus the rectangle will be drawn in 3D space) and other Hlms
            types may work differently and you'll have to check its documentation.
        @param bQuad
            When true, the rectangle is drawn with two triangles. When false, it is
            drawn as a single oversized triangle. Full screen triangles are faster
            than quads, but will only work correctly if they cover the entire screen,
            or are aided by scissor tests to clip the borders.
        @param sceneType
            Whether you will be moving the Rectangle2D's scene node around. Unless you're
            planning to use this Rectangle2D for 3D purposes, it is highly recomended that
            you use SCENE_STATIC (you can safely use SCENE_STATIC and change
            Rectangle2D::setCorners and Rectangle2D::setNormal every frame).
        @return
            The Rectangle2D.
        */
        virtual v1::Rectangle2D* createRectangle2D( bool bQuad,
                                                    SceneMemoryMgrTypes sceneType = SCENE_STATIC );

        /** Removes & destroys an Entity from the SceneManager.
        @warning
            It may be safer to wait to clear the whole scene if you are unsure use clearScene.
            @see SceneManager::clearScene
        */
        virtual void destroyRectangle2D( v1::Rectangle2D *rect );

        /** Removes & destroys all Rectangle2D.
            @warning
                Again, use caution since no Rectangle2D must be referred to
                elsewhere otherwise a crash is likely. Use clearScene if you
                are unsure (it clears SceneNode entries too.)
            @see
                SceneManager::clearScene
        */
        virtual void destroyAllRectangle2D(void);

        /** Used by Compositor, tells of which compositor textures active,
            so Materials can access them. If MRT, there could be more than one
        @param name
            Name of the texture(s) as referenced in the compositor
            (may not be the texture's real name in case it's just one)
        @param tex
            The actual texture(s) associated with that name
        */
        void _addCompositorTexture( IdString name, const TextureVec *texs );

        /// @see CompositorPassDef::mExposedTextures for the textures that are available
        /// in the current compositor pass. The compositor script keyword is "expose".
        const CompositorTextureVec& getCompositorTextures(void) const   { return mCompositorTextures; }

        /// Gets the number of currently active compositor textures
        size_t getNumCompositorTextures(void) const         { return mCompositorTextures.size(); }

        /// Removes all compositor textures from 'from' to end.
        void _removeCompositorTextures( size_t from );

        /// The compositor we are currently writing to.
        void _setCompositorTarget( const CompositorTexture &compoTarget );

        /// The compositor we are currently writing to.
        const CompositorTexture& getCompositorTarget(void) const   { return mCompositorTarget; }

        /// Creates an instance of a skeleton based on the given definition.
        SkeletonInstance* createSkeletonInstance( const SkeletonDef *skeletonDef );
        /// Destroys an instance of a skeleton created with @createSkeletonInstance.
        void destroySkeletonInstance( SkeletonInstance *skeletonInstance );

        /** Create a ManualObject, an object which you populate with geometry
            manually through a GL immediate-mode style interface.
        */
        virtual ManualObject* createManualObject( SceneMemoryMgrTypes sceneType = SCENE_DYNAMIC );
        /** Removes & destroys a ManualObject from the SceneManager.
        */
        virtual void destroyManualObject(ManualObject* obj);
        /** Removes & destroys all ManualObjects from the SceneManager.
        */
        virtual void destroyAllManualObjects(void);
        /** Create a BillboardChain, an object which you can use to render
            a linked chain of billboards.
        */
        virtual v1::BillboardChain* createBillboardChain();
        /** Removes & destroys a BillboardChain from the SceneManager.
        */
        virtual void destroyBillboardChain(v1::BillboardChain* obj);
        /** Removes & destroys all BillboardChains from the SceneManager.
        */
        virtual void destroyAllBillboardChains(void);       
        /** Create a RibbonTrail, an object which you can use to render
            a linked chain of billboards which follows one or more nodes.
        */
        virtual v1::RibbonTrail* createRibbonTrail();
        /** Removes & destroys a RibbonTrail from the SceneManager.
        */
        virtual void destroyRibbonTrail(v1::RibbonTrail* obj);
        /** Removes & destroys all RibbonTrails from the SceneManager.
        */
        virtual void destroyAllRibbonTrails(void);      

        /** Creates a particle system based on a template.
        @remarks
            This method creates a new ParticleSystem instance based on the named template
            (defined through ParticleSystemManager::createTemplate) and returns a 
            pointer to the caller. The caller should not delete this object, it will be freed at system shutdown, 
            or can be released earlier using the destroyParticleSystem method.
        @par
            Each system created from a template takes the template's settings at the time of creation, 
            but is completely separate from the template from there on. 
        @par
            Creating a particle system does not make it a part of the scene. As with other MovableObject
            subclasses, a ParticleSystem is not rendered until it is attached to a SceneNode. 
        @par
            This is probably the more useful particle system creation method since it does not require manual
            setup of the system. Note that the initial quota is based on the template but may be changed later.
        @param 
            templateName The name of the template to base the new instance on.
        */
        virtual ParticleSystem* createParticleSystem( const String& templateName );
        /** Create a blank particle system.
        @remarks
            This method creates a new, blank ParticleSystem instance and returns a pointer to it.
            The caller should not delete this object, it will be freed at system shutdown, or can
            be released earlier using the destroyParticleSystem method.
        @par
            The instance returned from this method won't actually do anything because on creation a
            particle system has no emitters. The caller should manipulate the instance through it's 
            ParticleSystem methods to actually create a real particle effect. 
        @par
            Creating a particle system does not make it a part of the scene. As with other MovableObject
            subclasses, a ParticleSystem is not rendered until it is attached to a SceneNode. 
        @param 
            quota The maximum number of particles to allow in this system. 
        @param
            resourceGroup The resource group which will be used to load dependent resources
        */
        virtual ParticleSystem* createParticleSystem( size_t quota = 500, 
            const String& resourceGroup = ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );

        /** Removes & destroys a ParticleSystem from the SceneManager.
        */
        virtual void destroyParticleSystem(ParticleSystem* obj);
        /** Removes & destroys all ParticleSystems from the SceneManager.
        */
        virtual void destroyAllParticleSystems(void);       

        /** Empties the entire scene, inluding all SceneNodes, Entities, Lights, 
            BillboardSets etc. Cameras are not deleted at this stage since
            they are still referenced by viewports, which are not destroyed during
            this process.
        @remarks
            When an indestructible SceneNode has a destructible parent, the parent
            will be destroyed and thus the former SceneNode will be parent-less,
            i.e. be at the same level as root. Note that parent-less SceneNodes can
            still render to the scene (this behavior is different from Ogre 1.x)
        @param deleteIndestructibleToo
            If a node is marked as indestructible, it won't be deleted unless
            deleteIndestructibleToo is marked as true.
            Note that some Ogre compoenents mark some of their objects as internal to
            prevent them from becoming dangling pointers.

            If you don't know what to put here, set it to false.
        @param reattachCameras
            When true, all Cameras that have no parent node after clearing
            will be reattached to the root node.
        */
        virtual void clearScene( bool deleteIndestructibleToo, bool reattachCameras=true );

        /** Sets the ambient light level to be used for the scene.
        @remarks
            Ambient lighting is a cheap fake to compensate the lack of global illumination.
            Hemisphere ambient lighting will set the final ambient light to the following
            formula:
                float w = dot( objNormal, hemisphereDir ) * 0.5 + 0.5;
                finalAmbient = lerp( lowerHemisphere, upperHemisphere, w );

            Setting upperHemisphere = lowerHemisphere is effectively a traditional
            fixed-colour ambient light.
        @par
            Tip: Set the upperHemisphere to a cold colour (e.g. blueish sky) and lowerHemisphere
            to a warm colour (e.g. sun-yellowish, orange) and the hemisphereDir in the opposite
            direction of your main directional light for a convincing look.
        @par
            By default the ambient light in the scene is ColourValue::Black, i.e. no ambient light.
            This means that any objects rendered with a Material which has lighting enabled
            (see Material::setLightingEnabled) will not be visible unless you have some dynamic
            lights in your scene.
        @par
            For compatibility reasons with legacy (e.g. low level materials) upperHemisphere's
            colour is the old ambient colour from 1.x

        @param upperHemisphere
            Ambient colour when the surface normal is close to hemisphereDir
        @param lowerHemisphere
            Ambient colour when the surface normal is pointing away from hemisphereDir
        @param hemisphereDir
            Hemisphere's direction reference to compare the surface normal to.
            The internal vector will be normalized.
        @param envmapScale
            Global scale to apply to all environment maps (for relevant Hlms implementations,
            like PBS). The value will be stored in upperHemisphere.a
            Use 1.0 to disable.
        */
        void setAmbientLight( const ColourValue& upperHemisphere, const ColourValue& lowerHemisphere,
                              const Vector3 &hemisphereDir, Real envmapScale = 1.0f );

        /** Returns the ambient light level to be used for the scene.
        */
        const ColourValue& getAmbientLightUpperHemisphere(void) const   { return mAmbientLight[0]; }
        const ColourValue& getAmbientLightLowerHemisphere(void) const   { return mAmbientLight[1]; }
        const Vector3& getAmbientLightHemisphereDir(void) const { return mAmbientLightHemisphereDir; }

        /** Sets the source of the 'world' geometry, i.e. the large, mainly static geometry
            making up the world e.g. rooms, landscape etc.
            This function can be called before setWorldGeometry in a background thread, do to
            some slow tasks (e.g. IO) that do not involve the backend render system.
            @remarks
                Depending on the type of SceneManager (subclasses will be specialised
                for particular world geometry types) you have requested via the Root or
                SceneManagerEnumerator classes, you can pass a filename to this method and it
                will attempt to load the world-level geometry for use. If you try to load
                an inappropriate type of world data an exception will be thrown. The default
                SceneManager cannot handle any sort of world geometry and so will always
                throw an exception. However subclasses like BspSceneManager can load
                particular types of world geometry e.g. "q3dm1.bsp".

        */
        virtual void prepareWorldGeometry(const String& filename);

        /** Sets the source of the 'world' geometry, i.e. the large, mainly 
            static geometry making up the world e.g. rooms, landscape etc.
            This function can be called before setWorldGeometry in a background thread, do to
            some slow tasks (e.g. IO) that do not involve the backend render system.
            @remarks
                Depending on the type of SceneManager (subclasses will be 
                specialised for particular world geometry types) you have 
                requested via the Root or SceneManagerEnumerator classes, you 
                can pass a stream to this method and it will attempt to load 
                the world-level geometry for use. If the manager can only 
                handle one input format the typeName parameter is not required.
                The stream passed will be read (and it's state updated). 
            @param stream Data stream containing data to load
            @param typeName String identifying the type of world geometry
                contained in the stream - not required if this manager only 
                supports one type of world geometry.
        */
        virtual void prepareWorldGeometry(DataStreamPtr& stream, 
            const String& typeName = BLANKSTRING);

        /** Sets the source of the 'world' geometry, i.e. the large, mainly static geometry
            making up the world e.g. rooms, landscape etc.
            @remarks
                Depending on the type of SceneManager (subclasses will be specialised
                for particular world geometry types) you have requested via the Root or
                SceneManagerEnumerator classes, you can pass a filename to this method and it
                will attempt to load the world-level geometry for use. If you try to load
                an inappropriate type of world data an exception will be thrown. The default
                SceneManager cannot handle any sort of world geometry and so will always
                throw an exception. However subclasses like BspSceneManager can load
                particular types of world geometry e.g. "q3dm1.bsp".
        */
        virtual void setWorldGeometry(const String& filename);

        /** Sets the source of the 'world' geometry, i.e. the large, mainly 
            static geometry making up the world e.g. rooms, landscape etc.
            @remarks
                Depending on the type of SceneManager (subclasses will be 
                specialised for particular world geometry types) you have 
                requested via the Root or SceneManagerEnumerator classes, you 
                can pass a stream to this method and it will attempt to load 
                the world-level geometry for use. If the manager can only 
                handle one input format the typeName parameter is not required.
                The stream passed will be read (and it's state updated). 
            @param stream Data stream containing data to load
            @param typeName String identifying the type of world geometry
                contained in the stream - not required if this manager only 
                supports one type of world geometry.
        */
        virtual void setWorldGeometry(DataStreamPtr& stream, 
            const String& typeName = BLANKSTRING);

        /** Estimate the number of loading stages required to load the named
            world geometry. 
        @remarks
            This method should be overridden by SceneManagers that provide
            custom world geometry that can take some time to load. They should
            return from this method a count of the number of stages of progress
            they can report on whilst loading. During real loading (setWorldGeometry),
            they should call ResourceGroupManager::_notifyWorldGeometryProgress exactly
            that number of times when loading the geometry for real.
        @note 
            The default is to return 0, ie to not report progress. 
        */
        virtual size_t estimateWorldGeometry(const String& filename)
        { (void)filename; return 0; }

        /** Estimate the number of loading stages required to load the named
            world geometry. 
        @remarks
            Operates just like the version of this method which takes a
            filename, but operates on a stream instead. Note that since the
            stream is updated, you'll need to reset the stream or reopen it
            when it comes to loading it for real.
        @param stream Data stream containing data to load
        @param typeName String identifying the type of world geometry
            contained in the stream - not required if this manager only 
            supports one type of world geometry.
        */      
        virtual size_t estimateWorldGeometry(DataStreamPtr& stream, 
            const String& typeName = BLANKSTRING)
        { (void)stream; (void)typeName; return 0; }

        /** Asks the SceneManager to provide a suggested viewpoint from which the scene should be viewed.
            @remarks
                Typically this method returns the origin unless a) world geometry has been loaded using
                SceneManager::setWorldGeometry and b) that world geometry has suggested 'start' points.
                If there is more than one viewpoint which the scene manager can suggest, it will always suggest
                the first one unless the random parameter is true.
            @param
                random If true, and there is more than one possible suggestion, a random one will be used. If false
                the same one will always be suggested.
            @return
                On success, true is returned.
            @par
                On failure, false is returned.
        */
        virtual ViewPoint getSuggestedViewpoint(bool random = false);

        /** Method for setting a specific option of the Scene Manager. These options are usually
            specific for a certain implemntation of the Scene Manager class, and may (and probably
            will) not exist across different implementations.
            @param
                strKey The name of the option to set
            @param
                pValue A pointer to the value - the size should be calculated by the scene manager
                based on the key
            @return
                On success, true is returned.
            @par
                On failure, false is returned.
        */
        virtual bool setOption( const String& strKey, const void* pValue )
        { (void)strKey; (void)pValue; return false; }

        /** Method for getting the value of an implementation-specific Scene Manager option.
            @param
                strKey The name of the option
            @param
                pDestValue A pointer to a memory location where the value will
                be copied. Currently, the memory will be allocated by the
                scene manager, but this may change
            @return
                On success, true is returned and pDestValue points to the value of the given
                option.
            @par
                On failure, false is returned and pDestValue is set to NULL.
        */
        virtual bool getOption( const String& strKey, void* pDestValue )
        { (void)strKey; (void)pDestValue; return false; }

        /** Method for verifying whether the scene manager has an implementation-specific
            option.
            @param
                strKey The name of the option to check for.
            @return
                If the scene manager contains the given option, true is returned.
            @remarks
                If it does not, false is returned.
        */
        virtual bool hasOption( const String& strKey ) const
        { (void)strKey; return false; }

        /** Method for getting all possible values for a specific option. When this list is too large
            (i.e. the option expects, for example, a float), the return value will be true, but the
            list will contain just one element whose size will be set to 0.
            Otherwise, the list will be filled with all the possible values the option can
            accept.
            @param
                strKey The name of the option to get the values for.
            @param
                refValueList A reference to a list that will be filled with the available values.
            @return
                On success (the option exists), true is returned.
            @par
                On failure, false is returned.
        */
        virtual bool getOptionValues( const String& strKey, StringVector& refValueList )
        { (void)strKey; (void)refValueList; return false; }

        /** Method for getting all the implementation-specific options of the scene manager.
            @param
                refKeys A reference to a list that will be filled with all the available options.
            @return
                On success, true is returned. On failure, false is returned.
        */
        virtual bool getOptionKeys( StringVector& refKeys )
        { (void)refKeys; return false; }

        /// @See mTmpVisibleObjects
        VisibleObjectsPerThreadArray& _getTmpVisibleObjectsList()           { return mTmpVisibleObjects; }

        InstancingThreadedCullingMethod getInstancingThreadedCullingMethod() const
                                                            { return mInstancingThreadedCullingMethod; }

        /** Notifies that the given MovableObject is dirty (i.e. the AABBs have changed).
            Note that the parent SceneNodes of this/these objects are not updated and you will
            have to call @see notifyStaticDirty on the SceneNode if the position/rotation/scale
            have changed.
        */
        void notifyStaticAabbDirty( MovableObject *movableObject );

        /** Notifies that the given Node is dirty (i.e. the position, orientation and/or scale has
            changed). The call will cascade to all children of the input node.
        @remarks
            Implies a call to @see notifyStaticAabbDirty if the node or any of its children has
            a MovableObject attached.
        @par
            Calling notifyStaticDirty( getRootSceneNode( SCENE_STATIC ) ) should flush the entire
            static system. It might be slower, but it is useful when you're witnessing artifacts
            after making changes to the static environment and don't know for sure which objects
            need to be updated.
        */
        void notifyStaticDirty( Node *node );

        /** Updates all skeletal animations in the scene. This is typically called once
            per frame during render, but the user might want to manually call this function.
        @remarks
            mSkeletonAnimManagerCulledList must be set. @See updateAllTransforms remarks
        */
        void updateAllAnimations();

        /** Updates the derived transforms of all nodes in the scene. This is typically called once
            per frame during render, but the user may want to manually call this function.
        @remarks
            mEntitiesMemoryManagerUpdateList must be set. It contains multiple memory manager
            containing all objects to be updated (i.e. Entities & Lights are both MovableObjects
            but are kept separate)
            Don't call this function from another thread other than Ogre's main one (we use worker
            threads that may be in use for something else, and touching the sync barrier
            could deadlock in the best of cases).
        */
        void updateAllTransforms();

        /** Updates all TagPoints, both TagPoints that are children of bones, and TagPoints that
            are children of other TagPoints.
        @remarks
            mTagPointNodeMemoryManagerUpdateList must be set. @see updateAllTransforms remarks
        */
        void updateAllTagPoints(void);

        /** Updates the world aabbs from all entities in the scene. Ought to be called right after
            updateAllTransforms. @See updateAllTransforms
        @remarks
            @See MovableObject::updateAllBounds
            Don't call this function from another thread other than Ogre's main one (we use worker
            threads that may be in use for something else, and touching the sync barrier
            could deadlock in the best of cases).
        */
        void updateAllBounds( const ObjectMemoryManagerVec &objectMemManager );

        /** Updates the Lod values of all objects relative to the given camera.
        */
        void updateAllLods( const Camera *lodCamera, Real lodBias, uint8 firstRq, uint8 lastRq );

        /** Updates the scene: Perform high level culling, Node transforms and entity animations.
        */
        void updateSceneGraph();

        /** Internal method for applying animations to scene nodes.
        @remarks
            Uses the internally stored AnimationState objects to apply animation to SceneNodes.
        */
        virtual void _applySceneAnimations(void);

        /** Performs the frustum culling that will later be needed by _renderPhase02
            @remarks
                @See CompositorShadowNode to understand why rendering is split in two phases
            @param camera Pointer to a camera from whose viewpoint the scene is to
                be rendered.
            @param vp The target viewport
            @param firstRq first render queue ID to render (gets clamped if too big)
            @param lastRq last render queue ID to render (gets clamped if too big)
        */
        virtual void _cullPhase01(Camera* camera, const Camera *lodCamera,
                                  Viewport* vp, uint8 firstRq, uint8 lastRq );

        /** Prompts the class to send its contents to the renderer.
            @remarks
                This method prompts the scene manager to send the
                contents of the scene it manages to the rendering
                pipeline. You must have called _cullPhase01 before calling
                this function.
                Note that this method is not normally called directly by the
                user application; it is called automatically by the Ogre
                rendering loop.
            @param camera Pointer to a camera from whose viewpoint the scene is to
                be rendered.
            @param lodCamera Pointer to a camera from whose LOD calculations will be
                based upon. Can't be null; can be equal to @camera.
            @param vp The target viewport
            @param firstRq first render queue ID to render (gets clamped if too big)
            @param lastRq last render queue ID to render (gets clamped if too big)
            @param includeOverlays Whether or not overlay objects should be rendered
        */
        virtual void _renderPhase02( Camera* camera, const Camera* lodCamera, Viewport* vp,
                                     uint8 firstRq, uint8 lastRq, bool includeOverlays );

        void cullLights( Camera *camera, Light::LightTypes startType,
                         Light::LightTypes endType, LightArray &outLights );

        /// Called when the frame has fully ended (ALL passes have been executed to all RTTs)
        void _frameEnded(void);

        /** Internal method for queueing the sky objects with the params as 
            previously set through setSkyBox, setSkyPlane and setSkyDome.
        */
        virtual void _queueSkiesForRendering(Camera* cam);



        /** Notifies the scene manager of its destination render system
            @remarks
                Called automatically by RenderSystem::addSceneManager
                this method simply notifies the manager of the render
                system to which its output must be directed.
            @param
                sys Pointer to the RenderSystem subclass to be used as a render target.
        */
        virtual void _setDestinationRenderSystem(RenderSystem* sys);

        void _setViewport( Viewport *vp )                               { setViewport( vp ); }
        void _setCameraInProgress( Camera *camera )                     { mCameraInProgress = camera; }

        /** Enables / disables a 'sky plane' i.e. a plane at constant
            distance from the camera representing the sky.
            @remarks
                You can create sky planes yourself using the standard mesh and
                entity methods, but this creates a plane which the camera can
                never get closer or further away from - it moves with the camera.
                (NB you could create this effect by creating a world plane which
                was attached to the same SceneNode as the Camera too, but this
                would only apply to a single camera whereas this plane applies to
                any camera using this scene manager).
            @note
                To apply scaling, scrolls etc to the sky texture(s) you
                should use the TextureUnitState class methods.
            @param
                enable True to enable the plane, false to disable it
            @param
                plane Details of the plane, i.e. it's normal and it's
                distance from the camera.
            @param
                materialName The name of the material the plane will use
            @param
                scale The scaling applied to the sky plane - higher values
                mean a bigger sky plane - you may want to tweak this
                depending on the size of plane.d and the other
                characteristics of your scene
            @param
                tiling How many times to tile the texture across the sky.
                Applies to all texture layers. If you need finer control use
                the TextureUnitState texture coordinate transformation methods.
            @param
                drawFirst If true, the plane is drawn before all other
                geometry in the scene, without updating the depth buffer.
                This is the safest rendering method since all other objects
                will always appear in front of the sky. However this is not
                the most efficient way if most of the sky is often occluded
                by other objects. If this is the case, you can set this
                parameter to false meaning it draws <em>after</em> all other
                geometry which can be an optimisation - however you must
                ensure that the plane.d value is large enough that no objects
                will 'poke through' the sky plane when it is rendered.
            @param
                bow If zero, the plane will be completely flat (like previous
                versions.  If above zero, the plane will be curved, allowing
                the sky to appear below camera level.  Curved sky planes are 
                simular to skydomes, but are more compatible with fog.
            @param xsegments, ysegments
                Determines the number of segments the plane will have to it. This
                is most important when you are bowing the plane, but may also be useful
                if you need tessellation on the plane to perform per-vertex effects.
            @param groupName
                The name of the resource group to which to assign the plane mesh.
        */

        virtual void setSkyPlane(
            bool enable,
            const Plane& plane, const String& materialName, Real scale = 1000,
            Real tiling = 10, bool drawFirst = true, Real bow = 0, 
            int xsegments = 1, int ysegments = 1, 
            const String& groupName = ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        /** Enables / disables a 'sky plane' i.e. a plane at constant
            distance from the camera representing the sky.
            @remarks
                You can create sky planes yourself using the standard mesh and
                entity methods, but this creates a plane which the camera can
                never get closer or further away from - it moves with the camera.
                (NB you could create this effect by creating a world plane which
                was attached to the same SceneNode as the Camera too, but this
                would only apply to a single camera whereas this plane applies to
                any camera using this scene manager).
            @note
                To apply scaling, scrolls etc to the sky texture(s) you
                should use the TextureUnitState class methods.
            @param
                enable True to enable the plane, false to disable it
            @param
                plane Details of the plane, i.e. it's normal and it's
                distance from the camera.
            @param
                materialName The name of the material the plane will use
            @param
                scale The scaling applied to the sky plane - higher values
                mean a bigger sky plane - you may want to tweak this
                depending on the size of plane.d and the other
                characteristics of your scene
            @param
                tiling How many times to tile the texture across the sky.
                Applies to all texture layers. If you need finer control use
                the TextureUnitState texture coordinate transformation methods.
            @param
                renderQueue The render queue to use when rendering this object
            @param
                bow If zero, the plane will be completely flat (like previous
                versions.  If above zero, the plane will be curved, allowing
                the sky to appear below camera level.  Curved sky planes are 
                simular to skydomes, but are more compatible with fog.
            @param xsegments, ysegments
                Determines the number of segments the plane will have to it. This
                is most important when you are bowing the plane, but may also be useful
                if you need tessellation on the plane to perform per-vertex effects.
            @param groupName
                The name of the resource group to which to assign the plane mesh.
        */        
        virtual void _setSkyPlane(
            bool enable,
            const Plane& plane, const String& materialName, uint8 renderQueue, Real scale = 1000,
            Real tiling = 10, Real bow = 0, int xsegments = 1, int ysegments = 1,
            const String& groupName = ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

        /** Enables / disables a 'sky plane' */
        virtual void setSkyPlaneEnabled(bool enable) { mSkyPlaneEnabled = enable; }

        /** Return whether a key plane is enabled */
        virtual bool isSkyPlaneEnabled(void) const { return mSkyPlaneEnabled; }

        /** Get the sky plane node, if enabled. */
        virtual SceneNode* getSkyPlaneNode(void) const { return mSkyPlaneNode; }

        /** Get the parameters used to construct the SkyPlane, if any **/
        virtual const SkyPlaneGenParameters& getSkyPlaneGenParameters(void) const { return mSkyPlaneGenParameters; }

        /** Enables / disables a 'sky box' i.e. a 6-sided box at constant
            distance from the camera representing the sky.
            @remarks
                You could create a sky box yourself using the standard mesh and
                entity methods, but this creates a plane which the camera can
                never get closer or further away from - it moves with the camera.
                (NB you could create this effect by creating a world box which
                was attached to the same SceneNode as the Camera too, but this
                would only apply to a single camera whereas this skybox applies
                to any camera using this scene manager).
            @par
                The material you use for the skybox can either contain layers
                which are single textures, or they can be cubic textures, i.e.
                made up of 6 images, one for each plane of the cube. See the
                TextureUnitState class for more information.
            @param
                enable True to enable the skybox, false to disable it
            @param
                materialName The name of the material the box will use
            @param
                distance Distance in world coorinates from the camera to
                each plane of the box. The default is normally OK.
            @param
                drawFirst If true, the box is drawn before all other
                geometry in the scene, without updating the depth buffer.
                This is the safest rendering method since all other objects
                will always appear in front of the sky. However this is not
                the most efficient way if most of the sky is often occluded
                by other objects. If this is the case, you can set this
                parameter to false meaning it draws <em>after</em> all other
                geometry which can be an optimisation - however you must
                ensure that the distance value is large enough that no
                objects will 'poke through' the sky box when it is rendered.
            @param
                orientation Optional parameter to specify the orientation
                of the box. By default the 'top' of the box is deemed to be
                in the +y direction, and the 'front' at the -z direction.
                You can use this parameter to rotate the sky if you want.
            @param groupName
                The name of the resource group to which to assign the plane mesh.
        */
        virtual void setSkyBox(
            bool enable, const String& materialName, Real distance = 5000,
            bool drawFirst = true, const Quaternion& orientation = Quaternion::IDENTITY,
            const String& groupName = ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

        /** Enables / disables a 'sky box' i.e. a 6-sided box at constant
            distance from the camera representing the sky.
            @remarks
                You could create a sky box yourself using the standard mesh and
                entity methods, but this creates a plane which the camera can
                never get closer or further away from - it moves with the camera.
                (NB you could create this effect by creating a world box which
                was attached to the same SceneNode as the Camera too, but this
                would only apply to a single camera whereas this skybox applies
                to any camera using this scene manager).
            @par
                The material you use for the skybox can either contain layers
                which are single textures, or they can be cubic textures, i.e.
                made up of 6 images, one for each plane of the cube. See the
                TextureUnitState class for more information.
            @param
                enable True to enable the skybox, false to disable it
            @param
                materialName The name of the material the box will use
            @param
                distance Distance in world coorinates from the camera to
                each plane of the box. The default is normally OK.
            @param
                renderQueue The render queue to use when rendering this object
            @param
                orientation Optional parameter to specify the orientation
                of the box. By default the 'top' of the box is deemed to be
                in the +y direction, and the 'front' at the -z direction.
                You can use this parameter to rotate the sky if you want.
            @param groupName
                The name of the resource group to which to assign the plane mesh.
        */
        virtual void _setSkyBox(
            bool enable, const String& materialName, uint8 renderQueue, Real distance = 5000,
            const Quaternion& orientation = Quaternion::IDENTITY,
            const String& groupName = ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

        /** Enables / disables a 'sky box' */
        virtual void setSkyBoxEnabled(bool enable) { mSkyBoxEnabled = enable; }

        /** Return whether a skybox is enabled */
        virtual bool isSkyBoxEnabled(void) const { return mSkyBoxEnabled; }

        /** Get the skybox node, if enabled. */
        virtual SceneNode* getSkyBoxNode(void) const { return mSkyBoxNode; }

        /** Get the parameters used to generate the current SkyBox, if any */
        virtual const SkyBoxGenParameters& getSkyBoxGenParameters(void) const { return mSkyBoxGenParameters; }

        /** Enables / disables a 'sky dome' i.e. an illusion of a curved sky.
            @remarks
                A sky dome is actually formed by 5 sides of a cube, but with
                texture coordinates generated such that the surface appears
                curved like a dome. Sky domes are appropriate where you need a
                realistic looking sky where the scene is not going to be
                'fogged', and there is always a 'floor' of some sort to prevent
                the viewer looking below the horizon (the distortion effect below
                the horizon can be pretty horrible, and there is never anyhting
                directly below the viewer). If you need a complete wrap-around
                background, use the setSkyBox method instead. You can actually
                combine a sky box and a sky dome if you want, to give a positional
                backdrop with an overlayed curved cloud layer.
            @par
                Sky domes work well with 2D repeating textures like clouds. You
                can change the apparent 'curvature' of the sky depending on how
                your scene is viewed - lower curvatures are better for 'open'
                scenes like landscapes, whilst higher curvatures are better for
                say FPS levels where you don't see a lot of the sky at once and
                the exaggerated curve looks good.
            @param
                enable True to enable the skydome, false to disable it
            @param
                materialName The name of the material the dome will use
            @param
                curvature The curvature of the dome. Good values are
                between 2 and 65. Higher values are more curved leading to
                a smoother effect, lower values are less curved meaning
                more distortion at the horizons but a better distance effect.
            @param
                tiling How many times to tile the texture(s) across the
                dome.
            @param
                distance Distance in world coorinates from the camera to
                each plane of the box the dome is rendered on. The default
                is normally OK.
            @param
                drawFirst If true, the dome is drawn before all other
                geometry in the scene, without updating the depth buffer.
                This is the safest rendering method since all other objects
                will always appear in front of the sky. However this is not
                the most efficient way if most of the sky is often occluded
                by other objects. If this is the case, you can set this
                parameter to false meaning it draws <em>after</em> all other
                geometry which can be an optimisation - however you must
                ensure that the distance value is large enough that no
                objects will 'poke through' the sky when it is rendered.
            @param
                orientation Optional parameter to specify the orientation
                of the dome. By default the 'top' of the dome is deemed to
                be in the +y direction, and the 'front' at the -z direction.
                You can use this parameter to rotate the sky if you want.
            @param groupName
                The name of the resource group to which to assign the plane mesh.
                */
        virtual void setSkyDome(
            bool enable, const String& materialName, Real curvature = 10,
            Real tiling = 8, Real distance = 4000, bool drawFirst = true,
            const Quaternion& orientation = Quaternion::IDENTITY,
            int xsegments = 16, int ysegments = 16, int ysegments_keep = -1,
            const String& groupName = ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

        /** Enables / disables a 'sky dome' i.e. an illusion of a curved sky.
            @remarks
                A sky dome is actually formed by 5 sides of a cube, but with
                texture coordinates generated such that the surface appears
                curved like a dome. Sky domes are appropriate where you need a
                realistic looking sky where the scene is not going to be
                'fogged', and there is always a 'floor' of some sort to prevent
                the viewer looking below the horizon (the distortion effect below
                the horizon can be pretty horrible, and there is never anyhting
                directly below the viewer). If you need a complete wrap-around
                background, use the setSkyBox method instead. You can actually
                combine a sky box and a sky dome if you want, to give a positional
                backdrop with an overlayed curved cloud layer.
            @par
                Sky domes work well with 2D repeating textures like clouds. You
                can change the apparent 'curvature' of the sky depending on how
                your scene is viewed - lower curvatures are better for 'open'
                scenes like landscapes, whilst higher curvatures are better for
                say FPS levels where you don't see a lot of the sky at once and
                the exaggerated curve looks good.
            @param
                enable True to enable the skydome, false to disable it
            @param
                materialName The name of the material the dome will use
            @param
                curvature The curvature of the dome. Good values are
                between 2 and 65. Higher values are more curved leading to
                a smoother effect, lower values are less curved meaning
                more distortion at the horizons but a better distance effect.
            @param
                tiling How many times to tile the texture(s) across the
                dome.
            @param
                distance Distance in world coorinates from the camera to
                each plane of the box the dome is rendered on. The default
                is normally OK.
            @param
                renderQueue The render queue to use when rendering this object
            @param
                orientation Optional parameter to specify the orientation
                of the dome. By default the 'top' of the dome is deemed to
                be in the +y direction, and the 'front' at the -z direction.
                You can use this parameter to rotate the sky if you want.
            @param groupName
                The name of the resource group to which to assign the plane mesh.
                */        
        virtual void _setSkyDome(
            bool enable, const String& materialName, uint8 renderQueue,
            Real curvature = 10, Real tiling = 8, Real distance = 4000,
            const Quaternion& orientation = Quaternion::IDENTITY,
            int xsegments = 16, int ysegments = 16, int ysegments_keep = -1,
            const String& groupName = ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

        /** Enables / disables a 'sky dome' */
        virtual void setSkyDomeEnabled(bool enable) { mSkyDomeEnabled = enable; }

        /** Return whether a skydome is enabled */
        virtual bool isSkyDomeEnabled(void) const { return mSkyDomeEnabled; }

        /** Get the sky dome node, if enabled. */
        virtual SceneNode* getSkyDomeNode(void) const { return mSkyDomeNode; }

        /** Get the parameters used to generate the current SkyDome, if any */
        virtual const SkyDomeGenParameters& getSkyDomeGenParameters(void) const { return mSkyDomeGenParameters; }

        /** Sets the fogging mode applied to the scene.
            @remarks
                This method sets up the scene-wide fogging effect. These settings
                apply to all geometry rendered, UNLESS the material with which it
                is rendered has it's own fog settings (see Material::setFog).
            @param
                mode Set up the mode of fog as described in the FogMode
                enum, or set to FOG_NONE to turn off.
            @param
                colour The colour of the fog. Either set this to the same
                as your viewport background colour, or to blend in with a
                skydome or skybox.
            @param
                expDensity The density of the fog in FOG_EXP or FOG_EXP2
                mode, as a value between 0 and 1. The default is 0.001. 
            @param
                linearStart Distance in world units at which linear fog starts to
                encroach. Only applicable if mode is
                FOG_LINEAR.
            @param
                linearEnd Distance in world units at which linear fog becomes completely
                opaque. Only applicable if mode is
                FOG_LINEAR.
        */
        void setFog(
            FogMode mode = FOG_NONE, const ColourValue& colour = ColourValue::White,
            Real expDensity = 0.001, Real linearStart = 0.0, Real linearEnd = 1.0);

        /** Returns the fog mode for the scene.
        */
        virtual FogMode getFogMode(void) const;

        /** Returns the fog colour for the scene.
        */
        virtual const ColourValue& getFogColour(void) const;

        /** Returns the fog start distance for the scene.
        */
        virtual Real getFogStart(void) const;

        /** Returns the fog end distance for the scene.
        */
        virtual Real getFogEnd(void) const;

        /** Returns the fog density for the scene.
        */
        virtual Real getFogDensity(void) const;


        /** Creates a new BillboardSet for use with this scene manager.
            @remarks
                This method creates a new BillboardSet which is registered with
                the SceneManager. The SceneManager will destroy this object when
                it shuts down or when the SceneManager::clearScene method is
                called, so the caller does not have to worry about destroying
                this object (in fact, it definitely should not do this).
            @par
                See the BillboardSet documentations for full details of the
                returned class.
            @param
                poolSize The initial size of the pool of billboards (see BillboardSet for more information)
            @see
                BillboardSet
        */
        virtual v1::BillboardSet* createBillboardSet(unsigned int poolSize = 20);

        /** Removes & destroys an BillboardSet from the SceneManager.
            @warning
                Must only be done if the BillboardSet is not attached
                to a SceneNode. It may be safer to wait to clear the whole
                scene. If you are unsure, use clearScene.
        */
        virtual void destroyBillboardSet(v1::BillboardSet* set);

        /** Removes & destroys all BillboardSets.
        @warning
        Again, use caution since no BillboardSet must be referred to
        elsewhere e.g. attached to a SceneNode otherwise a crash
        is likely. Use clearScene if you are unsure (it clears SceneNode
        entries too.)
        @see
        SceneManager::clearScene
        */
        virtual void destroyAllBillboardSets(void);

        /** Tells the SceneManager whether it should render the SceneNodes which 
            make up the scene as well as the objects in the scene.
        @remarks
            This method is mainly for debugging purposes. If you set this to 'true',
            each node will be rendered as a set of 3 axes to allow you to easily see
            the orientation of the nodes.
        */
        virtual void setDisplaySceneNodes(bool display);
        /** Returns true if all scene nodes axis are to be displayed */
        virtual bool getDisplaySceneNodes(void) const {return mDisplayNodes;}

        /** Creates an animation which can be used to animate scene nodes.
        @remarks
            An animation is a collection of 'tracks' which over time change the position / orientation
            of Node objects. In this case, the animation will likely have tracks to modify the position
            / orientation of SceneNode objects, e.g. to make objects move along a path.
        @par
            You don't need to use an Animation object to move objects around - you can do it yourself
            using the methods of the Node in your FrameListener class. However, when you need relatively
            complex scripted animation, this is the class to use since it will interpolate between
            keyframes for you and generally make the whole process easier to manage.
        @par
            A single animation can affect multiple Node objects (each AnimationTrack affects a single Node).
            In addition, through animation blending a single Node can be affected by multiple animations,
            athough this is more useful when performing skeletal animation (see Skeleton::createAnimation).
        @par
            Note that whilst it uses the same classes, the animations created here are kept separate from the
            skeletal animations of meshes (each Skeleton owns those animations).
        @param name The name of the animation, must be unique within this SceneManager.
        @param length The total length of the animation.
        */
        virtual v1::Animation* createAnimation(const String& name, Real length);

        /** Looks up an Animation object previously created with createAnimation. 
        @note Throws an exception if the named instance does not exist
        */
        virtual v1::Animation* getAnimation(const String& name) const;
        /** Returns whether an animation with the given name exists.
        */
        virtual bool hasAnimation(const String& name) const;

        /** Destroys an Animation. 
        @remarks
            You should ensure that none of your code is referencing this animation objects since the 
            memory will be freed.
        */
        virtual void destroyAnimation(const String& name);

        /** Removes all animations created using this SceneManager. */
        virtual void destroyAllAnimations(void);

        /** Create an AnimationState object for managing application of animations.
        @remarks
            You can create Animation objects for animating SceneNode obejcts using the
            createAnimation method. However, in order to actually apply those animations
            you have to call methods on Node and Animation in a particular order (namely
            Node::resetToInitialState and Animation::apply). To make this easier and to
            help track the current time position of animations, the AnimationState object
            is provided.
            So if you don't want to control animation application manually, call this method,
            update the returned object as you like every frame and let SceneManager apply 
            the animation state for you.
        @par
            Remember, AnimationState objects are disabled by default at creation time. 
            Turn them on when you want them using their setEnabled method.
        @par
            Note that any SceneNode affected by this automatic animation will have it's state
            reset to it's initial position before application of the animation. Unless specifically
            modified using Node::setInitialState the Node assumes it's initial state is at the
            origin. If you want the base state of the SceneNode to be elsewhere, make your changes
            to the node using the standard transform methods, then call setInitialState to 
            'bake' this reference position into the node.
        @par
            If the target of your animation is to be a generic AnimableValue, you
            should ensure that it has a base value set (unlike nodes this has no
            default). @see AnimableValue::setAsBaseValue.
        @param animName The name of an animation created already with createAnimation.
        */
        virtual v1::AnimationState* createAnimationState(const String& animName);

        /** Retrieves animation state as previously created using createAnimationState. 
        @note Throws an exception if the named instance does not exist
        */
        virtual v1::AnimationState* getAnimationState(const String& animName) const;
        /** Returns whether an animation state with the given name exists.
        */
        virtual bool hasAnimationState(const String& name) const;

        /** Destroys an AnimationState. 
        @remarks
            You should ensure that none of your code is referencing this animation 
            state object since the memory will be freed.
        */
        virtual void destroyAnimationState(const String& name);

        /** Removes all animation states created using this SceneManager. */
        virtual void destroyAllAnimationStates(void);

        /** Registers a new RenderQueueListener which will be notified when render queues
            are processed.
        */
        virtual void addRenderQueueListener(RenderQueueListener* newListener);

        /** Removes a listener previously added with addRenderQueueListener. */
        virtual void removeRenderQueueListener(RenderQueueListener* delListener);
        
        /** Registers a new Render Object Listener which will be notified when rendering an object.     
        */
        virtual void addRenderObjectListener(RenderObjectListener* newListener);
        /** Removes a listener previously added with addRenderObjectListener. */
        virtual void removeRenderObjectListener(RenderObjectListener* delListener);

        /** Allows all bounding boxes of scene nodes to be displayed. */
        virtual void showBoundingBoxes(bool bShow);

        /** Returns if all bounding boxes of scene nodes are to be displayed */
        virtual bool getShowBoundingBoxes() const;

        /** Internal method for notifying the manager that a SceneNode is autotracking. */
		virtual void _addAutotrackingSceneNode( SceneNode* source, SceneNode* target,
												const Vector3 &offset, const Vector3 &localDirection );
		virtual void _removeAutotrackingSceneNode( SceneNode* source );

        
        /** Creates an AxisAlignedBoxSceneQuery for this scene manager. 
        @remarks
            This method creates a new instance of a query object for this scene manager, 
            for an axis aligned box region. See SceneQuery and AxisAlignedBoxSceneQuery 
            for full details.
        @par
            The instance returned from this method must be destroyed by calling
            SceneManager::destroyQuery when it is no longer required.
        @param box Details of the box which describes the region for this query.
        @param mask The query mask to apply to this query; can be used to filter out
            certain objects; see SceneQuery for details.
        */
        virtual AxisAlignedBoxSceneQuery* 
            createAABBQuery(const AxisAlignedBox& box, uint32 mask = QUERY_ENTITY_DEFAULT_MASK);
        /** Creates a SphereSceneQuery for this scene manager. 
        @remarks
            This method creates a new instance of a query object for this scene manager, 
            for a spherical region. See SceneQuery and SphereSceneQuery 
            for full details.
        @par
            The instance returned from this method must be destroyed by calling
            SceneManager::destroyQuery when it is no longer required.
        @param sphere Details of the sphere which describes the region for this query.
        @param mask The query mask to apply to this query; can be used to filter out
            certain objects; see SceneQuery for details.
        */
        virtual SphereSceneQuery* 
            createSphereQuery(const Sphere& sphere, uint32 mask = QUERY_ENTITY_DEFAULT_MASK);
        /** Creates a PlaneBoundedVolumeListSceneQuery for this scene manager. 
        @remarks
        This method creates a new instance of a query object for this scene manager, 
        for a region enclosed by a set of planes (normals pointing inwards). 
        See SceneQuery and PlaneBoundedVolumeListSceneQuery for full details.
        @par
        The instance returned from this method must be destroyed by calling
        SceneManager::destroyQuery when it is no longer required.
        @param volumes Details of the volumes which describe the region for this query.
        @param mask The query mask to apply to this query; can be used to filter out
        certain objects; see SceneQuery for details.
        */
        virtual PlaneBoundedVolumeListSceneQuery*  createPlaneBoundedVolumeQuery(
                const PlaneBoundedVolumeList& volumes, uint32 mask = QUERY_ENTITY_DEFAULT_MASK);


        /** Creates a RaySceneQuery for this scene manager. 
        @remarks
            This method creates a new instance of a query object for this scene manager, 
            looking for objects which fall along a ray. See SceneQuery and RaySceneQuery 
            for full details.
        @par
            The instance returned from this method must be destroyed by calling
            SceneManager::destroyQuery when it is no longer required.
        @param ray Details of the ray which describes the region for this query.
        @param mask The query mask to apply to this query; can be used to filter out
            certain objects; see SceneQuery for details.
        */
        virtual RaySceneQuery*  createRayQuery(const Ray& ray, uint32 mask = QUERY_ENTITY_DEFAULT_MASK);
        //PyramidSceneQuery* createPyramidQuery(const Pyramid& p, unsigned long mask = 0xFFFFFFFF);
        /** Creates an IntersectionSceneQuery for this scene manager. 
        @remarks
            This method creates a new instance of a query object for locating
            intersecting objects. See SceneQuery and IntersectionSceneQuery
            for full details.
        @par
            The instance returned from this method must be destroyed by calling
            SceneManager::destroyQuery when it is no longer required.
        @param mask The query mask to apply to this query; can be used to filter out
            certain objects; see SceneQuery for details.
        */
        virtual IntersectionSceneQuery* createIntersectionQuery(uint32 mask = QUERY_ENTITY_DEFAULT_MASK);

        /** Destroys a scene query of any type. */
        virtual void destroyQuery(SceneQuery* query);

        typedef VectorIterator<CameraList> CameraIterator;
        typedef MapIterator<AnimationList> AnimationIterator;

        /** Returns a specialised MapIterator over all cameras in the scene. 
        */
        CameraIterator getCameraIterator(void) {
            return CameraIterator(mCameras.begin(), mCameras.end());
        }
        /** Returns a const version of the camera list. 
        */
        const CameraList& getCameras() const { return mCameras; }
        /** Returns a specialised MapIterator over all animations in the scene. */
        AnimationIterator getAnimationIterator(void) {
            return AnimationIterator(mAnimationsList.begin(), mAnimationsList.end());
        }
        /** Returns a const version of the animation list. 
        */
        const AnimationList& getAnimations() const { return mAnimationsList; }
        /** Returns a specialised MapIterator over all animation states in the scene. */
        v1::AnimationStateIterator getAnimationStateIterator(void) {
            return mAnimationStates.getAnimationStateIterator();
        }

        /** Set the colour used to modulate areas in shadow. 
        @remarks This is only applicable for shadow techniques which involve 
            darkening the area in shadow, as opposed to masking out the light. 
            This colour provided is used as a modulative value to darken the
            areas.
        */
        virtual void setShadowColour(const ColourValue& colour);
        /** Get the colour used to modulate areas in shadow. 
        @remarks This is only applicable for shadow techniques which involve 
        darkening the area in shadow, as opposed to masking out the light. 
        This colour provided is used as a modulative value to darken the
        areas.
        */
        virtual const ColourValue& getShadowColour(void) const;
        /** Sets the distance a shadow volume is extruded for a directional light.
        @remarks
            Although directional lights are essentially infinite, there are many
            reasons to limit the shadow extrusion distance to a finite number, 
            not least of which is compatibility with older cards (which do not
            support infinite positions), and shadow caster elimination.
        @par
            The default value is 10,000 world units. This does not apply to
            point lights or spotlights, since they extrude up to their 
            attenuation range.
        */
        virtual void setShadowDirectionalLightExtrusionDistance(Real dist); 
        /** Gets the distance a shadow volume is extruded for a directional light.
        */
        virtual Real getShadowDirectionalLightExtrusionDistance(void) const;
        /** Sets the default maximum distance away from the camera that shadows
        will be visible. You have to call this function before you create lights
        or the default distance of zero will be used.
        @remarks
        Shadow techniques can be expensive, therefore it is a good idea
        to limit them to being rendered close to the camera if possible,
        and to skip the expense of rendering shadows for distance objects.
        This method allows you to set the distance at which shadows will no
        longer be rendered.
        @note
        Each shadow technique can interpret this subtely differently.
        For example, one technique may use this to eliminate casters,
        another might use it to attenuate the shadows themselves.
        You should tweak this value to suit your chosen shadow technique
        and scene setup.
        */
        virtual void setShadowFarDistance(Real distance);
        /** Gets the default maximum distance away from the camera that shadows
        will be visible.
        */
        virtual Real getShadowFarDistance(void) const
        { return mDefaultShadowFarDist; }
        virtual Real getShadowFarDistanceSquared(void) const
        { return mDefaultShadowFarDistSquared; }

        /** Sets the proportional distance which a texture shadow which is generated from a
            directional light will be offset into the camera view to make best use of texture space.
        @remarks
            When generating a shadow texture from a directional light, an approximation is used
            since it is not possible to render the entire scene to one texture. 
            The texture is projected onto an area centred on the camera, and is
            the shadow far distance * 2 in length (it is square). This wastes
            a lot of texture space outside the frustum though, so this offset allows
            you to move the texture in front of the camera more. However, be aware
            that this can cause a little shadow 'jittering' during rotation, and
            that if you move it too far then you'll start to get artefacts close 
            to the camera. The value is represented as a proportion of the shadow
            far distance, and the default is 0.6.
        */
        virtual void setShadowDirLightTextureOffset(Real offset) { mShadowTextureOffset = offset;}
        /** Gets the proportional distance which a texture shadow which is generated from a
        directional light will be offset into the camera view to make best use of texture space.
        */
        virtual Real getShadowDirLightTextureOffset(void)  const { return mShadowTextureOffset; }
        /** Sets the proportional distance at which texture shadows begin to fade out.
        @remarks
            To hide the edges where texture shadows end (in directional lights)
            Ogre will fade out the shadow in the distance. This value is a proportional
            distance of the entire shadow visibility distance at which the shadow
            begins to fade out. The default is 0.7
        */
        virtual void setShadowTextureFadeStart(Real fadeStart) 
        { mShadowTextureFadeStart = fadeStart; }
        /** Sets the proportional distance at which texture shadows finish to fading out.
        @remarks
        To hide the edges where texture shadows end (in directional lights)
        Ogre will fade out the shadow in the distance. This value is a proportional
        distance of the entire shadow visibility distance at which the shadow
        is completely invisible. The default is 0.9.
        */
        virtual void setShadowTextureFadeEnd(Real fadeEnd) 
        { mShadowTextureFadeEnd = fadeEnd; }

        /** Sets the default material to use for rendering shadow casters.
        @remarks
            By default shadow casters are rendered into the shadow texture using
            an automatically generated fixed-function pass. This allows basic
            projective texture shadows, but it's possible to use more advanced
            shadow techniques by overriding the caster materials, for
            example providing vertex and fragment programs to implement shadow
            maps.
        @par
            You can rely on the ambient light in the scene being set to the 
            requested texture shadow colour, if that's useful. 
        @note
            Individual objects may also override the vertex program in
            your default material if their materials include 
            shadow_caster_vertex_program_ref, shadow_caster_material entries,
            so if you use both make sure they are compatible.           
        @note
            Only a single pass is allowed in your material, although multiple
            techniques may be used for hardware fallback.
        */
        virtual void setShadowTextureCasterMaterial(const String& name);

        void _setCurrentCompositorPass( CompositorPass *pass );
        /// Note: May be null.
        const CompositorPass* getCurrentCompositorPass(void) const      { return mCurrentPass; }

        void _setCurrentShadowNode( CompositorShadowNode *shadowNode, bool isReused );
        const CompositorShadowNode* getCurrentShadowNode(void) const    { return mCurrentShadowNode; }
        bool isCurrentShadowNodeReused(void) const                      { return mShadowNodeIsReused; }

        /** Sets whether to use late material resolving or not. If set, materials will be resolved
            from the materials at the pass-setting stage and not at the render queue building stage.
            This is useful when the active material scheme during the render queue building stage
            is different from the one during the rendering stage.
        */
        virtual void setLateMaterialResolving(bool isLate) { mLateMaterialResolving = isLate; }
        
        /** Gets whether using late material resolving or not.
            @see setLateMaterialResolving */
        virtual bool isLateMaterialResolving() const { return mLateMaterialResolving; }

        /** Add a listener which will get called back on scene manager events.
        */
        virtual void addListener(Listener* s);
        /** Remove a listener
        */
        virtual void removeListener(Listener* s);

        /** Creates a StaticGeometry instance suitable for use with this
            SceneManager.
        @remarks
            StaticGeometry is a way of batching up geometry into a more 
            efficient form at the expense of being able to move it. Please 
            read the StaticGeometry class documentation for full information.
        @param name The name to give the new object
        @return The new StaticGeometry instance
        */
        virtual v1::StaticGeometry* createStaticGeometry(const String& name);
        /** Retrieve a previously created StaticGeometry instance. 
        @note Throws an exception if the named instance does not exist
        */
        virtual v1::StaticGeometry* getStaticGeometry(const String& name) const;
        /** Returns whether a static geometry instance with the given name exists. */
        virtual bool hasStaticGeometry(const String& name) const;
        /** Remove & destroy a StaticGeometry instance. */
        virtual void destroyStaticGeometry(v1::StaticGeometry* geom);
        /** Remove & destroy a StaticGeometry instance. */
        virtual void destroyStaticGeometry(const String& name);
        /** Remove & destroy all StaticGeometry instances. */
        virtual void destroyAllStaticGeometry(void);

        /** Creates an InstanceManager interface to create & manipulate instanced entities
            You need to call this function at least once before start calling createInstancedEntity
            to build up an instance based on the given mesh.
        @remarks
            Instancing is a way of batching up geometry into a much more 
            efficient form, but with some limitations, and still be able to move & animate it.
            Please @see InstanceManager class documentation for full information.
        @param customName Custom name for referencing. Must be unique
        @param meshName The mesh name the instances will be based upon
        @param groupName The resource name where the mesh lives
        @param technique Technique to use, which may be shader based, or hardware based.
        @param numInstancesPerBatch Suggested number of instances per batch. The actual number
        may end up being lower if the technique doesn't support having so many. It can't be zero
        @param flags @see InstanceManagerFlags
        @param subMeshIdx InstanceManager only supports using one submesh from the base mesh. This parameter
        says which submesh to pick (must be <= Mesh::getNumSubMeshes())
        @return The new InstanceManager instance
        */
        virtual v1::InstanceManager* createInstanceManager( const String &customName,
                                                            const String &meshName,
                                                            const String &groupName,
                                                            v1::InstanceManager::InstancingTechnique technique,
                                                            size_t numInstancesPerBatch, uint16 flags=0,
                                                            unsigned short subMeshIdx=0 );

        /** Retrieves an existing InstanceManager by it's name.
        @note Throws an exception if the named InstanceManager does not exist
        */
        virtual v1::InstanceManager* getInstanceManager( IdString name ) const;

    /** Returns whether an InstanceManager with the given name exists. */
    virtual bool hasInstanceManager( IdString managerName ) const;

        /** Destroys an InstanceManager <b>if</b> it was created with createInstanceManager()
        @remarks
            Be sure you don't have any InstancedEntity referenced somewhere which was created with
            this manager, since it will become a dangling pointer.
        @param name Name of the manager to remove
        */
        virtual void destroyInstanceManager( IdString name );
        virtual void destroyInstanceManager( v1::InstanceManager *instanceManager );

        virtual void destroyAllInstanceManagers(void);

        /** @see InstanceManager::getMaxOrBestNumInstancesPerBatch
        @remarks
            If you've already created an InstanceManager, you can call it's
            getMaxOrBestNumInstancesPerBatch() function directly.
            Another (not recommended) way to know if the technique is unsupported is by creating
            an InstanceManager and use createInstancedEntity, which will return null pointer.
            The input parameter "numInstancesPerBatch" is a suggested value when using IM_VTFBESTFIT
            flag (in that case it should be non-zero)
        @return
            The ideal (or maximum, depending on flags) number of instances per batch for
            the given technique. Zero if technique is unsupported or errors were spotted
        */
        virtual size_t getNumInstancesPerBatch( const String &meshName, const String &groupName,
                                                const String &materialName,
                                                v1::InstanceManager::InstancingTechnique technique,
                                                size_t numInstancesPerBatch, uint16 flags=0,
                                                unsigned short subMeshIdx=0 );

        /** Creates an InstancedEntity based on an existing InstanceManager (@see createInstanceManager)
        @remarks
            * Return value may be null if the InstanceManger technique isn't supported
            * Try to keep the number of entities with different materials <b>to a minimum</b>
            * For more information @see InstancedManager @see InstancedBatch, @see InstancedEntity
            * Alternatively you can call InstancedManager::createInstanceEntity using the returned
            pointer from createInstanceManager
        @param materialName Material name 
        @param managerName Name of the instance manager
        @return An InstancedEntity ready to be attached to a SceneNode
        */
        virtual v1::InstancedEntity* createInstancedEntity( const String &materialName,
                                                            const String &managerName );

        /** Removes an InstancedEntity, @see SceneManager::createInstancedEntity &
            @see InstanceBatch::removeInstancedEntity
        @param instancedEntity Instance to remove
        */
        virtual void destroyInstancedEntity( v1::InstancedEntity *instancedEntity );

        /** Create a movable object of the type specified without a name.
        @remarks
        This is the generalised form of MovableObject creation where you can
        create a MovableObject of any specialised type generically, including
        any new types registered using plugins. The name is generated automatically.
        @param typeName The type of object to create
        @param objectMemMgr Memory Manager that will hold ObjectData data.
        @param params Optional name/value pair list to give extra parameters to
        the created object.
        */
        virtual MovableObject* createMovableObject(const String& typeName,
                                                    ObjectMemoryManager *objectMemMgr,
                                                    const NameValuePairList* params = 0);
        /**
        Returns if this SceneManager contains the specified MovableObject
        */
        virtual bool hasMovableObject( MovableObject *m );
        /** Destroys a MovableObject with the name specified, of the type specified.
        @remarks
            The MovableObject will automatically detach itself from any nodes
            on destruction.
        */
        virtual void destroyMovableObject( MovableObject *m, const String& typeName );
        /** Destroys a MovableObject.
        @remarks
            The MovableObject will automatically detach itself from any nodes
            on destruction.
        */
        virtual void destroyMovableObject(MovableObject* m);
        /** Destroy all MovableObjects of a given type. */
        virtual void destroyAllMovableObjectsByType(const String& typeName);
        /** Destroy all MovableObjects. */
        virtual void destroyAllMovableObjects(void);
        typedef VectorIterator<MovableObjectVec> MovableObjectIterator;
        /** Get an iterator over all MovableObect instances of a given type. 
        @note
            The iterator returned from this method is not thread safe, do not use this
            if you are creating or deleting objects of this type in another thread.
        */
        virtual MovableObjectIterator getMovableObjectIterator(const String& typeName);
        /** Inject a MovableObject instance created externally.
        @remarks
            This method 'injects' a MovableObject instance created externally into
            the MovableObject instance registry held in the SceneManager. You
            might want to use this if you have a MovableObject which you don't
            want to register a factory for; for example a MovableObject which 
            cannot be generally constructed by clients. 
        @note
            It is important that the MovableObject has a unique name for the type,
            and that its getMovableType() method returns a proper type name.
        */
        virtual void injectMovableObject(MovableObject* m);
        /** Extract a previously injected MovableObject.
        @remarks
            Essentially this does the same as destroyMovableObject, but only
            removes the instance from the internal lists, it does not attempt
            to destroy it.
        */
        virtual void extractMovableObject(MovableObject* m);
        /** Extract all injected MovableObjects of a given type.
        @remarks
            Essentially this does the same as destroyAllMovableObjectsByType, 
            but only removes the instances from the internal lists, it does not 
            attempt to destroy them.
        */
        virtual void extractAllMovableObjectsByType(const String& typeName);

        /** Sets a mask which is bitwise 'and'ed with objects own visibility masks
            to determine if the object is visible.
        @remarks
            Note that this is combined with any per-viewport visibility mask
            through an 'and' operation. @see Viewport::setVisibilityMask
        */
        virtual_l2 void setVisibilityMask(uint32 vmask)
                                { mVisibilityMask = vmask & VisibilityFlags::RESERVED_VISIBILITY_FLAGS; }

        /** Gets a mask which is bitwise 'and'ed with objects own visibility masks
            to determine if the object is visible.
        */
        virtual_l2 uint32 getVisibilityMask(void) const { return mVisibilityMask; }

        /** Internal method for getting the combination between the global visibility
            mask and the per-viewport visibility mask.
        */
        uint32 _getCombinedVisibilityMask(void) const;

        /** Sets whether the SceneManager should search for visible objects, or
            whether they are being manually handled.
        @remarks
            This is an advanced function, you should not use this unless you know
            what you are doing.
        */
        virtual void setFindVisibleObjects(bool find) { mFindVisibleObjects = find; }

        /** Gets whether the SceneManager should search for visible objects, or
            whether they are being manually handled.
        */
        virtual bool getFindVisibleObjects(void) { return mFindVisibleObjects; }

        /** Set whether to automatically flip the culling mode on objects whenever they
            are negatively scaled.
        @remarks
            Negativelyl scaling an object has the effect of flipping the triangles, 
            so the culling mode should probably be inverted to deal with this. 
            If you would prefer to manually manage this, set this option to 'false' 
            and use different materials with Pass::setCullingMode set manually as needed.
        */
        virtual void setFlipCullingOnNegativeScale(bool n) { mFlipCullingOnNegativeScale = n; }

        /** Get whether to automatically flip the culling mode on objects whenever they
            are negatively scaled.
        */
        virtual bool getFlipCullingOnNegativeScale() const { return mFlipCullingOnNegativeScale; }

        virtual void _renderSingleObject( Renderable* pRend, const MovableObject *pMovableObject,
                                          bool casterPass, bool dualParaboloid );

        /** Indicates to the SceneManager whether it should suppress changing
            the RenderSystem states when rendering objects.
        @remarks
            This method allows you to tell the SceneManager not to change any
            RenderSystem state until you tell it to. This method is only 
            intended for advanced use, don't use it if you're unsure of the 
            effect. The only RenderSystems calls made are to set the world 
            matrix for each object (note - view an projection matrices are NOT
            SET - they are under your control) and to render the object; it is up to 
            the caller to do everything else, including enabling any vertex / 
            fragment programs and updating their parameter state, and binding
            parameters to the RenderSystem.
        @note
            Calling this implicitly disables shadow processing since no shadows
            can be rendered without changing state.
        @param suppress If true, no RenderSystem state changes will be issued
            until this method is called again with a parameter of false.
        */
        virtual void _suppressRenderStateChanges(bool suppress);
        
        /** Are render state changes suppressed? 
        @see _suppressRenderStateChanges
        */
        virtual bool _areRenderStateChangesSuppressed(void) const
        { return mSuppressRenderStateChanges; }
        
        /** Method to allow you to mark gpu parameters as dirty, causing them to 
            be updated according to the mask that you set when updateGpuProgramParameters is
            next called. Only really useful if you're controlling parameter state in 
            inner rendering loop callbacks.
            @param mask Some combination of GpuParamVariability which is bitwise OR'ed with the
                current dirty state.
        */
        virtual void _markGpuParamsDirty(uint16 mask);

        /** Get the rendersystem subclass to which the output of this Scene Manager
            gets sent
        */
        RenderSystem *getDestinationRenderSystem();

        /** Gets the current viewport being rendered (advanced use only, only 
            valid during viewport update. */
        Viewport* getCurrentViewport(void) const { return mCurrentViewport; }

        /** Gets the current camera being rendered (advanced use only, only 
            valid during viewport update. */
        Camera* getCameraInProgress(void) const     { return mCameraInProgress; }

        AxisAlignedBox _calculateCurrentCastersBox( uint32 viewportVisibilityMask,
                                                    uint8 firstRq, uint8 lastRq ) const;

        /** @See CompositorShadowNode::getCastersBox
        @remarks
            Returns a null box if no active shadow node.
        */
        const AxisAlignedBox& getCurrentCastersBox(void) const;

        /** @See CompositorShadowNode::getMinMaxDepthRange
        @remarks
            Outputs 0 & 100000 if no active shadow node or camera not found.
        */
        void getMinMaxDepthRange( const Frustum *shadowMapCamera, Real &outMin, Real &outMax ) const;


        /** Set whether to use relative offset co-ordinates when rendering, ie
            offset to subtract to the camera, lights & objects.
        @remarks
            This is a technique to alleviate some of the precision issues associated with 
            rendering far from the origin, where single-precision floats as used in most
            GPUs begin to lose their precision. The origin "translates" to this new
            relativeOffset.
            Any previous non-permanent origin is overriden
        @par
            All that this function performs is just offseting the root scene node, and
            as such, will force to update the static nodes as well. Call this at a low
            frequency (i.e. when camera has gone too far from origin)
        @note
            If you need this option, you will probably also need to enable double-precision
            mode in Ogre (OGRE_DOUBLE_PRECISION), since even though this will 
            alleviate the rendering precision, the source camera and object positions will still 
            suffer from precision issues leading to jerky movement. 
        @param bPermanent
            When false, it only affects the root nodes (static & dynamic) so that everything is
            shifted by the relative origin, causing world & view matrices to contain smaller
            values. This improves the quality of skeletal animations and "Z fighting"-like
            artifacts because vertices don't snap to the right place.
            However, it won't fix the jittering of objects observed while translating them by
            small increments, including camera movement.

            Setting this value to true will force the SceneManager to propagate the change
            as much as possible to child nodes (including attached Cameras), causing
            the change to become permanent/irreversible. This achieves greater quality
            since translating objects or camera by small amounts now gets more accuracy.
            @See propagateRelativeOrigin.
        */
        virtual void setRelativeOrigin( const Vector3 &relativeOrigin, bool bPermanent );

        /// Returns the current relative origin. (Only when non-permanent)
        Vector3 getRelativeOrigin(void) const;

        /** Add a level of detail listener. */
        void addLodListener(LodListener *listener);

        /**
        Remove a level of detail listener.
        @remarks
            Do not call from inside an LodListener callback method.
        */
        void removeLodListener(LodListener *listener);

        /** Notify that a movable object LOD change event has occurred. */
        void _notifyMovableObjectLodChanged(MovableObjectLodChangedEvent& evt);

        /** Notify that an entity mesh LOD change event has occurred. */
        void _notifyEntityMeshLodChanged(EntityMeshLodChangedEvent& evt);

        /** Notify that an entity material LOD change event has occurred. */
        void _notifyEntityMaterialLodChanged(EntityMaterialLodChangedEvent& evt);

        /** Handle LOD events. */
        void _handleLodEvents();

        void _setCurrentRenderStage( IlluminationRenderStage stage ) { mIlluminationStage = stage; }
        IlluminationRenderStage _getCurrentRenderStage() const {return mIlluminationStage;}

    protected:

        void fireWorkerThreadsAndWait(void);

        /** Launches cullFrustum on all worker threads with the requested parameters
        @remarks
            Will block until all threads are done.
        */
        void fireCullFrustumThreads( const CullFrustumRequest &request );
        void fireCullFrustumInstanceBatchThreads( const InstanceBatchCullRequest &request );
        void startWorkerThreads();
        void stopWorkerThreads();

    public:

        /** Processes a user-defined UniformScalableTask in the worker threads
            spawned by SceneManager.
        @remarks
            If 'bBlock' is false, it is user responsibility to call
            waitForPendingUserScalableTask before the next call to either
            processUserScalableTask or renderOneFrame.
        @param task
            Task to perform. Pointer must be valid at least until the task is finished
        @param bBlock
            True if you want the function to block until the task is done.
            False if you want to do something in between, in this case you MUST
            call waitForPendingUserScalableTask later.
        */
        void executeUserScalableTask( UniformScalableTask *task, bool bBlock );

        /** Blocks until the the task from processUserScalableTask finishes.
        @remarks
            Do NOT call this function if you passed bBlock = true to processUserScalableTask
        */
        void waitForPendingUserScalableTask();

        /** Called from the worker thread, polls to process frustum culling
            requests when a sync is performed
        */
        unsigned long _updateWorkerThread( ThreadHandle *threadHandle );
    };

    /** Default implementation of IntersectionSceneQuery. */
    class _OgreExport DefaultIntersectionSceneQuery : 
        public IntersectionSceneQuery
    {
    public:
        DefaultIntersectionSceneQuery(SceneManager* creator);
        ~DefaultIntersectionSceneQuery();

        /** See IntersectionSceneQuery. */
        void execute(IntersectionSceneQueryListener* listener);
    };

    /** Default implementation of RaySceneQuery. */
    class _OgreExport DefaultRaySceneQuery : public RaySceneQuery
    {
    public:
        DefaultRaySceneQuery(SceneManager* creator);
        ~DefaultRaySceneQuery();

        /** See RayScenQuery. */
        virtual void execute(RaySceneQueryListener* listener);
        bool execute( ObjectData objData, size_t numNodes, RaySceneQueryListener* listener );
    };
    /** Default implementation of SphereSceneQuery. */
    class _OgreExport DefaultSphereSceneQuery : public SphereSceneQuery
    {
    public:
        DefaultSphereSceneQuery(SceneManager* creator);
        ~DefaultSphereSceneQuery();

        /** See SceneQuery. */
        virtual void execute(SceneQueryListener* listener);
        bool execute( ObjectData objData, size_t numNodes, SceneQueryListener* listener );
    };
    /** Default implementation of PlaneBoundedVolumeListSceneQuery. */
    class _OgreExport DefaultPlaneBoundedVolumeListSceneQuery : public PlaneBoundedVolumeListSceneQuery
    {
    public:
        DefaultPlaneBoundedVolumeListSceneQuery(SceneManager* creator);
        ~DefaultPlaneBoundedVolumeListSceneQuery();

        /** See SceneQuery. */
        void execute(SceneQueryListener* listener);
        bool execute(ObjectData objData, size_t numNodes, SceneQueryListener* listener);
    private:
        //SIMD friendly version of a plane
        struct ArrayPlane
        {
            ArrayVector3    planeNormal;
            ArrayVector3    signFlip;
            ArrayReal       planeNegD;
        };

        //List to store SIMD friendly planes and ensure that the memory is properly aligned
        RawSimdUniquePtr<ArrayPlane, MEMCATEGORY_GENERAL> mSimdPlaneList;
    };
    /** Default implementation of AxisAlignedBoxSceneQuery. */
    class _OgreExport DefaultAxisAlignedBoxSceneQuery : public AxisAlignedBoxSceneQuery
    {
    public:
        DefaultAxisAlignedBoxSceneQuery(SceneManager* creator);
        ~DefaultAxisAlignedBoxSceneQuery();

        /** See RayScenQuery. */
        virtual void execute(SceneQueryListener* listener);
        bool execute( ObjectData objData, size_t numNodes, SceneQueryListener* listener );
    };
    

    /// Bitmask containing scene types
    typedef uint16 SceneTypeMask;

    /** Classification of a scene to allow a decision of what type of
    SceenManager to provide back to the application.
    */
    enum SceneType
    {
        ST_GENERIC = 1,
        ST_EXTERIOR_CLOSE = 2,
        ST_EXTERIOR_FAR = 4,
        ST_EXTERIOR_REAL_FAR = 8,
        ST_INTERIOR = 16
    };

    /** Structure containing information about a scene manager. */
    struct SceneManagerMetaData
    {
        /// A globally unique string identifying the scene manager type
        String typeName;
        /// A text description of the scene manager
        String description;
        /// A mask describing which sorts of scenes this manager can handle
        SceneTypeMask sceneTypeMask;
        /// Flag indicating whether world geometry is supported
        bool worldGeometrySupported;
    };



    /** Class which will create instances of a given SceneManager. */
    class _OgreExport SceneManagerFactory : public SceneMgtAlloc
    {
    protected:
        mutable SceneManagerMetaData mMetaData;
        mutable bool mMetaDataInit;
        /// Internal method to initialise the metadata, must be implemented
        virtual void initMetaData(void) const = 0;
    public:
        SceneManagerFactory() : mMetaDataInit(true) {}
        virtual ~SceneManagerFactory() {}
        /** Get information about the SceneManager type created by this factory. */
        virtual const SceneManagerMetaData& getMetaData(void) const 
        {
            if (mMetaDataInit)
            {
                initMetaData();
                mMetaDataInit = false;
            }
            return mMetaData; 
        }
        /** Create a new instance of a SceneManager.
        @remarks
        Don't call directly, use SceneManagerEnumerator::createSceneManager.
        */
        virtual SceneManager* createInstance(const String& instanceName, size_t numWorkerThreads,
                                            InstancingThreadedCullingMethod threadedCullingMethod) = 0;
        /** Destroy an instance of a SceneManager. */
        virtual void destroyInstance(SceneManager* instance) = 0;

    };

    /** @} */
    /** @} */


} // Namespace

#include "OgreHeaderSuffix.h"

#endif
