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

#ifndef __MeshSerializerImpl_H__
#define __MeshSerializerImpl_H__

#include "OgrePrerequisites.h"
#include "OgreString.h"
#include "OgreSerializer.h"
#include "OgreMaterial.h"
#include "OgreMesh.h"
#include "OgreEdgeListBuilder.h"

namespace Ogre {
	
	class MeshSerializerListener;
    class LodStrategy;

    /** Internal implementation of Mesh reading / writing for the latest version of the
    .mesh format.
    @remarks
    In order to maintain compatibility with older versions of the .mesh format, there
    will be alternative subclasses of this class to load older versions, whilst this class
    will remain to load the latest version.
    */
    class _OgrePrivate MeshSerializerImpl : public Serializer
    {
    public:
        MeshSerializerImpl();
        virtual ~MeshSerializerImpl();
        /** Exports a mesh to the file specified. 
        @remarks
        This method takes an externally created Mesh object, and exports both it
        and optionally the Materials it uses to a .mesh file.
        @param pMesh Pointer to the Mesh to export
        @param filename The destination filename
		@param endianMode The endian mode for the written file
        */
        void exportMesh(const Mesh* pMesh, const String& filename,
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

        // Internal methods
        virtual void writeSubMeshNameTable(const Mesh* pMesh);
        virtual void writeMesh(const Mesh* pMesh);
        virtual void writeSubMesh(const SubMesh* s);
        virtual void writeSubMeshOperation(const SubMesh* s);
        virtual void writeSubMeshTextureAliases(const SubMesh* s);
        virtual void writeGeometry(const VertexData* pGeom);
        virtual void writeSkeletonLink(const String& skelName);
        virtual void writeMeshBoneAssignment(const VertexBoneAssignment& assign);
        virtual void writeSubMeshBoneAssignment(const VertexBoneAssignment& assign);
        virtual void writeLodInfo(const Mesh* pMesh);
        virtual void writeLodSummary(unsigned short numLevels, bool manual, const LodStrategy *strategy);
        virtual void writeLodUsageManual(const MeshLodUsage& usage);
        virtual void writeLodUsageGenerated(const Mesh* pMesh, const MeshLodUsage& usage, unsigned short lodNum);
        virtual void writeBoundsInfo(const Mesh* pMesh);
        virtual void writeEdgeList(const Mesh* pMesh);
		virtual void writeAnimations(const Mesh* pMesh);
		virtual void writeAnimation(const Animation* anim);
		virtual void writePoses(const Mesh* pMesh);
		virtual void writePose(const Pose* pose);
		virtual void writeAnimationTrack(const VertexAnimationTrack* track);
		virtual void writeMorphKeyframe(const VertexMorphKeyFrame* kf, size_t vertexCount);
		virtual void writePoseKeyframe(const VertexPoseKeyFrame* kf);
		virtual void writePoseKeyframePoseRef(const VertexPoseKeyFrame::PoseRef& poseRef);
        virtual void writeExtremes(const Mesh *pMesh);
        virtual void writeSubMeshExtremes(unsigned short idx, const SubMesh* s);

        virtual size_t calcMeshSize(const Mesh* pMesh);
        virtual size_t calcSubMeshSize(const SubMesh* pSub);
        virtual size_t calcGeometrySize(const VertexData* pGeom);
        virtual size_t calcSkeletonLinkSize(const String& skelName);
        virtual size_t calcBoneAssignmentSize(void);
        virtual size_t calcSubMeshOperationSize(const SubMesh* pSub);
        virtual size_t calcSubMeshNameTableSize(const Mesh* pMesh);
        virtual size_t calcEdgeListSize(const Mesh* pMesh);
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
		virtual size_t calcPoseVertexSize(void);
        virtual size_t calcSubMeshTextureAliasesSize(const SubMesh* pSub);


        virtual void readTextureLayer(DataStreamPtr& stream, Mesh* pMesh, MaterialPtr& pMat);
        virtual void readSubMeshNameTable(DataStreamPtr& stream, Mesh* pMesh);
        virtual void readMesh(DataStreamPtr& stream, Mesh* pMesh, MeshSerializerListener *listener);
        virtual void readSubMesh(DataStreamPtr& stream, Mesh* pMesh, MeshSerializerListener *listener);
        virtual void readSubMeshOperation(DataStreamPtr& stream, Mesh* pMesh, SubMesh* sub);
        virtual void readSubMeshTextureAlias(DataStreamPtr& stream, Mesh* pMesh, SubMesh* sub);
        virtual void readGeometry(DataStreamPtr& stream, Mesh* pMesh, VertexData* dest);
        virtual void readGeometryVertexDeclaration(DataStreamPtr& stream, Mesh* pMesh, VertexData* dest);
        virtual void readGeometryVertexElement(DataStreamPtr& stream, Mesh* pMesh, VertexData* dest);
        virtual void readGeometryVertexBuffer(DataStreamPtr& stream, Mesh* pMesh, VertexData* dest);

        virtual void readSkeletonLink(DataStreamPtr& stream, Mesh* pMesh, MeshSerializerListener *listener);
        virtual void readMeshBoneAssignment(DataStreamPtr& stream, Mesh* pMesh);
        virtual void readSubMeshBoneAssignment(DataStreamPtr& stream, Mesh* pMesh, 
            SubMesh* sub);
        virtual void readMeshLodInfo(DataStreamPtr& stream, Mesh* pMesh);
        virtual void readMeshLodUsageManual(DataStreamPtr& stream, Mesh* pMesh, 
            unsigned short lodNum, MeshLodUsage& usage);
        virtual void readMeshLodUsageGenerated(DataStreamPtr& stream, Mesh* pMesh, 
            unsigned short lodNum, MeshLodUsage& usage);
        virtual void readBoundsInfo(DataStreamPtr& stream, Mesh* pMesh);
        virtual void readEdgeList(DataStreamPtr& stream, Mesh* pMesh);
        virtual void readEdgeListLodInfo(DataStreamPtr& stream, EdgeData* edgeData);
		virtual void readPoses(DataStreamPtr& stream, Mesh* pMesh);
		virtual void readPose(DataStreamPtr& stream, Mesh* pMesh);
		virtual void readAnimations(DataStreamPtr& stream, Mesh* pMesh);
		virtual void readAnimation(DataStreamPtr& stream, Mesh* pMesh);
		virtual void readAnimationTrack(DataStreamPtr& stream, Animation* anim, 
			Mesh* pMesh);
		virtual void readMorphKeyFrame(DataStreamPtr& stream, VertexAnimationTrack* track);
		virtual void readPoseKeyFrame(DataStreamPtr& stream, VertexAnimationTrack* track);
		virtual void readExtremes(DataStreamPtr& stream, Mesh *pMesh);


        /// Flip an entire vertex buffer from little endian
        virtual void flipFromLittleEndian(void* pData, size_t vertexCount, size_t vertexSize, const VertexDeclaration::VertexElementList& elems);
        /// Flip an entire vertex buffer to little endian
        virtual void flipToLittleEndian(void* pData, size_t vertexCount, size_t vertexSize, const VertexDeclaration::VertexElementList& elems);
        /// Flip the endianness of an entire vertex buffer, passed in as a 
        /// pointer to locked or temporary memory 
        virtual void flipEndian(void* pData, size_t vertexCount, size_t vertexSize, const VertexDeclaration::VertexElementList& elems);



    };

