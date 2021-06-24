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

#ifndef __OgreManualObject_H__
#define __OgreManualObject_H__

#include "OgrePrerequisites.h"
#include "OgreMovableObject.h"
#include "OgreRenderable.h"
#include "OgreResourceGroupManager.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Scene
    *  @{
    */
    /** Class providing a much simplified interface to generating manual
        objects with custom geometry.

        Building one-off geometry objects manually usually requires getting
        down and dirty with the vertex buffer and vertex declaration API, 
        which some people find a steep learning curve. This class gives you 
        a simpler interface specifically for the purpose of building a 
        3D object simply and quickly. Note that if you intend to instance your
        object you will still need to become familiar with the Mesh class. 

        This class draws heavily on the interface for OpenGL 
        immediate-mode (@c glBegin, @c glVertex, @c glNormal etc), since this
        is generally well-liked by people. There are a couple of differences
        in the results though - internally this class still builds hardware 
        buffers which can be re-used, so you can render the resulting object
        multiple times without re-issuing all the same commands again. 
        Secondly, the rendering is not immediate, it is still queued just like
        all OGRE objects. This makes this object more efficient than the 
        equivalent GL immediate-mode commands, so it's feasible to use it for
        large objects if you really want to.

        To construct some geometry with this object:
          -# If you know roughly how many vertices (and indices, if you use them)
             you're going to submit, call estimateVertexCount() and estimateIndexCount().
             This is not essential but will make the process more efficient by saving
             memory reallocations.
          -# Call begin() to begin entering data
          -# For each vertex, call position(), normal(), textureCoord(), colour()
             to define your vertex data. Note that each time you call position()
             you start a new vertex. Note that the first vertex defines the 
             components of the vertex - you can't add more after that. For example
             if you didn't call normal() in the first vertex, you cannot call it
             in any others. You ought to call the same combination of methods per
             vertex.
          -# If you want to define triangles (or lines/points) by indexing into the vertex list, 
             you can call index() as many times as you need to define them.
             If you don't do this, the class will assume you want triangles drawn
             directly as defined by the vertex list, i.e. non-indexed geometry. Note
             that stencil shadows are only supported on indexed geometry, and that
             indexed geometry is a little faster; so you should try to use it.
          -# Call end() to finish entering data.
          -# Optionally repeat the begin-end cycle if you want more geometry 
            using different rendering operation types, or different materials
        After calling end(), the class will organise the data for that section
        internally and make it ready to render with. Like any other 
        MovableObject you should attach the object to a SceneNode to make it 
        visible. Other aspects like the relative render order can be controlled
        using standard MovableObject methods like setRenderQueueGroup.

        You can also use beginUpdate() to alter the geometry later on if you wish.
        If you do this, you should call setDynamic(true) before your first call 
        to begin(), and also consider using estimateVertexCount() / estimateIndexCount()
        if your geometry is going to be growing, to avoid buffer recreation during
        growth.

        @note like all OGRE geometry, triangles should be specified in 
        anti-clockwise winding order (whether you're doing it with just
        vertices, or using indexes too). That is to say that the front of the
        face is the one where the vertices are listed in anti-clockwise order.
    */
    class _OgreExport ManualObject : public MovableObject
    {
    public:
        ManualObject(const String& name);
        virtual ~ManualObject();

        /** @copydoc MovableObject::_releaseManualHardwareResources */
        void _releaseManualHardwareResources() { clear(); }

        //pre-declare ManualObjectSection
        class ManualObjectSection;

        /** Completely clear the contents of the object.
        @remarks
            Clearing the contents of this object and rebuilding from scratch
            is not the optimal way to manage dynamic vertex data, since the 
            buffers are recreated. If you want to keep the same structure but
            update the content within that structure, use beginUpdate() instead 
            of clear() begin(). However if you do want to modify the structure 
            from time to time you can do so by clearing and re-specifying the data.
        */
        virtual void clear(void);
        
        /** Estimate the number of vertices ahead of time.
        @remarks
            Calling this helps to avoid memory reallocation when you define
            vertices. Also very handy when using beginUpdate() to manage dynamic
            data - you can make the vertex buffers a little larger than their
            initial needs to allow for growth later with this method.
        */
        virtual void estimateVertexCount(size_t vcount);

        /** Estimate the number of indices ahead of time.
        @remarks
            Calling this helps to avoid memory reallocation when you define
            indices. Also very handy when using beginUpdate() to manage dynamic
            data - you can make the index buffer a little larger than the
            initial need to allow for growth later with this method.
        */
        virtual void estimateIndexCount(size_t icount);

        /** Start defining a part of the object.
        @remarks
            Each time you call this method, you start a new section of the
            object with its own material and potentially its own type of
            rendering operation (triangles, points or lines for example).
        @param materialName The name of the material to render this part of the
            object with.
        @param opType The type of operation to use to render.
        @param groupName The resource group of the material to render this part
            of the object with.
        */
        virtual void begin(const String& materialName,
            RenderOperation::OperationType opType = RenderOperation::OT_TRIANGLE_LIST,
            const String& groupName = ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

        /** @overload
        @param mat The material to render this part of the object with.
        @param opType The type of operation to use to render.
        */
        virtual void begin(const MaterialPtr& mat,
            RenderOperation::OperationType opType = RenderOperation::OT_TRIANGLE_LIST);

        /** Use before defining geometry to indicate how you intend to update the
            geometry.
        */
        void setBufferUsage(HardwareBuffer::Usage usage) { mBufferUsage = usage; }

        /// @overload
        void setDynamic(bool dyn)
        {
            mBufferUsage =
                dyn ? HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY : HardwareBuffer::HBU_STATIC_WRITE_ONLY;
        }

        /** Gets whether this object is marked as dynamic */
        bool getDynamic() const { return mBufferUsage & HardwareBuffer::HBU_DYNAMIC; }

        /** Start the definition of an update to a part of the object.
        @remarks
            Using this method, you can update an existing section of the object
            efficiently. You do not have the option of changing the operation type
            obviously, since it must match the one that was used before. 
        @note If your sections are changing size, particularly growing, use
            estimateVertexCount and estimateIndexCount to pre-size the buffers a little
            larger than the initial needs to avoid buffer reconstruction.
        @param sectionIndex The index of the section you want to update. The first
            call to begin() would have created section 0, the second section 1, etc.
        */
        virtual void beginUpdate(size_t sectionIndex);
        /** Add a vertex position, starting a new vertex at the same time. 
        @remarks A vertex position is slightly special among the other vertex data
            methods like normal() and textureCoord(), since calling it indicates
            the start of a new vertex. All other vertex data methods you call 
            after this are assumed to be adding more information (like normals or
            texture coordinates) to the last vertex started with position().
        */
        void position(const Vector3& pos)
        {
            OgreAssert(mCurrentSection, "You must call begin() before this method");
            if (mTempVertexPending)
            {
                // bake current vertex
                copyTempVertexToBuffer();
                mFirstVertex = false;
            }

            if (mFirstVertex && !mCurrentUpdating)
            {
                declareElement(VET_FLOAT3, VES_POSITION);
            }

            mTempVertex.position = pos;

            // update bounds
            mAABB.merge(mTempVertex.position);
            mRadius = std::max(mRadius, mTempVertex.position.length());

            // reset current texture coord
            mTexCoordIndex = 0;

            mTempVertexPending = true;
        }
        /// @overload
        void position(float x, float y, float z) { position({x, y, z}); }

        /** Add a vertex normal to the current vertex.
        @remarks
            Vertex normals are most often used for dynamic lighting, and 
            their components should be normalised.
        */
        void normal(const Vector3& norm)
        {
            OgreAssert(mCurrentSection, "You must call begin() before this method");
            if (mFirstVertex && !mCurrentUpdating)
            {
                declareElement(VET_FLOAT3, VES_NORMAL);
            }
            mTempVertex.normal = norm;
        }
        /// @overload
        void normal(float x, float y, float z)  { normal({x, y, z}); }

        /** Add a vertex tangent to the current vertex.
        @remarks
            Vertex tangents are most often used for dynamic lighting, and 
            their components should be normalised. 
            Also, using tangent() you enable VES_TANGENT vertex semantic, which is not
            supported on old non-SM2 cards.
        */
        void tangent(const Vector3& tan)
        {
            OgreAssert(mCurrentSection, "You must call begin() before this method");
            if (mFirstVertex && !mCurrentUpdating)
            {
                declareElement(VET_FLOAT3, VES_TANGENT);
            }
            mTempVertex.tangent = tan;
        }

        /// @overload
        void tangent(float x, float y, float z)  { tangent({x, y, z}); }

        /** Add a texture coordinate to the current vertex.
        @remarks
            You can call this method multiple times between position() calls
            to add multiple texture coordinates to a vertex. Each one can have
            between 1 and 3 dimensions, depending on your needs, although 2 is
            most common. There are several versions of this method for the 
            variations in number of dimensions.
        */
        void textureCoord(float u)
        {
            OgreAssert(mCurrentSection, "You must call begin() before this method");
            if (mFirstVertex && !mCurrentUpdating)
            {
                declareElement(VET_FLOAT1, VES_TEXTURE_COORDINATES);
            }
            mTempVertex.texCoordDims[mTexCoordIndex] = 1;
            mTempVertex.texCoord[mTexCoordIndex].x = u;

            ++mTexCoordIndex;
        }
        /// @overload
        void textureCoord(float u, float v)
        {
            OgreAssert(mCurrentSection, "You must call begin() before this method");
            if (mFirstVertex && !mCurrentUpdating)
            {
                declareElement(VET_FLOAT2, VES_TEXTURE_COORDINATES);
            }
            mTempVertex.texCoordDims[mTexCoordIndex] = 2;
            mTempVertex.texCoord[mTexCoordIndex].x = u;
            mTempVertex.texCoord[mTexCoordIndex].y = v;

            ++mTexCoordIndex;
        }
        /// @overload
        void textureCoord(float u, float v, float w)
        {
            OgreAssert(mCurrentSection, "You must call begin() before this method");
            if (mFirstVertex && !mCurrentUpdating)
            {
                declareElement(VET_FLOAT3, VES_TEXTURE_COORDINATES);
            }
            mTempVertex.texCoordDims[mTexCoordIndex] = 3;
            mTempVertex.texCoord[mTexCoordIndex].x = u;
            mTempVertex.texCoord[mTexCoordIndex].y = v;
            mTempVertex.texCoord[mTexCoordIndex].z = w;

            ++mTexCoordIndex;
        }
        /// @overload
        void textureCoord(float x, float y, float z, float w) { textureCoord(Vector4(x, y, z, w)); }
        /// @overload
        void textureCoord(const Vector2& uv) { textureCoord(uv.x, uv.y); }
        /// @overload
        void textureCoord(const Vector3& uvw) { textureCoord(uvw.x, uvw.y, uvw.z); }
        /// @@overload
        void textureCoord(const Vector4& xyzw)
        {
            OgreAssert(mCurrentSection, "You must call begin() before this method");
            if (mFirstVertex && !mCurrentUpdating)
            {
                declareElement(VET_FLOAT4, VES_TEXTURE_COORDINATES);
            }
            mTempVertex.texCoordDims[mTexCoordIndex] = 4;
            mTempVertex.texCoord[mTexCoordIndex] = xyzw;

            ++mTexCoordIndex;
        }

        /** Add a vertex colour to a vertex.
        */
        void colour(const ColourValue& col)
        {
            OgreAssert(mCurrentSection, "You must call begin() before this method");
            if (mFirstVertex && !mCurrentUpdating)
            {
                declareElement(VET_COLOUR, VES_DIFFUSE);
            }
            mTempVertex.colour = col;
        }
        /// @overload
        void colour(float r, float g, float b, float a = 1.0f) { colour(ColourValue(r, g, b, a)); };

        /** Add a vertex index to construct faces / lines / points via indexing
            rather than just by a simple list of vertices. 
        @remarks
            You will have to call this 3 times for each face for a triangle list, 
            or use the alternative 3-parameter version. Other operation types
            require different numbers of indexes, @see RenderOperation::OperationType.
        @note
            32-bit indexes are not supported on all cards and will only be used
            when required, if an index is > 65535.
        @param idx A vertex index from 0 to 4294967295. 
        */
        void index(uint32 idx)
        {
            OgreAssert(mCurrentSection, "You must call begin() before this method");
            mAnyIndexed = true;
            if (idx >= 65536)
                mCurrentSection->set32BitIndices(true);

            // make sure we have index data
            RenderOperation* rop = mCurrentSection->getRenderOperation();
            if (!rop->indexData)
            {
                rop->indexData = OGRE_NEW IndexData();
                rop->indexData->indexCount = 0;
            }
            rop->useIndexes = true;
            resizeTempIndexBufferIfNeeded(++rop->indexData->indexCount);

            mTempIndexBuffer[rop->indexData->indexCount - 1] = idx;
        }
        /** Add a set of 3 vertex indices to construct a triangle; this is a
            shortcut to calling index() 3 times. It is only valid for triangle 
            lists.
        @note
            32-bit indexes are not supported on all cards and will only be used
            when required, if an index is > 65535.
        @param i1, i2, i3 3 vertex indices from 0 to 4294967295 defining a face.
        */
        void triangle(uint32 i1, uint32 i2, uint32 i3)
        {
            OgreAssert(mCurrentSection, "You must call begin() before this method");
            OgreAssert(mCurrentSection->getRenderOperation()->operationType ==
                           RenderOperation::OT_TRIANGLE_LIST,
                       "This method is only valid on triangle lists");
            index(i1);
            index(i2);
            index(i3);
        }
        /** Add a set of 4 vertex indices to construct a quad (out of 2 
            triangles); this is a shortcut to calling index() 6 times, 
            or triangle() twice. It's only valid for triangle list operations.
        @note
            32-bit indexes are not supported on all cards and will only be used
            when required, if an index is > 65535.
        @param i1, i2, i3, i4 4 vertex indices from 0 to 4294967295 defining a quad. 
        */
        void quad(uint32 i1, uint32 i2, uint32 i3, uint32 i4)
        {
            // first tri
            triangle(i1, i2, i3);
            // second tri
            triangle(i3, i4, i1);
        }

        /// Get the number of vertices in the section currently being defined (returns 0 if no section is in progress).
        virtual size_t getCurrentVertexCount() const;

        /// Get the number of indices in the section currently being defined (returns 0 if no section is in progress).
        virtual size_t getCurrentIndexCount() const;
        
        /** Finish defining the object and compile the final renderable version. 
        @note
            Will return a pointer to the finished section or NULL if the section was discarded (i.e. has zero vertices/indices).
        */
        virtual ManualObjectSection* end(void);

        /** Alter the material for a subsection of this object after it has been
            specified.
        @remarks
            You specify the material to use on a section of this object during the
            call to begin(), however if you want to change the material afterwards
            you can do so by calling this method.
        @param subIndex The index of the subsection to alter
        @param name The name of the new material to use
        @param group The resource group of the new material to use
        */
        void setMaterialName(size_t subIndex, const String& name,
            const String & group = ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME)
        {
            mSectionList.at(subIndex)->setMaterialName(name, group);
        }

        /** @overload
        @param subIndex The index of the subsection to alter
        @param mat The new material to use
        */
        void setMaterial(size_t subIndex, const MaterialPtr& mat)
        {
            mSectionList.at(subIndex)->setMaterial(mat);
        }

        /** Convert this object to a Mesh. 
        @remarks
            After you've finished building this object, you may convert it to 
            a Mesh if you want in order to be able to create many instances of
            it in the world (via Entity). This is optional, since this instance
            can be directly attached to a SceneNode itself, but of course only
            one instance of it can exist that way. 
        @note Only objects which use indexed geometry may be converted to a mesh.
        @param meshName The name to give the mesh
        @param groupName The resource group to create the mesh in
        */
        virtual MeshPtr convertToMesh(const String& meshName, 
            const String& groupName = ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

        /** Sets whether or not to use an 'identity' projection.
        @remarks
            Usually ManualObjects will use a projection matrix as determined
            by the active camera. However, if they want they can cancel this out
            and use an identity projection, which effectively projects in 2D using
            a {-1, 1} view space. Useful for overlay rendering. Normally you don't
            need to change this. The default is false.
        @see ManualObject::getUseIdentityProjection
        */
        void setUseIdentityProjection(bool useIdentityProjection);

        /** Returns whether or not to use an 'identity' projection.
        @remarks
            Usually ManualObjects will use a projection matrix as determined
            by the active camera. However, if they want they can cancel this out
            and use an identity projection, which effectively projects in 2D using
            a {-1, 1} view space. Useful for overlay rendering. Normally you don't
            need to change this.
        @see ManualObject::setUseIdentityProjection
        */
        bool getUseIdentityProjection(void) const { return mUseIdentityProjection; }

        /** Sets whether or not to use an 'identity' view.
        @remarks
            Usually ManualObjects will use a view matrix as determined
            by the active camera. However, if they want they can cancel this out
            and use an identity matrix, which means all geometry is assumed
            to be relative to camera space already. Useful for overlay rendering. 
            Normally you don't need to change this. The default is false.
        @see ManualObject::getUseIdentityView
        */
        void setUseIdentityView(bool useIdentityView);

        /** Returns whether or not to use an 'identity' view.
        @remarks
            Usually ManualObjects will use a view matrix as determined
            by the active camera. However, if they want they can cancel this out
            and use an identity matrix, which means all geometry is assumed
            to be relative to camera space already. Useful for overlay rendering. 
            Normally you don't need to change this.
        @see ManualObject::setUseIdentityView
        */
        bool getUseIdentityView(void) const { return mUseIdentityView; }

        /** Sets the bounding box.
            @remarks Call this after having finished creating sections to modify the
                bounding box. E.g. if you're using ManualObject to create 2D overlays
                you can call things function to set an infinite bounding box so that
                the object always stays visible when attached.
            @see ManualObject::setUseIdentityProjection, ManualObject::setUseIdentityView,
                AxisAlignedBox::setInfinite */
        void setBoundingBox(const AxisAlignedBox& box) { mAABB = box; }

        /** Gets the list of ManualObjectSection, i.e. a part of a ManualObject.
        */
        const std::vector<ManualObjectSection*>& getSections() const { return mSectionList; }

        ManualObjectSection* getSection(size_t index) const { return mSectionList.at(index); }

        size_t getNumSections(void) const { return mSectionList.size(); }


        /** Sets whether or not to keep the original declaration order when 
            queuing the renderables.
        @remarks
            This overrides the default behavior of the rendering queue, 
            specifically stating the desired order of rendering. Might result in a 
            performance loss, but lets the user to have more direct control when 
            creating geometry through this class.
        @param keepOrder Whether to keep the declaration order or not.
        */
        void setKeepDeclarationOrder(bool keepOrder) { mKeepDeclarationOrder = keepOrder; }

        /** Gets whether or not the declaration order is to be kept or not.
        @return A flag indication if the declaration order will be kept when 
            queuing the renderables.
        */
        bool getKeepDeclarationOrder() const { return mKeepDeclarationOrder; }
        // MovableObject overrides

        /** @copydoc MovableObject::getMovableType */
        const String& getMovableType(void) const;
        /** @copydoc MovableObject::getBoundingBox */
        const AxisAlignedBox& getBoundingBox(void) const override { return mAABB; }
        /** @copydoc MovableObject::getBoundingRadius */
        Real getBoundingRadius(void) const override { return mRadius; }
        /** @copydoc MovableObject::_updateRenderQueue */
        void _updateRenderQueue(RenderQueue* queue);
        /** Implement this method to enable stencil shadows */
        EdgeData* getEdgeList(void) override;
        /** Implement this method to enable stencil shadows. */
        const ShadowRenderableList& getShadowVolumeRenderableList(
            const Light* light, const HardwareIndexBufferPtr& indexBuffer,
            size_t& indexBufferUsedSize, float extrusionDist, int flags = 0) override;

        /// Built, renderable section of geometry
        class _OgreExport ManualObjectSection : public Renderable, public MovableAlloc
        {
        protected:
            ManualObject* mParent;
            String mMaterialName;
            String mGroupName;
            mutable MaterialPtr mMaterial;
            RenderOperation mRenderOperation;
            bool m32BitIndices;


        public:
            ManualObjectSection(ManualObject* parent, const String& materialName,
                RenderOperation::OperationType opType,
                const String & groupName = ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            /// @remark mat should not be null.
            ManualObjectSection(ManualObject* parent, const MaterialPtr& mat,
                RenderOperation::OperationType opType);
            virtual ~ManualObjectSection();

            /// Retrieve render operation for manipulation
            RenderOperation* getRenderOperation(void);
            /// Retrieve the material name in use
            const String& getMaterialName(void) const { return mMaterialName; }
            /// Retrieve the material group in use
            const String& getMaterialGroup(void) const { return mGroupName; }
            /// update the material name in use
            void setMaterialName(const String& name,
                const String& groupName = ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);
            /// Update the material in use
            /// @remark mat should not be null.
            void setMaterial(const MaterialPtr& mat);

            /// Set whether we need 32-bit indices
            void set32BitIndices(bool n32) { m32BitIndices = n32; }
            /// Get whether we need 32-bit indices
            bool get32BitIndices() const { return m32BitIndices; }
            
            // Renderable overrides
            /** @copydoc Renderable::getMaterial */
            const MaterialPtr& getMaterial(void) const;
            /** @copydoc Renderable::getRenderOperation */
            void getRenderOperation(RenderOperation& op);
            /** @copydoc Renderable::getWorldTransforms */
            void getWorldTransforms(Matrix4* xform) const;
            /** @copydoc Renderable::getSquaredViewDepth */
            Real getSquaredViewDepth(const Ogre::Camera *) const;
            /** @copydoc Renderable::getLights */
            const LightList &getLights(void) const;

            /// convert this section to a SubMesh
            void convertToSubMesh(SubMesh* sm) const;
                    
        };

        typedef std::vector<ManualObjectSection*> SectionList;

        /// @copydoc MovableObject::visitRenderables
        void visitRenderables(Renderable::Visitor* visitor, 
            bool debugRenderables = false);
        
        
    private:
        /// Dynamic?
        HardwareBuffer::Usage mBufferUsage;
        /// List of subsections
        SectionList mSectionList;
        /// Current section
        ManualObjectSection* mCurrentSection;
        /// Are we updating?
        bool mCurrentUpdating;
        /// Temporary vertex structure
        struct TempVertex
        {
            Vector3 position;
            Vector3 normal;
            Vector3 tangent;
            Vector4 texCoord[OGRE_MAX_TEXTURE_COORD_SETS];
            ushort texCoordDims[OGRE_MAX_TEXTURE_COORD_SETS];
            ColourValue colour;
        };
        /// Temp storage
        TempVertex mTempVertex;
        /// First vertex indicator
        bool mFirstVertex;
        /// Temp vertex data to copy?
        bool mTempVertexPending;
        /// System-memory buffer whilst we establish the size required
        char* mTempVertexBuffer;
        /// System memory allocation size, in bytes
        size_t mTempVertexSize;
        /// System-memory buffer whilst we establish the size required
        uint32* mTempIndexBuffer;
        /// System memory allocation size, in bytes
        size_t mTempIndexSize;
        /// Current declaration vertex size
        size_t mDeclSize;
        /// Estimated vertex count
        size_t mEstVertexCount;
        /// Estimated index count
        size_t mEstIndexCount;
        /// Current texture coordinate
        ushort mTexCoordIndex;
        /// Bounding box
        AxisAlignedBox mAABB;
        /// Bounding sphere
        Real mRadius;
        /// Any indexed geometry on any sections?
        bool mAnyIndexed;
        /// Edge list, used if stencil shadow casting is enabled 
        EdgeData* mEdgeList;
        /// List of shadow renderables
        ShadowRenderableList mShadowRenderables;
        /// Whether to use identity projection for sections
        bool mUseIdentityProjection;
        /// Whether to use identity view for sections
        bool mUseIdentityView;
        /// Keep declaration order or let the queue optimize it
        bool mKeepDeclarationOrder;


        /// Delete temp buffers and reset init counts
        virtual void resetTempAreas(void);
        /// Resize the temp vertex buffer?
        virtual void resizeTempVertexBufferIfNeeded(size_t numVerts);
        /// Resize the temp index buffer?
        virtual void resizeTempIndexBufferIfNeeded(size_t numInds);

        /// Copy current temp vertex into buffer
        virtual void copyTempVertexToBuffer(void);

    private:
        void declareElement(VertexElementType t, VertexElementSemantic s);
    };


    /** Factory object for creating ManualObject instances */
    class _OgreExport ManualObjectFactory : public MovableObjectFactory
    {
    protected:
        MovableObject* createInstanceImpl( const String& name, const NameValuePairList* params);
    public:
        ManualObjectFactory() {}
        ~ManualObjectFactory() {}

        static String FACTORY_TYPE_NAME;

        const String& getType(void) const;
    };
    /** @} */
    /** @} */
}

#include "OgreHeaderSuffix.h"

#endif


