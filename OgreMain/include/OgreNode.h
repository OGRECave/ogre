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
#ifndef _Node_H__
#define _Node_H__

#include "OgrePrerequisites.h"

#include "OgreMatrix4.h"
#include "OgreRenderable.h"
#include "OgreUserObjectBindings.h"
#include "OgreId.h"
#include "OgreVector3.h"
#include "Math/Array/OgreTransform.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Scene
    *  @{
    */
    /** Class representing a general-purpose node an articulated scene graph.
    @remarks
        A node in the scene graph is a node in a structured tree. A node contains
        information about the transformation which will apply to
        it and all of it's children. Child nodes can have transforms of their own, which
        are combined with their parent's transformations.
    @par
        This is an abstract class - concrete classes are based on this for specific purposes,
        e.g. SceneNode, Bone
    */
    class _OgreExport Node : public NodeAlloc, public IdObject
    {
        friend class TagPoint;
    public:
        /** Enumeration denoting the spaces which a transform can be relative to.
        */
        enum TransformSpace
        {
            /// Transform is relative to the local space
            TS_LOCAL,
            /// Transform is relative to the space of the parent node
            TS_PARENT,
            /// Transform is relative to world space
            TS_WORLD
        };
        typedef vector<Node*>::type NodeVec;
        typedef VectorIterator<NodeVec> NodeVecIterator;
        typedef ConstVectorIterator<NodeVec> ConstNodeVecIterator;

        /** Listener which gets called back on Node events.
        */
        class _OgreExport Listener
        {
        public:
            Listener() {}
            virtual ~Listener() {}
            /** Called when a node gets updated.
            @remarks
                Note that this happens when the node's derived update happens,
                not every time a method altering it's state occurs. There may 
                be several state-changing calls but only one of these calls, 
                when the node graph is fully updated.
            */
            virtual void nodeUpdated(const Node*) {}
            /** Node is being destroyed */
            virtual void nodeDestroyed(const Node*) {}
            /** Node has been attached to a parent */
            virtual void nodeAttached(const Node*) {}
            /** Node has been detached from a parent */
            virtual void nodeDetached(const Node*) {}
        };

        /** Inner class for displaying debug renderable for Node. */
        /*class DebugRenderable : public Renderable, public NodeAlloc
        {
        protected:
            Node* mParent;
            MeshPtr mMeshPtr;
            MaterialPtr mMat;
            Real mScaling;
        public:
            DebugRenderable(Node* parent);
            ~DebugRenderable();
            const MaterialPtr& getMaterial(void) const;
            void getRenderOperation(RenderOperation& op);
            void getWorldTransforms(Matrix4* xform) const;
            Real getSquaredViewDepth(const Camera* cam) const;
            const LightList& getLights(void) const;
            void setScaling(Real s) { mScaling = s; }

        };*/

    protected:
        /// Depth level in the hierarchy tree (0: Root node, 1: Child of root, etc)
        uint16 mDepthLevel;
        /// Calling SceneManager::clearScene won't destroy this node nor detach its
        /// objects (but may still destroy parent and children nodes if they're not
        /// indestructible)
        bool mIndestructibleByClearScene;
        /// Pointer to parent node
        Node* mParent;
        /// Collection of pointers to direct children; hashmap for efficiency
        NodeVec mChildren;
        /// All the transform data needed in SoA form
        Transform mTransform;

        /// Friendly name of this node, can be empty
        String mName;

        /// Only available internally - notification of parent. Can't be null
        void setParent( Node* parent );
        void unsetParent(void);

        /// Notification from parent that we need to migrate to a different depth level
        void parentDepthLevelChanged(void);

        /** Triggers the node to update it's combined transforms.
        @par
            This method is called internally by Ogre to ask the node
            to update it's complete transformation based on it's parents
            derived transform.
        */
        void _updateFromParent(void);

        /** Class-specific implementation of _updateFromParent.
        @remarks
            Splitting the implementation of the update away from the update call
            itself allows the detail to be overridden without disrupting the 
            general sequence of updateFromParent (e.g. raising events)
        */
        virtual void updateFromParentImpl(void);

        /** Internal method for creating a new child node - must be overridden per subclass. */
        virtual Node* createChildImpl( SceneMemoryMgrTypes sceneType ) = 0;

#if OGRE_DEBUG_MODE >= OGRE_DEBUG_MEDIUM
        mutable bool mCachedTransformOutOfDate;
#endif

        /** Node listener - only one allowed (no list) for size & performance reasons. */
        Listener* mListener;

        /// The memory manager used to allocate the Transform.
        NodeMemoryManager *mNodeMemoryManager;

        /// User objects binding.
        UserObjectBindings mUserObjectBindings;

    public:
        /** Index in the vector holding this node reference (could be our parent node, or a global array
            tracking all created nodes to avoid memory leaks). Used for O(1) removals.
        @remarks
            It is the parent (or our creator) the one that sets this value, not ourselves. Do NOT modify
            it manually.
        */
        size_t mGlobalIndex;
        /// @copydoc mGlobalIndex
        size_t mParentIndex;

        /** Constructor, should only be called by parent, not directly.
        @remarks
            Parent pointer can be null.
        */
        Node( IdType id, NodeMemoryManager *nodeMemoryManager, Node *parent );

        /** Don't use this constructor unless you know what you're doing.
            @See NodeMemoryManager::mDummyNode
        */
        Node( const Transform &transformPtrs );

        virtual ~Node();

        /** Sets a custom name for this node. Doesn't have to be unique */
        void setName( const String &name )                          { mName = name; }

        /** Returns the name of the node. */
        const String& getName(void) const                           { return mName; }

        /** Gets this node's parent (NULL if this is the root). */
        Node* getParent(void) const;

        /** Calling SceneManager::clearScene won't destroy this node nor detach its
            objects (but may still destroy parent and children nodes if they're not
            indestructible) when this is true.
        @remarks
            This function provides trivial setter/getters rather than making
            mIndestructibleByClearScene public for two reasons:
                1. It's rare called
                2. There's a lot of value in debugging when a node is set to indestructible,
                   which could happen by accident; and would thus leak.
        */
        void setIndestructibleByClearScene( bool indestructible );
        bool getIndestructibleByClearScene(void) const;

        /** Migrates the node and all of its children to the new memory manager,
            at the same depth level.
        @param nodeMemoryManager
            New memory manager to migrate to.
        */
        void migrateTo( NodeMemoryManager *nodeMemoryManager );

        /// Checks whether this node is static. @See setStatic
        bool isStatic() const;

        /** Turns this Node into static or dynamic
        @remarks
            Switching between dynamic and static has some overhead and forces to update all
            static scene when converted to static. So don't do it frequently.
            Static objects are not updated every frame, only when requested explicitly. Use
            this feature if you plan to have this object unaltered for a very long times
        @par
            Changing this attribute to a node will cause to switch the attribute to all
            attached entities (but not children or parent nodes; it's perfectly valid
            and useful to have dynamic children of a static parent; although the opposite
            (static children, dynamic parent) is probably a bug.
        @return
            True if setStatic made an actual change. False otherwise. Can fail because the
            object was already static/dynamic, or because switching is not supported
        */
        virtual bool setStatic( bool bStatic );

        /// Returns how deep in the hierarchy we are (eg. 0 -> root node, 1 -> child of root)
        uint16 getDepthLevel() const                                    { return mDepthLevel; }

        /// Returns a direct access to the Transform state
        Transform& _getTransform()                                      { return mTransform; }

        /// Called by SceneManager when it is telling we're a static node being dirty
        /// Don't call this directly. @see SceneManager::notifyStaticDirty
        virtual void _notifyStaticDirty(void) const;

        /** Returns a quaternion representing the nodes orientation.
            @remarks
                Don't call this function too often, as we need to convert from SoA
        */
        virtual_l2 Quaternion getOrientation() const;

        /** Sets the orientation of this node via a quaternion.
        @remarks
            Orientations, unlike other transforms, are not always inherited by child nodes.
            Whether or not orientations affect the orientation of the child nodes depends on
            the setInheritOrientation option of the child. In some cases you want a orientating
            of a parent node to apply to a child node (e.g. where the child node is a part of
            the same object, so you want it to be the same relative orientation based on the
            parent's orientation), but not in other cases (e.g. where the child node is just
            for positioning another object, you want it to maintain it's own orientation).
            The default is to inherit as with other transforms.
            Don't call this function too often, as we need to convert to SoA
        @par
            Note that rotations are oriented around the node's origin.
        */
        virtual_l1 void setOrientation( Quaternion q );

        /** Sets the orientation of this node via quaternion parameters.
        @remarks
            Orientations, unlike other transforms, are not always inherited by child nodes.
            Whether or not orientations affect the orientation of the child nodes depends on
            the setInheritOrientation option of the child. In some cases you want a orientating
            of a parent node to apply to a child node (e.g. where the child node is a part of
            the same object, so you want it to be the same relative orientation based on the
            parent's orientation), but not in other cases (e.g. where the child node is just
            for positioning another object, you want it to maintain it's own orientation).
            The default is to inherit as with other transforms.
            Don't call this function too often, as we need to convert to SoA
        @par
            Note that rotations are oriented around the node's origin.
        */
        virtual_l1 void setOrientation( Real w, Real x, Real y, Real z);

        /** Resets the nodes orientation (local axes as world axes, no rotation).
        @remarks
            Orientations, unlike other transforms, are not always inherited by child nodes.
            Whether or not orientations affect the orientation of the child nodes depends on
            the setInheritOrientation option of the child. In some cases you want a orientating
            of a parent node to apply to a child node (e.g. where the child node is a part of
            the same object, so you want it to be the same relative orientation based on the
            parent's orientation), but not in other cases (e.g. where the child node is just
            for positioning another object, you want it to maintain it's own orientation).
            The default is to inherit as with other transforms.
        @par
            Note that rotations are oriented around the node's origin.
        */
        virtual_l1 void resetOrientation(void);

        /** Sets the position of the node relative to it's parent.
        @remarks
            Don't call this function too often, as we need to convert to SoA
        */
        virtual_l1 void setPosition( const Vector3& pos );

        /** Sets the position of the node relative to it's parent.
        @remarks
            Don't call this function too often, as we need to convert to SoA
        */
        virtual_l1 void setPosition(Real x, Real y, Real z);

        /** Gets the position of the node relative to it's parent.
            @remarks
                Don't call this function too often, as we need to convert from SoA
        */
        virtual_l2 Vector3 getPosition(void) const;

        /** Sets the scaling factor applied to this node.
        @remarks
            Scaling factors, unlike other transforms, are not always inherited by child nodes.
            Whether or not scalings affect the size of the child nodes depends on the setInheritScale
            option of the child. In some cases you want a scaling factor of a parent node to apply to
            a child node (e.g. where the child node is a part of the same object, so you want it to be
            the same relative size based on the parent's size), but not in other cases (e.g. where the
            child node is just for positioning another object, you want it to maintain it's own size).
            The default is to inherit as with other transforms.
            Don't call this function too often, as we need to convert to SoA
        @par
            Note that like rotations, scalings are oriented around the node's origin.
        */
        virtual_l1 void setScale(const Vector3& scale);

        /** Sets the scaling factor applied to this node.
        @remarks
            Scaling factors, unlike other transforms, are not always inherited by child nodes.
            Whether or not scalings affect the size of the child nodes depends on the setInheritScale
            option of the child. In some cases you want a scaling factor of a parent node to apply to
            a child node (e.g. where the child node is a part of the same object, so you want it to be
            the same relative size based on the parent's size), but not in other cases (e.g. where the
            child node is just for positioning another object, you want it to maintain it's own size).
            The default is to inherit as with other transforms.
            Don't call this function too often, as we need to convert to SoA
        @par
            Note that like rotations, scalings are oriented around the node's origin.
        */
        virtual_l1 void setScale(Real x, Real y, Real z);

        /** Gets the scaling factor of this node.
            @remarks
                Don't call this function too often, as we need to convert from SoA
        */
        virtual_l2 Vector3 getScale(void) const;

        /** Tells the node whether it should inherit orientation from it's parent node.
        @remarks
            Orientations, unlike other transforms, are not always inherited by child nodes.
            Whether or not orientations affect the orientation of the child nodes depends on
            the setInheritOrientation option of the child. In some cases you want a orientating
            of a parent node to apply to a child node (e.g. where the child node is a part of
            the same object, so you want it to be the same relative orientation based on the
            parent's orientation), but not in other cases (e.g. where the child node is just
            for positioning another object, you want it to maintain it's own orientation).
            The default is to inherit as with other transforms.
        @param inherit If true, this node's orientation will be affected by its parent's orientation.
            If false, it will not be affected.
        */
        virtual_l2 void setInheritOrientation(bool inherit);

        /** Returns true if this node is affected by orientation applied to the parent node. 
        @remarks
            Orientations, unlike other transforms, are not always inherited by child nodes.
            Whether or not orientations affect the orientation of the child nodes depends on
            the setInheritOrientation option of the child. In some cases you want a orientating
            of a parent node to apply to a child node (e.g. where the child node is a part of
            the same object, so you want it to be the same relative orientation based on the
            parent's orientation), but not in other cases (e.g. where the child node is just
            for positioning another object, you want it to maintain it's own orientation).
            The default is to inherit as with other transforms.
        @remarks
            See setInheritOrientation for more info.
        */
        virtual_l2 bool getInheritOrientation(void) const;

        /** Tells the node whether it should inherit scaling factors from it's parent node.
        @remarks
            Scaling factors, unlike other transforms, are not always inherited by child nodes.
            Whether or not scalings affect the size of the child nodes depends on the setInheritScale
            option of the child. In some cases you want a scaling factor of a parent node to apply to
            a child node (e.g. where the child node is a part of the same object, so you want it to be
            the same relative size based on the parent's size), but not in other cases (e.g. where the
            child node is just for positioning another object, you want it to maintain it's own size).
            The default is to inherit as with other transforms.
        @param inherit If true, this node's scale will be affected by its parent's scale. If false,
            it will not be affected.
        */
        virtual_l2 void setInheritScale(bool inherit);

        /** Returns true if this node is affected by scaling factors applied to the parent node. 
        @remarks
            See setInheritScale for more info.
        */
        virtual_l2 bool getInheritScale(void) const;

        /** Scales the node, combining it's current scale with the passed in scaling factor. 
        @remarks
            This method applies an extra scaling factor to the node's existing scale, (unlike setScale
            which overwrites it) combining it's current scale with the new one. E.g. calling this 
            method twice with Vector3(2,2,2) would have the same effect as setScale(Vector3(4,4,4)) if
            the existing scale was 1.
        @par
            Note that like rotations, scalings are oriented around the node's origin.
        */
        virtual_l2 void scale(const Vector3& scale);

        /** Scales the node, combining it's current scale with the passed in scaling factor. 
        @remarks
            This method applies an extra scaling factor to the node's existing scale, (unlike setScale
            which overwrites it) combining it's current scale with the new one. E.g. calling this 
            method twice with Vector3(2,2,2) would have the same effect as setScale(Vector3(4,4,4)) if
            the existing scale was 1.
        @par
            Note that like rotations, scalings are oriented around the node's origin.
        */
        virtual_l2 void scale(Real x, Real y, Real z);

        /** Moves the node along the Cartesian axes.
        @par
            This method moves the node by the supplied vector along the
            world Cartesian axes, i.e. along world x,y,z
        @param d
            Vector with x,y,z values representing the translation.
        @param relativeTo
            The space which this transform is relative to.
        */
        virtual_l2 void translate(const Vector3& d, TransformSpace relativeTo = TS_PARENT);
        /** Moves the node along the Cartesian axes.
        @par
            This method moves the node by the supplied vector along the
            world Cartesian axes, i.e. along world x,y,z
        @param x
            Real @c x value representing the translation.
        @param y
            Real @c y value representing the translation.
        @param z
            Real @c z value representing the translation.
        @param relativeTo
            The space which this transform is relative to.
        */
        virtual_l2 void translate(Real x, Real y, Real z, TransformSpace relativeTo = TS_PARENT);
        /** Moves the node along arbitrary axes.
        @remarks
            This method translates the node by a vector which is relative to
            a custom set of axes.
        @param axes
            A 3x3 Matrix containing 3 column vectors each representing the
            axes X, Y and Z respectively. In this format the standard cartesian
            axes would be expressed as:
            <pre>
            1 0 0
            0 1 0
            0 0 1
            </pre>
            i.e. the identity matrix.
        @param move
            Vector relative to the axes above.
        @param relativeTo
            The space which this transform is relative to.
        */
        virtual_l2 void translate(const Matrix3& axes, const Vector3& move, TransformSpace relativeTo = TS_PARENT);
        /** Moves the node along arbitrary axes.
        @remarks
            This method translates the node by a vector which is relative to
            a custom set of axes.
        @param axes
            A 3x3 Matrix containing 3 column vectors each representing the
            axes X, Y and Z respectively. In this format the standard cartesian
            axes would be expressed as
            <pre>
            1 0 0
            0 1 0
            0 0 1
            </pre>
            i.e. the identity matrix.
        @param x
            The @c x translation component relative to the axes above.
        @param y
            The @c y translation component relative to the axes above.
        @param z
            The @c z translation component relative to the axes above.
        @param relativeTo
            The space which this transform is relative to.
        */
        virtual_l2 void translate(const Matrix3& axes, Real x, Real y, Real z, TransformSpace relativeTo = TS_PARENT);

        /** Rotate the node around the Z-axis.
        */
        virtual_l2 void roll(const Radian& angle, TransformSpace relativeTo = TS_LOCAL);

        /** Rotate the node around the X-axis.
        */
        virtual_l2 void pitch(const Radian& angle, TransformSpace relativeTo = TS_LOCAL);

        /** Rotate the node around the Y-axis.
        */
        virtual_l2 void yaw(const Radian& angle, TransformSpace relativeTo = TS_LOCAL);

        /** Rotate the node around an arbitrary axis.
        */
        virtual_l2 void rotate(const Vector3& axis, const Radian& angle, TransformSpace relativeTo = TS_LOCAL);

        /** Rotate the node around an aritrary axis using a Quarternion.
        */
        virtual_l2 void rotate(const Quaternion& q, TransformSpace relativeTo = TS_LOCAL);

        /** Gets a matrix whose columns are the local axes based on
            the nodes orientation relative to it's parent. */
        virtual_l2 Matrix3 getLocalAxes(void) const;

        /** Creates an unnamed new Node as a child of this node.
        @param translate
            Initial translation offset of child relative to parent
        @param rotate
            Initial rotation relative to parent
        */
        virtual Node* createChild(
            SceneMemoryMgrTypes sceneType = SCENE_DYNAMIC,
            const Vector3& translate = Vector3::ZERO, 
            const Quaternion& rotate = Quaternion::IDENTITY );

        /** Adds a (precreated) child scene node to this node. If it is attached to another node,
            it must be detached first.
        @param child The Node which is to become a child node of this one
        */
        void addChild(Node* child);

        /** Reports the number of child nodes under this one.
        */
        size_t numChildren(void) const                                      { return mChildren.size(); }

        /** Gets a pointer to a child node. */
        Node* getChild( size_t index )                                      { return mChildren[index]; }
        const Node* getChild( size_t index ) const                          { return mChildren[index]; }

        /** Retrieves an iterator for efficiently looping through all children of this node.
        @remarks
            Using this is faster than repeatedly calling getChild if you want to go through
            all (or most of) the children of this node.
            Note that the returned iterator is only valid whilst no children are added or
            removed from this node. Thus you should not store this returned iterator for
            later use, nor should you add / remove children whilst iterating through it;
            store up changes for later. Note that calling methods on returned items in 
            the iterator IS allowed and does not invalidate the iterator.
        */
        NodeVecIterator getChildIterator(void);

        /** Retrieves an iterator for efficiently looping through all children of this node.
        @remarks
            Using this is faster than repeatedly calling getChild if you want to go through
            all (or most of) the children of this node.
            Note that the returned iterator is only valid whilst no children are added or
            removed from this node. Thus you should not store this returned iterator for
            later use, nor should you add / remove children whilst iterating through it;
            store up changes for later. Note that calling methods on returned items in 
            the iterator IS allowed and does not invalidate the iterator.
        */
        ConstNodeVecIterator getChildIterator(void) const;

        /** Drops the specified child from this node. 
        @remarks
            Does not delete the node, just detaches it from
            this parent, potentially to be reattached elsewhere. 
        @par
            Asserts if child is not one of our children.
        */
        virtual void removeChild( Node* child );

        /** Removes all child Nodes attached to this node. Does not delete the nodes, just detaches them from
            this parent, potentially to be reattached elsewhere.
        */
        virtual void removeAllChildren(void);
        
        /** Sets the final world position of the node directly.
        @remarks 
            It's advisable to use the local setPosition if possible
        */
        virtual_l2 void _setDerivedPosition(const Vector3& pos);

        /** Sets the final world orientation of the node directly.
        @remarks 
            It's advisable to use the local setOrientation if possible, this simply does
            the conversion for you.
        */
        virtual_l2 void _setDerivedOrientation(const Quaternion& q);

        /** Gets the orientation of the node as derived from all parents.
        @remarks
            Assumes the caches are already updated. Will trigger an assert
            otherwise.
            @See _getDerivedOrientationUpdated if you need the update process
            to be guaranteed
        */
        virtual_l2 Quaternion _getDerivedOrientation(void) const;

        /** Gets the orientation of the node as derived from all parents.
        @remarks
            Unlike _getDerivedOrientation, this function guarantees the
            cache stays up to date.
            It is strongly advised against calling this function for a large
            number of nodes. Refactor your queries so that they happen
            after SceneManager::UpdateAllTransforms() has been called
        */
        virtual_l2 Quaternion _getDerivedOrientationUpdated(void);

        /** Gets the position of the node as derived from all parents.
        @remarks
            Assumes the caches are already updated. Will trigger an assert
            otherwise.
            @See _getDerivedPositionUpdated if you need the update process
            to be guaranteed
        */
        virtual_l2 Vector3 _getDerivedPosition(void) const;

        /** Gets the position of the node as derived from all parents.
        @remarks
            Unlike _getDerivedPosition, this function guarantees the
            cache stays up to date.
            It is strongly advised against calling this function for a large
            number of nodes. Refactor your queries so that they happen
            after SceneManager::UpdateAllTransforms() has been called
        */
        virtual_l2 Vector3 _getDerivedPositionUpdated(void);

        /** Gets the scaling factor of the node as derived from all parents.
        @remarks
            Assumes the caches are already updated. Will trigger an assert
            otherwise.
            @See _getDerivedScaleUpdated if you need the update process
            to be guaranteed
        */
        virtual_l2 Vector3 _getDerivedScale(void) const;

        /** Gets the scalling factor of the node as derived from all parents.
        @remarks
            Unlike _getDerivedScale, this function guarantees the
            cache stays up to date.
            It is STRONGLY advised against calling this function for a large
            number of nodes. Refactor your queries so that they happen
            after SceneManager::UpdateAllTransforms() has been called
        */
        virtual_l2 Vector3 _getDerivedScaleUpdated(void);

        /** Gets the full transformation matrix for this node.
        @remarks
            This method returns the full transformation matrix
            for this node, including the effect of any parent node
            transformations, provided they have been updated using the Node::_update method.
            This should only be called by a SceneManager which knows the
            derived transforms have been updated before calling this method.
            Applications using Ogre should just use the relative transforms.
            Assumes the caches are already updated
        */
        virtual_l2 FORCEINLINE const Matrix4& _getFullTransform(void) const
        {
#if OGRE_DEBUG_MODE
            assert( !mCachedTransformOutOfDate );
#endif
            return mTransform.mDerivedTransform[mTransform.mIndex];
        }

        /** @See _getDerivedScaleUpdated remarks. @See _getFullTransform */
        virtual_l2 const Matrix4& _getFullTransformUpdated(void);

        /** Sets a listener for this Node.
        @remarks
            Note for size and performance reasons only one listener per node is
            allowed.
        */
        virtual void setListener(Listener* listener)                    { mListener = listener; }
        
        /** Gets the current listener for this Node.
        */
        Listener* getListener(void) const                               { return mListener; }

        /** @See SceneManager::updateAllTransforms()
        @remarks
            We don't pass by reference on purpose (avoid implicit aliasing)
        */
        static void updateAllTransforms( const size_t numNodes, Transform t );
        
        /** Gets the local position, relative to this node, of the given world-space position */
        virtual_l2 Vector3 convertWorldToLocalPosition( const Vector3 &worldPos );

        /** Gets the world position of a point in the node local space
            useful for simple transforms that don't require a child node.*/
        virtual_l2 Vector3 convertLocalToWorldPosition( const Vector3 &localPos );

        /** Gets the local orientation, relative to this node, of the given world-space orientation */
        virtual_l2 Quaternion convertWorldToLocalOrientation( const Quaternion &worldOrientation );

        /** Gets the world orientation of an orientation in the node local space
            useful for simple transforms that don't require a child node.*/
        virtual_l2 Quaternion convertLocalToWorldOrientation( const Quaternion &localOrientation );

        /** Helper function, get the squared view depth.  */
        virtual Real getSquaredViewDepth(const Camera* cam) const;

        /** @deprecated use UserObjectBindings::setUserAny via getUserObjectBindings() instead.
            Sets any kind of user value on this object.
        @remarks
            This method allows you to associate any user value you like with 
            this Node. This can be a pointer back to one of your own
            classes for instance.
        */
        OGRE_DEPRECATED virtual void setUserAny(const Any& anything) { getUserObjectBindings().setUserAny(anything); }

        /** @deprecated use UserObjectBindings::getUserAny via getUserObjectBindings() instead.
            Retrieves the custom user value associated with this object.
        */
        OGRE_DEPRECATED virtual const Any& getUserAny(void) const { return getUserObjectBindings().getUserAny(); }

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

        /** Manually set the mNodeMemoryManager to a null ptr.
        @remarks
            Node doesn't follow the rule of three. This function is useful when you make multiple
            hard copies but only the destructor must release the mTransform only slots once.
        */
        void _setNullNodeMemoryManager(void)                    { mNodeMemoryManager = 0; }

        /** Internal use, notifies all attached objects that our memory pointers
            (i.e. Transform) may have changed (e.g. during cleanups, change of parent, etc)
        */
        virtual void _callMemoryChangeListeners(void) = 0;

        virtual NodeMemoryManager* getDefaultNodeMemoryManager( SceneMemoryMgrTypes sceneType ) = 0;

#if OGRE_DEBUG_MODE >= OGRE_DEBUG_MEDIUM
        virtual void _setCachedTransformOutOfDate(void);
        bool isCachedTransformOutOfDate(void) const             { return mCachedTransformOutOfDate; }
#endif
    };
    /** @} */
    /** @} */

} // namespace Ogre

#include "OgreHeaderSuffix.h"

#endif // _Node_H__
