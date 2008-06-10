/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#ifndef __ShadowCaster_H__
#define __ShadowCaster_H__

#include "OgrePrerequisites.h"
#include "OgreRenderable.h"


namespace Ogre {


    /** Class which represents the renderable aspects of a set of shadow volume faces. 
    @remarks
        Note that for casters comprised of more than one set of vertex buffers (e.g. SubMeshes each
        using their own geometry), it will take more than one ShadowRenderable to render the 
        shadow volume. Therefore for shadow caster geometry, it is best to stick to one set of
        vertex buffers (not necessarily one buffer, but the positions for the entire geometry 
        should come from one buffer if possible)
    */
    class _OgreExport ShadowRenderable : public Renderable, public ShadowRenderableAlloc
    {
    protected:
        MaterialPtr mMaterial;
        RenderOperation mRenderOp;
        ShadowRenderable* mLightCap; // used only if isLightCapSeparate == true
    public:
        ShadowRenderable() : mMaterial(), mLightCap(0) {}
        virtual ~ShadowRenderable() { delete mLightCap; }
        /** Set the material to be used by the shadow, should be set by the caller 
          before adding to a render queue
        */
        void setMaterial(const MaterialPtr& mat) { mMaterial = mat; }
        /// Overridden from Renderable
        const MaterialPtr& getMaterial(void) const { return mMaterial; }
        /// Overridden from Renderable
        void getRenderOperation(RenderOperation& op) { op = mRenderOp; }
        /// Get the internal render operation for set up
        RenderOperation* getRenderOperationForUpdate(void) {return &mRenderOp;}
        /// Overridden from Renderable
        void getWorldTransforms(Matrix4* xform) const = 0;
        /// Overridden from Renderable
        Real getSquaredViewDepth(const Camera*) const{ return 0; /* not used */}
        /// Overridden from Renderable
        const LightList& getLights(void) const;
        /** Does this renderable require a separate light cap?
        @remarks
            If possible, the light cap (when required) should be contained in the
            usual geometry of the shadow renderable. However, if for some reason
            the normal depth function (less than) could cause artefacts, then a
            separate light cap with a depth function of 'always fail' can be used 
            instead. The primary example of this is when there are floating point
            inaccuracies caused by calculating the shadow geometry separately from
            the real geometry. 
        */
        bool isLightCapSeparate(void) const { return mLightCap != 0; }

        /// Get the light cap version of this renderable
        ShadowRenderable* getLightCapRenderable(void) { return mLightCap; }
        /// Should this ShadowRenderable be treated as visible?
        virtual bool isVisible(void) const { return true; }

    };

    /** A set of flags that can be used to influence ShadowRenderable creation. */
    enum ShadowRenderableFlags
    {
        /// For shadow volume techniques only, generate a light cap on the volume
        SRF_INCLUDE_LIGHT_CAP = 0x00000001,
        /// For shadow volume techniques only, generate a dark cap on the volume
        SRF_INCLUDE_DARK_CAP  = 0x00000002,
        /// For shadow volume techniques only, indicates volume is extruded to infinity
        SRF_EXTRUDE_TO_INFINITY  = 0x00000004
    };

    /** This class defines the interface that must be implemented by shadow casters.
    */
    class _OgreExport ShadowCaster
    {
    public:
        virtual ~ShadowCaster() { }
        /** Returns whether or not this object currently casts a shadow. */
        virtual bool getCastShadows(void) const = 0;

        /** Returns details of the edges which might be used to determine a silhouette. */
        virtual EdgeData* getEdgeList(void) = 0;
		/** Returns whether the object has a valid edge list. */
		virtual bool hasEdgeList(void) = 0;

        /** Get the world bounding box of the caster. */
        virtual const AxisAlignedBox& getWorldBoundingBox(bool derive = false) const = 0;
        /** Gets the world space bounding box of the light cap */
        virtual const AxisAlignedBox& getLightCapBounds(void) const = 0;
        /** Gets the world space bounding box of the dark cap, as extruded using the light provided */
        virtual const AxisAlignedBox& getDarkCapBounds(const Light& light, Real dirLightExtrusionDist) const = 0;

