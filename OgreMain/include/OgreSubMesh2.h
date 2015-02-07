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
#include "Vao/OgreVertexBufferPacked.h"
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
        VertexArrayObjectArray  mVao;

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

        uint8 rationaliseBoneAssignments( size_t vertexCount );


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


        /** Must be called once to compile bone assignments into geometry buffer. */
        void _compileBoneAssignments(void);

        /** Makes a copy of this submesh object and gives it a new name.
         @param newName
         The name to give the clone.
         @param parentMesh
         Optional mesh to make the parent of the newly created clone.
         If you leave this blank, the clone will be parented to the same Mesh as the original.
         */
        //SubMesh * clone(const String& newName, Mesh *parentMesh = 0);

        void setMaterialName( const String &name )          { mMaterialName = name; }
        String getMaterialName(void) const                  { return mMaterialName; }

        /// @See arrangeEfficient
        void importFromV1( v1::SubMesh *subMesh, bool halfPos, bool halfTexCoords, bool qTangents );

    protected:

        /// Converts a v1 IndexBuffer to a v2 format. Returns nullptr if indexData is also nullptr
        IndexBufferPacked* importFromV1( v1::IndexData *indexData );

        /** Rearranges the buffers to be efficiently rendered in Ogre 2.0 with Hlms
        @remarks
            vertexData->vertexDeclaration is modified and vertexData->vertexBufferBinding
            is destroyed. Caller must reallocate the vertex buffer filled with the returned
            pointer
        @param halfPos
            @See Mesh::importV1
        @param halfTexCoords
            @See Mesh::importV1
        @param qTangents
            @See Mesh::importV1
        @param outVertexElements [out]
            Description of the buffer in the new Vao system. Matches the same as
            vertexData->vertexDeclaration, provided as out param as convenience.
            Can be null.
        @return
            Buffer pointer with reorganized data.
            Caller MUST free the pointer with OGRE_FREE_SIMD( MEMCATEGORY_GEOMETRY ).
        */
        char* arrangeEfficient( v1::SubMesh *subMesh, bool halfPos, bool halfTexCoords,
                                bool qTangents, VertexElement2Vec *outVertexElements );
    };
    /** @} */
    /** @} */

} // namespace

#include "OgreHeaderSuffix.h"

#endif


