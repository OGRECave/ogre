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
#include "OgreCommon.h"

#include "OgreNode.h"
#include "OgreAxisAlignedBox.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {

    // forward decl
    struct VisibleObjectsBoundsInfo;
    template <typename T> class ConstVectorIterator;
    template <typename T> class VectorIterator;

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Scene
    *  @{
    */
    /** Class representing a node in the scene graph.

        A SceneNode is a type of Node which is used to organise objects in a scene.
        It has the same hierarchical transformation properties of the generic Node class,
        but also adds the ability to attach world objects to the node, and stores hierarchical
        bounding volumes of the nodes in the tree.
        Child nodes are contained within the bounds of the parent, and so on down the
        tree, allowing for fast culling.
    */
    class _OgreExport SceneNode : public Node
    {
        friend class SceneManager;
    public:
        typedef std::vector<MovableObject*> ObjectMap;
        typedef VectorIterator<ObjectMap> ObjectIterator;
        typedef ConstVectorIterator<ObjectMap> ConstObjectIterator;

    protected:
        ObjectMap mObjectsByName;

        /// SceneManager which created this node
        SceneManager* mCreator;

        /// Auto tracking target
        SceneNode* mAutoTrackTarget;

        /** Index in the vector holding this node reference. Used for O(1) removals.

            It is the parent (or our creator) the one that sets this value, not ourselves. Do NOT modify
            it manually.
        */
        size_t mGlobalIndex;

        /// World-Axis aligned bounding box, updated only through _update
        AxisAlignedBox mWorldAABB;

        void updateFromParentImpl(void) const override;

        /** See Node */
        void setParent(Node* parent) override;

        /// Tracking offset for fine tuning
        Vector3 mAutoTrackOffset;
        /// Local 'normal' direction vector
        Vector3 mAutoTrackLocalDirection;
        /// Fixed axis to yaw around
        Vector3 mYawFixedAxis;

        /// Whether to yaw around a fixed axis.
        bool mYawFixed : 1;
        /// Is this node a current part of the scene graph?
        bool mIsInSceneGraph : 1;
    private: // private in 1.13
        /// Flag that determines if the bounding box of the node should be displayed
        bool mShowBoundingBox : 1;

        /** Internal method for setting whether the node is in the scene
            graph.
        */
        virtual void setInSceneGraph(bool inGraph);
        /** See Node. */
        Node* createChildImpl(void) override;

        /** See Node. */
        Node* createChildImpl(const String& name) override;
    public:
        /** Constructor, only to be called by the creator SceneManager.

            Creates a node with a specified name.
        */
        SceneNode(SceneManager* creator, const String& name = "");
        ~SceneNode();

        /** Adds an instance of a scene object to this node.

            Scene objects can include Entity objects, Camera objects, Light objects, 
            ParticleSystem objects etc. Anything that subclasses from MovableObject.
        */
        virtual void attachObject(MovableObject* obj);

        /** Reports the number of objects attached to this node.
        */
        size_t numAttachedObjects(void) const { return mObjectsByName.size(); }

        /** Retrieves a pointer to an attached object by index
        @note The index of an object may change as other objects are added / removed.
        */
        MovableObject* getAttachedObject(size_t index) const { return mObjectsByName.at(index); }

        /** Retrieves a pointer to an attached object by name
        */
        MovableObject* getAttachedObject(const String& name) const;

        /** Detaches the indexed object from this scene node.

            Detaches by index, see the alternate version to detach by name. Object indexes
            may change as other objects are added / removed.
        */
        virtual MovableObject* detachObject(unsigned short index);
        /** Detaches an object by pointer. */
        virtual void detachObject(MovableObject* obj);

        /** Detaches the named object from this node and returns a pointer to it. */
        virtual MovableObject* detachObject(const String& name);

        /** Detaches all objects attached to this node.
        */
        virtual void detachAllObjects(void);

        /** Detaches and destroys all objects attached to this node.
         *
         * Does not destroy objects attached to children of this node
        */
        void destroyAllObjects(void);

        /** Determines whether this node is in the scene graph, i.e.
            whether it's ultimate ancestor is the root scene node.
        */
        bool isInSceneGraph(void) const { return mIsInSceneGraph; }

        /** Notifies this SceneNode that it is the root scene node. 

            Only SceneManager should call this!
        */
        void _notifyRootNode(void) { mIsInSceneGraph = true; }
            

        /** Internal method to update the Node.
            @note
                Updates this scene node and any relevant children to incorporate transforms etc.
                Don't call this yourself unless you are writing a SceneManager implementation.
            @param
                updateChildren If true, the update cascades down to all children. Specify false if you wish to
                update children separately, e.g. because of a more selective SceneManager implementation.
            @param
                parentHasChanged This flag indicates that the parent transform has changed,
                    so the child should retrieve the parent's transform and combine it with its own
                    even if it hasn't changed itself.
        */
        void _update(bool updateChildren, bool parentHasChanged) override;

        /** Tells the SceneNode to update the world bound info it stores.
        */
        virtual void _updateBounds(void);

        /** Internal method which locates any visible objects attached to this node and adds them to the passed in queue.

            Should only be called by a SceneManager implementation, and only after the _updat method has been called to
            ensure transforms and world bounds are up to date.
            SceneManager implementations can choose to let the search cascade automatically, or choose to prevent this
            and select nodes themselves based on some other criteria.
            @param
                cam The active camera
            @param
                queue The SceneManager's rendering queue
            @param
                visibleBounds bounding information created on the fly containing all visible objects by the camera
            @param
                includeChildren If true, the call is cascaded down to all child nodes automatically.
            @param
                displayNodes If true, the nodes themselves are rendered as a set of 3 axes as well
                    as the objects being rendered. For debugging purposes.
            @param onlyShadowCasters
        */
        void _findVisibleObjects(Camera* cam, RenderQueue* queue,
            VisibleObjectsBoundsInfo* visibleBounds, 
            bool includeChildren = true, bool displayNodes = false, bool onlyShadowCasters = false);

        /** Gets the axis-aligned bounding box of this node (and hence all subnodes).

            Recommended only if you are extending a SceneManager, because the bounding box returned
            from this method is only up to date after the SceneManager has called _update.
        */
        const AxisAlignedBox& _getWorldAABB(void) const { return mWorldAABB; }

        /// @deprecated use getAttachedObjects()
        OGRE_DEPRECATED ObjectIterator getAttachedObjectIterator(void);
        /// @deprecated use getAttachedObjects()
        OGRE_DEPRECATED ConstObjectIterator getAttachedObjectIterator(void) const;

        /** The MovableObjects attached to this node
         *
         * This is a much faster way to go through <B>all</B> the objects attached to the node than
         * using getAttachedObject.
         */
        const ObjectMap& getAttachedObjects() const {
            return mObjectsByName;
        }

        /** Gets the creator of this scene node. 

            This method returns the SceneManager which created this node.
            This can be useful for destroying this node.
        */
        SceneManager* getCreator(void) const { return mCreator; }

        /** This method removes and destroys the named child and all of its children.

            Unlike removeChild, which removes a single named child from this
            node but does not destroy it, this method destroys the child
            and all of it's children. 
        @par
            Use this if you wish to recursively destroy a node as well as
            detaching it from it's parent. Note that any objects attached to
            the nodes will be detached but will not themselves be destroyed.
        */
        void removeAndDestroyChild(const String& name);

        /// @overload
        void removeAndDestroyChild(unsigned short index);

        /// @overload
        void removeAndDestroyChild(SceneNode* child);


        /** Removes and destroys all children of this node.

            Use this to destroy all child nodes of this node and remove
            them from the scene graph. Note that all objects attached to this
            node will be detached but will not be destroyed.
        */
        void removeAndDestroyAllChildren(void);

        /** Removes and destroys the child and all movable objects attached to the child,
         * and does the same to any children of that child node.
         *
         * Does **not** destroy animation, textures, meshes associated with those movable objects
         * */
        void destroyChildAndObjects(const String& name);
        ///@overload
        void destroyChildAndObjects(unsigned short index);
        ///@overload
        void destroyChildAndObjects(SceneNode * child);

        /** Destroys everything attatched to or decended from this node
         * @par
         * Detaches and destroys all objects attached to this node or
         * its children.
         * Removes and destroys all the children of this node
         * @par
         * Use this method to complete destroy a node, for example,
         * if you want to recreate its render tree from scratch.
         * @par
         * Does **not** destroy animations, textures, meshes associated with those movable objects
         * Does not destroy the node itself
         * */
        void destroyAllChildrenAndObjects();

        /**
         * Load a scene from a file as children of this node
         *
         * The file and any referenced resources will be searched in @ref ResourceGroupManager::getWorldResourceGroupName
         * Depending on the type of SceneManager you can load different scene file-formats.
         * @param filename source file containing the scene structure
         */
        void loadChildren(const String& filename);

        /**
         * Save the scene hierarchy starting at this node to file
         *
         * @param filename destination file
         */
        void saveChildren(const String& filename);

        /** Allows the showing of the node's bounding box.

            Use this to show or hide the bounding box of the node.
        */
        void showBoundingBox(bool bShow) { mShowBoundingBox = bShow; }

        /** This allows scene managers to determine if the node's bounding box
            should be added to the rendering queue.

            Scene Managers that implement their own _findVisibleObjects will have to 
            check this flag and then use _addBoundingBoxToQueue to add the bounding box
            wireframe.
        */
        bool getShowBoundingBox() const { return mShowBoundingBox; }

        /** Creates an unnamed new SceneNode as a child of this node.
        @param
            translate Initial translation offset of child relative to parent
        @param
            rotate Initial rotation relative to parent
        */
        virtual SceneNode* createChildSceneNode(
            const Vector3& translate = Vector3::ZERO, 
            const Quaternion& rotate = Quaternion::IDENTITY );

        /** Creates a new named SceneNode as a child of this node.

            This creates a child node with a given name, which allows you to look the node up from 
            the parent which holds this collection of nodes.
            @param name name of the node
            @param
                translate Initial translation offset of child relative to parent
            @param
                rotate Initial rotation relative to parent
        */
        virtual SceneNode* createChildSceneNode(const String& name, const Vector3& translate = Vector3::ZERO, const Quaternion& rotate = Quaternion::IDENTITY);

        /** Allows retrieval of the nearest lights to the centre of this SceneNode.

            This method allows a list of lights, ordered by proximity to the centre
            of this SceneNode, to be retrieved. Can be useful when implementing
            MovableObject::queryLights and Renderable::getLights.
        @par
            Note that only lights could be affecting the frustum will take into
            account, which cached in scene manager.
        @see SceneManager::_getLightsAffectingFrustum
        @see SceneManager::_populateLightList
        @param destList List to be populated with ordered set of lights; will be
            cleared by this method before population.
        @param radius Parameter to specify lights intersecting a given radius of
            this SceneNode's centre.
        @param lightMask The mask with which to include / exclude lights
        */
        void findLights(LightList& destList, Real radius, uint32 lightMask = 0xFFFFFFFF) const;

        /** Tells the node whether to yaw around it's own local Y axis or a fixed axis of choice.

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

        /** Rotate the node around the Y-axis.
        */
        void yaw(const Radian& angle, TransformSpace relativeTo = TS_LOCAL) override;
        /** Sets the node's direction vector ie it's local -z.

        Note that the 'up' vector for the orientation will automatically be 
        recalculated based on the current 'up' vector (i.e. the roll will 
        remain the same). If you need more control, use setOrientation.
        @param x,y,z The components of the direction vector
        @param relativeTo The space in which this direction vector is expressed
        @param localDirectionVector The vector which normally describes the natural
        direction of the node, usually -Z
        */
        void setDirection(Real x, Real y, Real z,
            TransformSpace relativeTo = TS_PARENT,
            const Vector3& localDirectionVector = Vector3::NEGATIVE_UNIT_Z);

        /// @overload
        void setDirection(const Vector3& vec, TransformSpace relativeTo = TS_PARENT,
            const Vector3& localDirectionVector = Vector3::NEGATIVE_UNIT_Z);
        /** Points the local -Z direction of this node at a point in space.
        @param targetPoint A vector specifying the look at point.
        @param relativeTo The space in which the point resides
        @param localDirectionVector The vector which normally describes the natural
        direction of the node, usually -Z
        */
        void lookAt( const Vector3& targetPoint, TransformSpace relativeTo,
            const Vector3& localDirectionVector = Vector3::NEGATIVE_UNIT_Z);
        /** Enables / disables automatic tracking of another SceneNode.

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
        void setAutoTracking(bool enabled, SceneNode* const target = 0,
            const Vector3& localDirectionVector = Vector3::NEGATIVE_UNIT_Z,
            const Vector3& offset = Vector3::ZERO);
        /** Get the auto tracking target for this node, if any. */
        SceneNode* getAutoTrackTarget(void) const { return mAutoTrackTarget; }
        /** Get the auto tracking offset for this node, if the node is auto tracking. */
        const Vector3& getAutoTrackOffset(void) const { return mAutoTrackOffset; }
        /** Get the auto tracking local direction for this node, if it is auto tracking. */
        const Vector3& getAutoTrackLocalDirection(void) const { return mAutoTrackLocalDirection; }
        /** Internal method used by OGRE to update auto-tracking cameras. */
        void _autoTrack(void);
        /** Gets the parent of this SceneNode. */
        SceneNode* getParentSceneNode(void) const;
        /** Makes all objects attached to this node become visible / invisible.

            This is a shortcut to calling setVisible() on the objects attached
            to this node, and optionally to all objects attached to child
            nodes. 
        @param visible Whether the objects are to be made visible or invisible
        @param cascade If true, this setting cascades into child nodes too.
        */
        void setVisible(bool visible, bool cascade = true) const;
        /** Inverts the visibility of all objects attached to this node.

        This is a shortcut to calling setVisible(!isVisible()) on the objects attached
        to this node, and optionally to all objects attached to child
        nodes. 
        @param cascade If true, this setting cascades into child nodes too.
        */
        void flipVisibility(bool cascade = true) const;

        /** Tells all objects attached to this node whether to display their
            debug information or not.

            This is a shortcut to calling setDebugDisplayEnabled() on the objects attached
            to this node, and optionally to all objects attached to child
            nodes. 
        @param enabled Whether the objects are to display debug info or not
        @param cascade If true, this setting cascades into child nodes too.
        */
        void setDebugDisplayEnabled(bool enabled, bool cascade = true) const;
    };
    /** @} */
    /** @} */


}// namespace

#include "OgreHeaderSuffix.h"

#endif
