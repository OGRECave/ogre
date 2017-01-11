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
#ifndef _SceneNode_H__
#define _SceneNode_H__

#include "OgrePrerequisites.h"

#include "OgreNode.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {

    typedef vector<Bone*>::type BoneVec;
    typedef map<SkeletonInstance*, BoneVec>::type BonesPerSkeletonInstance;

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Scene
    *  @{
    */
    /** Class representing a node in the scene graph.
        @remarks
            A SceneNode is a type of Node which is used to organise objects in a scene.
            It has the same hierarchical transformation properties of the generic Node class,
            but also adds the ability to attach world objects to the node, and stores hierarchical
            bounding volumes of the nodes in the tree.
            Child nodes are contained within the bounds of the parent, and so on down the
            tree, allowing for fast culling.
    */
    class _OgreExport SceneNode : public Node
    {
    public:
        typedef vector<MovableObject*>::type ObjectVec;
        typedef VectorIterator<ObjectVec> ObjectIterator;
        typedef ConstVectorIterator<ObjectVec> ConstObjectIterator;

    protected:
        ObjectVec mAttachments;
        BonesPerSkeletonInstance    mBoneChildren;

        /// SceneManager which created this node
        SceneManager* mCreator;

        /** See Node. */
        Node* createChildImpl( SceneMemoryMgrTypes sceneType );

        /// Whether to yaw around a fixed axis.
        bool mYawFixed;
        /// Fixed axis to yaw around
        Vector3 mYawFixedAxis;

        /** Retrieves a the iterator to an attached object.
        @remarks Retrieves by object name, see alternate version to retrieve by index.
        Retrieving by name forces a linear search O(N), prefer using the index, which is O(1)
        */
        ObjectVec::iterator getAttachedObjectIt( const String& name );
        ObjectVec::const_iterator getAttachedObjectIt( const String& name ) const;
    public:
        /** Constructor, only to be called by the creator SceneManager. */
        SceneNode( IdType id, SceneManager* creator, NodeMemoryManager *nodeMemoryManager,
                    SceneNode *parent );

        /** Don't use this constructor unless you know what you're doing.
            @See NodeMemoryManager::mDummyNode
        */
        SceneNode( const Transform &transformPtrs );

        virtual ~SceneNode();

        /// @copydoc Node::setStatic
        virtual bool setStatic( bool bStatic );

        /// @copydoc Node::_notifyStaticDirty
        virtual void _notifyStaticDirty(void) const;

        /** Adds an instance of a scene object to this node.
        @remarks
            Scene objects can include Entity objects, Camera objects, Light objects, 
            ParticleSystem objects etc. Anything that subclasses from MovableObject.
        */
        virtual_l2 void attachObject(MovableObject* obj);

        /** Reports the number of objects attached to this node.
        */
        size_t numAttachedObjects(void) const                       { return mAttachments.size(); }

        /** Retrieves a pointer to an attached object.
        @remarks Retrieves by index, see alternate version to retrieve by name. The index
        of an object may change as other objects are added / removed.
        */
        MovableObject* getAttachedObject( size_t index )            { return mAttachments[index]; }

        /** Retrieves a pointer to an attached object.
        @remarks Retrieves by object name, see alternate version to retrieve by index.
        Retrieving by name forces a linear search O(N), prefer using the index, which is O(1)
        */
        MovableObject* getAttachedObject( const String& name );

        /** Detaches an object by pointer.
        @remarks
            It's fast, takes only O(1)
        */
        virtual_l2 void detachObject(MovableObject* obj);

        /** Detaches all objects attached to this node.
        */
        virtual void detachAllObjects(void);

        /// Attaches a bone to this SceneNode. Don't use directly.
        /// @see SkeletonInstance::setSceneNodeAsParentOfBone
        virtual_l1 void _attachBone( SkeletonInstance *skeletonInstance, Bone *bone );

        /// Detaches a bone from this SceneNode. Don't use directly.
        /// @see SkeletonInstance::setSceneNodeAsParentOfBone
        virtual_l1 void _detachBone( SkeletonInstance *skeletonInstance, Bone *bone );

        /// Detaches all bones from this SceneNode that belong to the given SkeletonInstance.
        /// Don't use directly. @see SkeletonInstance::setSceneNodeAsParentOfBone
        virtual_l1 void _detachAllBones( SkeletonInstance *skeletonInstance );

        /// Detaches all bones from from this SceneNode. It is safe to use directly.
        virtual void detachAllBones(void);

        /// @copydoc Node::_callMemoryChangeListeners
        virtual void _callMemoryChangeListeners(void);

        /** Retrieves an iterator which can be used to efficiently step through the objects 
            attached to this node.
        @remarks
            This is a much faster way to go through <B>all</B> the objects attached to the node
            than using getAttachedObject. But the iterator returned is only valid until a change
            is made to the collection (ie an addition or removal) so treat the returned iterator
            as transient, and don't add / remove items as you go through the iterator, save changes
            until the end, or retrieve a new iterator after making the change. Making changes to
            the object returned through the iterator is OK though.
        */
        virtual ObjectIterator getAttachedObjectIterator(void);
        /** Retrieves an iterator which can be used to efficiently step through the objects 
            attached to this node.
        @remarks
            This is a much faster way to go through <B>all</B> the objects attached to the node
            than using getAttachedObject. But the iterator returned is only valid until a change
            is made to the collection (ie an addition or removal) so treat the returned iterator
            as transient, and don't add / remove items as you go through the iterator, save changes
            until the end, or retrieve a new iterator after making the change. Making changes to
            the object returned through the iterator is OK though.
        */
        virtual ConstObjectIterator getAttachedObjectIterator(void) const;

        /** Gets the creator of this scene node. 
        @remarks
            This method returns the SceneManager which created this node.
            This can be useful for destroying this node.
        */
        SceneManager* getCreator(void) const { return mCreator; }

        /** This method removes and destroys the child and all of its children.
        @remarks
            Unlike removeChild, which removes a single child from this
            node but does not destroy it, this method destroys the child
            and all of it's children. 
        @par
            Use this if you wish to recursively destroy a node as well as 
            detaching it from it's parent. Note that any objects attached to
            the nodes will be detached but will not themselves be destroyed.
        @param
            SceneNode, must be a child of ours
        */
        virtual void removeAndDestroyChild( SceneNode *sceneNode );

        /** Removes and destroys all children of this node.
        @remarks
            Use this to destroy all child nodes of this node and remove
            them from the scene graph. Note that all objects attached to this
            node will be detached but will not be destroyed.
        */
        virtual void removeAndDestroyAllChildren(void);

        /** Creates an unnamed new SceneNode as a child of this node.
        @param
            translate Initial translation offset of child relative to parent
        @param
            rotate Initial rotation relative to parent
        */
        virtual SceneNode* createChildSceneNode(
                SceneMemoryMgrTypes sceneType = SCENE_DYNAMIC,
                const Vector3& translate = Vector3::ZERO, 
                const Quaternion& rotate = Quaternion::IDENTITY );

        virtual void setListener( Listener* listener );

        /** Tells the node whether to yaw around it's own local Y axis or a fixed axis of choice.
        @remarks
        This method allows you to change the yaw behaviour of the node - by default, it
        yaws around it's own local Y axis when told to yaw with TS_LOCAL, this makes it
        yaw around a fixed axis. 
        You only really need this when you're using auto tracking (see setAutoTracking,
        because when you're manually rotating a node you can specify the TransformSpace
        in which you wish to work anyway.
        @param
        useFixed If true, the axis passed in the second parameter will always be the yaw axis no
        matter what the node orientation. If false, the node returns to it's default behaviour.
        @param
        fixedAxis The axis to use if the first parameter is true.
        */
        void setFixedYawAxis( bool useFixed, const Vector3& fixedAxis = Vector3::UNIT_Y );

        bool isYawFixed(void) const                                     { return mYawFixed; }

        /** Rotate the node around the Y-axis.
        */
        void yaw(const Radian& angle, TransformSpace relativeTo = TS_LOCAL);
        /** Sets the node's direction vector ie it's local -z.
        @remarks
        Note that the 'up' vector for the orientation will automatically be 
        recalculated based on the current 'up' vector (i.e. the roll will 
        remain the same). If you need more control, use setOrientation.
        @param x,y,z The components of the direction vector
        @param relativeTo The space in which this direction vector is expressed
        @param localDirectionVector The vector which normally describes the natural
        direction of the node, usually -Z
        */
        virtual void setDirection(Real x, Real y, Real z, 
            TransformSpace relativeTo = TS_LOCAL, 
            const Vector3& localDirectionVector = Vector3::NEGATIVE_UNIT_Z);

        /** Sets the node's direction vector ie it's local -z.
        @remarks
        Note that the 'up' vector for the orientation will automatically be 
        recalculated based on the current 'up' vector (i.e. the roll will 
        remain the same). If you need more control, use setOrientation.
        @param vec The direction vector
        @param relativeTo The space in which this direction vector is expressed
        @param localDirectionVector The vector which normally describes the natural
        direction of the node, usually -Z
        */
        virtual void setDirection(const Vector3& vec, TransformSpace relativeTo = TS_LOCAL, 
            const Vector3& localDirectionVector = Vector3::NEGATIVE_UNIT_Z);
        /** Points the local -Z direction of this node at a point in space.
        @param targetPoint A vector specifying the look at point.
        @param relativeTo The space in which the point resides
        @param localDirectionVector The vector which normally describes the natural
        direction of the node, usually -Z
        */
        virtual void lookAt( const Vector3& targetPoint, TransformSpace relativeTo,
            const Vector3& localDirectionVector = Vector3::NEGATIVE_UNIT_Z);
        /** Enables / disables automatic tracking of another SceneNode.
        @remarks
        If you enable auto-tracking, this SceneNode will automatically rotate to
        point it's -Z at the target SceneNode every frame, no matter how 
        it or the other SceneNode move. Note that by default the -Z points at the 
        origin of the target SceneNode, if you want to tweak this, provide a 
        vector in the 'offset' parameter and the target point will be adjusted.
        @param enabled If true, tracking will be enabled and the next 
        parameter cannot be null. If false tracking will be disabled and the 
        current orientation will be maintained.
        @param target Pointer to the SceneNode to track. Make sure you don't
        delete this SceneNode before turning off tracking (e.g. SceneManager::clearScene will
        delete it so be careful of this). Can be null if and only if the enabled param is false.
        @param localDirectionVector The local vector considered to be the usual 'direction'
        of the node; normally the local -Z but can be another direction.
        @param offset If supplied, this is the target point in local space of the target node
        instead of the origin of the target node. Good for fine tuning the look at point.
        */
        virtual void setAutoTracking(bool enabled, SceneNode* const target = 0, 
            const Vector3& localDirectionVector = Vector3::NEGATIVE_UNIT_Z,
            const Vector3& offset = Vector3::ZERO);

        /** Gets the parent of this SceneNode. */
        SceneNode* getParentSceneNode(void) const;
        /** Makes all objects attached to this node become visible / invisible.
        @remarks    
            This is a shortcut to calling setVisible() on the objects attached
            to this node, and optionally to all objects attached to child
            nodes. 
        @param visible Whether the objects are to be made visible or invisible
        @param cascade If true, this setting cascades into child nodes too.
        */
        virtual void setVisible(bool visible, bool cascade = true);
        /** Inverts the visibility of all objects attached to this node.
        @remarks    
        This is a shortcut to calling setVisible(!isVisible()) on the objects attached
        to this node, and optionally to all objects attached to child
        nodes. 
        @param cascade If true, this setting cascades into child nodes too.
        */
        virtual void flipVisibility(bool cascade = true);

        /// As Node::getDebugRenderable, except scaling is automatically determined
        //virtual DebugRenderable* getDebugRenderable();

        virtual NodeMemoryManager* getDefaultNodeMemoryManager( SceneMemoryMgrTypes sceneType );

#if OGRE_DEBUG_MODE
        virtual void _setCachedTransformOutOfDate(void);
#endif
    };
    /** @} */
    /** @} */


}// namespace

#include "OgreHeaderSuffix.h"

#endif
