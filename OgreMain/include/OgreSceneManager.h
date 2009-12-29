/*-------------------------------------------------------------------------
This source file is a part of OGRE
(Object-oriented Graphics Rendering Engine)

For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd
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

#include "OgreString.h"
#include "OgreSceneNode.h"
#include "OgrePlane.h"
#include "OgreQuaternion.h"
#include "OgreColourValue.h"
#include "OgreCommon.h"
#include "OgreSceneQuery.h"
#include "OgreAutoParamDataSource.h"
#include "OgreAnimationState.h"
#include "OgreRenderQueue.h"
#include "OgreRenderQueueSortingGrouping.h"
#include "OgreRectangle2D.h"
#include "OgrePixelFormat.h"
#include "OgreResourceGroupManager.h"
#include "OgreTexture.h"
#include "OgreShadowCameraSetup.h"
#include "OgreShadowTextureManager.h"
#include "OgreCamera.h"
#include "OgreInstancedGeometry.h"
#include "OgreLodListener.h"
#include "OgreRenderSystem.h"
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

	// Forward declarations
	class DefaultIntersectionSceneQuery;
	class DefaultRaySceneQuery;
	class DefaultSphereSceneQuery;
	class DefaultAxisAlignedBoxSceneQuery;
	class CompositorChain;

	/** Structure collecting together information about the visible objects
	that have been discovered in a scene.
	*/
	struct _OgreExport VisibleObjectsBoundsInfo
	{
		/// The axis-aligned bounds of the visible objects
		AxisAlignedBox aabb;
		/// The axis-aligned bounds of the visible shadow receiver objects
		AxisAlignedBox receiverAabb;
		/// The closest a visible object is to the camera
		Real minDistance;
		/// The farthest a visible objects is from the camera
		Real maxDistance;
		/// The closest a object in the frustum regardless of visibility / shadow caster flags
		Real minDistanceInFrustum;
		/// The farthest object in the frustum regardless of visibility / shadow caster flags
		Real maxDistanceInFrustum;

		VisibleObjectsBoundsInfo();
		void reset();
		void merge(const AxisAlignedBox& boxBounds, const Sphere& sphereBounds, 
			const Camera* cam, bool receiver=true);
		/** Merge an object that is not being rendered because it's not a shadow caster, 
			but is a shadow receiver so should be included in the range.
		*/
		void mergeNonRenderedButInFrustum(const AxisAlignedBox& boxBounds, 
			const Sphere& sphereBounds, const Camera* cam);


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
	class _OgreExport SceneManager : public SceneMgtAlloc
    {
    public:
        /// Query type mask which will be used for world geometry @see SceneQuery
        static uint32 WORLD_GEOMETRY_TYPE_MASK;
		/// Query type mask which will be used for entities @see SceneQuery
		static uint32 ENTITY_TYPE_MASK;
		/// Query type mask which will be used for effects like billboardsets / particle systems @see SceneQuery
		static uint32 FX_TYPE_MASK;
		/// Query type mask which will be used for StaticGeometry  @see SceneQuery
		static uint32 STATICGEOMETRY_TYPE_MASK;
		/// Query type mask which will be used for lights  @see SceneQuery
		static uint32 LIGHT_TYPE_MASK;
		/// Query type mask which will be used for frusta and cameras @see SceneQuery
		static uint32 FRUSTUM_TYPE_MASK;
		/// User type mask limit
		static uint32 USER_TYPE_MASK_LIMIT;
        /** Comparator for material map, for sorting materials into render order (e.g. transparent last).
        */
        struct materialLess
        {
            _OgreExport bool operator()(const Material* x, const Material* y) const;
        };
        /// Comparator for sorting lights relative to a point
        struct lightLess
        {
            _OgreExport bool operator()(const Light* a, const Light* b) const;
        };

        /// Describes the stage of rendering when performing complex illumination
        enum IlluminationRenderStage
        {
            /// No special illumination stage
            IRS_NONE,
            /// Render to texture stage, used for texture based shadows
            IRS_RENDER_TO_TEXTURE,
            /// Render from shadow texture to receivers stage
            IRS_RENDER_RECEIVER_PASS
        };

		/** Enumeration of the possible modes allowed for processing the special case
		render queue list.
		@see SceneManager::setSpecialCaseRenderQueueMode
		*/
		enum SpecialCaseRenderQueueMode
		{
			/// Render only the queues in the special case list
			SCRQM_INCLUDE,
			/// Render all except the queues in the special case list
			SCRQM_EXCLUDE
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
				(including main scene geometry and any additional shadow receiver 
				passes). 
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
			virtual void shadowTextureCasterPreViewProj(Light* light, 
				Camera* camera, size_t iteration)
                        { (void)light; (void)camera; (void)iteration; }

			/** This event occurs just before the view & projection matrices are
		 		set for re-rendering a shadow receiver.
			@remarks
				You can use this event hook to perform some custom processing,
				such as altering the projection frustum being used for rendering 
				the shadow onto the receiver to perform an advanced shadow 
				technique.
			@note
				This event will only be fired when texture shadows are in use.
			@param light Pointer to the light for which shadows are being rendered
			@param frustum Pointer to the projection frustum being used to project
				the shadow texture
			*/
			virtual void shadowTextureReceiverPreViewProj(Light* light, 
				Frustum* frustum)
                        { (void)light; (void)frustum; }

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
			@returns true if you sorted the list, false otherwise.
			*/
			virtual bool sortLightsAffectingFrustum(LightList& lightList)
                        { (void)lightList; return false; }

			/** Event notifying the listener of the SceneManager's destruction. */
			virtual void sceneManagerDestroyed(SceneManager* source)
                        { (void)source; }
		};

		/** Inner helper class to implement the visitor pattern for rendering objects
			in a queue. 
		*/
		class _OgreExport SceneMgrQueuedRenderableVisitor : public QueuedRenderableVisitor
		{
		protected:
			/// Pass that was actually used at the grouping level
			const Pass* mUsedPass;
		public:
			SceneMgrQueuedRenderableVisitor() 
				:transparentShadowCastersMode(false) {}
			~SceneMgrQueuedRenderableVisitor() {}
			void visit(Renderable* r);
			bool visit(const Pass* p);
			void visit(RenderablePass* rp);

			/// Target SM to send renderables to
			SceneManager* targetSceneMgr;
			/// Are we in transparent shadow caster mode?
			bool transparentShadowCastersMode;
			/// Automatic light handling?
			bool autoLights;
			/// Manual light list
			const LightList* manualLightList;
			/// Scissoring if requested?
			bool scissoring;

		};
		/// Allow visitor helper to access protected methods
		friend class SceneMgrQueuedRenderableVisitor;

    protected:

        /// Subclasses can override this to ensure their specialised SceneNode is used.
        virtual SceneNode* createSceneNodeImpl(void);
        /// Subclasses can override this to ensure their specialised SceneNode is used.
        virtual SceneNode* createSceneNodeImpl(const String& name);

		/// Instance name
		String mName;

        /// Queue of objects for rendering
        RenderQueue* mRenderQueue;
		bool mLastRenderQueueInvocationCustom;

        /// Current ambient light, cached for RenderSystem
        ColourValue mAmbientLight;

        /// The rendering system to send the scene to
        RenderSystem *mDestRenderSystem;

        typedef map<String, Camera* >::type CameraList;

        /** Central list of cameras - for easy memory management and lookup.
        */
        CameraList mCameras;

		typedef map<String, StaticGeometry* >::type StaticGeometryList;
		StaticGeometryList mStaticGeometryList;
		typedef map<String, InstancedGeometry* >::type InstancedGeometryList;
		InstancedGeometryList mInstancedGeometryList;

        typedef map<String, SceneNode*>::type SceneNodeList;

        /** Central list of SceneNodes - for easy memory management.
            @note
                Note that this list is used only for memory management; the structure of the scene
                is held using the hierarchy of SceneNodes starting with the root node. However you
                can look up nodes this way.
        */
        SceneNodeList mSceneNodes;

        /// Camera in progress
        Camera* mCameraInProgress;
        /// Current Viewport
        Viewport* mCurrentViewport;

        /// Root scene node
        SceneNode* mSceneRoot;

        /// Autotracking scene nodes
        typedef set<SceneNode*>::type AutoTrackingSceneNodes;
        AutoTrackingSceneNodes mAutoTrackingSceneNodes;

        // Sky params
        // Sky plane
        Entity* mSkyPlaneEntity;
        Entity* mSkyDomeEntity[5];
        ManualObject* mSkyBoxObj;

        SceneNode* mSkyPlaneNode;
        SceneNode* mSkyDomeNode;
        SceneNode* mSkyBoxNode;

        // Sky plane
        bool mSkyPlaneEnabled;
        uint8 mSkyPlaneRenderQueue;
        Plane mSkyPlane;
        SkyPlaneGenParameters mSkyPlaneGenParameters;
        // Sky box
        bool mSkyBoxEnabled;
        uint8 mSkyBoxRenderQueue;
        Quaternion mSkyBoxOrientation;
        SkyBoxGenParameters mSkyBoxGenParameters;
        // Sky dome
        bool mSkyDomeEnabled;
        uint8 mSkyDomeRenderQueue;
        Quaternion mSkyDomeOrientation;
        SkyDomeGenParameters mSkyDomeGenParameters;

        // Fog
        FogMode mFogMode;
        ColourValue mFogColour;
        Real mFogStart;
        Real mFogEnd;
        Real mFogDensity;

		typedef set<uint8>::type SpecialCaseRenderQueueList;
		SpecialCaseRenderQueueList mSpecialCaseQueueList;
		SpecialCaseRenderQueueMode mSpecialCaseQueueMode;
		uint8 mWorldGeometryRenderQueue;
		
		unsigned long mLastFrameNumber;
		Matrix4 mTempXform[256];
		bool mResetIdentityView;
		bool mResetIdentityProj;

		bool mNormaliseNormalsOnScale;
		bool mFlipCullingOnNegativeScale;
		CullingMode mPassCullingMode;

	protected:

		/** Visible objects bounding box list.
			@remarks
				Holds an ABB for each camera that contains the physical extends of the visible
				scene elements by each camera. The map is crucial for shadow algorithms which
				have a focus step to limit the shadow sample distribution to only valid visible
				scene elements.
		*/
		typedef map< const Camera*, VisibleObjectsBoundsInfo>::type CamVisibleObjectsMap;
		CamVisibleObjectsMap mCamVisibleObjectsMap; 

		/** ShadowCamera to light mapping */
		typedef map< const Camera*, const Light* >::type ShadowCamLightMapping;
		ShadowCamLightMapping mShadowCamLightMapping;

		/// Array defining shadow count per light type.
		size_t mShadowTextureCountPerType[3];

		/// Array defining shadow texture index in light list.
		vector<size_t>::type mShadowTextureIndexLightList;

        /// Cached light information, used to tracking light's changes
        struct _OgreExport LightInfo
        {
            Light* light;       // Just a pointer for comparison, the light might destroyed for some reason
            int type;           // Use int instead of Light::LightTypes to avoid header file dependence
            Real range;         // Sets to zero if directional light
            Vector3 position;   // Sets to zero if directional light
			uint32 lightMask;   // Light mask

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

        LightList mLightsAffectingFrustum;
        LightInfoList mCachedLightInfos;
		LightInfoList mTestLightInfos; // potentially new list
        ulong mLightsDirtyCounter;
		LightList mShadowTextureCurrentCasterLightList;

		typedef map<String, MovableObject*>::type MovableObjectMap;
		/// Simple structure to hold MovableObject map and a mutex to go with it.
		struct MovableObjectCollection
		{
			MovableObjectMap map;
			OGRE_MUTEX(mutex)
		};
		typedef map<String, MovableObjectCollection*>::type MovableObjectCollectionMap;
		MovableObjectCollectionMap mMovableObjectCollectionMap;
		NameGenerator mMovableNameGenerator;
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
		OGRE_MUTEX(mMovableObjectCollectionMapMutex)

        /** Internal method for initialising the render queue.
        @remarks
            Subclasses can use this to install their own RenderQueue implementation.
        */
        virtual void initRenderQueue(void);
        /// A pass designed to let us render shadow colour on white for texture shadows
        Pass* mShadowCasterPlainBlackPass;
        /// A pass designed to let us render shadow receivers for texture shadows
        Pass* mShadowReceiverPass;
        /** Internal method for turning a regular pass into a shadow caster pass.
        @remarks
            This is only used for texture shadows, basically we're trying to
            ensure that objects are rendered solid black.
            This method will usually return the standard solid black pass for
            all fixed function passes, but will merge in a vertex program
            and fudge the AutpoParamDataSource to set black lighting for
            passes with vertex programs. 
        */
        virtual const Pass* deriveShadowCasterPass(const Pass* pass);
        /** Internal method for turning a regular pass into a shadow receiver pass.
        @remarks
        This is only used for texture shadows, basically we're trying to
        ensure that objects are rendered with a projective texture.
        This method will usually return a standard single-texture pass for
        all fixed function passes, but will merge in a vertex program
        for passes with vertex programs. 
        */
        virtual const Pass* deriveShadowReceiverPass(const Pass* pass);
    
        /** Internal method to validate whether a Pass should be allowed to render.
        @remarks
            Called just before a pass is about to be used for rendering a group to
            allow the SceneManager to omit it if required. A return value of false
            skips this pass. 
        */
        virtual bool validatePassForRendering(const Pass* pass);

        /** Internal method to validate whether a Renderable should be allowed to render.
        @remarks
        Called just before a pass is about to be used for rendering a Renderable to
        allow the SceneManager to omit it if required. A return value of false
        skips it. 
        */
        virtual bool validateRenderableForRendering(const Pass* pass, const Renderable* rend);

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
        virtual MeshPtr createSkyboxPlane(
            BoxPlane bp,
            Real distance,
            const Quaternion& orientation,
            const String& groupName);

        /* Internal utility method for creating the planes of a skydome.
        */
        virtual MeshPtr createSkydomePlane(
            BoxPlane bp,
            Real curvature, Real tiling, Real distance,
            const Quaternion& orientation,
            int xsegments, int ysegments, int ySegmentsToKeep, 
            const String& groupName);

        // Flag indicating whether SceneNodes will be rendered as a set of 3 axes
        bool mDisplayNodes;

        /// Storage of animations, lookup by name
        typedef map<String, Animation*>::type AnimationList;
        AnimationList mAnimationsList;
		OGRE_MUTEX(mAnimationsListMutex)
        AnimationStateSet mAnimationStates;


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
		virtual void fireRenderSingleObject(Renderable* rend, const Pass* pass, const AutoParamDataSource* source, 
			const LightList* pLightList, bool suppressRenderStateChanges);

		/// Internal method for firing the texture shadows updated event
        virtual void fireShadowTexturesUpdated(size_t numberOfShadowTextures);
		/// Internal method for firing the pre caster texture shadows event
        virtual void fireShadowTexturesPreCaster(Light* light, Camera* camera, size_t iteration);
		/// Internal method for firing the pre receiver texture shadows event
        virtual void fireShadowTexturesPreReceiver(Light* light, Frustum* f);
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

		/** Internal method for rendering all objects using the default queue sequence. */
		virtual void renderVisibleObjectsDefaultSequence(void);
		/** Internal method for rendering all objects using a custom queue sequence. */
		virtual void renderVisibleObjectsCustomSequence(RenderQueueInvocationSequence* s);
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
        virtual void renderSingleObject(Renderable* rend, const Pass* pass, 
			bool lightScissoringClipping, bool doLightIteration, const LightList* manualLightList = 0);

		/** Internal method for creating the AutoParamDataSource instance. */
		virtual AutoParamDataSource* createAutoParamDataSource(void) const
		{
			return OGRE_NEW AutoParamDataSource();
		}

        /// Utility class for calculating automatic parameters for gpu programs
        AutoParamDataSource* mAutoParamDataSource;

		CompositorChain* mActiveCompositorChain;
		bool mLateMaterialResolving;

        ShadowTechnique mShadowTechnique;
        bool mDebugShadows;
        ColourValue mShadowColour;
        Pass* mShadowDebugPass;
        Pass* mShadowStencilPass;
        Pass* mShadowModulativePass;
		bool mShadowMaterialInitDone;
        HardwareIndexBufferSharedPtr mShadowIndexBuffer;
		size_t mShadowIndexBufferSize;
        Rectangle2D* mFullScreenQuad;
        Real mShadowDirLightExtrudeDist;
        IlluminationRenderStage mIlluminationStage;
		ShadowTextureConfigList mShadowTextureConfigList;
		bool mShadowTextureConfigDirty;
        ShadowTextureList mShadowTextures;
		TexturePtr mNullShadowTexture;
		typedef vector<Camera*>::type ShadowTextureCameraList;
		ShadowTextureCameraList mShadowTextureCameras;
        Texture* mCurrentShadowTexture;
		bool mShadowUseInfiniteFarPlane;
		bool mShadowCasterRenderBackFaces;
		bool mShadowAdditiveLightClip;
		/// Struct for cacheing light clipping information for re-use in a frame
		struct LightClippingInfo
		{
			RealRect scissorRect;
			PlaneList clipPlanes;
			bool scissorValid;
			unsigned long clipPlanesValid;
			LightClippingInfo() : scissorValid(false), clipPlanesValid(false) {}

		};
		typedef map<Light*, LightClippingInfo>::type LightClippingInfoMap;
		LightClippingInfoMap mLightClippingInfoMap;
		unsigned long mLightClippingInfoMapFrameNumber;

		/// default shadow camera setup
		ShadowCameraSetupPtr mDefaultShadowCameraSetup;

		/** Default sorting routine which sorts lights which cast shadows
			to the front of a list, sub-sorting by distance.
		@remarks
			Since shadow textures are generated from lights based on the
			frustum rather than individual objects, a shadow and camera-wise sort is
			required to pick the best lights near the start of the list. Up to 
			the number of shadow textures will be generated from this.
		*/
		struct lightsForShadowTextureLess
		{
			_OgreExport bool operator()(const Light* l1, const Light* l2) const;
		};


        /** Internal method for locating a list of lights which could be affecting the frustum. 
        @remarks
            Custom scene managers are encouraged to override this method to make use of their
            scene partitioning scheme to more efficiently locate lights, and to eliminate lights
            which may be occluded by word geometry.
        */
        virtual void findLightsAffectingFrustum(const Camera* camera);
        /// Internal method for setting up materials for shadows
        virtual void initShadowVolumeMaterials(void);
        /// Internal method for creating shadow textures (texture-based shadows)
        virtual void ensureShadowTexturesCreated();
        /// Internal method for destroying shadow textures (texture-based shadows)
        virtual void destroyShadowTextures(void);
        
	public:
		/// Method for preparing shadow textures ready for use in a regular render
		/// Do not call manually unless before frame start or rendering is paused
		/// If lightList is not supplied, will render all lights in frustum
        virtual void prepareShadowTextures(Camera* cam, Viewport* vp, const LightList* lightList = 0);

		//A render context, used to store internal data for pausing/resuming rendering
		struct RenderContext
		{
			RenderQueue* renderQueue;	
			Viewport* viewport;
			Camera* camera;
			CompositorChain* activeChain;
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
        /** Internal method for rendering all the objects for a given light into the 
            stencil buffer.
        @param light The light source
        @param cam The camera being viewed from
		@param calcScissor Whether the method should set up any scissor state, or
			false if that's already been done
        */
        virtual void renderShadowVolumesToStencil(const Light* light, const Camera* cam, 
			bool calcScissor);
        /** Internal utility method for setting stencil state for rendering shadow volumes. 
        @param secondpass Is this the second pass?
        @param zfail Should we be using the zfail method?
        @param twosided Should we use a 2-sided stencil?
        */
        virtual void setShadowVolumeStencilState(bool secondpass, bool zfail, bool twosided);
        /** Render a set of shadow renderables. */
        void renderShadowVolumeObjects(ShadowCaster::ShadowRenderableListIterator iShadowRenderables,
            Pass* pass, const LightList *manualLightList, unsigned long flags,
            bool secondpass, bool zfail, bool twosided);
        typedef vector<ShadowCaster*>::type ShadowCasterList;
        ShadowCasterList mShadowCasterList;
        SphereSceneQuery* mShadowCasterSphereQuery;
        AxisAlignedBoxSceneQuery* mShadowCasterAABBQuery;
        Real mDefaultShadowFarDist;
        Real mDefaultShadowFarDistSquared;
        Real mShadowTextureOffset; // proportion of texture offset in view direction e.g. 0.4
        Real mShadowTextureFadeStart; // as a proportion e.g. 0.6
        Real mShadowTextureFadeEnd; // as a proportion e.g. 0.9
		bool mShadowTextureSelfShadow;
		Pass* mShadowTextureCustomCasterPass;
		Pass* mShadowTextureCustomReceiverPass;
		String mShadowTextureCustomCasterVertexProgram;
		String mShadowTextureCustomReceiverVertexProgram;
		String mShadowTextureCustomReceiverFragmentProgram;
		GpuProgramParametersSharedPtr mShadowTextureCustomCasterVPParams;
		GpuProgramParametersSharedPtr mShadowTextureCustomReceiverVPParams;
		GpuProgramParametersSharedPtr mShadowTextureCustomReceiverFPParams;

		/// Visibility mask used to show / hide objects
		uint32 mVisibilityMask;
		bool mFindVisibleObjects;

		/// Suppress render state changes?
		bool mSuppressRenderStateChanges;
		/// Suppress shadows?
		bool mSuppressShadows;


        GpuProgramParametersSharedPtr mInfiniteExtrusionParams;
        GpuProgramParametersSharedPtr mFiniteExtrusionParams;

        /// Inner class to use as callback for shadow caster scene query
        class _OgreExport ShadowCasterSceneQueryListener : public SceneQueryListener, public SceneMgtAlloc
        {
        protected:
			SceneManager* mSceneMgr;
            ShadowCasterList* mCasterList;
            bool mIsLightInFrustum;
            const PlaneBoundedVolumeList* mLightClipVolumeList;
            const Camera* mCamera;
            const Light* mLight;
            Real mFarDistSquared;
        public:
            ShadowCasterSceneQueryListener(SceneManager* sm) : mSceneMgr(sm),
				mCasterList(0), mIsLightInFrustum(false), mLightClipVolumeList(0), 
                mCamera(0) {}
            // Prepare the listener for use with a set of parameters  
            void prepare(bool lightInFrustum, 
                const PlaneBoundedVolumeList* lightClipVolumes, 
                const Light* light, const Camera* cam, ShadowCasterList* casterList, 
                Real farDistSquared) 
            {
                mCasterList = casterList;
                mIsLightInFrustum = lightInFrustum;
                mLightClipVolumeList = lightClipVolumes;
                mCamera = cam;
                mLight = light;
                mFarDistSquared = farDistSquared;
            }
            bool queryResult(MovableObject* object);
            bool queryResult(SceneQuery::WorldFragment* fragment);
        };

        ShadowCasterSceneQueryListener* mShadowCasterQueryListener;

        /** Internal method for locating a list of shadow casters which 
            could be affecting the frustum for a given light. 
        @remarks
            Custom scene managers are encouraged to override this method to add optimisations, 
            and to add their own custom shadow casters (perhaps for world geometry)
        */
        virtual const ShadowCasterList& findShadowCastersForLight(const Light* light, 
            const Camera* camera);
        /** Render a group in the ordinary way */
		virtual void renderBasicQueueGroupObjects(RenderQueueGroup* pGroup, 
			QueuedRenderableCollection::OrganisationMode om);
		/** Render a group with the added complexity of additive stencil shadows. */
		virtual void renderAdditiveStencilShadowedQueueGroupObjects(RenderQueueGroup* group, 
			QueuedRenderableCollection::OrganisationMode om);
		/** Render a group with the added complexity of modulative stencil shadows. */
		virtual void renderModulativeStencilShadowedQueueGroupObjects(RenderQueueGroup* group, 
			QueuedRenderableCollection::OrganisationMode om);
        /** Render a group rendering only shadow casters. */
		virtual void renderTextureShadowCasterQueueGroupObjects(RenderQueueGroup* group, 
			QueuedRenderableCollection::OrganisationMode om);
        /** Render a group rendering only shadow receivers. */
		virtual void renderTextureShadowReceiverQueueGroupObjects(RenderQueueGroup* group, 
			QueuedRenderableCollection::OrganisationMode om);
        /** Render a group with the added complexity of modulative texture shadows. */
		virtual void renderModulativeTextureShadowedQueueGroupObjects(RenderQueueGroup* group, 
			QueuedRenderableCollection::OrganisationMode om);

		/** Render a group with additive texture shadows. */
		virtual void renderAdditiveTextureShadowedQueueGroupObjects(RenderQueueGroup* group, 
			QueuedRenderableCollection::OrganisationMode om);
		/** Render a set of objects, see renderSingleObject for param definitions */
		virtual void renderObjects(const QueuedRenderableCollection& objs, 
			QueuedRenderableCollection::OrganisationMode om, bool lightScissoringClipping,
            bool doLightIteration, const LightList* manualLightList = 0);
		/** Render those objects in the transparent pass list which have shadow casting forced on
		@remarks
			This function is intended to be used to render the shadows of transparent objects which have
			transparency_casts_shadows set to 'on' in their material
		*/
		virtual void renderTransparentShadowCasterObjects(const QueuedRenderableCollection& objs, 
			QueuedRenderableCollection::OrganisationMode om, bool lightScissoringClipping,
			bool doLightIteration, const LightList* manualLightList = 0);

		/** Update the state of the global render queue splitting based on a shadow
		option change. */
		virtual void updateRenderQueueSplitOptions(void);
		/** Update the state of the render queue group splitting based on a shadow
		option change. */
		virtual void updateRenderQueueGroupSplitOptions(RenderQueueGroup* group, 
			bool suppressShadows, bool suppressRenderState);

		/// Set up a scissor rectangle from a group of lights
		virtual ClipResult buildAndSetScissor(const LightList& ll, const Camera* cam);
		/// Update a scissor rectangle from a single light
		virtual void buildScissor(const Light* l, const Camera* cam, RealRect& rect);
		virtual void resetScissor();
		/// Build a set of user clip planes from a single non-directional light
		virtual ClipResult buildAndSetLightClip(const LightList& ll);
		virtual void buildLightClip(const Light* l, PlaneList& planes);
		virtual void resetLightClip();
		virtual void checkCachedLightClippingInfo();

		/// The active renderable visitor class - subclasses could override this
		SceneMgrQueuedRenderableVisitor* mActiveQueuedRenderableVisitor;
		/// Storage for default renderable visitor
		SceneMgrQueuedRenderableVisitor mDefaultQueuedRenderableVisitor;

		/// Whether to use camera-relative rendering
		bool mCameraRelativeRendering;
		Matrix4 mCachedViewMatrix;
		Vector3 mCameraRelativePosition;

		/// Last light sets
		uint32 mLastLightHash;
		unsigned short mLastLightLimit;
		uint32 mLastLightHashGpuProgram;
		/// Gpu params that need rebinding (mask of GpuParamVariability)
		uint16 mGpuParamsDirty;

		virtual void useLights(const LightList& lights, unsigned short limit);
		virtual void setViewMatrix(const Matrix4& m);
		virtual void useLightsGpuProgram(const Pass* pass, const LightList* lights);
		virtual void bindGpuProgram(GpuProgram* prog);
		virtual void updateGpuProgramParameters(const Pass* p);








        /// Set of registered lod listeners
        typedef set<LodListener*>::type LodListenerSet;
        LodListenerSet mLodListeners;

        /// List of movable object lod changed events
		typedef vector<MovableObjectLodChangedEvent>::type MovableObjectLodChangedEventList;
        MovableObjectLodChangedEventList mMovableObjectLodChangedEvents;

        /// List of entity mesh lod changed events
        typedef vector<EntityMeshLodChangedEvent>::type EntityMeshLodChangedEventList;
        EntityMeshLodChangedEventList mEntityMeshLodChangedEvents;

        /// List of entity material lod changed events
        typedef vector<EntityMaterialLodChangedEvent>::type EntityMaterialLodChangedEventList;
        EntityMaterialLodChangedEventList mEntityMaterialLodChangedEvents;

    public:
        /** Constructor.
        */
        SceneManager(const String& instanceName);

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
		OGRE_MUTEX(sceneGraphMutex)

		/** Return the instance name of this SceneManager. */
		const String& getName(void) const { return mName; }

		/** Retrieve the type name of this scene manager.
		@remarks
			This method has to be implemented by subclasses. It should
			return the type name of this SceneManager which agrees with 
			the type name of the SceneManagerFactory which created it.
		*/
		virtual const String& getTypeName(void) const = 0;

        /** Creates a camera to be managed by this scene manager.
            @remarks
                This camera must be added to the scene at a later time using
                the attachObject method of the SceneNode class.
            @param
                name Name to give the new camera.
        */
        virtual Camera* createCamera(const String& name);

        /** Retrieves a pointer to the named camera.
		@note Throws an exception if the named instance does not exist
        */
        virtual Camera* getCamera(const String& name) const;

		/** Returns whether a camera with the given name exists.
		*/
		virtual bool hasCamera(const String& name) const;

        /** Removes a camera from the scene.
            @remarks
                This method removes a previously added camera from the scene.
                The camera is deleted so the caller must ensure no references
                to it's previous instance (e.g. in a SceneNode) are used.
            @param
                cam Pointer to the camera to remove
        */
        virtual void destroyCamera(Camera *cam);

        /** Removes a camera from the scene.
            @remarks
                This method removes an camera from the scene based on the
                camera's name rather than a pointer.
        */
        virtual void destroyCamera(const String& name);

        /** Removes (and destroys) all cameras from the scene.
            @remarks
                Some cameras are internal created to dealing with texture shadow,
                their aren't supposed to destroy outside. So, while you are using
                texture shadow, don't call this method, or you can set the shadow
                technique other than texture-based, which will destroy all internal
                created shadow cameras and textures.
        */
        virtual void destroyAllCameras(void);

        /** Creates a light for use in the scene.
            @remarks
                Lights can either be in a fixed position and independent of the
                scene graph, or they can be attached to SceneNodes so they derive
                their position from the parent node. Either way, they are created
                using this method so that the SceneManager manages their
                existence.
            @param
                name The name of the new light, to identify it later.
        */
        virtual Light* createLight(const String& name);

		/** Creates a light with a generated name. */
		virtual Light* createLight();

        /** Returns a pointer to the named Light which has previously been added to the scene.
		@note Throws an exception if the named instance does not exist
        */
        virtual Light* getLight(const String& name) const;

		/** Returns whether a light with the given name exists.
		*/
		virtual bool hasLight(const String& name) const;

		/** Retrieve a set of clipping planes for a given light. 
		*/
		virtual const PlaneList& getLightClippingPlanes(Light* l);

		/** Retrieve a scissor rectangle for a given light and camera. 
		*/
		virtual const RealRect& getLightScissorRect(Light* l, const Camera* cam);

		/** Removes the named light from the scene and destroys it.
            @remarks
                Any pointers held to this light after calling this method will be invalid.
        */
        virtual void destroyLight(const String& name);

        /** Removes the light from the scene and destroys it based on a pointer.
            @remarks
                Any pointers held to this light after calling this method will be invalid.
        */
        virtual void destroyLight(Light* light);
        /** Removes and destroys all lights in the scene.
        */
        virtual void destroyAllLights(void);

        /** Advance method to increase the lights dirty counter due lights changed.
        @remarks
            Scene manager tracking lights that affecting the frustum, if changes
            detected (the changes includes light list itself and the light's position
            and attenuation range), then increase the lights dirty counter.
        @par
            For some reason, you can call this method to force whole scene objects
            re-populate their light list. But near in mind, call to this method
            will harm performance, so should avoid if possible.
        */
        virtual void _notifyLightsDirty(void);

        /** Advance method to gets the lights dirty counter.
        @remarks
            Scene manager tracking lights that affecting the frustum, if changes
            detected (the changes includes light list itself and the light's position
            and attenuation range), then increase the lights dirty counter.
        @par
            When implementing customise lights finding algorithm relied on either
            SceneManager::_getLightsAffectingFrustum or SceneManager::_populateLightList,
            might check this value for sure that the light list are really need to
            re-populate, otherwise, returns cached light list (if exists) for better
            performance.
        */
        ulong _getLightsDirtyCounter(void) const { return mLightsDirtyCounter; }

        /** Get the list of lights which could be affecting the frustum.
        @remarks
            Note that default implementation of this method returns a cached light list,
            which is populated when rendering the scene. So by default the list of lights 
			is only available during scene rendering.
        */
        virtual const LightList& _getLightsAffectingFrustum(void) const;

        /** Populate a light list with an ordered set of the lights which are closest
        to the position specified.
        @remarks
            Note that since directional lights have no position, they are always considered
            closer than any point lights and as such will always take precedence. 
        @par
            Subclasses of the default SceneManager may wish to take into account other issues
            such as possible visibility of the light if that information is included in their
            data structures. This basic scenemanager simply orders by distance, eliminating 
            those lights which are out of range or could not be affecting the frustum (i.e.
            only the lights returned by SceneManager::_getLightsAffectingFrustum are take into
            account).
        @par
            The number of items in the list max exceed the maximum number of lights supported
            by the renderer, but the extraneous ones will never be used. In fact the limit will
            be imposed by Pass::getMaxSimultaneousLights.
        @param position The position at which to evaluate the list of lights
        @param radius The bounding radius to test
        @param destList List to be populated with ordered set of lights; will be cleared by 
            this method before population.
		@param lightMask The mask with which to include / exclude lights
        */
        virtual void _populateLightList(const Vector3& position, Real radius, LightList& destList, uint32 lightMask = 0xFFFFFFFF);

		/** Populates a light list with an ordered set of the lights which are closest
        to the position of the SceneNode given.
        @remarks
            Note that since directional lights have no position, they are always considered
            closer than any point lights and as such will always take precedence. 
			This overloaded version will take the SceneNode's position and use the second method
			to populate the list.
        @par
            Subclasses of the default SceneManager may wish to take into account other issues
            such as possible visibility of the light if that information is included in their
            data structures. This basic scenemanager simply orders by distance, eliminating 
            those lights which are out of range or could not be affecting the frustum (i.e.
            only the lights returned by SceneManager::_getLightsAffectingFrustum are take into
            account). 
		@par   
			Also note that subclasses of the SceneNode might be used here to provide cached
			scene related data, accelerating the list population (for example light lists for
			SceneNodes could be cached inside subclassed SceneNode objects).
        @par
            The number of items in the list may exceed the maximum number of lights supported
            by the renderer, but the extraneous ones will never be used. In fact the limit will
            be imposed by Pass::getMaxSimultaneousLights.
        @param sn The SceneNode for which to evaluate the list of lights
        @param radius The bounding radius to test
        @param destList List to be populated with ordered set of lights; will be cleared by 
            this method before population.
		@param lightMask The mask with which to include / exclude lights
        */
        virtual void _populateLightList(const SceneNode* sn, Real radius, LightList& destList, uint32 lightMask = 0xFFFFFFFF);

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
        */
        virtual SceneNode* createSceneNode(void);

        /** Creates an instance of a SceneNode with a given name.
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
                Note that this method takes a name parameter, which makes the node easier to
                retrieve directly again later.
        */
        virtual SceneNode* createSceneNode(const String& name);

        /** Destroys a SceneNode with a given name.
        @remarks
            This allows you to physically delete an individual SceneNode if you want to.
            Note that this is not normally recommended, it's better to allow SceneManager
            to delete the nodes when the scene is cleared.
        */
        virtual void destroySceneNode(const String& name);

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
        */
        virtual SceneNode* getRootSceneNode(void);

        /** Retrieves a named SceneNode from the scene graph.
        @remarks
            If you chose to name a SceneNode as you created it, or if you
            happened to make a note of the generated name, you can look it
            up wherever it is in the scene graph using this method.
			@note Throws an exception if the named instance does not exist
        */
        virtual SceneNode* getSceneNode(const String& name) const;

		/** Returns whether a scene node with the given name exists.
		*/
		virtual bool hasSceneNode(const String& name) const;


        /** Create an Entity (instance of a discrete mesh).
            @param
                entityName The name to be given to the entity (must be unique).
            @param
                meshName The name of the Mesh it is to be based on (e.g. 'knot.oof'). The
                mesh will be loaded if it is not already.
        */
        virtual Entity* createEntity(const String& entityName, const String& meshName, const String& groupName = ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME );

        /** Create an Entity (instance of a discrete mesh) with an autogenerated name.
            @param
                meshName The name of the Mesh it is to be based on (e.g. 'knot.oof'). The
                mesh will be loaded if it is not already.
        */
        virtual Entity* createEntity(const String& meshName);
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

        /** Create an Entity (instance of a discrete mesh) from a range of prefab shapes.
            @param
                entityName The name to be given to the entity (must be unique).
            @param
                ptype The prefab type.
        */
        virtual Entity* createEntity(const String& entityName, PrefabType ptype);

        /** Create an Entity (instance of a discrete mesh) from a range of prefab shapes, generating the name.
            @param ptype The prefab type.
        */
        virtual Entity* createEntity(PrefabType ptype);
        /** Retrieves a pointer to the named Entity. 
		@note Throws an exception if the named instance does not exist
		*/
        virtual Entity* getEntity(const String& name) const;
		/** Returns whether an entity with the given name exists.
		*/
		virtual bool hasEntity(const String& name) const;

        /** Removes & destroys an Entity from the SceneManager.
            @warning
                Must only be done if the Entity is not attached
                to a SceneNode. It may be safer to wait to clear the whole
                scene if you are unsure use clearScene.
            @see
                SceneManager::clearScene
        */
        virtual void destroyEntity(Entity* ent);

        /** Removes & destroys an Entity from the SceneManager by name.
            @warning
                Must only be done if the Entity is not attached
                to a SceneNode. It may be safer to wait to clear the whole
                scene if you are unsure use clearScene.
            @see
                SceneManager::clearScene
        */
        virtual void destroyEntity(const String& name);

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

        /** Create a ManualObject, an object which you populate with geometry
			manually through a GL immediate-mode style interface.
        @param
            name The name to be given to the object (must be unique).
        */
        virtual ManualObject* createManualObject(const String& name);
		/** Create a ManualObject, an object which you populate with geometry
		manually through a GL immediate-mode style interface, generating the name.
		*/
		virtual ManualObject* createManualObject();
        /** Retrieves a pointer to the named ManualObject. 
		@note Throws an exception if the named instance does not exist
		*/
        virtual ManualObject* getManualObject(const String& name) const;
		/** Returns whether a manual object with the given name exists.
		*/
		virtual bool hasManualObject(const String& name) const;

        /** Removes & destroys a ManualObject from the SceneManager.
        */
        virtual void destroyManualObject(ManualObject* obj);
		/** Removes & destroys a ManualObject from the SceneManager.
		*/
		virtual void destroyManualObject(const String& name);
		/** Removes & destroys all ManualObjects from the SceneManager.
		*/
		virtual void destroyAllManualObjects(void);
        /** Create a BillboardChain, an object which you can use to render
            a linked chain of billboards.
        @param
            name The name to be given to the object (must be unique).
        */
        virtual BillboardChain* createBillboardChain(const String& name);
		/** Create a BillboardChain, an object which you can use to render
		a linked chain of billboards, with a generated name.
		*/
		virtual BillboardChain* createBillboardChain();
        /** Retrieves a pointer to the named BillboardChain. 
		@note Throws an exception if the named instance does not exist
		*/
        virtual BillboardChain* getBillboardChain(const String& name) const;
		/** Returns whether a billboard chain with the given name exists.
		*/
		virtual bool hasBillboardChain(const String& name) const;

        /** Removes & destroys a BillboardChain from the SceneManager.
        */
        virtual void destroyBillboardChain(BillboardChain* obj);
		/** Removes & destroys a BillboardChain from the SceneManager.
		*/
		virtual void destroyBillboardChain(const String& name);
		/** Removes & destroys all BillboardChains from the SceneManager.
		*/
		virtual void destroyAllBillboardChains(void);		
        /** Create a RibbonTrail, an object which you can use to render
            a linked chain of billboards which follows one or more nodes.
        @param
            name The name to be given to the object (must be unique).
        */
        virtual RibbonTrail* createRibbonTrail(const String& name);
		/** Create a RibbonTrail, an object which you can use to render
		a linked chain of billboards which follows one or more nodes, generating the name.
		*/
		virtual RibbonTrail* createRibbonTrail();
        /** Retrieves a pointer to the named RibbonTrail. 
		@note Throws an exception if the named instance does not exist
		*/
        virtual RibbonTrail* getRibbonTrail(const String& name) const;
		/** Returns whether a ribbon trail with the given name exists.
		*/
		virtual bool hasRibbonTrail(const String& name) const;

        /** Removes & destroys a RibbonTrail from the SceneManager.
        */
        virtual void destroyRibbonTrail(RibbonTrail* obj);
		/** Removes & destroys a RibbonTrail from the SceneManager.
		*/
		virtual void destroyRibbonTrail(const String& name);
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
            name The name to give the new particle system instance.
        @param 
            templateName The name of the template to base the new instance on.
        */
        virtual ParticleSystem* createParticleSystem(const String& name,
			const String& templateName);
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
            name The name to give the ParticleSystem.
        @param 
            quota The maximum number of particles to allow in this system. 
        @param
            resourceGroup The resource group which will be used to load dependent resources
        */
        virtual ParticleSystem* createParticleSystem(const String& name,
			size_t quota = 500, 
            const String& resourceGroup = ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

        /** Create a blank particle system with a generated name.
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
		virtual ParticleSystem* createParticleSystem(size_t quota = 500, 
			const String& resourceGroup = ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        /** Retrieves a pointer to the named ParticleSystem. 
		@note Throws an exception if the named instance does not exist
		*/
        virtual ParticleSystem* getParticleSystem(const String& name) const;
		/** Returns whether a particle system with the given name exists.
		*/
		virtual bool hasParticleSystem(const String& name) const;

        /** Removes & destroys a ParticleSystem from the SceneManager.
        */
        virtual void destroyParticleSystem(ParticleSystem* obj);
		/** Removes & destroys a ParticleSystem from the SceneManager.
		*/
		virtual void destroyParticleSystem(const String& name);
		/** Removes & destroys all ParticleSystems from the SceneManager.
		*/
		virtual void destroyAllParticleSystems(void);		

		/** Empties the entire scene, inluding all SceneNodes, Entities, Lights, 
            BillboardSets etc. Cameras are not deleted at this stage since
            they are still referenced by viewports, which are not destroyed during
            this process.
        */
        virtual void clearScene(void);

        /** Sets the ambient light level to be used for the scene.
            @remarks
                This sets the colour and intensity of the ambient light in the scene, i.e. the
                light which is 'sourceless' and illuminates all objects equally.
                The colour of an object is affected by a combination of the light in the scene,
                and the amount of light that object reflects (in this case based on the Material::ambient
                property).
            @remarks
                By default the ambient light in the scene is ColourValue::Black, i.e. no ambient light. This
                means that any objects rendered with a Material which has lighting enabled (see Material::setLightingEnabled)
                will not be visible unless you have some dynamic lights in your scene.
        */
        void setAmbientLight(const ColourValue& colour);

        /** Returns the ambient light level to be used for the scene.
        */
        const ColourValue& getAmbientLight(void) const;

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
			const String& typeName = StringUtil::BLANK);

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
			const String& typeName = StringUtil::BLANK);

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
			const String& typeName = StringUtil::BLANK)
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
                On failiure, false is returned.
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
                On failiure, false is returned.
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
                On failiure, false is returned and pDestValue is set to NULL.
        */
        virtual bool getOption( const String& strKey, void* pDestValue )
        { (void)strKey; (void)pDestValue; return false; }

        /** Method for verifying wether the scene manager has an implementation-specific
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
                On success, true is returned. On failiure, false is returned.
        */
        virtual bool getOptionKeys( StringVector& refKeys )
        { (void)refKeys; return false; }

        /** Internal method for updating the scene graph ie the tree of SceneNode instances managed by this class.
            @remarks
                This must be done before issuing objects to the rendering pipeline, since derived transformations from
                parent nodes are not updated until required. This SceneManager is a basic implementation which simply
                updates all nodes from the root. This ensures the scene is up to date but requires all the nodes
                to be updated even if they are not visible. Subclasses could trim this such that only potentially visible
                nodes are updated.
        */
        virtual void _updateSceneGraph(Camera* cam);

        /** Internal method which parses the scene to find visible objects to render.
            @remarks
                If you're implementing a custom scene manager, this is the most important method to
                override since it's here you can apply your custom world partitioning scheme. Once you
                have added the appropriate objects to the render queue, you can let the default
                SceneManager objects _renderVisibleObjects handle the actual rendering of the objects
                you pick.
            @par
                Any visible objects will be added to a rendering queue, which is indexed by material in order
                to ensure objects with the same material are rendered together to minimise render state changes.
        */
        virtual void _findVisibleObjects(Camera* cam, VisibleObjectsBoundsInfo* visibleBounds, bool onlyShadowCasters);

        /** Internal method for applying animations to scene nodes.
        @remarks
            Uses the internally stored AnimationState objects to apply animation to SceneNodes.
        */
        virtual void _applySceneAnimations(void);

        /** Sends visible objects found in _findVisibleObjects to the rendering engine.
        */
        virtual void _renderVisibleObjects(void);

        /** Prompts the class to send its contents to the renderer.
            @remarks
                This method prompts the scene manager to send the
                contents of the scene it manages to the rendering
                pipeline, possibly preceded by some sorting, culling
                or other scene management tasks. Note that this method is not normally called
                directly by the user application; it is called automatically
                by the Ogre rendering loop.
            @param camera Pointer to a camera from whose viewpoint the scene is to
                be rendered.
            @param vp The target viewport
            @param includeOverlays Whether or not overlay objects should be rendered
        */
        virtual void _renderScene(Camera* camera, Viewport* vp, bool includeOverlays);

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
				simular to skydomes, but are more compatable with fog.
            @param xsegments, ysegments
                Determines the number of segments the plane will have to it. This
                is most important when you are bowing the plane, but may also be useful
                if you need tesselation on the plane to perform per-vertex effects.
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
				simular to skydomes, but are more compatable with fog.
            @param xsegments, ysegments
                Determines the number of segments the plane will have to it. This
                is most important when you are bowing the plane, but may also be useful
                if you need tesselation on the plane to perform per-vertex effects.
            @param groupName
                The name of the resource group to which to assign the plane mesh.
        */        
        virtual void _setSkyPlane(
            bool enable,
            const Plane& plane, const String& materialName, Real scale = 1000,
            Real tiling = 10, uint8 renderQueue = RENDER_QUEUE_SKIES_EARLY, Real bow = 0, 
            int xsegments = 1, int ysegments = 1, 
            const String& groupName = ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

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
            bool enable, const String& materialName, Real distance = 5000,
            uint8 renderQueue = RENDER_QUEUE_SKIES_EARLY, const Quaternion& orientation = Quaternion::IDENTITY,
            const String& groupName = ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

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
                can change the apparant 'curvature' of the sky depending on how
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
                can change the apparant 'curvature' of the sky depending on how
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
            bool enable, const String& materialName, Real curvature = 10,
            Real tiling = 8, Real distance = 4000, uint8 renderQueue = RENDER_QUEUE_SKIES_EARLY,
            const Quaternion& orientation = Quaternion::IDENTITY,
            int xsegments = 16, int ysegments = 16, int ysegments_keep = -1,
            const String& groupName = ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

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
                name The name to give to this billboard set. Must be unique.
            @param
                poolSize The initial size of the pool of billboards (see BillboardSet for more information)
            @see
                BillboardSet
        */
        virtual BillboardSet* createBillboardSet(const String& name, unsigned int poolSize = 20);

        /** Creates a new BillboardSet for use with this scene manager, with a generated name.
            @param
                poolSize The initial size of the pool of billboards (see BillboardSet for more information)
            @see
                BillboardSet
        */
        virtual BillboardSet* createBillboardSet(unsigned int poolSize = 20);
        /** Retrieves a pointer to the named BillboardSet.
		@note Throws an exception if the named instance does not exist
        */
        virtual BillboardSet* getBillboardSet(const String& name) const;
		/** Returns whether a billboardset with the given name exists.
		*/
		virtual bool hasBillboardSet(const String& name) const;

        /** Removes & destroys an BillboardSet from the SceneManager.
            @warning
                Must only be done if the BillboardSet is not attached
                to a SceneNode. It may be safer to wait to clear the whole
                scene. If you are unsure, use clearScene.
        */
        virtual void destroyBillboardSet(BillboardSet* set);

        /** Removes & destroys an BillboardSet from the SceneManager by name.
            @warning
                Must only be done if the BillboardSet is not attached
                to a SceneNode. It may be safer to wait to clear the whole
                scene. If you are unsure, use clearScene.
        */
        virtual void destroyBillboardSet(const String& name);

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
        virtual Animation* createAnimation(const String& name, Real length);

        /** Looks up an Animation object previously created with createAnimation. 
		@note Throws an exception if the named instance does not exist
		*/
        virtual Animation* getAnimation(const String& name) const;
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
            is provided. </p>
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
        virtual AnimationState* createAnimationState(const String& animName);

        /** Retrieves animation state as previously created using createAnimationState. 
		@note Throws an exception if the named instance does not exist
		*/
        virtual AnimationState* getAnimationState(const String& animName) const;
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

        /** Manual rendering method, for advanced users only.
        @remarks
            This method allows you to send rendering commands through the pipeline on
            demand, bypassing OGRE's normal world processing. You should only use this if you
            really know what you're doing; OGRE does lots of things for you that you really should
            let it do. However, there are times where it may be useful to have this manual interface,
            for example overlaying something on top of the scene rendered by OGRE.
        @par
            Because this is an instant rendering method, timing is important. The best 
            time to call it is from a RenderTargetListener event handler.
        @par
            Don't call this method a lot, it's designed for rare (1 or 2 times per frame) use. 
            Calling it regularly per frame will cause frame rate drops!
        @param rend A RenderOperation object describing the rendering op
        @param pass The Pass to use for this render
        @param vp Pointer to the viewport to render to, or 0 to use the current viewport
        @param worldMatrix The transform to apply from object to world space
        @param viewMatrix The transform to apply from world to view space
        @param projMatrix The transform to apply from view to screen space
        @param doBeginEndFrame If true, beginFrame() and endFrame() are called, 
            otherwise not. You should leave this as false if you are calling
            this within the main render loop.
        */
        virtual void manualRender(RenderOperation* rend, Pass* pass, Viewport* vp, 
            const Matrix4& worldMatrix, const Matrix4& viewMatrix, const Matrix4& projMatrix, 
            bool doBeginEndFrame = false) ;

		/** Manual rendering method for rendering a single object. 
		@remarks
		@param rend The renderable to issue to the pipeline
		@param pass The pass to use
		@param vp Pointer to the viewport to render to, or 0 to use the existing viewport
		@param doBeginEndFrame If true, beginFrame() and endFrame() are called, 
		otherwise not. You should leave this as false if you are calling
		this within the main render loop.
        @param viewMatrix The transform to apply from world to view space
        @param projMatrix The transform to apply from view to screen space
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
		virtual void manualRender(Renderable* rend, const Pass* pass, Viewport* vp, 
			const Matrix4& viewMatrix, const Matrix4& projMatrix, bool doBeginEndFrame = false, bool lightScissoringClipping = true, 
			bool doLightIteration = true, const LightList* manualLightList = 0);

		/** Retrieves the internal render queue, for advanced users only.
        @remarks
            The render queue is mainly used internally to manage the scene object 
			rendering queue, it also exports some methods to allow advanced users 
			to configure the behavior of rendering process.
            Most methods provided by RenderQueue are supposed to be used 
			internally only, you should reference to the RenderQueue API for 
			more information. Do not access this directly unless you know what 
			you are doing.
        */
        virtual RenderQueue* getRenderQueue(void);

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

		/** Adds an item to the 'special case' render queue list.
		@remarks
			Normally all render queues are rendered, in their usual sequence, 
			only varying if a RenderQueueListener nominates for the queue to be 
			repeated or skipped. This method allows you to add a render queue to 
			a 'special case' list, which varies the behaviour. The effect of this
			list depends on the 'mode' in which this list is in, which might be
			to exclude these render queues, or to include them alone (excluding
			all other queues). This allows you to perform broad selective
			rendering without requiring a RenderQueueListener.
		@param qid The identifier of the queue which should be added to the
			special case list. Nothing happens if the queue is already in the list.
		*/
		virtual void addSpecialCaseRenderQueue(uint8 qid);
		/** Removes an item to the 'special case' render queue list.
		@see SceneManager::addSpecialCaseRenderQueue
		@param qid The identifier of the queue which should be removed from the
			special case list. Nothing happens if the queue is not in the list.
		*/
		virtual void removeSpecialCaseRenderQueue(uint8 qid);
		/** Clears the 'special case' render queue list.
		@see SceneManager::addSpecialCaseRenderQueue
		*/
		virtual void clearSpecialCaseRenderQueues(void);
		/** Sets the way the special case render queue list is processed.
		@see SceneManager::addSpecialCaseRenderQueue
		@param mode The mode of processing
		*/
		virtual void setSpecialCaseRenderQueueMode(SpecialCaseRenderQueueMode mode);
		/** Gets the way the special case render queue list is processed. */
		virtual SpecialCaseRenderQueueMode getSpecialCaseRenderQueueMode(void);
		/** Returns whether or not the named queue will be rendered based on the
			current 'special case' render queue list and mode.
		@see SceneManager::addSpecialCaseRenderQueue
		@param qid The identifier of the queue which should be tested
		@returns true if the queue will be rendered, false otherwise
		*/
		virtual bool isRenderQueueToBeProcessed(uint8 qid);

		/** Sets the render queue that the world geometry (if any) this SceneManager
			renders will be associated with.
		@remarks
			SceneManagers which provide 'world geometry' should place it in a 
			specialised render queue in order to make it possible to enable / 
			disable it easily using the addSpecialCaseRenderQueue method. Even 
			if the SceneManager does not use the render queues to render the 
			world geometry, it should still pick a queue to represent it's manual
			rendering, and check isRenderQueueToBeProcessed before rendering.
		@note
			Setting this may not affect the actual ordering of rendering the
			world geometry, if the world geometry is being rendered manually
			by the SceneManager. If the SceneManager feeds world geometry into
			the queues, however, the ordering will be affected. 
		*/
		virtual void setWorldGeometryRenderQueue(uint8 qid);
		/** Gets the render queue that the world geometry (if any) this SceneManager
			renders will be associated with.
		@remarks
			SceneManagers which provide 'world geometry' should place it in a 
			specialised render queue in order to make it possible to enable / 
			disable it easily using the addSpecialCaseRenderQueue method. Even 
			if the SceneManager does not use the render queues to render the 
			world geometry, it should still pick a queue to represent it's manual
			rendering, and check isRenderQueueToBeProcessed before rendering.
		*/
		virtual uint8 getWorldGeometryRenderQueue(void);

		/** Allows all bounding boxes of scene nodes to be displayed. */
		virtual void showBoundingBoxes(bool bShow);

		/** Returns if all bounding boxes of scene nodes are to be displayed */
		virtual bool getShowBoundingBoxes() const;

        /** Internal method for notifying the manager that a SceneNode is autotracking. */
        virtual void _notifyAutotrackingSceneNode(SceneNode* node, bool autoTrack);

        
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
            createAABBQuery(const AxisAlignedBox& box, unsigned long mask = 0xFFFFFFFF);
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
            createSphereQuery(const Sphere& sphere, unsigned long mask = 0xFFFFFFFF);
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
        virtual PlaneBoundedVolumeListSceneQuery* 
            createPlaneBoundedVolumeQuery(const PlaneBoundedVolumeList& volumes, unsigned long mask = 0xFFFFFFFF);


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
        virtual RaySceneQuery* 
            createRayQuery(const Ray& ray, unsigned long mask = 0xFFFFFFFF);
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
        virtual IntersectionSceneQuery* 
            createIntersectionQuery(unsigned long mask = 0xFFFFFFFF);

        /** Destroys a scene query of any type. */
        virtual void destroyQuery(SceneQuery* query);

        typedef MapIterator<CameraList> CameraIterator;
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
        AnimationStateIterator getAnimationStateIterator(void) {
            return mAnimationStates.getAnimationStateIterator();
        }

        /** Sets the general shadow technique to be used in this scene.
        @remarks   
            There are multiple ways to generate shadows in a scene, and each has 
            strengths and weaknesses. 
            <ul><li>Stencil-based approaches can be used to 
            draw very long, extreme shadows without loss of precision and the 'additive'
            version can correctly show the shadowing of complex effects like bump mapping
            because they physically exclude the light from those areas. However, the edges
            are very sharp and stencils cannot handle transparency, and they involve a 
            fair amount of CPU work in order to calculate the shadow volumes, especially
            when animated objects are involved.</li>
            <li>Texture-based approaches are good for handling transparency (they can, for
            example, correctly shadow a mesh which uses alpha to represent holes), and they
            require little CPU overhead, and can happily shadow geometry which is deformed
            by a vertex program, unlike stencil shadows. However, they have a fixed precision 
            which can introduce 'jaggies' at long range and have fillrate issues of their own.</li>
            </ul>
        @par
            We support 2 kinds of stencil shadows, and 2 kinds of texture-based shadows, and one
            simple decal approach. The 2 stencil approaches differ in the amount of multipass work 
            that is required - the modulative approach simply 'darkens' areas in shadow after the 
            main render, which is the least expensive, whilst the additive approach has to perform 
            a render per light and adds the cumulative effect, whcih is more expensive but more 
            accurate. The texture based shadows both work in roughly the same way, the only difference is
            that the shadowmap approach is slightly more accurate, but requires a more recent
            graphics card.
        @par
            Note that because mixing many shadow techniques can cause problems, only one technique
            is supported at once. Also, you should call this method at the start of the 
            scene setup. 
        @param technique The shadowing technique to use for the scene.
        */
        virtual void setShadowTechnique(ShadowTechnique technique);
        
        /** Gets the current shadow technique. */
        virtual ShadowTechnique getShadowTechnique(void) const { return mShadowTechnique; }

        /** Enables / disables the rendering of debug information for shadows. */
        virtual void setShowDebugShadows(bool debug) { mDebugShadows = debug; }
        /** Are debug shadows shown? */
        virtual bool getShowDebugShadows(void ) const { return mDebugShadows; }

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

		/** Sets the maximum size of the index buffer used to render shadow
		 	primitives.
		@remarks
			This method allows you to tweak the size of the index buffer used
			to render shadow primitives (including stencil shadow volumes). The
			default size is 51,200 entries, which is 100k of GPU memory, or
			enough to render approximately 17,000 triangles. You can reduce this
			as long as you do not have any models / world geometry chunks which 
			could require more than the amount you set.
		@par
			The maximum number of triangles required to render a single shadow 
			volume (including light and dark caps when needed) will be 3x the 
			number of edges on the light silhouette, plus the number of 
			light-facing triangles.	On average, half the 
			triangles will be facing toward the light, but the number of 
			triangles in the silhouette entirely depends on the mesh - 
			angular meshes will have a higher silhouette tris/mesh tris
			ratio than a smooth mesh. You can estimate the requirements for
			your particular mesh by rendering it alone in a scene with shadows
			enabled and a single light - rotate it or the light and make a note
			of how high the triangle count goes (remembering to subtract the 
			mesh triangle count)
		@param size The number of indexes; divide this by 3 to determine the
			number of triangles.
		*/
		virtual void setShadowIndexBufferSize(size_t size);
        /// Get the size of the shadow index buffer
		virtual size_t getShadowIndexBufferSize(void) const
		{ return mShadowIndexBufferSize; }
        /** Set the size of the texture used for all texture-based shadows.
        @remarks
            The larger the shadow texture, the better the detail on 
            texture based shadows, but obviously this takes more memory.
            The default size is 512. Sizes must be a power of 2.
		@note This is the simple form, see setShadowTextureConfig for the more 
			complex form.
        */
        virtual void setShadowTextureSize(unsigned short size);

		/** Set the detailed configuration for a shadow texture.
		@param shadowIndex The index of the texture to configure, must be < the
			number of shadow textures setting
		@param width, height The dimensions of the texture
		@param format The pixel format of the texture
		*/
		virtual void setShadowTextureConfig(size_t shadowIndex, unsigned short width, 
			unsigned short height, PixelFormat format);
		/** Set the detailed configuration for a shadow texture.
		@param shadowIndex The index of the texture to configure, must be < the
			number of shadow textures setting
		@param config Configuration structure
		*/
		virtual void setShadowTextureConfig(size_t shadowIndex, 
			const ShadowTextureConfig& config);

		/** Get an iterator over the current shadow texture settings. */
		ConstShadowTextureConfigIterator getShadowTextureConfigIterator() const;

        /** Set the pixel format of the textures used for texture-based shadows.
        @remarks
			By default, a colour texture is used (PF_X8R8G8B8) for texture shadows,
			but if you want to use more advanced texture shadow types you can 
			alter this. If you do, you will have to also call
			setShadowTextureCasterMaterial and setShadowTextureReceiverMaterial
			to provide shader-based materials to use these customised shadow
			texture formats.
		@note This is the simple form, see setShadowTextureConfig for the more 
			complex form.
        */
        virtual void setShadowTexturePixelFormat(PixelFormat fmt);
        /** Set the number of textures allocated for texture-based shadows.
        @remarks
            The default number of textures assigned to deal with texture based
            shadows is 1; however this means you can only have one light casting
            shadows at the same time. You can increase this number in order to 
            make this more flexible, but be aware of the texture memory it will use.
        */
        virtual void setShadowTextureCount(size_t count);
        /// Get the number of the textures allocated for texture based shadows
        size_t getShadowTextureCount(void) const {return mShadowTextureConfigList.size(); }

		/** Set the number of shadow textures a light type uses.
		@remarks
			The default for all light types is 1. This means that each light uses only 1 shadow
			texture. Call this if you need more than 1 shadow texture per light, E.G. PSSM. 
		@note
			This feature only works with the Integrated shadow technique.
			Also remember to increase the total number of shadow textures you request
			appropriately (e.g. via setShadowTextureCount)!!
		*/
		void setShadowTextureCountPerLightType(Light::LightTypes type, size_t count)
		{ mShadowTextureCountPerType[type] = count; }
		/// Get the number of shadow textures is assigned for the given light type.
		size_t getShadowTextureCountPerLightType(Light::LightTypes type) const
		{return mShadowTextureCountPerType[type]; }

        /** Sets the size and count of textures used in texture-based shadows. 
        @remarks
            @see setShadowTextureSize and setShadowTextureCount for details, this
            method just allows you to change both at once, which can save on 
            reallocation if the textures have already been created.
		@note This is the simple form, see setShadowTextureConfig for the more 
			complex form.
        */
        virtual void setShadowTextureSettings(unsigned short size, unsigned short count, 
			PixelFormat fmt = PF_X8R8G8B8);

		/** Get a reference to the shadow texture currently in use at the given index.
		@note
			If you change shadow settings, this reference may no longer
			be correct, so be sure not to hold the returned reference over 
			texture shadow configuration changes.
		*/
		virtual const TexturePtr& getShadowTexture(size_t shadowIndex);

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

		/** Sets whether or not texture shadows should attempt to self-shadow.
		@remarks
			The default implementation of texture shadows uses a fixed-function 
			colour texture projection approach for maximum compatibility, and 
			as such cannot support self-shadowing. However, if you decide to 
			implement a more complex shadowing technique using the 
			setShadowTextureCasterMaterial and setShadowTextureReceiverMaterial 
			there is a possibility you may be able to support 
			self-shadowing (e.g by implementing a shader-based shadow map). In 
			this case you might want to enable this option.
		@param selfShadow Whether to attempt self-shadowing with texture shadows
		*/
		virtual void setShadowTextureSelfShadow(bool selfShadow); 

		/// Gets whether or not texture shadows attempt to self-shadow.
		virtual bool getShadowTextureSelfShadow(void) const 
		{ return mShadowTextureSelfShadow; }
		/** Sets the default material to use for rendering shadow casters.
		@remarks
			By default shadow casters are rendered into the shadow texture using
			an automatically generated fixed-function pass. This allows basic
			projective texture shadows, but it's possible to use more advanced
			shadow techniques by overriding the caster and receiver materials, for
			example providing vertex and fragment programs to implement shadow
			maps.
		@par
			You can rely on the ambient light in the scene being set to the 
			requested texture shadow colour, if that's useful. 
		@note
			Individual objects may also override the vertex program in
			your default material if their materials include 
			shadow_caster_vertex_program_ref, shadow_receiver_vertex_program_ref
			shadow_caster_material entries, so if you use both make sure they are compatible.			
		@note
			Only a single pass is allowed in your material, although multiple
			techniques may be used for hardware fallback.
		*/
		virtual void setShadowTextureCasterMaterial(const String& name);
		/** Sets the default material to use for rendering shadow receivers.
		@remarks
			By default shadow receivers are rendered as a post-pass using basic
			modulation. This allows basic projective texture shadows, but it's 
			possible to use more advanced shadow techniques by overriding the 
			caster and receiver materials, for example providing vertex and 
			fragment programs to implement shadow maps.
		@par
			You can rely on texture unit 0 containing the shadow texture, and 
			for the unit to be set to use projective texturing from the light 
			(only useful if you're using fixed-function, which is unlikely; 
			otherwise you should rely on the texture_viewproj_matrix auto binding)
		@note
			Individual objects may also override the vertex program in
			your default material if their materials include 
			shadow_caster_vertex_program_ref shadow_receiver_vertex_program_ref
			shadow_receiver_material entries, so if you use both make sure they are compatible.
		@note
			Only a single pass is allowed in your material, although multiple
			techniques may be used for hardware fallback.
		*/
		virtual void setShadowTextureReceiverMaterial(const String& name);

		/** Sets whether or not shadow casters should be rendered into shadow
			textures using their back faces rather than their front faces. 
		@remarks
			Rendering back faces rather than front faces into a shadow texture
			can help minimise depth comparison issues, if you're using depth
			shadowmapping. You will probably still need some biasing but you
			won't need as much. For solid objects the result is the same anyway,
			if you have objects with holes you may want to turn this option off.
			The default is to enable this option.
		*/
		virtual void setShadowCasterRenderBackFaces(bool bf) { mShadowCasterRenderBackFaces = bf; }

		/** Gets whether or not shadow casters should be rendered into shadow
			textures using their back faces rather than their front faces. 
		*/
		virtual bool getShadowCasterRenderBackFaces() const { return mShadowCasterRenderBackFaces; }

		/** Set the shadow camera setup to use for all lights which don't have
			their own shadow camera setup.
		@see ShadowCameraSetup
		*/
		virtual void setShadowCameraSetup(const ShadowCameraSetupPtr& shadowSetup);

		/** Get the shadow camera setup in use for all lights which don't have
			their own shadow camera setup.
		@see ShadowCameraSetup
		*/
		virtual const ShadowCameraSetupPtr& getShadowCameraSetup() const;

		/** Sets whether we should use an inifinite camera far plane
			when rendering stencil shadows.
		@remarks
			Stencil shadow coherency is very reliant on the shadow volume
			not being clipped by the far plane. If this clipping happens, you
			get a kind of 'negative' shadow effect. The best way to achieve
			coherency is to move the far plane of the camera out to infinity,
			thus preventing the far plane from clipping the shadow volumes.
			When combined with vertex program extrusion of the volume to 
			infinity, which	Ogre does when available, this results in very
			robust shadow volumes. For this reason, when you enable stencil 
			shadows, Ogre automatically changes your camera settings to 
			project to infinity if the card supports it. You can disable this
			behaviour if you like by calling this method; although you can 
			never enable infinite projection if the card does not support it.
		@par	
			If you disable infinite projection, or it is not available, 
			you need to be far more careful with your light attenuation /
			directional light extrusion distances to avoid clipping artefacts
			at the far plane.
		@note
			Recent cards will generally support infinite far plane projection.
			However, we have found some cases where they do not, especially
			on Direct3D. There is no standard capability we can check to 
			validate this, so we use some heuristics based on experience:
			<UL>
			<LI>OpenGL always seems to support it no matter what the card</LI>
			<LI>Direct3D on non-vertex program capable systems (including 
			vertex program capable cards on Direct3D7) does not
			support it</LI>
			<LI>Direct3D on GeForce3 and GeForce4 Ti does not seem to support
			infinite projection<LI>
			</UL>
			Therefore in the RenderSystem implementation, we may veto the use
			of an infinite far plane based on these heuristics. 
		*/
        virtual void setShadowUseInfiniteFarPlane(bool enable) {
            mShadowUseInfiniteFarPlane = enable; }

		/** Is there a stencil shadow based shadowing technique in use? */
		virtual bool isShadowTechniqueStencilBased(void) const 
		{ return (mShadowTechnique & SHADOWDETAILTYPE_STENCIL) != 0; }
		/** Is there a texture shadow based shadowing technique in use? */
		virtual bool isShadowTechniqueTextureBased(void) const 
		{ return (mShadowTechnique & SHADOWDETAILTYPE_TEXTURE) != 0; }
		/** Is there a modulative shadowing technique in use? */
		virtual bool isShadowTechniqueModulative(void) const 
		{ return (mShadowTechnique & SHADOWDETAILTYPE_MODULATIVE) != 0; }
		/** Is there an additive shadowing technique in use? */
		virtual bool isShadowTechniqueAdditive(void) const 
		{ return (mShadowTechnique & SHADOWDETAILTYPE_ADDITIVE) != 0; }
		/** Is the shadow technique integrated into primary materials? */
		virtual bool isShadowTechniqueIntegrated(void) const 
		{ return (mShadowTechnique & SHADOWDETAILTYPE_INTEGRATED) != 0; }
		/** Is there any shadowing technique in use? */
		virtual bool isShadowTechniqueInUse(void) const 
		{ return mShadowTechnique != SHADOWTYPE_NONE; }
		/** Sets whether when using a built-in additive shadow mode, user clip
			planes should be used to restrict light rendering.
		*/
		virtual void setShadowUseLightClipPlanes(bool enabled) { mShadowAdditiveLightClip = enabled; }
		/** Gets whether when using a built-in additive shadow mode, user clip
		planes should be used to restrict light rendering.
		*/
		virtual bool getShadowUseLightClipPlanes() const { return mShadowAdditiveLightClip; }

		/** Sets the active compositor chain of the current scene being rendered.
			@note CompositorChain does this automatically, no need to call manually.
		*/
		virtual void _setActiveCompositorChain(CompositorChain* chain) { mActiveCompositorChain = chain; }

		/** Sets whether to use late material resolving or not. If set, materials will be resolved
			from the materials at the pass-setting stage and not at the render queue building stage.
			This is useful when the active material scheme during the render queue building stage
			is different from the one during the rendering stage.
		*/
		virtual void setLateMaterialResolving(bool isLate) { mLateMaterialResolving = isLate; }
		
		/** Gets whether using late material resolving or not.
			@see setLateMaterialResolving */
		virtual bool isLateMaterialResolving() const { return mLateMaterialResolving; }

		/** Gets the active compositor chain of the current scene being rendered */
		virtual CompositorChain* _getActiveCompositorChain() const { return mActiveCompositorChain; }

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
		@returns The new StaticGeometry instance
		*/
		virtual StaticGeometry* createStaticGeometry(const String& name);
		/** Retrieve a previously created StaticGeometry instance. 
		@note Throws an exception if the named instance does not exist
		*/
		virtual StaticGeometry* getStaticGeometry(const String& name) const;
		/** Returns whether a static geometry instance with the given name exists. */
		virtual bool hasStaticGeometry(const String& name) const;
		/** Remove & destroy a StaticGeometry instance. */
		virtual void destroyStaticGeometry(StaticGeometry* geom);
		/** Remove & destroy a StaticGeometry instance. */
		virtual void destroyStaticGeometry(const String& name);
		/** Remove & destroy all StaticGeometry instances. */
		virtual void destroyAllStaticGeometry(void);

		/** Creates a InstancedGeometry instance suitable for use with this
			SceneManager.
		@remarks
			InstancedGeometry is a way of batching up geometry into a more 
			efficient form, and still be able to move it. Please 
			read the InstancedGeometry class documentation for full information.
		@param name The name to give the new object
		@returns The new InstancedGeometry instance
		*/
		virtual InstancedGeometry* createInstancedGeometry(const String& name);
		/** Retrieve a previously created InstancedGeometry instance. */
		virtual InstancedGeometry* getInstancedGeometry(const String& name) const;
		/** Remove & destroy a InstancedGeometry instance. */
		virtual void destroyInstancedGeometry(InstancedGeometry* geom);
		/** Remove & destroy a InstancedGeometry instance. */
		virtual void destroyInstancedGeometry(const String& name);
		/** Remove & destroy all InstancedGeometry instances. */
		virtual void destroyAllInstancedGeometry(void);


		/** Create a movable object of the type specified.
		@remarks
			This is the generalised form of MovableObject creation where you can
			create a MovableObject of any specialised type generically, including
			any new types registered using plugins.
		@param name The name to give the object. Must be unique within type.
		@param typeName The type of object to create
		@param params Optional name/value pair list to give extra parameters to
			the created object.
		*/
		virtual MovableObject* createMovableObject(const String& name, 
			const String& typeName, const NameValuePairList* params = 0);
		/** Create a movable object of the type specified without a name.
		@remarks
		This is the generalised form of MovableObject creation where you can
		create a MovableObject of any specialised type generically, including
		any new types registered using plugins. The name is generated automatically.
		@param typeName The type of object to create
		@param params Optional name/value pair list to give extra parameters to
		the created object.
		*/
		virtual MovableObject* createMovableObject(const String& typeName, const NameValuePairList* params = 0);
		/** Destroys a MovableObject with the name specified, of the type specified.
		@remarks
			The MovableObject will automatically detach itself from any nodes
			on destruction.
		*/
		virtual void destroyMovableObject(const String& name, const String& typeName);
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
		/** Get a reference to a previously created MovableObject. 
		@note Throws an exception if the named instance does not exist
		*/
		virtual MovableObject* getMovableObject(const String& name, const String& typeName) const;
		/** Returns whether a movable object instance with the given name exists. */
		virtual bool hasMovableObject(const String& name, const String& typeName) const;
		typedef MapIterator<MovableObjectMap> MovableObjectIterator;
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
		virtual void extractMovableObject(const String& name, const String& typeName);
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
		virtual void setVisibilityMask(uint32 vmask) { mVisibilityMask = vmask; }

		/** Gets a mask which is bitwise 'and'ed with objects own visibility masks
			to determine if the object is visible.
		*/
		virtual uint32 getVisibilityMask(void) { return mVisibilityMask; }

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

		/** Set whether to automatically normalise normals on objects whenever they
			are scaled.
		@remarks
			Scaling can distort normals so the default behaviour is to compensate
			for this, but it has a cost. If you would prefer to manually manage 
			this, set this option to 'false' and use Pass::setNormaliseNormals
			only when needed.
		*/
		virtual void setNormaliseNormalsOnScale(bool n) { mNormaliseNormalsOnScale = n; }

		/** Get whether to automatically normalise normals on objects whenever they
			are scaled.
		*/
		virtual bool getNormaliseNormalsOnScale() const { return mNormaliseNormalsOnScale; }

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

		/** Render something as if it came from the current queue.
			@param pass		Material pass to use for setting up this quad.
			@param rend		Renderable to render
			@param shadowDerivation Whether passes should be replaced with shadow caster / receiver passes
		 */
		virtual void _injectRenderWithPass(Pass *pass, Renderable *rend, bool shadowDerivation = true,
			bool doLightIteration = false, const LightList* manualLightList = 0);

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

        /** Internal method for setting up the renderstate for a rendering pass.
            @param pass The Pass details to set.
			@param evenIfSuppressed Sets the pass details even if render state
				changes are suppressed; if you are using this to manually set state
				when render state changes are suppressed, you should set this to 
				true.
			@param shadowDerivation If false, disables the derivation of shadow
				passes from original passes
            @returns
                A Pass object that was used instead of the one passed in, can
                happen when rendering shadow passes
        */
        virtual const Pass* _setPass(const Pass* pass, 
			bool evenIfSuppressed = false, bool shadowDerivation = true);


		/** Indicates to the SceneManager whether it should suppress the 
			active shadow rendering technique until told otherwise.
		@remarks
			This is a temporary alternative to setShadowTechnique to suppress
			the rendering of shadows and forcing all processing down the 
			standard rendering path. This is intended for internal use only.
		@param suppress If true, no shadow rendering will occur until this
			method is called again with a parameter of false.
		*/
		virtual void _suppressShadows(bool suppress); 

		/** Are shadows suppressed? 
		@see _suppressShadows
		*/
		virtual bool _areShadowsSuppressed(void) const
		{ return mSuppressShadows; }

		/** Render the objects in a given queue group 
		@remarks You should only call this from a RenderQueueInvocation implementation
		*/
		virtual void _renderQueueGroupObjects(RenderQueueGroup* group, 
			QueuedRenderableCollection::OrganisationMode om);

		/** Advanced method for supplying an alternative visitor, used for parsing the
			render queues and sending the results to the renderer.
		@remarks
			You can use this method to insert your own implementation of the 
			QueuedRenderableVisitor interface, which receives calls as the queued
			renderables are parsed in a given order (determined by RenderQueueInvocationSequence)
			and are sent to the renderer. If you provide your own implementation of
			this visitor, you are responsible for either calling the rendersystem, 
			or passing the calls on to the base class implementation.
		@note
			Ownership is not taken of this pointer, you are still required to 
			delete it yourself once you're finished.
		@param visitor Your implementation of SceneMgrQueuedRenderableVisitor. 
			If you pass 0, the default implementation will be used.
		*/
		void setQueuedRenderableVisitor(SceneMgrQueuedRenderableVisitor* visitor);

		/** Gets the current visitor object which processes queued renderables. */
		SceneMgrQueuedRenderableVisitor* getQueuedRenderableVisitor(void) const;


		/** Get the rendersystem subclass to which the output of this Scene Manager
			gets sent
		*/
		RenderSystem *getDestinationRenderSystem();

		/** Gets the current viewport being rendered (advanced use only, only 
			valid during viewport update. */
		Viewport* getCurrentViewport(void) const { return mCurrentViewport; }

		/** Returns a visibility boundary box for a specific camera. */
		const VisibleObjectsBoundsInfo& getVisibleObjectsBoundsInfo(const Camera* cam) const;

		/**  Returns the shadow caster AAB for a specific light-camera combination */
		const VisibleObjectsBoundsInfo& getShadowCasterBoundsInfo(const Light* light, size_t iteration = 0) const;

		/** Set whether to use camera-relative co-ordinates when rendering, ie
			to always place the camera at the origin and move the world around it.
		@remarks
			This is a technique to alleviate some of the precision issues associated with 
			rendering far from the origin, where single-precision floats as used in most
			GPUs begin to lose their precision. Instead of including the camera
			translation in the view matrix, it only includes the rotation, and
			the world matrices of objects must be expressed relative to this.
		@note
			If you need this option, you will probably also need to enable double-precision
			mode in Ogre (OGRE_DOUBLE_PRECISION), since even though this will 
			alleviate the rendering precision, the source camera and object positions will still 
			suffer from precision issues leading to jerky movement. 
		*/
		virtual void setCameraRelativeRendering(bool rel) { mCameraRelativeRendering = rel; }

		/** Get whether to use camera-relative co-ordinates when rendering, ie
			to always place the camera at the origin and move the world around it.
		*/
		virtual bool getCameraRelativeRendering() const { return mCameraRelativeRendering; }


        /** Add a level of detail listener. */
        void addLodListener(LodListener *listener);

        /**
        Remove a level of detail listener.
        @remarks
            Do not call from inside an LodListener callback method.
        */
        void removeLodListener(LodListener *listener);

        /** Notify that a movable object lod change event has occurred. */
        void _notifyMovableObjectLodChanged(MovableObjectLodChangedEvent& evt);

        /** Notify that an entity mesh lod change event has occurred. */
        void _notifyEntityMeshLodChanged(EntityMeshLodChangedEvent& evt);

        /** Notify that an entity material lod change event has occurred. */
        void _notifyEntityMaterialLodChanged(EntityMaterialLodChangedEvent& evt);

        /** Handle lod events. */
        void _handleLodEvents();
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
        void execute(RaySceneQueryListener* listener);
    };
    /** Default implementation of SphereSceneQuery. */
	class _OgreExport DefaultSphereSceneQuery : public SphereSceneQuery
    {
    public:
        DefaultSphereSceneQuery(SceneManager* creator);
        ~DefaultSphereSceneQuery();

        /** See SceneQuery. */
        void execute(SceneQueryListener* listener);
    };
    /** Default implementation of PlaneBoundedVolumeListSceneQuery. */
    class _OgreExport DefaultPlaneBoundedVolumeListSceneQuery : public PlaneBoundedVolumeListSceneQuery
    {
    public:
        DefaultPlaneBoundedVolumeListSceneQuery(SceneManager* creator);
        ~DefaultPlaneBoundedVolumeListSceneQuery();

        /** See SceneQuery. */
        void execute(SceneQueryListener* listener);
    };
    /** Default implementation of AxisAlignedBoxSceneQuery. */
	class _OgreExport DefaultAxisAlignedBoxSceneQuery : public AxisAlignedBoxSceneQuery
    {
    public:
        DefaultAxisAlignedBoxSceneQuery(SceneManager* creator);
        ~DefaultAxisAlignedBoxSceneQuery();

        /** See RayScenQuery. */
        void execute(SceneQueryListener* listener);
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
		virtual SceneManager* createInstance(const String& instanceName) = 0;
		/** Destroy an instance of a SceneManager. */
		virtual void destroyInstance(SceneManager* instance) = 0;

	};

	/** @} */
	/** @} */


} // Namespace



#endif