        typedef std::vector<ShadowRenderable*> ShadowRenderableList;
        typedef VectorIterator<ShadowRenderableList> ShadowRenderableListIterator;

        /** Gets an iterator over the renderables required to render the shadow volume. 
        @remarks
            Shadowable geometry should ideally be designed such that there is only one
            ShadowRenderable required to render the the shadow; however this is not a necessary
            limitation and it can be exceeded if required.
        @param shadowTechnique The technique being used to generate the shadow
        @param light The light to generate the shadow from
        @param indexBuffer The index buffer to build the renderables into, 
            the current contents are assumed to be disposable.
        @param extrudeVertices If true, this means this class should extrude
            the vertices of the back of the volume in software. If false, it
            will not be done (a vertex program is assumed).
        @param extrusionDistance The distance to extrude the shadow volume
        @param flags Technique-specific flags, see ShadowRenderableFlags
        */
        virtual ShadowRenderableListIterator getShadowVolumeRenderableIterator(
            ShadowTechnique shadowTechnique, const Light* light, 
            HardwareIndexBufferSharedPtr* indexBuffer, 
            bool extrudeVertices, Real extrusionDistance, unsigned long flags = 0 ) = 0;

        /** Utility method for extruding vertices based on a light. 
        @remarks
            Unfortunately, because D3D cannot handle homogeneous (4D) position
            coordinates in the fixed-function pipeline (GL can, but we have to
            be cross-API), when we extrude in software we cannot extrude to 
            infinity the way we do in the vertex program (by setting w to
            0.0f). Therefore we extrude by a fixed distance, which may cause 
            some problems with larger scenes. Luckily better hardware (ie
            vertex programs) can fix this.
        @param vertexBuffer The vertex buffer containing ONLY xyz position
        values, which must be originalVertexCount * 2 * 3 floats long.
        @param originalVertexCount The count of the original number of
        vertices, i.e. the number in the mesh, not counting the doubling
        which has already been done (by VertexData::prepareForShadowVolume)
        to provide the extruded area of the buffer.
        @param lightPos 4D light position in object space, when w=0.0f this
        represents a directional light
        @param extrudeDist The distance to extrude
        */
        static void extrudeVertices(const HardwareVertexBufferSharedPtr& vertexBuffer, 
            size_t originalVertexCount, const Vector4& lightPos, Real extrudeDist);
        /** Get the distance to extrude for a point/spot light */
        virtual Real getPointExtrusionDistance(const Light* l) const = 0;
    protected:
        /// Helper method for calculating extrusion distance
        Real getExtrusionDistance(const Vector3& objectPos, const Light* light) const;
        /** Tells the caster to perform the tasks necessary to update the 
            edge data's light listing. Can be overridden if the subclass needs 
            to do additional things. 
        @param edgeData The edge information to update
        @param lightPos 4D vector representing the light, a directional light
            has w=0.0
       */
        virtual void updateEdgeListLightFacing(EdgeData* edgeData, 
            const Vector4& lightPos);

        /** Generates the indexes required to render a shadow volume into the 
            index buffer which is passed in, and updates shadow renderables
            to use it.
        @param edgeData The edge information to use
        @param indexBuffer The buffer into which to write data into; current 
            contents are assumed to be discardable.
        @param light The light, mainly for type info as silhouette calculations
            should already have been done in updateEdgeListLightFacing
        @param shadowRenderables A list of shadow renderables which has 
            already been constructed but will need populating with details of
            the index ranges to be used.
        @param flags Additional controller flags, see ShadowRenderableFlags
        */
        virtual void generateShadowVolume(EdgeData* edgeData, 
            const HardwareIndexBufferSharedPtr& indexBuffer, const Light* light,
            ShadowRenderableList& shadowRenderables, unsigned long flags);
        /** Utility method for extruding a bounding box. 
        @param box Original bounding box, will be updated in-place
        @param lightPos 4D light position in object space, when w=0.0f this
        represents a directional light
        @param extrudeDist The distance to extrude
        */
        virtual void extrudeBounds(AxisAlignedBox& box, const Vector4& lightPos, 
            Real extrudeDist) const;


    };
}

#endif