    /** Class for providing backwards-compatibility for loading version 1.3 of the .mesh format. */
    class _OgrePrivate MeshSerializerImpl_v1_4 : public MeshSerializerImpl
    {
    public:
        MeshSerializerImpl_v1_4();
        ~MeshSerializerImpl_v1_4();
    protected:
        virtual void writeLodSummary(unsigned short numLevels, bool manual, const LodStrategy *strategy);
        virtual void readMeshLodInfo(DataStreamPtr& stream, Mesh* pMesh);
    };

    /** Class for providing backwards-compatibility for loading version 1.3 of the .mesh format. */
    class _OgrePrivate MeshSerializerImpl_v1_3 : public MeshSerializerImpl
    {
    public:
        MeshSerializerImpl_v1_3();
        ~MeshSerializerImpl_v1_3();
    protected:
        virtual void readEdgeListLodInfo(DataStreamPtr& stream, EdgeData* edgeData);

        /// Reorganise triangles of the edge list to group by vertex set
        virtual void reorganiseTriangles(EdgeData* edgeData);
    };

    /** Class for providing backwards-compatibility for loading version 1.2 of the .mesh format. */
    class _OgrePrivate MeshSerializerImpl_v1_2 : public MeshSerializerImpl_v1_3
    {
    public:
        MeshSerializerImpl_v1_2();
        ~MeshSerializerImpl_v1_2();
    protected:
        virtual void readMesh(DataStreamPtr& stream, Mesh* pMesh, MeshSerializerListener *listener);
        virtual void readGeometry(DataStreamPtr& stream, Mesh* pMesh, VertexData* dest);
        virtual void readGeometryPositions(unsigned short bindIdx, DataStreamPtr& stream, 
            Mesh* pMesh, VertexData* dest);
        virtual void readGeometryNormals(unsigned short bindIdx, DataStreamPtr& stream, 
            Mesh* pMesh, VertexData* dest);
        virtual void readGeometryColours(unsigned short bindIdx, DataStreamPtr& stream, 
            Mesh* pMesh, VertexData* dest);
        virtual void readGeometryTexCoords(unsigned short bindIdx, DataStreamPtr& stream, 
            Mesh* pMesh, VertexData* dest, unsigned short set);
    };

    /** Class for providing backwards-compatibility for loading version 1.1 of the .mesh format. */
    class _OgrePrivate MeshSerializerImpl_v1_1 : public MeshSerializerImpl_v1_2
    {
    public:
        MeshSerializerImpl_v1_1();
        ~MeshSerializerImpl_v1_1();
    protected:
        void readGeometryTexCoords(unsigned short bindIdx, DataStreamPtr& stream, 
            Mesh* pMesh, VertexData* dest, unsigned short set);
    };


}

#endif
