/*
-----------------------------------------------------------------------------
This source file is part of OGRE
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
THE SOFTWARE.
-----------------------------------------------------------------------------
*/

#ifndef __MovableObject_H__
#define __MovableObject_H__

// Precompiler options
#include "OgrePrerequisites.h"
#include "OgreAxisAlignedBox.h"
#include "OgreSphere.h"
#include "OgreAnimable.h"
#include "OgreSceneNode.h"
#include "Math/Array/OgreObjectData.h"
#include "OgreId.h"
#include "OgreVisibilityFlags.h"
#include "OgreLodStrategy.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {
    typedef vector<Frustum*>::type FrustumVec;

    // Forward declaration
    class MovableObjectFactory;

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Scene
    *  @{
    */

    typedef FastArray<Renderable*> RenderableArray;

    /** Abstract class defining a movable object in a scene.
        @remarks
            Instances of this class are discrete, relatively small, movable objects
            which are attached to SceneNode objects to define their position.
    */
    class _OgreExport MovableObject : public AnimableObject, public MovableAlloc, public IdObject
    {
    public:
        static const FastArray<Real> c_DefaultLodMesh;

        /** Listener which gets called back on MovableObject events.
        */
        class _OgreExport Listener
        {
        public:
            Listener(void) {}
            virtual ~Listener() {}
            /** MovableObject is being destroyed */
            virtual void objectDestroyed(MovableObject*) {}
            /** MovableObject has been attached to a node */
            virtual void objectAttached(MovableObject*) {}
            /** MovableObject has been detached from a node */
            virtual void objectDetached(MovableObject*) {}
        };

        RenderableArray   mRenderables;
    protected:
        /// Node to which this object is attached
        Node* mParentNode;
        /// The render queue to use when rendering this object
        uint8 mRenderQueueID;
        /// All the object data needed in SoA form
        ObjectData mObjectData;
        /// SceneManager holding this object (if applicable)
        SceneManager* mManager;

        //One for each submesh/Renderable
        FastArray<Real> const               *mLodMesh;
        unsigned char                       mCurrentMeshLod;

        /// Minimum pixel size to still render
        Real mMinPixelSize;
        /// User objects binding.
        UserObjectBindings mUserObjectBindings;

        /// MovableObject listener - only one allowed (no list) for size & performance reasons.
        Listener* mListener;

        /// List of lights for this object
        LightList mLightList;

        /// Only valid for V2 objects. Derived classes are in charge of
        /// creating and/or destroying it. Placed here since it's the
        /// most efficient method of retrieval during rendering, iterating
        /// over each Item.
        SkeletonInstance    *mSkeletonInstance;

        /// The memory manager used to allocate the ObjectData.
        ObjectMemoryManager *mObjectMemoryManager;

#if OGRE_DEBUG_MODE
        mutable bool mCachedAabbOutOfDate;
#endif

        /// Friendly name of this object, can be empty
        String mName;

        // Static members
        /// Default query flags
        static uint32 msDefaultQueryFlags;
        /// Default visibility flags
        static uint32 msDefaultVisibilityFlags;

    protected:
        Aabb updateSingleWorldAabb();
        float updateSingleWorldRadius();

    public:
        /** Index in the vector holding this MO reference (could be our parent node, or a global
            array tracking all movable objecst to avoid memory leaks). Used for O(1) removals.
        @remarks
            It is the parent (or our creator) the one that sets this value, not ourselves.
            Do NOT modify it manually.
        */
        size_t mGlobalIndex;
        /// @copydoc mGlobalIndex
        size_t mParentIndex;

        /** Constructor
        @remarks
            Valid render queue Id is between 0 & 254 inclusive
        */
        MovableObject( IdType id, ObjectMemoryManager *objectMemoryManager,
                       SceneManager* manager, uint8 renderQueueId );

        /** Don't use this constructor unless you know what you're doing.
            @See ObjectMemoryManager::mDummyNode
        */
        MovableObject( ObjectData *objectDataPtrs );

        /** Virtual destructor - read Scott Meyers if you don't know why this is needed.
        */
        virtual ~MovableObject();

        /** Notify the object of it's manager (internal use only) */
        void _notifyManager(SceneManager* man) { mManager = man; }
        /** Get the manager of this object, if any (internal use only) */
        SceneManager* _getManager(void) const { return mManager; }

        /** Sets a custom name for this node. Doesn't have to be unique */
        void setName( const String &name )                                  { mName = name; }

        /** Returns the name of this object. */
        const String& getName(void) const                                   { return mName; }

        /** Returns the type name of this object. */
        virtual const String& getMovableType(void) const = 0;

        /// Returns the node to which this object is attached.
        Node* getParentNode(void) const                                     { return mParentNode; }

        inline SceneNode* getParentSceneNode(void) const;

        /** Internal method called to notify the object that it has been attached to a node.
        */
        virtual void _notifyAttached( Node* parent );

        /** Returns true if this object is attached to a Node. */
        bool isAttached(void) const                                         { return mParentNode != 0; }

        /** Detaches an object from a parent SceneNode if attached. */
        void detachFromParent(void);

        /// @See Node::_callMemoryChangeListeners
        virtual void _notifyParentNodeMemoryChanged(void) {}

        unsigned char getCurrentMeshLod(void) const                         { return mCurrentMeshLod; }

        /// Checks whether this MovableObject is static. @See setStatic
        bool isStatic() const;

        /** Turns this Node into static or dynamic
        @remarks
            Switching between dynamic and static has some overhead and forces to update all
            static scene when converted to static. So don't do it frequently.
            Static objects are not updated every frame, only when requested explicitly. Use
            this feature if you plan to have this object unaltered for a very long times
        @par
            Note all MovableObjects support switching between static & dynamic after they
            have been created (eg. InstancedEntity)
        @par
            Changing this attribute will cause to switch the attribute to our parent node,
            and as a result, all of its other attached entities. @See Node::setStatic
        @return
            True if setStatic made an actual change. False otherwise. Can fail because the
            object was already static/dynamic, or because switching is not supported
        */
        bool setStatic( bool bStatic );

        /// Called by SceneManager when it is telling we're a static MovableObject being dirty
        /// Don't call this directly. @see SceneManager::notifyStaticDirty
        virtual void _notifyStaticDirty(void) const {}

        /** Internal method by which the movable object must add Renderable subclass instances to the rendering queue.
            @remarks
                The engine will call this method when this object is to be rendered. The object must then create one or more
                Renderable subclass instances which it places on the passed in Queue for rendering.
        */
        virtual void _updateRenderQueue(RenderQueue* queue, Camera *camera, const Camera *lodCamera) {}

        /** @See SceneManager::updateAllBounds
        @remarks
            We don't pass by reference on purpose (avoid implicit aliasing)
        */
        static void updateAllBounds( const size_t numNodes, ObjectData t );

        /** @See SceneManager::cullFrustum
        @remarks
            We don't pass by reference on purpose (avoid implicit aliasing)
            We perform frustum culling AND test visibility mask at the same time
        @param frustum
            Frustum to clip against
        @param sceneVisibilityFlags
            Combined scene's visibility flags (i.e. viewport | scene). Set LAYER_SHADOW_CASTER
            bit if you want to exclude non-shadow casters.
        @param outCulledObjects
            Out. List of objects that are (fully or partially) inside the frustum and
            should be rendered
        @param lodCamera
            Camera in which lod levels calculations are based (i.e. during shadow pass renders)
            Note however, we only use this camera to calulate if should be visible according to
            mUpperDistance
        */
        typedef FastArray<MovableObject*> MovableObjectArray;
        static void cullFrustum( const size_t numNodes, ObjectData t, const Camera *frustum,
                                 uint32 sceneVisibilityFlags, MovableObjectArray &outCulledObjects,
                                 const Camera *lodCamera );

        /// @See InstancingTheadedCullingMethod, @see InstanceBatch::instanceBatchCullFrustumThreaded
        virtual void instanceBatchCullFrustumThreaded( const Frustum *frustum, const Camera *lodCamera,
                                                        uint32 combinedVisibilityFlags ) {}

        /** @See SceneManager::cullLights & @see MovableObject::cullFrustum
            Produces the global list of visible lights that is needed in buildLightList
        @remarks
            We don't pass ObjectData by reference on purpose (avoid implicit aliasing)
            It's declared here because all affected elements are from MovableObject
            IMPORTANT: It is assumed that all objects in ObjectData are Lights.
        @param outGlobalLightList
            Output, a list of lights, contiguously placed
        @param frustums
            An array of all frustums we need to check against
        @param cubemapFrustums
            An array of all frustums that are used at least once as cubemaps
            (@See SceneManager::createCamera)
        */
        static void cullLights( const size_t numNodes, ObjectData t, LightListInfo &outGlobalLightList,
                                const FrustumVec &frustums , const FrustumVec &cubemapFrustums );

        /** @See SceneManager::buildLightList
        @remarks
            We don't pass by reference on purpose (avoid implicit aliasing)
        @param globalLightList
            List of lights already culled against all possible frustums and
            reorganized contiguously for SoA
        */
        static void buildLightList( const size_t numNodes, ObjectData t,
                                    const LightListInfo &globalLightList );

        static void calculateCastersBox( const size_t numNodes, ObjectData t,
                                         uint32 sceneVisibilityFlags, AxisAlignedBox *outBox );

        friend void LodStrategy::lodUpdateImpl( const size_t numNodes, ObjectData t,
                                                const Camera *camera, Real bias ) const;
        friend void LodStrategy::lodSet( ObjectData &t, Real lodValues[ARRAY_PACKED_REALS] );

        /** Tells this object whether to be visible or not, if it has a renderable component. 
        @note An alternative approach of making an object invisible is to detach it
            from it's SceneNode, or to remove the SceneNode entirely. 
            Detaching a node means that structurally the scene graph changes. 
            Once this change has taken place, the objects / nodes that have been 
            removed have less overhead to the visibility detection pass than simply
            making the object invisible, so if you do this and leave the objects 
            out of the tree for a long time, it's faster. However, the act of 
            detaching / reattaching nodes is in itself more expensive than 
            setting an object visibility flag, since in the latter case 
            structural changes are not made. Therefore, small or frequent visibility
            changes are best done using this method; large or more longer term
            changes are best done by detaching.
        */
        inline void setVisible( bool visible );

        /** Gets this object whether to be visible or not, if it has a renderable component. 
        @remarks
            Returns the value set by MovableObject::setVisible only.
        */
        inline bool getVisible(void) const;

        /** Returns whether or not this object is supposed to be visible or not. 
        @remarks
            Takes into account visibility flags and the setVisible, but not rendering distance.
        */
        bool isVisible(void) const;

        /** Sets the distance at which the object is no longer rendered.
        @param
            dist Distance beyond which the object will not be rendered (the default is FLT_MAX,
            which means objects are always rendered). Values equal or below zero will be ignored,
            and cause an assertion in debug mode.
        */
        inline void setRenderingDistance(Real dist);

        /** Gets the distance at which batches are no longer rendered. */
        inline Real getRenderingDistance(void) const;

        /** Sets the minimum pixel size an object needs to be in both screen axes in order to be rendered
        @note Camera::setUseMinPixelSize() needs to be called for this parameter to be used.
        @param pixelSize Number of minimum pixels
            (the default is 0, which means objects are always rendered).
        */
        void setRenderingMinPixelSize(Real pixelSize) { 
            mMinPixelSize = pixelSize; 
        }

        /** Returns the minimum pixel size an object needs to be in both screen axes in order to be
            rendered
        */
        Real getRenderingMinPixelSize() const                               { return mMinPixelSize; }

        /** Return an instance of user objects binding associated with this class.
            You can use it to associate one or more custom objects with this class instance.
        @see UserObjectBindings::setUserAny.        
        */
        UserObjectBindings& getUserObjectBindings() { return mUserObjectBindings; }

        /** Return an instance of user objects binding associated with this class.
        You can use it to associate one or more custom objects with this class instance.
        @see UserObjectBindings::setUserAny.        
        */
        const UserObjectBindings& getUserObjectBindings() const { return mUserObjectBindings; }

        /** Sets the render queue group this entity will be rendered through.
        @remarks
            Render queues are grouped to allow you to more tightly control the ordering
            of rendered objects. If you do not call this method, all Entity objects default
            to the default queue (RenderQueue::getDefaultQueueGroup), which is fine for most objects. You may want to alter this
            if you want this entity to always appear in front of other objects, e.g. for
            a 3D menu system or such.
        @par
            See RenderQueue for more details.
        @param queueID Enumerated value of the queue group to use. See the
            enum RenderQueueGroupID for what kind of values can be used here.
        @par
            Valid render queue ids are between 0 & 254 inclusive
        */
        virtual void setRenderQueueGroup(uint8 queueID);

        /** Gets the queue group for this entity, see setRenderQueueGroup for full details. */
        inline uint8 getRenderQueueGroup(void) const;

        /// Returns a direct access to the ObjectData state
        ObjectData& _getObjectData()                                        { return mObjectData; }

        /// Returns the full transformation of the parent sceneNode or the attachingPoint node
        const Matrix4& _getParentNodeFullTransform(void) const;

        /** Retrieves the local axis-aligned bounding box for this object.
            @remarks
                This bounding box is in local coordinates.
        */
        Aabb getLocalAabb(void) const;

        /** Sets the local axis-aligned bounding box for this object.
         @remarks
         This bounding box is in local coordinates.
         */
        void setLocalAabb(const Aabb box);

        /** Gets the axis aligned box in world space.
        @remarks
            Assumes the caches are already updated. Will trigger an assert
            otherwise. @See getWorldAabbUpdated if you need the update process
            to be guaranteed
        */
        Aabb getWorldAabb() const;

        /** Gets the axis aligned box in world space.
        @remarks
            Unlike getWorldAabb, this function guarantees the cache stays up to date.
            It is STRONGLY advised against calling this function for a large
            number of MovableObject. Refactor your queries so that they happen
            after SceneManager::updateAllBounds() has been called
        */
        Aabb getWorldAabbUpdated();

        /// See getLocalAabb and getWorldRadius
        float getLocalRadius(void) const;

        /** Gets the bounding Radius scaled by max( scale.x, scale.y, scale.z ).
        @remarks
            Assumes the caches are already updated. Will trigger an assert
            otherwise. @See getWorldRadiusUpdated if you need the update process
            to be guaranteed
        */
        float getWorldRadius() const;

        /** Gets the bounding Radius scaled by max( scale.x, scale.y, scale.z ).
        @remarks
            Unlike getWorldRadius, this function guarantees the cache stays up to date.
            It is STRONGLY advised against calling this function for a large
            number of MovableObject. Refactor your queries so that they happen
            after SceneManager::updateAllBounds() has been called
        */
        float getWorldRadiusUpdated();

        /** Sets the query flags for this object.
        @remarks
            When performing a scene query, this object will be included or excluded according
            to flags on the object and flags on the query. This is a bitwise value, so only when
            a bit on these flags is set, will it be included in a query asking for that flag. The
            meaning of the bits is application-specific.
        */
        inline void setQueryFlags(uint32 flags);

        /** As setQueryFlags, except the flags passed as parameters are appended to the
            existing flags on this object. */
        inline void addQueryFlags(uint32 flags);

        /** As setQueryFlags, except the flags passed as parameters are removed from the
            existing flags on this object. */
        inline void removeQueryFlags(uint32 flags);

        /// Returns the query flags relevant for this object
        inline uint32 getQueryFlags(void) const;

        /** Set the default query flags for all future MovableObject instances.
        */
        static void setDefaultQueryFlags(uint32 flags) { msDefaultQueryFlags = flags; }

        /** Get the default query flags for all future MovableObject instances.
        */
        static uint32 getDefaultQueryFlags() { return msDefaultQueryFlags; }

        /// Returns the distance to camera as calculated in @cullFrustum
        inline RealAsUint getCachedDistanceToCamera(void) const;

        /// Returns the distance to camera as calculated in @cullFrustum
        inline Real getCachedDistanceToCameraAsReal(void) const;

        /** Sets the visibility flags for this object.
        @remarks
            As well as a simple true/false value for visibility (as seen in setVisible), 
            you can also set visibility flags that is applied a binary 'and' with the SceneManager's
            mask and a compositor node pass. To exclude particular objects from rendering.
            Changes to reserved visibility flags are ignored (won't take effect).
        */
        inline void setVisibilityFlags(uint32 flags);

        /** As setVisibilityFlags, except the flags passed as parameters are appended to the
        existing flags on this object. */
        inline void addVisibilityFlags(uint32 flags);
            
        /** As setVisibilityFlags, except the flags passed as parameters are removed from the
        existing flags on this object. */
        inline void removeVisibilityFlags(uint32 flags);
        
        /** Returns the visibility flags relevant for this object. Reserved visibility flags are
            not returned.
        */
        inline uint32 getVisibilityFlags(void) const;

        /** Set the default visibility flags for all future MovableObject instances.
        */
        inline static void setDefaultVisibilityFlags(uint32 flags);
        
        /** Get the default visibility flags for all future MovableObject instances.
        */
        static uint32 getDefaultVisibilityFlags() { return msDefaultVisibilityFlags; }

        /** Sets a listener for this object.
        @remarks
            Note for size and performance reasons only one listener per object
            is allowed.
        */
        void setListener(Listener* listener) { mListener = listener; }

        /** Gets the current listener for this object.
        */
        Listener* getListener(void) const { return mListener; }

        /** Gets a list of lights, ordered relative to how close they are to this movable object.
        @remarks
            The lights are filled in @see buildLightList
        @return The list of lights use to lighting this object.
        */
        const LightList& queryLights(void) const                                { return mLightList; }

        /** Get a bitwise mask which will filter the lights affecting this object
        @remarks
            By default, this mask is fully set meaning all lights will affect this object
        */
        inline uint32 getLightMask()const;
        /** Set a bitwise mask which will filter the lights affecting this object
        @remarks
        This mask will be compared against the mask held against Light to determine
            if a light should affect a given object. 
            By default, this mask is fully set meaning all lights will affect this object
        */
        inline void setLightMask(uint32 lightMask);

        /** Returns a pointer to the current list of lights for this object.
        @remarks
            You should not modify this list outside of MovableObject::Listener::objectQueryLights
            (say if you want to use it to implement this method, and use the pointer
            as a return value) and for reading it's only accurate as at the last frame.
        */
        LightList* _getLightList() { return &mLightList; }

        /** Sets whether or not this object will cast shadows.
        @remarks
        This setting simply allows you to turn on/off shadows for a given object.
        An object will not cast shadows unless the scene supports it in any case
        (see SceneManager::setShadowTechnique), and also the material which is
        in use must also have shadow casting enabled. By default all entities cast
        shadows. If, however, for some reason you wish to disable this for a single 
        object then you can do so using this method.
        @note This method normally refers to objects which block the light, but
        since Light is also a subclass of MovableObject, in that context it means
        whether the light causes shadows itself.
        */
        inline void setCastShadows( bool enabled );
        /** Returns whether shadow casting is enabled for this object. */
        inline bool getCastShadows(void) const;

        SkeletonInstance* getSkeletonInstance(void) const   { return mSkeletonInstance; }

#if OGRE_DEBUG_MODE
        void _setCachedAabbOutOfDate(void)                  { mCachedAabbOutOfDate = true; }
        bool isCachedAabbOutOfDate() const                  { return mCachedAabbOutOfDate; }
#endif

    };

    /** Interface definition for a factory class which produces a certain
        kind of MovableObject, and can be registered with Root in order
        to allow all clients to produce new instances of this object, integrated
        with the standard Ogre processing.
    */
    class _OgreExport MovableObjectFactory : public MovableAlloc
    {
    protected:
        /// Internal implementation of create method - must be overridden
        virtual MovableObject* createInstanceImpl( IdType id, ObjectMemoryManager *objectMemoryManager,
                                                   SceneManager* manager,
                                                   const NameValuePairList* params = 0) = 0;
    public:
        MovableObjectFactory() {}
        virtual ~MovableObjectFactory() {}
        /// Get the type of the object to be created
        virtual const String& getType(void) const = 0;

        /** Create a new instance of the object.
        @param manager The SceneManager instance that will be holding the
            instance once created.
        @param params Name/value pair list of additional parameters required to 
            construct the object (defined per subtype). Optional.
        */
        virtual MovableObject* createInstance( IdType id, ObjectMemoryManager *objectMemoryManager,
                                        SceneManager* manager, const NameValuePairList* params = 0);
        /** Destroy an instance of the object */
        virtual void destroyInstance(MovableObject* obj) = 0;
    };

    class _OgreExport NullEntity : public MovableObject
    {
        static const String msMovableType;
    public:
        NullEntity() : MovableObject( 0 )
        {
        }

        virtual const String& getMovableType(void) const
        {
            return msMovableType;
        }
        virtual void _updateRenderQueue(RenderQueue* queue, Camera *camera, const Camera *lodCamera,
                                        RealAsUint depth) {}
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#include "OgreMovableObject.inl"

#endif
