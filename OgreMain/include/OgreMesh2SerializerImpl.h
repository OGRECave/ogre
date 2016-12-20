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

#ifndef _OgreMesh2SerializerImpl_H_
#define _OgreMesh2SerializerImpl_H_

#include "OgrePrerequisites.h"
#include "OgreSerializer.h"
#include "OgreEdgeListBuilder.h"
#include "OgreKeyFrame.h"
#include "OgreVertexBoneAssignment.h"

#include "Vao/OgreVertexBufferPacked.h"

namespace Ogre {
    class MeshSerializerListener;
    struct MeshLodUsage;

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Resources
    *  @{
    */
    /** Internal implementation of Mesh reading / writing for the latest version of the
    .mesh format.
    @remarks
    In order to maintain compatibility with older versions of the .mesh format, there
    will be alternative subclasses of this class to load older versions, whilst this class
    will remain to load the latest version.

     @note
        This mesh format was used from Ogre v1.10.

    */
    class _OgrePrivate MeshSerializerImpl : public Serializer
    {
    public:
        MeshSerializerImpl( VaoManager *vaoManager );
        virtual ~MeshSerializerImpl();
        /** Exports a mesh to the file specified. 
        @remarks
        This method takes an externally created Mesh object, and exports both it
        and optionally the Materials it uses to a .mesh file.
        @param pMesh Pointer to the Mesh to export
        @param stream The destination stream
        @param endianMode The endian mode for the written file
        */
        void exportMesh(const Mesh* pMesh, DataStreamPtr stream,
            Endian endianMode = ENDIAN_NATIVE);

        /** Imports Mesh and (optionally) Material data from a .mesh file DataStream.
        @remarks
        This method imports data from a DataStream opened from a .mesh file and places it's
        contents into the Mesh object which is passed in. 
        @param stream The DataStream holding the .mesh data. Must be initialised (pos at the start of the buffer).
        @param pDest Pointer to the Mesh object which will receive the data. Should be blank already.
        */
        void importMesh(DataStreamPtr& stream, Mesh* pDest, MeshSerializerListener *listener);

    protected:
        typedef vector<uint8>::type LodLevelVertexBufferTable;
        typedef vector<LodLevelVertexBufferTable>::type LodLevelVertexBufferTableVec; //One per submesh
        typedef vector<uint8*>::type Uint8Vec;

        struct SubMeshLod
        {
            uint32                  numVertices;
            VertexElement2VecVec    vertexDeclarations;
            Uint8Vec                vertexBuffers;
            uint8                   lodSource;
            bool                    index32Bit;
            uint32                  numIndices;
            void                    *indexData;
            OperationType operationType;

            SubMeshLod();
        };

        typedef vector<SubMeshLod>::type SubMeshLodVec;

        // Internal methods
        virtual void writeSubMeshNameTable(const Mesh* pMesh);
        virtual void writeMesh(const Mesh* pMesh);
        virtual void writeSubMesh( const SubMesh* s, const LodLevelVertexBufferTable &lodVertexTable );
        virtual void writeSubMeshLod( const VertexArrayObject *vao, uint8 lodLevel, uint8 lodSource );
        virtual void writeSubMeshLodOperation( const VertexArrayObject *vao );
        virtual void writeIndexes(IndexBufferPacked *indexBuffer);
        virtual void writeGeometry(const VertexBufferPackedVec &pGeom);
        virtual void writeSkeletonLink(const String& skelName);

        virtual void writeBoundsInfo(const Mesh* pMesh);
        /*virtual void writeEdgeList(const Mesh* pMesh);
        virtual void writeAnimations(const Mesh* pMesh);
        virtual void writeAnimation(const Animation* anim);
        virtual void writePoses(const Mesh* pMesh);
        virtual void writePose(const Pose* pose);
        virtual void writeAnimationTrack(const VertexAnimationTrack* track);
        virtual void writeMorphKeyframe(const VertexMorphKeyFrame* kf, size_t vertexCount);
        virtual void writePoseKeyframe(const VertexPoseKeyFrame* kf);
        virtual void writePoseKeyframePoseRef(const VertexPoseKeyFrame::PoseRef& poseRef);*/

        virtual size_t calcMeshSize(const Mesh* pMesh, const LodLevelVertexBufferTableVec &lodVertexTable);
        virtual size_t calcSubMeshSize(const SubMesh* pSub, const LodLevelVertexBufferTable &lodVertexTable);
        virtual size_t calcSubMeshLodSize( const VertexArrayObject *vao, bool skipVertexBuffer );
        virtual size_t calcGeometrySize(const VertexBufferPackedVec &vertexData );
        virtual size_t calcVertexDeclSize(const VertexBufferPackedVec &vertexData);
        virtual size_t calcSkeletonLinkSize(const String& skelName);
        virtual size_t calcSubMeshLodOperationSize(const VertexArrayObject *vao);
        virtual size_t calcSubMeshNameTableSize(const Mesh* pMesh);
        /*virtual size_t calcEdgeListSize(const Mesh* pMesh);
        virtual size_t calcEdgeListLodSize(const EdgeData* data, bool isManual);
        virtual size_t calcEdgeGroupSize(const EdgeData::EdgeGroup& group);
        virtual size_t calcPosesSize(const Mesh* pMesh);
        virtual size_t calcPoseSize(const Pose* pose);
        virtual size_t calcAnimationsSize(const Mesh* pMesh);
        virtual size_t calcAnimationSize(const Animation* anim);
        virtual size_t calcAnimationTrackSize(const VertexAnimationTrack* track);
        virtual size_t calcMorphKeyframeSize(const VertexMorphKeyFrame* kf, size_t vertexCount);
        virtual size_t calcPoseKeyframeSize(const VertexPoseKeyFrame* kf);
        virtual size_t calcPoseKeyframePoseRefSize(void);
        virtual size_t calcPoseVertexSize(const Pose* pose);*/
        virtual size_t calcBoundsInfoSize(const Mesh* pMesh);

        virtual void readTextureLayer(DataStreamPtr& stream, Mesh* pMesh, MaterialPtr& pMat);
        virtual void readSubMeshNameTable(DataStreamPtr& stream, Mesh* pMesh);
        virtual void readMesh(DataStreamPtr& stream, Mesh* pMesh, MeshSerializerListener *listener);
        virtual void readSubMesh(DataStreamPtr& stream, Mesh* pMesh, MeshSerializerListener *listener, uint8 numVaoPasses);
        virtual void readSubMeshLod( DataStreamPtr& stream, Mesh *pMesh,
                                     SubMeshLod *subLod, uint8 currentLod );
        virtual void readIndexes(DataStreamPtr& stream, SubMeshLod *subLod);
        virtual void readGeometry(DataStreamPtr& stream, SubMeshLod *subLod);
        virtual void readVertexDeclaration(DataStreamPtr& stream, SubMeshLod *subLod);
        virtual void readVertexBuffer(DataStreamPtr& stream, SubMeshLod *subLod);
        virtual void readSubMeshLodOperation(DataStreamPtr& stream, SubMeshLod *subLod);
        /*virtual void readGeometry(DataStreamPtr& stream, Mesh* pMesh, VertexData* dest);
        virtual void readGeometryVertexDeclaration(DataStreamPtr& stream, Mesh* pMesh, VertexData* dest);
        virtual void readGeometryVertexElement(DataStreamPtr& stream, Mesh* pMesh, VertexData* dest);
        virtual void readGeometryVertexBuffer(DataStreamPtr& stream, Mesh* pMesh, VertexData* dest);*/

        virtual void readSkeletonLink(DataStreamPtr& stream, Mesh* pMesh, MeshSerializerListener *listener);
        virtual void readBoundsInfo(DataStreamPtr& stream, Mesh* pMesh);
        /*virtual void readEdgeList(DataStreamPtr& stream, Mesh* pMesh);
        virtual void readEdgeListLodInfo(DataStreamPtr& stream, EdgeData* edgeData);
        virtual void readPoses(DataStreamPtr& stream, Mesh* pMesh);
        virtual void readPose(DataStreamPtr& stream, Mesh* pMesh);
        virtual void readAnimations(DataStreamPtr& stream, Mesh* pMesh);
        virtual void readAnimation(DataStreamPtr& stream, Mesh* pMesh);
        virtual void readAnimationTrack(DataStreamPtr& stream, Animation* anim, 
            Mesh* pMesh);
        virtual void readMorphKeyFrame(DataStreamPtr& stream, VertexAnimationTrack* track);
        virtual void readPoseKeyFrame(DataStreamPtr& stream, VertexAnimationTrack* track);*/

        virtual void createSubMeshVao( SubMesh *sm, SubMeshLodVec &submeshLods,
                                       uint8 numVaoPasses );

        /// Flip an entire vertex buffer to/from little endian
        /// working on the data pointer passed in pData
        void flipLittleEndian( void* pData, VertexBufferPacked *vertexBuffer );
        void flipLittleEndian( void* pData, size_t numVertices, size_t bytesPerVertex,
                               const VertexElement2Vec &vertexElements );

        /// Flip the endianness of an entire vertex buffer, passed in as a 
        /// pointer to locked or temporary memory 
        void flipEndian( void* pData, size_t vertexCount,
                         size_t vertexSize, size_t baseOffset,
                         const VertexElementType elementType );
        
        /// This function can be overloaded to disable validation in debug builds.
        virtual void enableValidation();

        ushort exportedLodCount; // Needed to limit exported Edge data, when exporting
        VaoManager *mVaoManager;
    };

    class _OgrePrivate MeshSerializerImpl_v2_1_R1 : public MeshSerializerImpl
    {
    public:
        MeshSerializerImpl_v2_1_R1( VaoManager *vaoManager );
        virtual ~MeshSerializerImpl_v2_1_R1();

    protected:
        virtual void readSubMesh(DataStreamPtr& stream, Mesh* pMesh, MeshSerializerListener *listener, uint8 numVaoPasses);
    };

    class _OgrePrivate MeshSerializerImpl_v2_1_R0 : public MeshSerializerImpl_v2_1_R1
    {
    public:
        MeshSerializerImpl_v2_1_R0( VaoManager *vaoManager );
        virtual ~MeshSerializerImpl_v2_1_R0();

    protected:
        virtual void readMesh(DataStreamPtr& stream, Mesh* pMesh, MeshSerializerListener *listener);
    };

    /** @} */
    /** @} */
}

#endif
