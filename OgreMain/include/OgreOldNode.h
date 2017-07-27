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
#ifndef _OldNode_H__
#define _OldNode_H__

#include "OgrePrerequisites.h"

#include "OgreCommon.h"
#include "OgreMatrix3.h"
#include "OgreMatrix4.h"
#include "OgreVector3.h"
#include "OgreQuaternion.h"
#include "OgreString.h"
#include "OgreRenderable.h"
#include "OgreIteratorWrappers.h"
#include "OgreMesh.h"
#include "OgreUserObjectBindings.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {
namespace v1 {

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Scene
    *  @{
    */
    /** Class representing a general-purpose OldNode an articulated scene graph.
    @remarks
        A OldNode in the scene graph is a OldNode in a structured tree. A OldNode contains
        information about the transformation which will apply to
        it and all of it's children. Child OldNodes can have transforms of their own, which
        are combined with their parent's transformations.
    @par
        This is an abstract class - concrete classes are based on this for specific purposes,
        e.g. OldBone
    */
    class _OgreExport OldNode : public NodeAlloc
    {
    public:
        /** Enumeration denoting the spaces which a transform can be relative to.
        */
        enum TransformSpace
        {
            /// Transform is relative to the local space
            TS_LOCAL,
            /// Transform is relative to the space of the parent OldNode
            TS_PARENT,
            /// Transform is relative to world space
            TS_WORLD
        };
        typedef unordered_map<String, OldNode*>::type ChildOldNodeMap;
        typedef MapIterator<ChildOldNodeMap> ChildOldNodeIterator;
        typedef ConstMapIterator<ChildOldNodeMap> ConstChildOldNodeIterator;

        /** Listener which gets called back on OldNode events.
        */
        class _OgreExport Listener
        {
        public:
            Listener() {}
            virtual ~Listener() {}
            /** Called when a OldNode gets updated.
            @remarks
                Note that this happens when the OldNode's derived update happens,
                not every time a method altering it's state occurs. There may 
                be several state-changing calls but only one of these calls, 
                when the OldNode graph is fully updated.
            */
            virtual void OldNodeUpdated(const OldNode*) {}
            /** OldNode is being destroyed */
            virtual void OldNodeDestroyed(const OldNode*) {}
            /** OldNode has been attached to a parent */
            virtual void OldNodeAttached(const OldNode*) {}
            /** OldNode has been detached from a parent */
            virtual void OldNodeDetached(const OldNode*) {}
        };

    protected:
        /// Pointer to parent OldNode
        OldNode* mParent;
        /// Collection of pointers to direct children; hashmap for efficiency
        ChildOldNodeMap mChildren;

        typedef set<OldNode*>::type ChildUpdateSet;
        /// List of children which need updating, used if self is not out of date but children are
        mutable ChildUpdateSet mChildrenToUpdate;
        /// Flag to indicate own transform from parent is out of date
        mutable bool mNeedParentUpdate;
        /// Flag indicating that all children need to be updated
        mutable bool mNeedChildUpdate;
        /// Flag indicating that parent has been notified about update request
        mutable bool mParentNotified ;
        /// Flag indicating that the OldNode has been queued for update
        mutable bool mQueuedForUpdate;

        /// Friendly name of this OldNode, can be automatically generated if you don't care
        String mName;

        /// Stores the orientation of the OldNode relative to it's parent.
        Quaternion mOrientation;

        /// Stores the position/translation of the OldNode relative to its parent.
        Vector3 mPosition;

        /// Stores the scaling factor applied to this OldNode
        Vector3 mScale;

        /// Stores whether this OldNode inherits orientation from it's parent
        bool mInheritOrientation;

        /// Stores whether this OldNode inherits scale from it's parent
        bool mInheritScale;

        /// Only available internally - notification of parent.
        virtual void setParent(OldNode* parent);

        /** Cached combined orientation.
        @par
            This member is the orientation derived by combining the
            local transformations and those of it's parents.
            This is updated when _updateFromParent is called by the
            SceneManager or the OldNodes parent.
        */
        mutable Quaternion mDerivedOrientation;

        /** Cached combined position.
        @par
            This member is the position derived by combining the
            local transformations and those of it's parents.
            This is updated when _updateFromParent is called by the
            SceneManager or the OldNodes parent.
        */
        mutable Vector3 mDerivedPosition;

        /** Cached combined scale.
        @par
            This member is the position derived by combining the
            local transformations and those of it's parents.
            This is updated when _updateFromParent is called by the
            SceneManager or the OldNodes parent.
        */
        mutable Vector3 mDerivedScale;

        /** Triggers the OldNode to update it's combined transforms.
        @par
            This method is called internally by Ogre to ask the OldNode
            to update it's complete transformation based on it's parents
            derived transform.
        */
        virtual void _updateFromParent(void) const;

        /** Class-specific implementation of _updateFromParent.
        @remarks
            Splitting the implementation of the update away from the update call
            itself allows the detail to be overridden without disrupting the 
            general sequence of updateFromParent (e.g. raising events)
        */
        virtual void updateFromParentImpl(void) const;


        /** Internal method for creating a new child OldNode - must be overridden per subclass. */
        virtual OldNode* createChildImpl(void) = 0;

        /** Internal method for creating a new child OldNode - must be overridden per subclass. */
        virtual OldNode* createChildImpl(const String& name) = 0;

        /// The position to use as a base for keyframe animation
        Vector3 mInitialPosition;
        /// The orientation to use as a base for keyframe animation
        Quaternion mInitialOrientation;
        /// The scale to use as a base for keyframe animation
        Vector3 mInitialScale;

        /// Cached derived transform as a 4x4 matrix
        mutable Matrix4 mCachedTransform;
        mutable bool mCachedTransformOutOfDate;

        /** OldNode listener - only one allowed (no list) for size & performance reasons. */
        Listener* mListener;

        typedef vector<OldNode*>::type QueuedUpdates;
        static QueuedUpdates msQueuedUpdates;

        /// User objects binding.
        UserObjectBindings mUserObjectBindings;

    public:
        /** Constructor, should only be called by parent, not directly.
        @remarks
            Generates a name.
        */
        OldNode();
        /** Constructor, should only be called by parent, not directly.
        @remarks
            Assigned a name.
        */
        OldNode(const String& name);

        virtual ~OldNode();  

        /** Returns the name of the OldNode. */
        const String& getName(void) const;

        /** Gets this OldNode's parent (NULL if this is the root).
        */
        virtual OldNode* getParent(void) const;

        /** Returns a quaternion representing the OldNodes orientation.
        */
        virtual const Quaternion & getOrientation() const;

        /** Sets the orientation of this OldNode via a quaternion.
        @remarks
            Orientations, unlike other transforms, are not always inherited by child OldNodes.
            Whether or not orientations affect the orientation of the child OldNodes depends on
            the setInheritOrientation option of the child. In some cases you want a orientating
            of a parent OldNode to apply to a child OldNode (e.g. where the child OldNode is a part of
            the same object, so you want it to be the same relative orientation based on the
            parent's orientation), but not in other cases (e.g. where the child OldNode is just
            for positioning another object, you want it to maintain it's own orientation).
            The default is to inherit as with other transforms.
        @par
            Note that rotations are oriented around the OldNode's origin.
        */
        virtual void setOrientation( const Quaternion& q );

        /** Sets the orientation of this OldNode via quaternion parameters.
        @remarks
            Orientations, unlike other transforms, are not always inherited by child OldNodes.
            Whether or not orientations affect the orientation of the child OldNodes depends on
            the setInheritOrientation option of the child. In some cases you want a orientating
            of a parent OldNode to apply to a child OldNode (e.g. where the child OldNode is a part of
            the same object, so you want it to be the same relative orientation based on the
            parent's orientation), but not in other cases (e.g. where the child OldNode is just
            for positioning another object, you want it to maintain it's own orientation).
            The default is to inherit as with other transforms.
        @par
            Note that rotations are oriented around the OldNode's origin.
        */
        virtual void setOrientation( Real w, Real x, Real y, Real z);

        /** Resets the OldNodes orientation (local axes as world axes, no rotation).
        @remarks
            Orientations, unlike other transforms, are not always inherited by child OldNodes.
            Whether or not orientations affect the orientation of the child OldNodes depends on
            the setInheritOrientation option of the child. In some cases you want a orientating
            of a parent OldNode to apply to a child OldNode (e.g. where the child OldNode is a part of
            the same object, so you want it to be the same relative orientation based on the
            parent's orientation), but not in other cases (e.g. where the child OldNode is just
            for positioning another object, you want it to maintain it's own orientation).
            The default is to inherit as with other transforms.
        @par
            Note that rotations are oriented around the OldNode's origin.
        */
        virtual void resetOrientation(void);

        /** Sets the position of the OldNode relative to it's parent.
        */
        virtual void setPosition(const Vector3& pos);

        /** Sets the position of the OldNode relative to it's parent.
        */
        virtual void setPosition(Real x, Real y, Real z);

        /** Gets the position of the OldNode relative to it's parent.
        */
        virtual const Vector3 & getPosition(void) const;

        /** Sets the scaling factor applied to this OldNode.
        @remarks
            Scaling factors, unlike other transforms, are not always inherited by child OldNodes.
            Whether or not scalings affect the size of the child OldNodes depends on the setInheritScale
            option of the child. In some cases you want a scaling factor of a parent OldNode to apply to
            a child OldNode (e.g. where the child OldNode is a part of the same object, so you want it to be
            the same relative size based on the parent's size), but not in other cases (e.g. where the
            child OldNode is just for positioning another object, you want it to maintain it's own size).
            The default is to inherit as with other transforms.
        @par
            Note that like rotations, scalings are oriented around the OldNode's origin.
        */
        virtual void setScale(const Vector3& scale);

        /** Sets the scaling factor applied to this OldNode.
        @remarks
            Scaling factors, unlike other transforms, are not always inherited by child OldNodes.
            Whether or not scalings affect the size of the child OldNodes depends on the setInheritScale
            option of the child. In some cases you want a scaling factor of a parent OldNode to apply to
            a child OldNode (e.g. where the child OldNode is a part of the same object, so you want it to be
            the same relative size based on the parent's size), but not in other cases (e.g. where the
            child OldNode is just for positioning another object, you want it to maintain it's own size).
            The default is to inherit as with other transforms.
        @par
            Note that like rotations, scalings are oriented around the OldNode's origin.
        */
        virtual void setScale(Real x, Real y, Real z);

        /** Gets the scaling factor of this OldNode.
        */
        virtual const Vector3 & getScale(void) const;

        /** Tells the OldNode whether it should inherit orientation from it's parent OldNode.
        @remarks
            Orientations, unlike other transforms, are not always inherited by child OldNodes.
            Whether or not orientations affect the orientation of the child OldNodes depends on
            the setInheritOrientation option of the child. In some cases you want a orientating
            of a parent OldNode to apply to a child OldNode (e.g. where the child OldNode is a part of
            the same object, so you want it to be the same relative orientation based on the
            parent's orientation), but not in other cases (e.g. where the child OldNode is just
            for positioning another object, you want it to maintain it's own orientation).
            The default is to inherit as with other transforms.
        @param inherit If true, this OldNode's orientation will be affected by its parent's orientation.
            If false, it will not be affected.
        */
        virtual void setInheritOrientation(bool inherit);

        /** Returns true if this OldNode is affected by orientation applied to the parent OldNode. 
        @remarks
            Orientations, unlike other transforms, are not always inherited by child OldNodes.
            Whether or not orientations affect the orientation of the child OldNodes depends on
            the setInheritOrientation option of the child. In some cases you want a orientating
            of a parent OldNode to apply to a child OldNode (e.g. where the child OldNode is a part of
            the same object, so you want it to be the same relative orientation based on the
            parent's orientation), but not in other cases (e.g. where the child OldNode is just
            for positioning another object, you want it to maintain it's own orientation).
            The default is to inherit as with other transforms.
        @remarks
            See setInheritOrientation for more info.
        */
        virtual bool getInheritOrientation(void) const;

        /** Tells the OldNode whether it should inherit scaling factors from it's parent OldNode.
        @remarks
            Scaling factors, unlike other transforms, are not always inherited by child OldNodes.
            Whether or not scalings affect the size of the child OldNodes depends on the setInheritScale
            option of the child. In some cases you want a scaling factor of a parent OldNode to apply to
            a child OldNode (e.g. where the child OldNode is a part of the same object, so you want it to be
            the same relative size based on the parent's size), but not in other cases (e.g. where the
            child OldNode is just for positioning another object, you want it to maintain it's own size).
            The default is to inherit as with other transforms.
        @param inherit If true, this OldNode's scale will be affected by its parent's scale. If false,
            it will not be affected.
        */
        virtual void setInheritScale(bool inherit);

        /** Returns true if this OldNode is affected by scaling factors applied to the parent OldNode. 
        @remarks
            See setInheritScale for more info.
        */
        virtual bool getInheritScale(void) const;

        /** Scales the OldNode, combining it's current scale with the passed in scaling factor. 
        @remarks
            This method applies an extra scaling factor to the OldNode's existing scale, (unlike setScale
            which overwrites it) combining it's current scale with the new one. E.g. calling this 
            method twice with Vector3(2,2,2) would have the same effect as setScale(Vector3(4,4,4)) if
            the existing scale was 1.
        @par
            Note that like rotations, scalings are oriented around the OldNode's origin.
        */
        virtual void scale(const Vector3& scale);

        /** Scales the OldNode, combining it's current scale with the passed in scaling factor. 
        @remarks
            This method applies an extra scaling factor to the OldNode's existing scale, (unlike setScale
            which overwrites it) combining it's current scale with the new one. E.g. calling this 
            method twice with Vector3(2,2,2) would have the same effect as setScale(Vector3(4,4,4)) if
            the existing scale was 1.
        @par
            Note that like rotations, scalings are oriented around the OldNode's origin.
        */
        virtual void scale(Real x, Real y, Real z);

        /** Moves the OldNode along the Cartesian axes.
        @par
            This method moves the OldNode by the supplied vector along the
            world Cartesian axes, i.e. along world x,y,z
        @param d
            Vector with x,y,z values representing the translation.
        @param relativeTo
            The space which this transform is relative to.
        */
        virtual void translate(const Vector3& d, TransformSpace relativeTo = TS_PARENT);
        /** Moves the OldNode along the Cartesian axes.
        @par
            This method moves the OldNode by the supplied vector along the
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
        virtual void translate(Real x, Real y, Real z, TransformSpace relativeTo = TS_PARENT);
        /** Moves the OldNode along arbitrary axes.
        @remarks
            This method translates the OldNode by a vector which is relative to
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
        virtual void translate(const Matrix3& axes, const Vector3& move, TransformSpace relativeTo = TS_PARENT);
        /** Moves the OldNode along arbitrary axes.
        @remarks
            This method translates the OldNode by a vector which is relative to
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
        virtual void translate(const Matrix3& axes, Real x, Real y, Real z, TransformSpace relativeTo = TS_PARENT);

        /** Rotate the OldNode around the Z-axis.
        */
        virtual void roll(const Radian& angle, TransformSpace relativeTo = TS_LOCAL);

        /** Rotate the OldNode around the X-axis.
        */
        virtual void pitch(const Radian& angle, TransformSpace relativeTo = TS_LOCAL);

        /** Rotate the OldNode around the Y-axis.
        */
        virtual void yaw(const Radian& angle, TransformSpace relativeTo = TS_LOCAL);

        /** Rotate the OldNode around an arbitrary axis.
        */
        virtual void rotate(const Vector3& axis, const Radian& angle, TransformSpace relativeTo = TS_LOCAL);

        /** Rotate the OldNode around an aritrary axis using a Quarternion.
        */
        virtual void rotate(const Quaternion& q, TransformSpace relativeTo = TS_LOCAL);

        /** Gets a matrix whose columns are the local axes based on
            the OldNodes orientation relative to it's parent. */
        virtual Matrix3 getLocalAxes(void) const;

        /** Creates an unnamed new OldNode as a child of this OldNode.
        @param translate
            Initial translation offset of child relative to parent
        @param rotate
            Initial rotation relative to parent
        */
        virtual OldNode* createChild(
            const Vector3& translate = Vector3::ZERO, 
            const Quaternion& rotate = Quaternion::IDENTITY );

        /** Creates a new named OldNode as a child of this OldNode.
        @remarks
            This creates a child OldNode with a given name, which allows you to look the OldNode up from 
            the parent which holds this collection of OldNodes.
        @param translate
            Initial translation offset of child relative to parent
        @param rotate
            Initial rotation relative to parent
        */
        virtual OldNode* createChild(const String& name, const Vector3& translate = Vector3::ZERO, const Quaternion& rotate = Quaternion::IDENTITY);

        /** Adds a (precreated) child scene OldNode to this OldNode. If it is attached to another OldNode,
            it must be detached first.
        @param child The OldNode which is to become a child OldNode of this one
        */
        virtual void addChild(OldNode* child);

        /** Reports the number of child OldNodes under this one.
        */
        virtual unsigned short numChildren(void) const;

        /** Gets a pointer to a child OldNode.
        @remarks
            There is an alternate getChild method which returns a named child.
        */
        virtual OldNode* getChild(unsigned short index) const;    

        /** Gets a pointer to a named child OldNode.
        */
        virtual OldNode* getChild(const String& name) const;

        /** Retrieves an iterator for efficiently looping through all children of this OldNode.
        @remarks
            Using this is faster than repeatedly calling getChild if you want to go through
            all (or most of) the children of this OldNode.
            Note that the returned iterator is only valid whilst no children are added or
            removed from this OldNode. Thus you should not store this returned iterator for
            later use, nor should you add / remove children whilst iterating through it;
            store up changes for later. Note that calling methods on returned items in 
            the iterator IS allowed and does not invalidate the iterator.
        */
        virtual ChildOldNodeIterator getChildIterator(void);

        /** Retrieves an iterator for efficiently looping through all children of this OldNode.
        @remarks
            Using this is faster than repeatedly calling getChild if you want to go through
            all (or most of) the children of this OldNode.
            Note that the returned iterator is only valid whilst no children are added or
            removed from this OldNode. Thus you should not store this returned iterator for
            later use, nor should you add / remove children whilst iterating through it;
            store up changes for later. Note that calling methods on returned items in 
            the iterator IS allowed and does not invalidate the iterator.
        */
        virtual ConstChildOldNodeIterator getChildIterator(void) const;

        /** Drops the specified child from this OldNode. 
        @remarks
            Does not delete the OldNode, just detaches it from
            this parent, potentially to be reattached elsewhere. 
            There is also an alternate version which drops a named
            child from this OldNode.
        */
        virtual OldNode* removeChild(unsigned short index);
        /** Drops the specified child from this OldNode. 
        @remarks
            Does not delete the OldNode, just detaches it from
            this parent, potentially to be reattached elsewhere. 
            There is also an alternate version which drops a named
            child from this OldNode.
        */
        virtual OldNode* removeChild(OldNode* child);

        /** Drops the named child from this OldNode. 
        @remarks
            Does not delete the OldNode, just detaches it from
            this parent, potentially to be reattached elsewhere.
        */
        virtual OldNode* removeChild(const String& name);
        /** Removes all child OldNodes attached to this OldNode. Does not delete the OldNodes, just detaches them from
            this parent, potentially to be reattached elsewhere.
        */
        virtual void removeAllChildren(void);
        
        /** Sets the final world position of the OldNode directly.
        @remarks 
            It's advisable to use the local setPosition if possible
        */
        virtual void _setDerivedPosition(const Vector3& pos);

        /** Sets the final world orientation of the OldNode directly.
        @remarks 
            It's advisable to use the local setOrientation if possible, this simply does
            the conversion for you.
        */
        virtual void _setDerivedOrientation(const Quaternion& q);

        /** Gets the orientation of the OldNode as derived from all parents.
        */
        virtual const Quaternion & _getDerivedOrientation(void) const;

        /** Gets the position of the OldNode as derived from all parents.
        */
        virtual const Vector3 & _getDerivedPosition(void) const;

        /** Gets the scaling factor of the OldNode as derived from all parents.
        */
        virtual const Vector3 & _getDerivedScale(void) const;

        /** Gets the full transformation matrix for this OldNode.
        @remarks
            This method returns the full transformation matrix
            for this OldNode, including the effect of any parent OldNode
            transformations, provided they have been updated using the OldNode::_update method.
            This should only be called by a SceneManager which knows the
            derived transforms have been updated before calling this method.
            Applications using Ogre should just use the relative transforms.
        */
        virtual const Matrix4& _getFullTransform(void) const;

        /** Internal method to update the OldNode.
        @note
            Updates this OldNode and any relevant children to incorporate transforms etc.
            Don't call this yourself unless you are writing a SceneManager implementation.
        @param updateChildren
            If @c true, the update cascades down to all children. Specify false if you wish to
            update children separately, e.g. because of a more selective SceneManager implementation.
        @param parentHasChanged
            This flag indicates that the parent transform has changed,
            so the child should retrieve the parent's transform and combine
            it with its own even if it hasn't changed itself.
        */
        virtual void _update(bool updateChildren, bool parentHasChanged);

        /** Sets a listener for this OldNode.
        @remarks
            Note for size and performance reasons only one listener per OldNode is
            allowed.
        */
        virtual void setListener(Listener* listener) { mListener = listener; }
        
        /** Gets the current listener for this OldNode.
        */
        virtual Listener* getListener(void) const { return mListener; }
        

        /** Sets the current transform of this OldNode to be the 'initial state' ie that
            position / orientation / scale to be used as a basis for delta values used
            in keyframe animation.
        @remarks
            You never need to call this method unless you plan to animate this OldNode. If you do
            plan to animate it, call this method once you've loaded the OldNode with it's base state,
            ie the state on which all keyframes are based.
        @par
            If you never call this method, the initial state is the identity transform, ie do nothing.
        */
        virtual void setInitialState(void);

        /** Resets the position / orientation / scale of this OldNode to it's initial state, see setInitialState for more info. */
        virtual void resetToInitialState(void);

        /** Gets the initial position of this OldNode, see setInitialState for more info. 
        @remarks
            Also resets the cumulative animation weight used for blending.
        */
        virtual const Vector3& getInitialPosition(void) const;
        
        /** Gets the local position, relative to this OldNode, of the given world-space position */
        virtual Vector3 convertWorldToLocalPosition( const Vector3 &worldPos );

        /** Gets the world position of a point in the OldNode local space
            useful for simple transforms that don't require a child OldNode.*/
        virtual Vector3 convertLocalToWorldPosition( const Vector3 &localPos );

        /** Gets the local orientation, relative to this OldNode, of the given world-space orientation */
        virtual Quaternion convertWorldToLocalOrientation( const Quaternion &worldOrientation );

        /** Gets the world orientation of an orientation in the OldNode local space
            useful for simple transforms that don't require a child OldNode.*/
        virtual Quaternion convertLocalToWorldOrientation( const Quaternion &localOrientation );

        /** Gets the initial orientation of this OldNode, see setInitialState for more info. */
        virtual const Quaternion& getInitialOrientation(void) const;

        /** Gets the initial position of this OldNode, see setInitialState for more info. */
        virtual const Vector3& getInitialScale(void) const;

        /** Helper function, get the squared view depth.  */
        virtual Real getSquaredViewDepth(const Camera* cam) const;

        /** To be called in the event of transform changes to this OldNode that require it's recalculation.
        @remarks
            This not only tags the OldNode state as being 'dirty', it also requests it's parent to 
            know about it's dirtiness so it will get an update next time.
        @param forceParentUpdate Even if the OldNode thinks it has already told it's
            parent, tell it anyway
        */
        virtual void needUpdate(bool forceParentUpdate = false);
        /** Called by children to notify their parent that they need an update. 
        @param forceParentUpdate Even if the OldNode thinks it has already told it's
            parent, tell it anyway
        */
        virtual void requestUpdate(OldNode* child, bool forceParentUpdate = false);
        /** Called by children to notify their parent that they no longer need an update. */
        virtual void cancelUpdate(OldNode* child);

        /** Queue a 'needUpdate' call to a OldNode safely.
        @remarks
            You can't call needUpdate() during the scene graph update, e.g. in
            response to a OldNode::Listener hook, because the graph is already being 
            updated, and update flag changes cannot be made reliably in that context. 
            Call this method if you need to queue a needUpdate call in this case.
        */
        static void queueNeedUpdate(OldNode* n);
        /** Process queued 'needUpdate' calls. */
        static void processQueuedUpdates(void);


        /** @deprecated use UserObjectBindings::setUserAny via getUserObjectBindings() instead.
            Sets any kind of user value on this object.
        @remarks
            This method allows you to associate any user value you like with 
            this OldNode. This can be a pointer back to one of your own
            classes for instance.
        */
        virtual void setUserAny(const Any& anything) { getUserObjectBindings().setUserAny(anything); }

        /** @deprecated use UserObjectBindings::getUserAny via getUserObjectBindings() instead.
            Retrieves the custom user value associated with this object.
        */
        virtual const Any& getUserAny(void) const { return getUserObjectBindings().getUserAny(); }

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

    };
    /** @} */
    /** @} */

}
} // namespace Ogre

#include "OgreHeaderSuffix.h"

#endif // _OldNode_H__
