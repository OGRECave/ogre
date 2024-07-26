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
#ifndef __SubEntity_H__
#define __SubEntity_H__

#include "OgrePrerequisites.h"

#include "OgreRenderable.h"
#include "OgreHardwareBufferManager.h"
#include "OgreResourceGroupManager.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Scene
    *  @{
    */
    /** Utility class which defines the sub-parts of an Entity.

        Just as meshes are split into submeshes, an Entity is made up of
        potentially multiple SubMeshes. These are mainly here to provide the
        link between the Material which the SubEntity uses (which may be the
        default Material for the SubMesh or may have been changed for this
        object) and the SubMesh data.
        @par
            The SubEntity also allows the application some flexibility in the
            material properties for this section of a particular instance of this
            Mesh, e.g. tinting the windows on a car model.
        @par
            SubEntity instances are never created manually. They are created at
            the same time as their parent Entity by the SceneManager method
            createEntity.
    */
    class _OgreExport SubEntity: public Renderable, public SubEntityAlloc
    {
        // Note no virtual functions for efficiency
        friend class Entity;
        friend class SceneManager;
    private:
        /** Private constructor - don't allow creation by anybody else.
        */
        SubEntity(Entity* parent, SubMesh* subMeshBasis);
        ~SubEntity();

        /// Pointer to parent.
        Entity* mParentEntity;

        /// Cached pointer to material.
        MaterialPtr mMaterialPtr;

        /// Pointer to the SubMesh defining geometry.
        SubMesh* mSubMesh;

        /// override the start index for the RenderOperation
        uint32 mIndexStart;
        /// override the end index for the RenderOperation
        uint32 mIndexEnd;

        /// Is this SubEntity visible?
        bool mVisible;

        /// The render queue to use when rendering this renderable
        uint8 mRenderQueueID;
        /// Flags whether the RenderQueue's default should be used.
        bool mRenderQueueIDSet;
        /// Flags whether the RenderQueue's default should be used.
        bool mRenderQueuePrioritySet;
        /// The render queue priority to use when rendering this renderable
        ushort mRenderQueuePriority;
        /// Blend buffer details for dedicated geometry
        std::unique_ptr<VertexData> mSkelAnimVertexData;
        /// Quick lookup of buffers
        Entity::TempBlendedBufferInfo mTempSkelAnimInfo;
        /// Temp buffer details for software Vertex anim geometry
        Entity::TempBlendedBufferInfo mTempVertexAnimInfo;
        /// Vertex data details for software Vertex anim of shared geometry
        std::unique_ptr<VertexData> mSoftwareVertexAnimVertexData;
        /// Vertex data details for hardware Vertex anim of shared geometry
        /// - separate since we need to s/w anim for shadows whilst still altering
        ///   the vertex data for hardware morphing (pos2 binding)
        std::unique_ptr<VertexData> mHardwareVertexAnimVertexData;
        /// Cached distance to last camera for getSquaredViewDepth
        mutable Real mCachedCameraDist;
        /// Number of hardware blended poses supported by material
        ushort mHardwarePoseCount;
        /// Have we applied any vertex animation to geometry?
        bool mVertexAnimationAppliedThisFrame;
        /// The camera for which the cached distance is valid
        mutable const Camera *mCachedCamera;

        /** Internal method for preparing this Entity for use in animation. */
        void prepareTempBlendBuffers(void);

    public:
        /** Gets the name of the Material in use by this instance.
        */
        const String& getMaterialName() const;

        /** Sets the name of the Material to be used.

            By default a SubEntity uses the default Material that the SubMesh
            uses. This call can alter that so that the Material is different
            for this instance.
        */
        void setMaterialName( const String& name, const String& groupName = ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME );

        /// @copydoc setMaterialName
        void setMaterial( const MaterialPtr& material );

        /** Tells this SubEntity whether to be visible or not. */
        void setVisible(bool visible);

        /** Returns whether or not this SubEntity is supposed to be visible. */
        bool isVisible(void) const { return mVisible; }

        /** Sets the render queue group this SubEntity will be rendered through.

            Render queues are grouped to allow you to more tightly control the ordering
            of rendered objects. If you do not call this method, the SubEntity will use
            either the Entity's queue or it will use the default
            (RenderQueue::getDefaultQueueGroup).
        @par
            See Entity::setRenderQueueGroup for more details.
        @param queueID Enumerated value of the queue group to use. See the
            enum RenderQueueGroupID for what kind of values can be used here.
        */
        void setRenderQueueGroup(uint8 queueID);

        /** Sets the render queue group and group priority this SubEntity will be rendered through.

            Render queues are grouped to allow you to more tightly control the ordering
            of rendered objects. Within a single render group there another type of grouping
            called priority which allows further control.  If you do not call this method, 
            all Entity objects default to the default queue and priority 
            (RenderQueue::getDefaultQueueGroup, RenderQueue::getDefaultRenderablePriority).
        @par
            See Entity::setRenderQueueGroupAndPriority for more details.
        @param queueID Enumerated value of the queue group to use. See the
            enum RenderQueueGroupID for what kind of values can be used here.
        @param priority The priority within a group to use.
        */
        void setRenderQueueGroupAndPriority(uint8 queueID, ushort priority);

        /** Gets the queue group for this entity, see setRenderQueueGroup for full details. */
        uint8 getRenderQueueGroup(void) const { return mRenderQueueID; }

        /** Gets the queue group for this entity, see setRenderQueueGroup for full details. */
        ushort getRenderQueuePriority(void) const { return mRenderQueuePriority; }

        /** Gets the queue group for this entity, see setRenderQueueGroup for full details. */
        bool isRenderQueueGroupSet(void) const { return mRenderQueueIDSet; }

        /** Gets the queue group for this entity, see setRenderQueueGroup for full details. */
        bool isRenderQueuePrioritySet(void) const { return mRenderQueuePrioritySet; }

        /** Accessor method to read mesh data.
        */
        SubMesh* getSubMesh(void) const { return mSubMesh; }

        /** Accessor to get parent Entity */
        Entity* getParent(void) const { return mParentEntity; }


        const MaterialPtr& getMaterial(void) const override { return mMaterialPtr; }
        void getRenderOperation(RenderOperation& op) override;

        /// @deprecated api about to be removed
        OGRE_DEPRECATED void setIndexDataStartIndex(uint32 start_index);

        /// @deprecated api about to be removed
        OGRE_DEPRECATED uint32 getIndexDataStartIndex() const { return mIndexStart; }

        /// @deprecated api about to be removed
        OGRE_DEPRECATED void setIndexDataEndIndex(uint32 end_index);

        /// @deprecated api about to be removed
        OGRE_DEPRECATED uint32 getIndexDataEndIndex() const { return mIndexEnd; }

        /** Reset the custom start/end index to the default values.
        */
        void resetIndexDataStartEndIndex();

        void getWorldTransforms(Matrix4* xform) const override;
        unsigned short getNumWorldTransforms(void) const override;
        Real getSquaredViewDepth(const Camera* cam) const override;
        const LightList& getLights(void) const override;
        bool getCastsShadows(void) const override;
        /** Advanced method to get the temporarily blended vertex information
        for entities which are software skinned. 

        Internal engine will eliminate software animation if possible, this
        information is unreliable unless added request for software animation
        via Entity::addSoftwareAnimationRequest.
        @note
            The positions/normals of the returned vertex data is in object space.
        */
        VertexData* _getSkelAnimVertexData(void);
        /** Advanced method to get the temporarily blended software morph vertex information

            Internal engine will eliminate software animation if possible, this
            information is unreliable unless added request for software animation
            via Entity::addSoftwareAnimationRequest.
        @note
            The positions/normals of the returned vertex data is in object space.
        */
        VertexData* _getSoftwareVertexAnimVertexData(void);
        /** Advanced method to get the hardware morph vertex information
        @note
            The positions/normals of the returned vertex data is in object space.
        */
        VertexData* _getHardwareVertexAnimVertexData(void);
        /// Retrieve the VertexData which should be used for GPU binding
        VertexData* getVertexDataForBinding(void);

        /** Mark all vertex data as so far unanimated. 
        */
        void _markBuffersUnusedForAnimation(void);
        /** Mark all vertex data as animated. 
        */
        void _markBuffersUsedForAnimation(void);
        /** Are buffers already marked as vertex animated? */
        bool _getBuffersMarkedForAnimation(void) const { return mVertexAnimationAppliedThisFrame; }
        /** Internal method to copy original vertex data to the morph structures
        should there be no active animation in use.
        */
        void _restoreBuffersForUnusedAnimation(bool hardwareAnimation);

        /** Overridden from Renderable to provide some custom behaviour. */
        void _updateCustomGpuParameter(
            const GpuProgramParameters::AutoConstantEntry& constantEntry,
            GpuProgramParameters* params) const override;

        /** Invalidate the camera distance cache */
        void _invalidateCameraCache ()
        { mCachedCamera = 0; }
    };
    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
