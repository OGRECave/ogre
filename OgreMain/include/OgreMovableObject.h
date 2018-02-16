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
#include "OgreShadowCaster.h"
#include "OgreAnimable.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {

    // Forward declaration
    class MovableObjectFactory;

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Scene
    *  @{
    */
    /** Abstract class defining a movable object in a scene.
        @remarks
            Instances of this class are discrete, relatively small, movable objects
            which are attached to SceneNode objects to define their position.
    */
    class _OgreExport MovableObject : public ShadowCaster, public AnimableObject, public MovableAlloc
    {
    public:
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
            /** MovableObject has been moved */
            virtual void objectMoved(MovableObject*) {}
            /** Called when the movable object of the camera to be used for rendering.
            @return
                true if allows queue for rendering, false otherwise.
            */
            virtual bool objectRendering(const MovableObject*, const Camera*) { return true; }
            /** Called when the movable object needs to query a light list.
            @remarks
                If you want to customize light finding for this object, you should override 
                this method and hook into MovableObject via MovableObject::setListener.
                Be aware that the default method caches results within a frame to 
                prevent unnecessary recalculation, so if you override this you 
                should provide your own caching to maintain performance.
            @note
                If you use texture shadows, there is an additional restriction - 
                since the lights which should have shadow textures rendered for
                them are determined based on the entire frustum, and not per-object,
                it is important that the lights returned at the start of this 
                list (up to the number of shadow textures available) are the same 
                lights that were used to generate the shadow textures, 
                and they are in the same order (particularly for additive effects).
            @note
                This method will not be called for additive stencil shadows since the
                light list cannot be varied per object with this technique.
            @return
                A pointer to a light list if you populated the light list yourself, or
                NULL to fall back on the default finding process.
            */
            virtual const LightList* objectQueryLights(const MovableObject*) { return 0; }
        };

    protected:
        /// Name of this object
        String mName;
        /// Creator of this object (if created by a factory)
        MovableObjectFactory* mCreator;
        /// SceneManager holding this object (if applicable)
        SceneManager* mManager;
        /// node to which this object is attached
        Node* mParentNode;
        /// MovableObject listener - only one allowed (no list) for size & performance reasons. */
        Listener* mListener;
        bool mParentIsTagPoint : 1;
        /// Is this object visible?
        bool mVisible : 1;
        /// Is debug display enabled?
        bool mDebugDisplay : 1;
        /// Hidden because of distance?
        bool mBeyondFarDistance : 1;
        /// Does this object cast shadows?
        bool mCastShadows : 1;
        /// Flags whether the RenderQueue's default should be used.
        bool mRenderQueueIDSet : 1;
        /// Flags whether the RenderQueue's default should be used.
        bool mRenderQueuePrioritySet : 1;
        /// Does rendering this object disabled by listener?
        bool mRenderingDisabled : 1;
        /// The render queue to use when rendering this object
        uint8 mRenderQueueID;
        /// The render queue group to use when rendering this object
        ushort mRenderQueuePriority;
        /// Upper distance to still render
        Real mUpperDistance;
        Real mSquaredUpperDistance;
        /// Minimum pixel size to still render
        Real mMinPixelSize;
        /// User objects binding.
        UserObjectBindings mUserObjectBindings;
        /// Flags determining whether this object is included / excluded from scene queries
        uint32 mQueryFlags;
        /// Flags determining whether this object is visible (compared to SceneManager mask)
        uint32 mVisibilityFlags;
        /// Cached world AABB of this object
        mutable AxisAlignedBox mWorldAABB;
        // Cached world bounding sphere
        mutable Sphere mWorldBoundingSphere;
        /// World space AABB of this object's dark cap
        mutable AxisAlignedBox mWorldDarkCapBounds;
        /// List of lights for this object
        mutable LightList mLightList;
        /// The last frame that this light list was updated in
        mutable ulong mLightListUpdated;
        /// the light mask defined for this movable. This will be taken into consideration when deciding which light should affect this movable
        uint32 mLightMask;

        // Static members
        /// Default query flags
        static uint32 msDefaultQueryFlags;
        /// Default visibility flags
        static uint32 msDefaultVisibilityFlags;



    public:
        /// Constructor
        MovableObject();

        /// Named constructor
        MovableObject(const String& name);
        /** Virtual destructor - read Scott Meyers if you don't know why this is needed.
        */
        virtual ~MovableObject();

        /** Notify the object of it's creator (internal use only) */
        virtual void _notifyCreator(MovableObjectFactory* fact) { mCreator = fact; }
        /** Get the creator of this object, if any (internal use only) */
        MovableObjectFactory*  _getCreator(void) const { return mCreator; }
        /** Notify the object of it's manager (internal use only) */
        virtual void _notifyManager(SceneManager* man) { mManager = man; }
        /** Get the manager of this object, if any (internal use only) */
        SceneManager* _getManager(void) const { return mManager; }

        /** Notifies the movable object that hardware resources were lost
            @remarks
                Called automatically by RenderSystem if hardware resources
                were lost and can not be restored using some internal mechanism.
                Among affected resources are nested shadow renderables, ManualObjects, etc.
        */
        virtual void _releaseManualHardwareResources() {}

        /** Notifies the movable object that hardware resources should be restored
            @remarks
                Called automatically by RenderSystem if hardware resources
                were lost and can not be restored using some internal mechanism.
                Among affected resources are nested shadow renderables, ManualObjects, etc.
        */
        virtual void _restoreManualHardwareResources() {}

        /** Returns the name of this object. */
        const String& getName(void) const { return mName; }

        /** Returns the type name of this object. */
        virtual const String& getMovableType(void) const = 0;

        /** Returns the node to which this object is attached.
        @remarks
            A MovableObject may be attached to either a SceneNode or to a TagPoint, 
            the latter case if it's attached to a bone on an animated entity. 
            Both are Node subclasses so this method will return either.
        */
        Node* getParentNode(void) const { return mParentNode; }

        /** Returns the scene node to which this object is attached.
        @remarks
            A MovableObject may be attached to either a SceneNode or to a TagPoint, 
            the latter case if it's attached to a bone on an animated entity. 
            This method will return the scene node of the parent entity 
            if the latter is true.
        */
        SceneNode* getParentSceneNode(void) const;

        /// Gets whether the parent node is a TagPoint (or a SceneNode)
        bool isParentTagPoint() const { return mParentIsTagPoint; }

        /** Internal method called to notify the object that it has been attached to a node.
        */
        virtual void _notifyAttached(Node* parent, bool isTagPoint = false);

        /** Returns true if this object is attached to a SceneNode or TagPoint. */
        bool isAttached(void) const { return (mParentNode != NULL); }

        /** Detaches an object from a parent SceneNode or TagPoint, if attached. */
        void detachFromParent(void);

        /** Returns true if this object is attached to a SceneNode or TagPoint, 
            and this SceneNode / TagPoint is currently in an active part of the
            scene graph. */
        virtual bool isInScene(void) const;

        /** Internal method called to notify the object that it has been moved.
        */
        virtual void _notifyMoved(void);

        /** Internal method to notify the object of the camera to be used for the next rendering operation.
            @remarks
                Certain objects may want to do specific processing based on the camera position. This method notifies
                them in case they wish to do this.
        */
        virtual void _notifyCurrentCamera(Camera* cam);

        /** Retrieves the local axis-aligned bounding box for this object.
            @remarks
                This bounding box is in local coordinates.
        */
        virtual const AxisAlignedBox& getBoundingBox(void) const = 0;

        /** Retrieves the radius of the origin-centered bounding sphere 
             for this object.
        */
        virtual Real getBoundingRadius(void) const = 0;

        /** Retrieves the axis-aligned bounding box for this object in world coordinates. */
        virtual const AxisAlignedBox& getWorldBoundingBox(bool derive = false) const;
        /** Retrieves the worldspace bounding sphere for this object. */
        virtual const Sphere& getWorldBoundingSphere(bool derive = false) const;
        /** Internal method by which the movable object must add Renderable subclass instances to the rendering queue.
            @remarks
                The engine will call this method when this object is to be rendered. The object must then create one or more
                Renderable subclass instances which it places on the passed in Queue for rendering.
        */
        virtual void _updateRenderQueue(RenderQueue* queue) = 0;

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
        void setVisible(bool visible) { mVisible = visible; }

        /** Gets this object whether to be visible or not, if it has a renderable component. 
        @remarks
            Returns the value set by MovableObject::setVisible only.
        */
        bool getVisible(void) const { return mVisible; }

        /** Returns whether or not this object is supposed to be visible or not. 
        @remarks
            Takes into account both upper rendering distance and visible flag.
        */
        virtual bool isVisible(void) const;

        /** Sets the distance at which the object is no longer rendered.
        @note Camera::setUseRenderingDistance() needs to be called for this parameter to be used.
        @param dist Distance beyond which the object will not be rendered 
            (the default is 0, which means objects are always rendered).
        */
        void setRenderingDistance(Real dist) {
            mUpperDistance = dist; 
            mSquaredUpperDistance = mUpperDistance * mUpperDistance;
        }

        /** Gets the distance at which batches are no longer rendered. */
        Real getRenderingDistance(void) const { return mUpperDistance; }

        /** Sets the minimum pixel size an object needs to be in both screen axes in order to be rendered
        @note Camera::setUseMinPixelSize() needs to be called for this parameter to be used.
        @param pixelSize Number of minimum pixels
            (the default is 0, which means objects are always rendered).
        */
        void setRenderingMinPixelSize(Real pixelSize) {
            mMinPixelSize = pixelSize; 
        }

        /** Returns the minimum pixel size an object needs to be in both screen axes in order to be rendered
        */
        Real getRenderingMinPixelSize() const {
            return mMinPixelSize; 
        }

        /** @deprecated use UserObjectBindings::setUserAny via getUserObjectBindings() instead.
            Sets any kind of user value on this object.
        @remarks
            This method allows you to associate any user value you like with 
            this MovableObject. This can be a pointer back to one of your own
            classes for instance.       
        */
        OGRE_DEPRECATED void setUserAny(const Any& anything) { getUserObjectBindings().setUserAny(anything); }

        /** @deprecated use UserObjectBindings::getUserAny via getUserObjectBindings() instead.
            Retrieves the custom user value associated with this object.
        */
        OGRE_DEPRECATED const Any& getUserAny(void) const { return getUserObjectBindings().getUserAny(); }

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
        */
        virtual void setRenderQueueGroup(uint8 queueID);

        /** Sets the render queue group and group priority this entity will be rendered through.
        @remarks
            Render queues are grouped to allow you to more tightly control the ordering
            of rendered objects. Within a single render group there another type of grouping
            called priority which allows further control.  If you do not call this method, 
            all Entity objects default to the default queue and priority 
            (RenderQueue::getDefaultQueueGroup, RenderQueue::getDefaultRenderablePriority), 
            which is fine for most objects. You may want to alter this if you want this entity 
            to always appear in front of other objects, e.g. for a 3D menu system or such.
        @par
            See RenderQueue for more details.
        @param queueID Enumerated value of the queue group to use. See the
            enum RenderQueueGroupID for what kind of values can be used here.
        @param priority The priority within a group to use.
        */
        virtual void setRenderQueueGroupAndPriority(uint8 queueID, ushort priority);

        /** Gets the queue group for this entity
        @see setRenderQueueGroup
        */
        uint8 getRenderQueueGroup(void) const { return mRenderQueueID; }

        /// Return the full transformation of the parent sceneNode or the attachingPoint node
        virtual const Affine3& _getParentNodeFullTransform(void) const;

        /** Sets the query flags for this object.
        @remarks
            When performing a scene query, this object will be included or excluded according
            to flags on the object and flags on the query. This is a bitwise value, so only when
            a bit on these flags is set, will it be included in a query asking for that flag. The
            meaning of the bits is application-specific.
        */
        void setQueryFlags(uint32 flags) { mQueryFlags = flags; }

        /** As setQueryFlags, except the flags passed as parameters are appended to the
        existing flags on this object. */
        void addQueryFlags(uint32 flags) { mQueryFlags |= flags; }
            
        /** As setQueryFlags, except the flags passed as parameters are removed from the
        existing flags on this object. */
        void removeQueryFlags(uint32 flags) { mQueryFlags &= ~flags; }
        
        /// Returns the query flags relevant for this object
        uint32 getQueryFlags(void) const { return mQueryFlags; }

        /** Set the default query flags for all future MovableObject instances.
        */
        static void setDefaultQueryFlags(uint32 flags) { msDefaultQueryFlags = flags; }

        /** Get the default query flags for all future MovableObject instances.
        */
        static uint32 getDefaultQueryFlags() { return msDefaultQueryFlags; }

        
        /** Sets the visibility flags for this object.
        @remarks
            As well as a simple true/false value for visibility (as seen in setVisible), 
            you can also set visibility flags which when 'and'ed with the SceneManager's
            visibility mask can also make an object invisible.
        */
        void setVisibilityFlags(uint32 flags) { mVisibilityFlags = flags; }

        /** As setVisibilityFlags, except the flags passed as parameters are appended to the
        existing flags on this object. */
        void addVisibilityFlags(uint32 flags) { mVisibilityFlags |= flags; }
            
        /** As setVisibilityFlags, except the flags passed as parameters are removed from the
        existing flags on this object. */
        void removeVisibilityFlags(uint32 flags) { mVisibilityFlags &= ~flags; }
        
        /// Returns the visibility flags relevant for this object
        uint32 getVisibilityFlags(void) const { return mVisibilityFlags; }

        /** Set the default visibility flags for all future MovableObject instances.
        */
        static void setDefaultVisibilityFlags(uint32 flags) { msDefaultVisibilityFlags = flags; }
        
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
            By default, this method gives the listener a chance to populate light list first,
            if there is no listener or Listener::objectQueryLights returns NULL, it'll
            query the light list from parent entity if it is present, or returns
            SceneNode::findLights if it has parent scene node, otherwise it just returns
            an empty list.
        @par
            The object internally caches the light list, so it will recalculate
            it only when object is moved, or lights that affect the frustum have
            been changed (@see SceneManager::_getLightsDirtyCounter),
            but if listener exists, it will be called each time, so the listener 
            should implement their own cache mechanism to optimise performance.
        @par
            This method can be useful when implementing Renderable::getLights in case
            the renderable is a part of the movable.
        @return The list of lights use to lighting this object.
        */
        const LightList& queryLights(void) const;

        /** Get a bitwise mask which will filter the lights affecting this object
        @remarks
        By default, this mask is fully set meaning all lights will affect this object
        */
        uint32 getLightMask()const { return mLightMask; }
        /** Set a bitwise mask which will filter the lights affecting this object
        @remarks
        This mask will be compared against the mask held against Light to determine
        if a light should affect a given object. 
        By default, this mask is fully set meaning all lights will affect this object
        */
        void setLightMask(uint32 lightMask);

        /** Returns a pointer to the current list of lights for this object.
        @remarks
            You should not modify this list outside of MovableObject::Listener::objectQueryLights
            (say if you want to use it to implement this method, and use the pointer
            as a return value) and for reading it's only accurate as at the last frame.
        */
        LightList* _getLightList() { return &mLightList; }

        /// Returns details of the edges which might be used to determine a silhouette
        EdgeData* getEdgeList(void) { return NULL; }
        /// Returns whether the object has a valid edge list.
        bool hasEdgeList(void) { return false; }
        /// Define a default implementation of method from ShadowCaster which implements no shadows
        ShadowRenderableListIterator getShadowVolumeRenderableIterator(
            ShadowTechnique shadowTechnique, const Light* light, 
            HardwareIndexBufferSharedPtr* indexBuffer, size_t* indexBufferUsedSize,
            bool extrudeVertices, Real extrusionDist, unsigned long flags = 0);
        
        /** Overridden member from ShadowCaster. */
        const AxisAlignedBox& getLightCapBounds(void) const;
        /** Overridden member from ShadowCaster. */
        const AxisAlignedBox& getDarkCapBounds(const Light& light, Real dirLightExtrusionDist) const;
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
        void setCastShadows(bool enabled) { mCastShadows = enabled; }
        /** Returns whether shadow casting is enabled for this object. */
        bool getCastShadows(void) const { return mCastShadows; }
        /** Returns whether the Material of any Renderable that this MovableObject will add to 
            the render queue will receive shadows. 
        */
        bool getReceivesShadows();
            
        /** Get the distance to extrude for a point/spot light */
        Real getPointExtrusionDistance(const Light* l) const;
        /** Get the 'type flags' for this MovableObject.
        @remarks
            A type flag identifies the type of the MovableObject as a bitpattern. 
            This is used for categorical inclusion / exclusion in SceneQuery
            objects. By default, this method returns all ones for objects not 
            created by a MovableObjectFactory (hence always including them); 
            otherwise it returns the value assigned to the MovableObjectFactory.
            Custom objects which don't use MovableObjectFactory will need to 
            override this if they want to be included in queries.
        */
        virtual uint32 getTypeFlags(void) const;

        /** Method to allow a caller to abstractly iterate over the Renderable
            instances that this MovableObject will add to the render queue when
            asked, if any. 
        @param visitor Pointer to a class implementing the Renderable::Visitor 
            interface which will be called back for each Renderable which will
            be queued. Bear in mind that the state of the Renderable instances
            may not be finalised depending on when you call this.
        @param debugRenderables If false, only regular renderables will be visited
            (those for normal display). If true, debug renderables will be
            included too.
        */
        virtual void visitRenderables(Renderable::Visitor* visitor, 
            bool debugRenderables = false) = 0;

        /** Sets whether or not the debug display of this object is enabled.
        @remarks
            Some objects aren't visible themselves but it can be useful to display
            a debug representation of them. Or, objects may have an additional 
            debug display on top of their regular display. This option enables / 
            disables that debug display. Objects that are not visible never display
            debug geometry regardless of this setting.
        */
        void setDebugDisplayEnabled(bool enabled) { mDebugDisplay = enabled; }
        /// Gets whether debug display of this object is enabled. 
        bool isDebugDisplayEnabled(void) const { return mDebugDisplay; }





    };

    /** Interface definition for a factory class which produces a certain
        kind of MovableObject, and can be registered with Root in order
        to allow all clients to produce new instances of this object, integrated
        with the standard Ogre processing.
    */
    class _OgreExport MovableObjectFactory : public MovableAlloc
    {
    protected:
        /// Type flag, allocated if requested
        uint32 mTypeFlag;

        /// Internal implementation of create method - must be overridden
        virtual MovableObject* createInstanceImpl(
            const String& name, const NameValuePairList* params = 0) = 0;
    public:
        MovableObjectFactory() : mTypeFlag(0xFFFFFFFF) {}
        virtual ~MovableObjectFactory() {}
        /// Get the type of the object to be created
        virtual const String& getType(void) const = 0;

        /** Create a new instance of the object.
        @param name The name of the new object
        @param manager The SceneManager instance that will be holding the
            instance once created.
        @param params Name/value pair list of additional parameters required to 
            construct the object (defined per subtype). Optional.
        */
        MovableObject* createInstance(
            const String& name, SceneManager* manager, 
            const NameValuePairList* params = 0);
        /** Destroy an instance of the object */
        virtual void destroyInstance(MovableObject* obj) = 0;

        /** Does this factory require the allocation of a 'type flag', used to 
            selectively include / exclude this type from scene queries?
        @remarks
            The default implementation here is to return 'false', ie not to 
            request a unique type mask from Root. For objects that
            never need to be excluded in SceneQuery results, that's fine, since
            the default implementation of MovableObject::getTypeFlags is to return
            all ones, hence matching any query type mask. However, if you want the
            objects created by this factory to be filterable by queries using a 
            broad type, you have to give them a (preferably unique) type mask - 
            and given that you don't know what other MovableObject types are 
            registered, Root will allocate you one. 
        */
        virtual bool requestTypeFlags(void) const { return false; }
        /** Notify this factory of the type mask to apply. 
        @remarks
            This should normally only be called by Root in response to
            a 'true' result from requestTypeMask. However, you can actually use
            it yourself if you're careful; for example to assign the same mask
            to a number of different types of object, should you always wish them
            to be treated the same in queries.
        */
        void _notifyTypeFlags(uint32 flag) { mTypeFlag = flag; }

        /** Gets the type flag for this factory.
        @remarks
            A type flag is like a query flag, except that it applies to all instances
            of a certain type of object.
        */
        uint32 getTypeFlags(void) const { return mTypeFlag; }

    };
    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
