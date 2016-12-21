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
#ifndef _OgreSubMesh2_H_
#define _OgreSubMesh2_H_

#include "OgrePrerequisites.h"

#include "OgreVertexBoneAssignment.h"
#include "Vao/OgreVertexArrayObject.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {

    typedef FastArray<VertexArrayObject*> VertexArrayObjectArray;

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Resources
    *  @{
    */
    /** Defines a part of a complete mesh.
        @remarks
            Meshes which make up the definition of a discrete 3D object
            are made up of potentially multiple parts. This is because
            different parts of the mesh may use different materials or
            use different vertex formats, such that a rendering state
            change is required between them.
        @par
            Like the Mesh class, instantiations of 3D objects in the scene
            share the SubMesh instances, and have the option of overriding
            their material differences on a per-object basis if required.
            See the SubEntity class for more information.
    */
    class _OgreExport SubMesh : public SubMeshAlloc
    {
        friend class Mesh;
        friend class MeshSerializerImpl;
    public:
        typedef FastArray<unsigned short> IndexMap;
        typedef vector<VertexBoneAssignment>::type VertexBoneAssignmentVec;

        /// VAO to render the submesh. One per LOD level. Each LOD may or
        /// may not share the vertex and index buffers the other levels
        /// [0] = Used for regular rendering
        /// [1] = Used for shadow map caster passes
        /// Note that mVao[1] = mVao[0] is valid.
        /// But if they're not exactly the same VertexArrayObject pointers,
        /// then they won't share any pointer.
        VertexArrayObjectArray  mVao[NumVertexPass];

        /** Dedicated index map for translate blend index to bone index
            @par
                We collect actually used bones of all bone assignments, and build the
                blend index in 'packed' form, then the range of the blend index in vertex
                data VES_BLEND_INDICES element is continuous, with no gaps. Thus, by
                minimising the world matrix array constants passing to GPU, we can support
                more bones for a mesh when hardware skinning is used. The hardware skinning
                support limit is applied to each set of vertex data in the mesh, in other words, the
                hardware skinning support limit is applied only to the actually used bones of each
                SubMeshes, not all bones across the entire Mesh.
            @par
                Because the blend index is different to the bone index, therefore, we use
                the index map to translate the blend index to bone index.
            @par
                The use of shared or non-shared index map is determined when
                model data is converted to the OGRE .mesh format.
        */
        IndexMap mBlendIndexToBoneIndexMap;

        /// Name of the material this SubMesh uses.
        String  mMaterialName;

        /// Reference to parent Mesh (not a smart pointer so child does not keep parent alive).
        Mesh    *mParent;

    protected:
        VertexBoneAssignmentVec mBoneAssignments;

        /// Flag indicating that bone assignments need to be recompiled
        bool    mBoneAssignmentsOutOfDate;

        /// Limits mBoneAssignments to OGRE_MAX_BLEND_WEIGHTS and
        /// if we need to strip, normalizes all weights to sum 1.
        uint8 rationaliseBoneAssignments(void);

    public:
        SubMesh();
        ~SubMesh();

        /** Assigns a vertex to a bone with a given weight, for skeletal animation. 
        @remarks    
            This method is only valid after calling setSkeletonName.
            Since this is a one-off process there exists only 'addBoneAssignment' and
            'clearBoneAssignments' methods, no 'editBoneAssignment'. You should not need
            to modify bone assignments during rendering (only the positions of bones) and OGRE
            reserves the right to do some internal data reformatting of this information, depending
            on render system requirements.
        @par
            This method is for assigning weights to the dedicated geometry of the SubMesh. To assign
            weights to the shared Mesh geometry, see the equivalent methods on Mesh.
        */
        void addBoneAssignment(const VertexBoneAssignment& vertBoneAssign);

        /** Removes all bone assignments for this mesh. 
        @par
            This method is for assigning weights to the dedicated geometry of the SubMesh. To assign
            weights to the shared Mesh geometry, see the equivalent methods on Mesh.
        */
        void clearBoneAssignments(void);

        /** Gets a const reference to the list of bone assignments
        */
        const VertexBoneAssignmentVec& getBoneAssignments() { return mBoneAssignments; }

        /// Call this function if you've manually called addBoneAssignment to setup
        /// bones for vertices.
        /// WARNING: Will destroy LODs.
        void _compileBoneAssignments(void);

        /// Builds mBlendIndexToBoneIndexMap based on mBoneAssignments.
        /// mBlendIndexToBoneIndexMap is necessary for enabling skeletal animation.
        void _buildBoneIndexMap(void);

        /// Populates mBoneAssignments by reading the vertex data.
        /// See the other overload.
        void _buildBoneAssignmentsFromVertexData(void);

        /** Populates mBoneAssignments by reading the vertex data.
            This version accepts an external buffer in case you already have
            the vertex data on CPU (instead of having to bring it back from GPU)
        @remarks
            mBlendIndexToBoneIndexMap MUST be up to date. OTHERWISE IT MAY CRASH.
            Despite accepting external vertexData, mVao must be correctly
            populated with information about vertexData
        @param vertexData
            Vertex data with blend indices and weights.
        */
        void _buildBoneAssignmentsFromVertexData( uint8 const *vertexData );

        /** Makes a copy of this submesh object and gives it a new name.
        @param newName
            The name to give the clone.
        @param parentMesh
            Optional mesh to make the parent of the newly created clone.
            If you leave this blank, the clone will be parented to the same Mesh as the original.
        @param vertexBufferType
            See BufferType. Must be set to a valid BufferType. Pass a negative
            value to keep the same type of the original buffer being cloned.
        @param indexBufferType
            See BufferType. Must be set to a valid BufferType. Pass a negative
            value to keep the same type of the original buffer being cloned.
        */
        SubMesh* clone( Mesh *parentMesh = 0, int vertexBufferType = -1, int indexBufferType = -1 );

        void setMaterialName( const String &name )          { mMaterialName = name; }
        String getMaterialName(void) const                  { return mMaterialName; }

        /// Imports a v1 SubMesh @See Mesh::importV1. Automatically performs what arrangeEfficient does.
        void importFromV1( v1::SubMesh *subMesh, bool halfPos, bool halfTexCoords, bool qTangents );

        /// Converts this SubMesh to an efficient arrangement. @See Mesh::importV1 for an
        /// explanation on the parameters. @see dearrangeEfficientToInefficient
        /// to perform the opposite operation.
        void arrangeEfficient( bool halfPos, bool halfTexCoords, bool qTangents );

        /// Reverts the effects from arrangeEfficient by converting all 16-bit half float back
        /// to 32-bit float; and QTangents to Normal, Tangent + Reflection representation,
        /// which are more compatible for doing certain operations vertex operations in the CPU.
        void dearrangeToInefficient(void);

        void _prepareForShadowMapping( bool forceSameBuffers );

    protected:
        void importBuffersFromV1( v1::SubMesh *subMesh, bool halfPos, bool halfTexCoords, bool qTangents,
                                  size_t vaoPassIdx );

        /// Converts a v1 IndexBuffer to a v2 format. Returns nullptr if indexData is also nullptr
        IndexBufferPacked* importFromV1( v1::IndexData *indexData );

        /** @see arrangeEfficient overload
        @param vao
            The Vao to convert to.
        @param sharedBuffers
            Maps old Vertex Buffers to the new converted ones, so we can reuse them
            when we detect the vertex buffers were being shared across Vaos.
            Reads and writes from/to it.
        @param vaoManager
            Needed to create the new Vao
        @return
            New Vao containing the dearranged buffers. It will share the index buffers
            with the original vao.
        */
        static VertexArrayObject* arrangeEfficient( bool halfPos, bool halfTexCoords, bool qTangents,
                                                    VertexArrayObject *vao,
                                                    SharedVertexBufferMap &sharedBuffers,
                                                    VaoManager *vaoManager );

        /** @see dearrangeEfficientToInefficient. Works on an individual VertexArrayObject.
            Delegates work to the generic method @see _dearrangeEfficient which
            performs the actual buffer conversion.
        @param vao
            The Vao to convert to.
        @param sharedBuffers
            Maps old Vertex Buffers to the new converted ones, so we can reuse them
            when we detect the vertex buffers were being shared across Vaos.
            Reads and writes from/to it.
        @param vaoManager
            Needed to create the new Vao
        @return
            New Vao containing the dearranged buffers. It will share the index buffers
            with the original vao.
        */
        static VertexArrayObject* dearrangeEfficient( const VertexArrayObject *vao,
                                                      SharedVertexBufferMap &sharedBuffers,
                                                      VaoManager *vaoManager );

    public:
        struct SourceData
        {
            char const      *data;
            size_t          bytesPerVertex;
            VertexElement2  element;

            SourceData( char const *_data, size_t _bytesPerVertex, VertexElement2 _element ) :
                data( _data ), bytesPerVertex( _bytesPerVertex ), element( _element ) {}
        };

        typedef FastArray<SourceData> SourceDataArray;

        /** Rearranges the buffers to be efficiently rendered in Ogre 2.1 with Hlms
            Takes a v1 SubMesh and returns a pointer with the data interleaved,
            and a VertexElement2Vec with the new vertex format.
        @remarks
            Final work is delegated to the generic overload.
        @param halfPos
            @See Mesh::importV1
        @param halfTexCoords
            @See Mesh::importV1
        @param qTangents
            @See Mesh::importV1
        @param outVertexElements [out]
            Description of the buffer in the new v2 system.
        @return
            Buffer pointer with reorganized data.
            Caller MUST free the pointer with OGRE_FREE_SIMD( MEMCATEGORY_GEOMETRY ).
        */
        static char* _arrangeEfficient( v1::SubMesh *subMesh, bool halfPos, bool halfTexCoords,
                                        bool qTangents, VertexElement2Vec *outVertexElements,
                                        size_t vaoPassIdx );

        /** Generic form that does the actual job for both v1 and v2 objects. Takes
            an array of pointers to source each vertex element, and returns a
            pointer with the valid data.
        @param srcData
            Array that points to the source data for every vertex element.
            VES_TANGENT and VES_BINORMAL must be at the end if converting
            to QTangents. The pointer in SourceData::data must already
            be offsetted.
        @param vertexElements
            The vertex format we're converting to.
        @param vertexCount
            Number of vertices
        @return
            Buffer pointer with reorganized data.
        static char* _arrangeEfficient( SourceDataArray srcData,
            Caller MUST free the pointer with OGRE_FREE_SIMD( MEMCATEGORY_GEOMETRY ).
        */
        static char* _arrangeEfficient( SourceDataArray srcData,
                                        const VertexElement2Vec &vertexElements,
                                        uint32 vertexCount );

        /** Generic form that does the actual job for both v1 and v2 objects
            @see dearrangeEfficientToInefficient.
        @param srcData
            Single pointer with interleaved data.
        @param numElements
            Number of vertices.
        @param vertexElements
            Vertex format of the original vertex buffer.
        @param outVertexElements [out]
            Vertex format of the new converted vertex buffer.
        @return
            Pointer to the decompressed/dearranged data, interleaved in a single buffer.
        */
        static char* _dearrangeEfficient( char const * RESTRICT_ALIAS srcData, uint32 numElements,
                                          const VertexElement2Vec &vertexElements,
                                          VertexElement2Vec *outVertexElements );

        static void destroyVaos( VertexArrayObjectArray &vaos, VaoManager *vaoManager,
                                 bool destroyIndexBuffer = true );

    protected:
        void destroyShadowMappingVaos(void);
    };
    /** @} */
    /** @} */

} // namespace

#include "OgreHeaderSuffix.h"

#endif


