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
#include "OgreStableHeaders.h"

#include "OgreMeshSerializerImpl.h"
#include "OgreMeshFileFormat.h"
#include "OgreMeshSerializer.h"
#include "OgreMesh.h"
#include "OgreSubMesh.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreSkeleton.h"
#include "OgreHardwareBufferManager.h"
#include "OgreMaterial.h"
#include "OgreTechnique.h"
#include "OgrePass.h"
#include "OgreAnimation.h"
#include "OgreAnimationTrack.h"
#include "OgreKeyFrame.h"
#include "OgreRoot.h"
#include "OgreLodStrategyManager.h"
#include "OgreDistanceLodStrategy.h"

#if OGRE_COMPILER == OGRE_COMPILER_MSVC
// Disable conversion warnings, we do a lot of them, intentionally
#   pragma warning (disable : 4267)
#endif


namespace Ogre {

    /// stream overhead = ID + size
    const long STREAM_OVERHEAD_SIZE = sizeof(uint16) + sizeof(uint32);
    //---------------------------------------------------------------------
    MeshSerializerImpl::MeshSerializerImpl()
    {

        // Version number
        mVersion = "[MeshSerializer_v1.41]";
    }
    //---------------------------------------------------------------------
    MeshSerializerImpl::~MeshSerializerImpl()
    {
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::exportMesh(const Mesh* pMesh, 
		const String& filename, Endian endianMode)
    {
        LogManager::getSingleton().logMessage("MeshSerializer writing mesh data to " + filename + "...");

		// Decide on endian mode
		determineEndianness(endianMode);

        // Check that the mesh has it's bounds set
        if (pMesh->getBounds().isNull() || pMesh->getBoundingSphereRadius() == 0.0f)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "The Mesh you have supplied does not have its"
                " bounds completely defined. Define them first before exporting.",
                "MeshSerializerImpl::exportMesh");
        }
        mpfFile = fopen(filename.c_str(), "wb");
		if (!mpfFile)
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
				"Unable to open file " + filename + " for writing",
				"MeshSerializerImpl::exportMesh");
		}

        writeFileHeader();
        LogManager::getSingleton().logMessage("File header written.");


        LogManager::getSingleton().logMessage("Writing mesh data...");
        writeMesh(pMesh);
        LogManager::getSingleton().logMessage("Mesh data exported.");

        fclose(mpfFile);
        LogManager::getSingleton().logMessage("MeshSerializer export successful.");
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::importMesh(DataStreamPtr& stream, Mesh* pMesh, MeshSerializerListener *listener)
    {
		// Determine endianness (must be the first thing we do!)
		determineEndianness(stream);

        // Check header
        readFileHeader(stream);

        unsigned short streamID;
        while(!stream->eof())
        {
            streamID = readChunk(stream);
            switch (streamID)
            {
            case M_MESH:
                readMesh(stream, pMesh, listener);
                break;
			}

        }
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writeMesh(const Mesh* pMesh)
    {
        // Header
        writeChunkHeader(M_MESH, calcMeshSize(pMesh));

		// bool skeletallyAnimated
		bool skelAnim = pMesh->hasSkeleton();
		writeBools(&skelAnim, 1);

        // Write shared geometry
        if (pMesh->sharedVertexData)
            writeGeometry(pMesh->sharedVertexData);

        // Write Submeshes
        for (int i = 0; i < pMesh->getNumSubMeshes(); ++i)
        {
            LogManager::getSingleton().logMessage("Writing submesh...");
            writeSubMesh(pMesh->getSubMesh(i));
            LogManager::getSingleton().logMessage("Submesh exported.");
        }

        // Write skeleton info if required
        if (pMesh->hasSkeleton())
        {
            LogManager::getSingleton().logMessage("Exporting skeleton link...");
            // Write skeleton link
            writeSkeletonLink(pMesh->getSkeletonName());
            LogManager::getSingleton().logMessage("Skeleton link exported.");

            // Write bone assignments
            if (!pMesh->mBoneAssignments.empty())
            {
                LogManager::getSingleton().logMessage("Exporting shared geometry bone assignments...");

                Mesh::VertexBoneAssignmentList::const_iterator vi;
                for (vi = pMesh->mBoneAssignments.begin();
                vi != pMesh->mBoneAssignments.end(); ++vi)
                {
                    writeMeshBoneAssignment(vi->second);
                }

                LogManager::getSingleton().logMessage("Shared geometry bone assignments exported.");
            }
        }

        // Write LOD data if any
        if (pMesh->getNumLodLevels() > 1)
        {
            LogManager::getSingleton().logMessage("Exporting LOD information....");
            writeLodInfo(pMesh);
            LogManager::getSingleton().logMessage("LOD information exported.");

        }
        // Write bounds information
        LogManager::getSingleton().logMessage("Exporting bounds information....");
        writeBoundsInfo(pMesh);
        LogManager::getSingleton().logMessage("Bounds information exported.");

		// Write submesh name table
		LogManager::getSingleton().logMessage("Exporting submesh name table...");
		writeSubMeshNameTable(pMesh);
		LogManager::getSingleton().logMessage("Submesh name table exported.");

		// Write edge lists
		if (pMesh->isEdgeListBuilt())
		{
			LogManager::getSingleton().logMessage("Exporting edge lists...");
			writeEdgeList(pMesh);
			LogManager::getSingleton().logMessage("Edge lists exported");
		}

		// Write morph animation
		writePoses(pMesh);
		if (pMesh->hasVertexAnimation())
		{
			writeAnimations(pMesh);
        }

        // Write submesh extremes
        writeExtremes(pMesh);
    }
    //---------------------------------------------------------------------
	// Added by DrEvil
	void MeshSerializerImpl::writeSubMeshNameTable(const Mesh* pMesh)
	{
		// Header
		writeChunkHeader(M_SUBMESH_NAME_TABLE, calcSubMeshNameTableSize(pMesh));

		// Loop through and save out the index and names.
		Mesh::SubMeshNameMap::const_iterator it = pMesh->mSubMeshNameMap.begin();

		while(it != pMesh->mSubMeshNameMap.end())
		{
			// Header
			writeChunkHeader(M_SUBMESH_NAME_TABLE_ELEMENT, STREAM_OVERHEAD_SIZE +
				sizeof(unsigned short) + (unsigned long)it->first.length() + 1);

			// write the index
			writeShorts(&it->second, 1);
			// name
	        writeString(it->first);

			++it;
		}
	}
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writeSubMesh(const SubMesh* s)
    {
        // Header
        writeChunkHeader(M_SUBMESH, calcSubMeshSize(s));

        // char* materialName
        writeString(s->getMaterialName());

        // bool useSharedVertices
        writeBools(&s->useSharedVertices, 1);

		unsigned int indexCount = s->indexData->indexCount;
        writeInts(&indexCount, 1);

        // bool indexes32Bit
        bool idx32bit = (!s->indexData->indexBuffer.isNull() &&
			s->indexData->indexBuffer->getType() == HardwareIndexBuffer::IT_32BIT);
        writeBools(&idx32bit, 1);

		if (indexCount > 0)
		{
			// unsigned short* faceVertexIndices ((indexCount)
			HardwareIndexBufferSharedPtr ibuf = s->indexData->indexBuffer;
			void* pIdx = ibuf->lock(HardwareBuffer::HBL_READ_ONLY);
			if (idx32bit)
			{
				unsigned int* pIdx32 = static_cast<unsigned int*>(pIdx);
				writeInts(pIdx32, s->indexData->indexCount);
			}
			else
			{
				unsigned short* pIdx16 = static_cast<unsigned short*>(pIdx);
				writeShorts(pIdx16, s->indexData->indexCount);
			}
			ibuf->unlock();
		}

        // M_GEOMETRY stream (Optional: present only if useSharedVertices = false)
        if (!s->useSharedVertices)
        {
            writeGeometry(s->vertexData);
        }

        // end of sub mesh chunk

        // write out texture alias chunks
        writeSubMeshTextureAliases(s);

        // Operation type
        writeSubMeshOperation(s);

        // Bone assignments
        if (!s->mBoneAssignments.empty())
        {
            LogManager::getSingleton().logMessage("Exporting dedicated geometry bone assignments...");

            SubMesh::VertexBoneAssignmentList::const_iterator vi;
            for (vi = s->mBoneAssignments.begin();
            vi != s->mBoneAssignments.end(); ++vi)
            {
                writeSubMeshBoneAssignment(vi->second);
            }

            LogManager::getSingleton().logMessage("Dedicated geometry bone assignments exported.");
        }


    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writeExtremes(const Mesh *pMesh)
    {
        bool has_extremes = false;
        for (int i = 0; i < pMesh->getNumSubMeshes(); ++i)
        {
            SubMesh *sm = pMesh->getSubMesh(i);
            if (sm->extremityPoints.empty())
                continue;
            if (!has_extremes)
            {
                has_extremes = true;
                LogManager::getSingleton().logMessage("Writing submesh extremes...");
            }
            writeSubMeshExtremes(i, sm);
        }
        if (has_extremes)
            LogManager::getSingleton().logMessage("Extremes exported.");
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writeSubMeshExtremes(unsigned short idx, const SubMesh* s)
    {
        size_t chunkSize = STREAM_OVERHEAD_SIZE + sizeof (unsigned short) +
            s->extremityPoints.size () * sizeof (float) * 3;
        writeChunkHeader(M_TABLE_EXTREMES, chunkSize);

        writeShorts(&idx, 1);

        float *vertices = OGRE_ALLOC_T(float, s->extremityPoints.size() * 3, MEMCATEGORY_GEOMETRY);
		float *pVert = vertices;

        for (vector<Vector3>::type::const_iterator i = s->extremityPoints.begin();
             i != s->extremityPoints.end(); ++i)
        {
			*pVert++ = i->x;
			*pVert++ = i->y;
			*pVert++ = i->z;
        }

        writeFloats(vertices, s->extremityPoints.size () * 3);
        OGRE_FREE(vertices, MEMCATEGORY_GEOMETRY);
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writeSubMeshTextureAliases(const SubMesh* s)
    {
        size_t chunkSize;
        AliasTextureNamePairList::const_iterator i;

		LogManager::getSingleton().logMessage("Exporting submesh texture aliases...");

        // iterate through texture aliases and write them out as a chunk
        for (i = s->mTextureAliases.begin(); i != s->mTextureAliases.end(); ++i)
        {
            // calculate chunk size based on string length + 1.  Add 1 for the line feed.
            chunkSize = STREAM_OVERHEAD_SIZE + i->first.length() + i->second.length() + 2;
			writeChunkHeader(M_SUBMESH_TEXTURE_ALIAS, chunkSize);
            // write out alias name
            writeString(i->first);
            // write out texture name
            writeString(i->second);
        }

		LogManager::getSingleton().logMessage("Submesh texture aliases exported.");
    }

    //---------------------------------------------------------------------
    void MeshSerializerImpl::writeSubMeshOperation(const SubMesh* sm)
    {
        // Header
        writeChunkHeader(M_SUBMESH_OPERATION, calcSubMeshOperationSize(sm));

        // unsigned short operationType
        unsigned short opType = static_cast<unsigned short>(sm->operationType);
        writeShorts(&opType, 1);
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writeGeometry(const VertexData* vertexData)
    {
		// calc size
        const VertexDeclaration::VertexElementList& elemList =
            vertexData->vertexDeclaration->getElements();
        const VertexBufferBinding::VertexBufferBindingMap& bindings =
            vertexData->vertexBufferBinding->getBindings();
        VertexBufferBinding::VertexBufferBindingMap::const_iterator vbi, vbiend;

		size_t size = STREAM_OVERHEAD_SIZE + sizeof(unsigned int) + // base
			(STREAM_OVERHEAD_SIZE + elemList.size() * (STREAM_OVERHEAD_SIZE + sizeof(unsigned short) * 5)); // elements
        vbiend = bindings.end();
		for (vbi = bindings.begin(); vbi != vbiend; ++vbi)
		{
			const HardwareVertexBufferSharedPtr& vbuf = vbi->second;
			size += (STREAM_OVERHEAD_SIZE * 2) + (sizeof(unsigned short) * 2) + vbuf->getSizeInBytes();
		}

		// Header
        writeChunkHeader(M_GEOMETRY, size);

        unsigned int vertexCount = vertexData->vertexCount;
        writeInts(&vertexCount, 1);

		// Vertex declaration
		size = STREAM_OVERHEAD_SIZE + elemList.size() * (STREAM_OVERHEAD_SIZE + sizeof(unsigned short) * 5);
		writeChunkHeader(M_GEOMETRY_VERTEX_DECLARATION, size);

        VertexDeclaration::VertexElementList::const_iterator vei, veiend;
		veiend = elemList.end();
		unsigned short tmp;
		size = STREAM_OVERHEAD_SIZE + sizeof(unsigned short) * 5;
		for (vei = elemList.begin(); vei != veiend; ++vei)
		{
			const VertexElement& elem = *vei;
			writeChunkHeader(M_GEOMETRY_VERTEX_ELEMENT, size);
			// unsigned short source;  	// buffer bind source
			tmp = elem.getSource();
			writeShorts(&tmp, 1);
			// unsigned short type;    	// VertexElementType
			tmp = static_cast<unsigned short>(elem.getType());
			writeShorts(&tmp, 1);
			// unsigned short semantic; // VertexElementSemantic
			tmp = static_cast<unsigned short>(elem.getSemantic());
			writeShorts(&tmp, 1);
			// unsigned short offset;	// start offset in buffer in bytes
			tmp = static_cast<unsigned short>(elem.getOffset());
			writeShorts(&tmp, 1);
			// unsigned short index;	// index of the semantic (for colours and texture coords)
			tmp = elem.getIndex();
			writeShorts(&tmp, 1);

		}

		// Buffers and bindings
		vbiend = bindings.end();
		for (vbi = bindings.begin(); vbi != vbiend; ++vbi)
		{
			const HardwareVertexBufferSharedPtr& vbuf = vbi->second;
			size = (STREAM_OVERHEAD_SIZE * 2) + (sizeof(unsigned short) * 2) + vbuf->getSizeInBytes();
			writeChunkHeader(M_GEOMETRY_VERTEX_BUFFER,  size);
			// unsigned short bindIndex;	// Index to bind this buffer to
			tmp = vbi->first;
			writeShorts(&tmp, 1);
			// unsigned short vertexSize;	// Per-vertex size, must agree with declaration at this index
			tmp = (unsigned short)vbuf->getVertexSize();
			writeShorts(&tmp, 1);

			// Data
			size = STREAM_OVERHEAD_SIZE + vbuf->getSizeInBytes();
			writeChunkHeader(M_GEOMETRY_VERTEX_BUFFER_DATA, size);
			void* pBuf = vbuf->lock(HardwareBuffer::HBL_READ_ONLY);

			if (mFlipEndian)
			{
				// endian conversion
				// Copy data
				unsigned char* tempData = OGRE_ALLOC_T(unsigned char, vbuf->getSizeInBytes(), MEMCATEGORY_GEOMETRY);
				memcpy(tempData, pBuf, vbuf->getSizeInBytes());
				flipToLittleEndian(
					tempData,
					vertexData->vertexCount,
					vbuf->getVertexSize(),
					vertexData->vertexDeclaration->findElementsBySource(vbi->first));
				writeData(tempData, vbuf->getVertexSize(), vertexData->vertexCount);
				OGRE_FREE(tempData, MEMCATEGORY_GEOMETRY);
			}
			else
			{
				writeData(pBuf, vbuf->getVertexSize(), vertexData->vertexCount);
			}
            vbuf->unlock();
		}


    }
    //---------------------------------------------------------------------
	size_t MeshSerializerImpl::calcSubMeshNameTableSize(const Mesh* pMesh)
	{
		size_t size = STREAM_OVERHEAD_SIZE;
		// Figure out the size of the Name table.
		// Iterate through the subMeshList & add up the size of the indexes and names.
		Mesh::SubMeshNameMap::const_iterator it = pMesh->mSubMeshNameMap.begin();
		while(it != pMesh->mSubMeshNameMap.end())
		{
			// size of the index + header size for each element chunk
			size += STREAM_OVERHEAD_SIZE + sizeof(uint16);
			// name
			size += it->first.length() + 1;

			++it;
		}

		// size of the sub-mesh name table.
		return size;
	}
    //---------------------------------------------------------------------
    size_t MeshSerializerImpl::calcMeshSize(const Mesh* pMesh)
    {
        size_t size = STREAM_OVERHEAD_SIZE;

        // Num shared vertices
        size += sizeof(uint32);

        // Geometry
        if (pMesh->sharedVertexData && pMesh->sharedVertexData->vertexCount > 0)
        {
            size += calcGeometrySize(pMesh->sharedVertexData);
        }

        // Submeshes
        for (unsigned short i = 0; i < pMesh->getNumSubMeshes(); ++i)
        {
            size += calcSubMeshSize(pMesh->getSubMesh(i));
        }

        // Skeleton link
        if (pMesh->hasSkeleton())
        {
            size += calcSkeletonLinkSize(pMesh->getSkeletonName());
        }

		// Submesh name table
		size += calcSubMeshNameTableSize(pMesh);

		// Edge list
		if (pMesh->isEdgeListBuilt())
		{
			size += calcEdgeListSize(pMesh);
		}

		// Animations
		for (unsigned short a = 0; a < pMesh->getNumAnimations(); ++a)
		{
			Animation* anim = pMesh->getAnimation(a);
			size += calcAnimationSize(anim);
		}

		return size;
    }
    //---------------------------------------------------------------------
    size_t MeshSerializerImpl::calcSubMeshSize(const SubMesh* pSub)
    {
        size_t size = STREAM_OVERHEAD_SIZE;

        // Material name
        size += pSub->getMaterialName().length() + 1;

        // bool useSharedVertices
        size += sizeof(bool);
        // unsigned int indexCount
        size += sizeof(unsigned int);
        // bool indexes32bit
        size += sizeof(bool);
        // unsigned int* faceVertexIndices
        size += sizeof(unsigned int) * pSub->indexData->indexCount;

        // Geometry
        if (!pSub->useSharedVertices)
        {
            size += calcGeometrySize(pSub->vertexData);
        }

        size += calcSubMeshTextureAliasesSize(pSub);
        size += calcSubMeshOperationSize(pSub);

        // Bone assignments
        if (!pSub->mBoneAssignments.empty())
        {
            SubMesh::VertexBoneAssignmentList::const_iterator vi;
            for (vi = pSub->mBoneAssignments.begin();
                 vi != pSub->mBoneAssignments.end(); ++vi)
            {
                size += calcBoneAssignmentSize();
            }
        }

        return size;
    }
    //---------------------------------------------------------------------
    size_t MeshSerializerImpl::calcSubMeshOperationSize(const SubMesh* pSub)
    {
        return STREAM_OVERHEAD_SIZE + sizeof(uint16);
    }
    //---------------------------------------------------------------------
    size_t MeshSerializerImpl::calcSubMeshTextureAliasesSize(const SubMesh* pSub)
    {
        size_t chunkSize = 0;
        AliasTextureNamePairList::const_iterator i;

        // iterate through texture alias map and calc size of strings
        for (i = pSub->mTextureAliases.begin(); i != pSub->mTextureAliases.end(); ++i)
        {
            // calculate chunk size based on string length + 1.  Add 1 for the line feed.
            chunkSize += STREAM_OVERHEAD_SIZE + i->first.length() + i->second.length() + 2;
        }

        return chunkSize;
    }
    //---------------------------------------------------------------------
    size_t MeshSerializerImpl::calcGeometrySize(const VertexData* vertexData)
    {
        size_t size = STREAM_OVERHEAD_SIZE;

        // Num vertices
        size += sizeof(unsigned int);

        const VertexDeclaration::VertexElementList& elems =
            vertexData->vertexDeclaration->getElements();

        VertexDeclaration::VertexElementList::const_iterator i, iend;
        iend = elems.end();
        for (i = elems.begin(); i != iend; ++i)
        {
            const VertexElement& elem = *i;
            // Vertex element
            size += VertexElement::getTypeSize(elem.getType()) * vertexData->vertexCount;
        }
        return size;
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::readGeometry(DataStreamPtr& stream, Mesh* pMesh,
        VertexData* dest)
    {

        dest->vertexStart = 0;

        unsigned int vertexCount = 0;
        readInts(stream, &vertexCount, 1);
        dest->vertexCount = vertexCount;

        // Find optional geometry streams
        if (!stream->eof())
        {
            unsigned short streamID = readChunk(stream);
            while(!stream->eof() &&
                (streamID == M_GEOMETRY_VERTEX_DECLARATION ||
                 streamID == M_GEOMETRY_VERTEX_BUFFER ))
            {
                switch (streamID)
                {
                case M_GEOMETRY_VERTEX_DECLARATION:
                    readGeometryVertexDeclaration(stream, pMesh, dest);
                    break;
                case M_GEOMETRY_VERTEX_BUFFER:
                    readGeometryVertexBuffer(stream, pMesh, dest);
                    break;
                }
                // Get next stream
                if (!stream->eof())
                {
                    streamID = readChunk(stream);
                }
            }
            if (!stream->eof())
            {
                // Backpedal back to start of non-submesh stream
                stream->skip(-STREAM_OVERHEAD_SIZE);
            }
        }

		// Perform any necessary colour conversion for an active rendersystem
		if (Root::getSingletonPtr() && Root::getSingleton().getRenderSystem())
		{
			// We don't know the source type if it's VET_COLOUR, but assume ARGB
			// since that's the most common. Won't get used unless the mesh is
			// ambiguous anyway, which will have been warned about in the log
			dest->convertPackedColour(VET_COLOUR_ARGB, 
				VertexElement::getBestColourVertexElementType());
		}
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::readGeometryVertexDeclaration(DataStreamPtr& stream,
        Mesh* pMesh, VertexData* dest)
    {
        // Find optional geometry streams
        if (!stream->eof())
        {
            unsigned short streamID = readChunk(stream);
            while(!stream->eof() &&
                (streamID == M_GEOMETRY_VERTEX_ELEMENT ))
            {
                switch (streamID)
                {
                case M_GEOMETRY_VERTEX_ELEMENT:
                    readGeometryVertexElement(stream, pMesh, dest);
                    break;
                }
                // Get next stream
                if (!stream->eof())
                {
                    streamID = readChunk(stream);
                }
            }
            if (!stream->eof())
            {
                // Backpedal back to start of non-submesh stream
                stream->skip(-STREAM_OVERHEAD_SIZE);
            }
        }

	}
    //---------------------------------------------------------------------
    void MeshSerializerImpl::readGeometryVertexElement(DataStreamPtr& stream,
        Mesh* pMesh, VertexData* dest)
    {
		unsigned short source, offset, index, tmp;
		VertexElementType vType;
		VertexElementSemantic vSemantic;
		// unsigned short source;  	// buffer bind source
		readShorts(stream, &source, 1);
		// unsigned short type;    	// VertexElementType
		readShorts(stream, &tmp, 1);
		vType = static_cast<VertexElementType>(tmp);
		// unsigned short semantic; // VertexElementSemantic
		readShorts(stream, &tmp, 1);
		vSemantic = static_cast<VertexElementSemantic>(tmp);
		// unsigned short offset;	// start offset in buffer in bytes
		readShorts(stream, &offset, 1);
		// unsigned short index;	// index of the semantic
		readShorts(stream, &index, 1);

		dest->vertexDeclaration->addElement(source, offset, vType, vSemantic, index);

		if (vType == VET_COLOUR)
		{
			LogManager::getSingleton().stream()
				<< "Warning: VET_COLOUR element type is deprecated, you should use "
				<< "one of the more specific types to indicate the byte order. "
				<< "Use OgreMeshUpgrade on " << pMesh->getName() << " as soon as possible. ";
		}

	}
    //---------------------------------------------------------------------
    void MeshSerializerImpl::readGeometryVertexBuffer(DataStreamPtr& stream,
        Mesh* pMesh, VertexData* dest)
    {
		unsigned short bindIndex, vertexSize;
		// unsigned short bindIndex;	// Index to bind this buffer to
		readShorts(stream, &bindIndex, 1);
		// unsigned short vertexSize;	// Per-vertex size, must agree with declaration at this index
		readShorts(stream, &vertexSize, 1);

		// Check for vertex data header
		unsigned short headerID;
		headerID = readChunk(stream);
		if (headerID != M_GEOMETRY_VERTEX_BUFFER_DATA)
		{
			OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Can't find vertex buffer data area",
            	"MeshSerializerImpl::readGeometryVertexBuffer");
		}
		// Check that vertex size agrees
		if (dest->vertexDeclaration->getVertexSize(bindIndex) != vertexSize)
		{
			OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Buffer vertex size does not agree with vertex declaration",
            	"MeshSerializerImpl::readGeometryVertexBuffer");
		}

		// Create / populate vertex buffer
		HardwareVertexBufferSharedPtr vbuf;
        vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(
            vertexSize,
            dest->vertexCount,
            pMesh->mVertexBufferUsage,
			pMesh->mVertexBufferShadowBuffer);
        void* pBuf = vbuf->lock(HardwareBuffer::HBL_DISCARD);
        stream->read(pBuf, dest->vertexCount * vertexSize);

		// endian conversion for OSX
		flipFromLittleEndian(
			pBuf,
			dest->vertexCount,
			vertexSize,
			dest->vertexDeclaration->findElementsBySource(bindIndex));
        vbuf->unlock();

		// Set binding
        dest->vertexBufferBinding->setBinding(bindIndex, vbuf);

	}
    //---------------------------------------------------------------------
	void MeshSerializerImpl::readSubMeshNameTable(DataStreamPtr& stream, Mesh* pMesh)
	{
		// The map for
		map<unsigned short, String>::type subMeshNames;
		unsigned short streamID, subMeshIndex;

		// Need something to store the index, and the objects name
		// This table is a method that imported meshes can retain their naming
		// so that the names established in the modelling software can be used
		// to get the sub-meshes by name. The exporter must support exporting
		// the optional stream M_SUBMESH_NAME_TABLE.

        // Read in all the sub-streams. Each sub-stream should contain an index and Ogre::String for the name.
		if (!stream->eof())
		{
			streamID = readChunk(stream);
			while(!stream->eof() && (streamID == M_SUBMESH_NAME_TABLE_ELEMENT ))
			{
				// Read in the index of the submesh.
				readShorts(stream, &subMeshIndex, 1);
				// Read in the String and map it to its index.
				subMeshNames[subMeshIndex] = readString(stream);

				// If we're not end of file get the next stream ID
				if (!stream->eof())
					streamID = readChunk(stream);
			}
			if (!stream->eof())
			{
				// Backpedal back to start of stream
				stream->skip(-STREAM_OVERHEAD_SIZE);
			}
		}

		// Set all the submeshes names
		// ?

		// Loop through and save out the index and names.
		map<unsigned short, String>::type::const_iterator it = subMeshNames.begin();

		while(it != subMeshNames.end())
		{
			// Name this submesh to the stored name.
			pMesh->nameSubMesh(it->second, it->first);
			++it;
		}



	}
    //---------------------------------------------------------------------
    void MeshSerializerImpl::readMesh(DataStreamPtr& stream, Mesh* pMesh, MeshSerializerListener *listener)
    {
        unsigned short streamID;

        // Never automatically build edge lists for this version
        // expect them in the file or not at all
        pMesh->mAutoBuildEdgeLists = false;

		// bool skeletallyAnimated
		bool skeletallyAnimated;
		readBools(stream, &skeletallyAnimated, 1);

        // Find all substreams
        if (!stream->eof())
        {
            streamID = readChunk(stream);
            while(!stream->eof() &&
                (streamID == M_GEOMETRY ||
				 streamID == M_SUBMESH ||
                 streamID == M_MESH_SKELETON_LINK ||
                 streamID == M_MESH_BONE_ASSIGNMENT ||
				 streamID == M_MESH_LOD ||
                 streamID == M_MESH_BOUNDS ||
				 streamID == M_SUBMESH_NAME_TABLE ||
				 streamID == M_EDGE_LISTS ||
				 streamID == M_POSES ||
				 streamID == M_ANIMATIONS ||
				 streamID == M_TABLE_EXTREMES))
            {
                switch(streamID)
                {
				case M_GEOMETRY:
					pMesh->sharedVertexData = OGRE_NEW VertexData();
					try {
						readGeometry(stream, pMesh, pMesh->sharedVertexData);
					}
					catch (Exception& e)
					{
						if (e.getNumber() == Exception::ERR_ITEM_NOT_FOUND)
						{
							// duff geometry data entry with 0 vertices
							OGRE_DELETE pMesh->sharedVertexData;
							pMesh->sharedVertexData = 0;
							// Skip this stream (pointer will have been returned to just after header)
							stream->skip(mCurrentstreamLen - STREAM_OVERHEAD_SIZE);
						}
						else
						{
							throw;
						}
					}
					break;
                case M_SUBMESH:
                    readSubMesh(stream, pMesh, listener);
                    break;
                case M_MESH_SKELETON_LINK:
                    readSkeletonLink(stream, pMesh, listener);
                    break;
                case M_MESH_BONE_ASSIGNMENT:
                    readMeshBoneAssignment(stream, pMesh);
                    break;
                case M_MESH_LOD:
					readMeshLodInfo(stream, pMesh);
					break;
                case M_MESH_BOUNDS:
                    readBoundsInfo(stream, pMesh);
                    break;
				case M_SUBMESH_NAME_TABLE:
    	            readSubMeshNameTable(stream, pMesh);
					break;
                case M_EDGE_LISTS:
                    readEdgeList(stream, pMesh);
                    break;
				case M_POSES:
					readPoses(stream, pMesh);
					break;
				case M_ANIMATIONS:
					readAnimations(stream, pMesh);
                    break;
                case M_TABLE_EXTREMES:
                    readExtremes(stream, pMesh);
                    break;
                }

                if (!stream->eof())
                {
                    streamID = readChunk(stream);
                }

            }
            if (!stream->eof())
            {
                // Backpedal back to start of stream
                stream->skip(-STREAM_OVERHEAD_SIZE);
            }
        }

    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::readSubMesh(DataStreamPtr& stream, Mesh* pMesh, MeshSerializerListener *listener)
    {
        unsigned short streamID;

        SubMesh* sm = pMesh->createSubMesh();

        // char* materialName
        String materialName = readString(stream);
		if(listener)
			listener->processMaterialName(pMesh, &materialName);
        sm->setMaterialName(materialName, pMesh->getGroup());

        // bool useSharedVertices
        readBools(stream,&sm->useSharedVertices, 1);

        sm->indexData->indexStart = 0;
        unsigned int indexCount = 0;
        readInts(stream, &indexCount, 1);
        sm->indexData->indexCount = indexCount;

        HardwareIndexBufferSharedPtr ibuf;
        // bool indexes32Bit
        bool idx32bit;
        readBools(stream, &idx32bit, 1);
        if (indexCount > 0)
        {
            if (idx32bit)
            {
                ibuf = HardwareBufferManager::getSingleton().
                    createIndexBuffer(
                        HardwareIndexBuffer::IT_32BIT,
                        sm->indexData->indexCount,
                        pMesh->mIndexBufferUsage,
					    pMesh->mIndexBufferShadowBuffer);
                // unsigned int* faceVertexIndices
                unsigned int* pIdx = static_cast<unsigned int*>(
                    ibuf->lock(HardwareBuffer::HBL_DISCARD)
                    );
                readInts(stream, pIdx, sm->indexData->indexCount);
                ibuf->unlock();

            }
            else // 16-bit
            {
                ibuf = HardwareBufferManager::getSingleton().
                    createIndexBuffer(
                        HardwareIndexBuffer::IT_16BIT,
                        sm->indexData->indexCount,
                        pMesh->mIndexBufferUsage,
					    pMesh->mIndexBufferShadowBuffer);
                // unsigned short* faceVertexIndices
                unsigned short* pIdx = static_cast<unsigned short*>(
                    ibuf->lock(HardwareBuffer::HBL_DISCARD)
                    );
                readShorts(stream, pIdx, sm->indexData->indexCount);
                ibuf->unlock();
            }
        }
        sm->indexData->indexBuffer = ibuf;

        // M_GEOMETRY stream (Optional: present only if useSharedVertices = false)
        if (!sm->useSharedVertices)
        {
            streamID = readChunk(stream);
            if (streamID != M_GEOMETRY)
            {
                OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Missing geometry data in mesh file",
                    "MeshSerializerImpl::readSubMesh");
            }
            sm->vertexData = OGRE_NEW VertexData();
            readGeometry(stream, pMesh, sm->vertexData);
        }


        // Find all bone assignments, submesh operation, and texture aliases (if present)
        if (!stream->eof())
        {
            streamID = readChunk(stream);
            while(!stream->eof() &&
                (streamID == M_SUBMESH_BONE_ASSIGNMENT ||
                 streamID == M_SUBMESH_OPERATION ||
                 streamID == M_SUBMESH_TEXTURE_ALIAS))
            {
                switch(streamID)
                {
                case M_SUBMESH_OPERATION:
                    readSubMeshOperation(stream, pMesh, sm);
                    break;
                case M_SUBMESH_BONE_ASSIGNMENT:
                    readSubMeshBoneAssignment(stream, pMesh, sm);
                    break;
                case M_SUBMESH_TEXTURE_ALIAS:
                    readSubMeshTextureAlias(stream, pMesh, sm);
                    break;
                }

                if (!stream->eof())
                {
                    streamID = readChunk(stream);
                }

            }
            if (!stream->eof())
            {
                // Backpedal back to start of stream
                stream->skip(-STREAM_OVERHEAD_SIZE);
            }
        }


    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::readSubMeshOperation(DataStreamPtr& stream,
        Mesh* pMesh, SubMesh* sm)
    {
        // unsigned short operationType
        unsigned short opType;
        readShorts(stream, &opType, 1);
        sm->operationType = static_cast<RenderOperation::OperationType>(opType);
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::readSubMeshTextureAlias(DataStreamPtr& stream, Mesh* pMesh, SubMesh* sub)
    {
        String aliasName = readString(stream);
        String textureName = readString(stream);
        sub->addTextureAlias(aliasName, textureName);
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writeSkeletonLink(const String& skelName)
    {
        writeChunkHeader(M_MESH_SKELETON_LINK, calcSkeletonLinkSize(skelName));

        writeString(skelName);

    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::readSkeletonLink(DataStreamPtr& stream, Mesh* pMesh, MeshSerializerListener *listener)
    {
        String skelName = readString(stream);

		if(listener)
			listener->processSkeletonName(pMesh, &skelName);

        pMesh->setSkeletonName(skelName);
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::readTextureLayer(DataStreamPtr& stream, Mesh* pMesh,
        MaterialPtr& pMat)
    {
        // Material definition section phased out of 1.1
    }
    //---------------------------------------------------------------------
    size_t MeshSerializerImpl::calcSkeletonLinkSize(const String& skelName)
    {
        size_t size = STREAM_OVERHEAD_SIZE;

        size += skelName.length() + 1;

        return size;

    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writeMeshBoneAssignment(const VertexBoneAssignment& assign)
    {
        writeChunkHeader(M_MESH_BONE_ASSIGNMENT, calcBoneAssignmentSize());

        // unsigned int vertexIndex;
        writeInts(&(assign.vertexIndex), 1);
        // unsigned short boneIndex;
        writeShorts(&(assign.boneIndex), 1);
        // float weight;
        writeFloats(&(assign.weight), 1);
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writeSubMeshBoneAssignment(const VertexBoneAssignment& assign)
    {
        writeChunkHeader(M_SUBMESH_BONE_ASSIGNMENT, calcBoneAssignmentSize());

        // unsigned int vertexIndex;
        writeInts(&(assign.vertexIndex), 1);
        // unsigned short boneIndex;
        writeShorts(&(assign.boneIndex), 1);
        // float weight;
        writeFloats(&(assign.weight), 1);
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::readMeshBoneAssignment(DataStreamPtr& stream, Mesh* pMesh)
    {
        VertexBoneAssignment assign;

        // unsigned int vertexIndex;
        readInts(stream, &(assign.vertexIndex),1);
        // unsigned short boneIndex;
        readShorts(stream, &(assign.boneIndex),1);
        // float weight;
        readFloats(stream, &(assign.weight), 1);

        pMesh->addBoneAssignment(assign);

    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::readSubMeshBoneAssignment(DataStreamPtr& stream,
        Mesh* pMesh, SubMesh* sub)
    {
        VertexBoneAssignment assign;

        // unsigned int vertexIndex;
        readInts(stream, &(assign.vertexIndex),1);
        // unsigned short boneIndex;
        readShorts(stream, &(assign.boneIndex),1);
        // float weight;
        readFloats(stream, &(assign.weight), 1);

        sub->addBoneAssignment(assign);

    }
    //---------------------------------------------------------------------
    size_t MeshSerializerImpl::calcBoneAssignmentSize(void)
    {
        size_t size = STREAM_OVERHEAD_SIZE;

        // Vert index
        size += sizeof(unsigned int);
        // Bone index
        size += sizeof(unsigned short);
        // weight
        size += sizeof(float);

        return size;
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writeLodInfo(const Mesh* pMesh)
    {
        const LodStrategy *strategy = pMesh->getLodStrategy();
        unsigned short numLods = pMesh->getNumLodLevels();
        bool manual = pMesh->isLodManual();
        writeLodSummary(numLods, manual, strategy);

		// Loop from LOD 1 (not 0, this is full detail)
        for (unsigned short i = 1; i < numLods; ++i)
        {
			const MeshLodUsage& usage = pMesh->getLodLevel(i);
			if (manual)
			{
				writeLodUsageManual(usage);
			}
			else
			{
				writeLodUsageGenerated(pMesh, usage, i);
			}

        }


    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writeLodSummary(unsigned short numLevels, bool manual, const LodStrategy *strategy)
    {
        // Header
        size_t size = STREAM_OVERHEAD_SIZE;
        // unsigned short numLevels;
        size += sizeof(unsigned short);
        // bool manual;  (true for manual alternate meshes, false for generated)
        size += sizeof(bool);
        writeChunkHeader(M_MESH_LOD, size);

        // Details
        // string strategyName;
        writeString(strategy->getName());
        // unsigned short numLevels;
        writeShorts(&numLevels, 1);
        // bool manual;  (true for manual alternate meshes, false for generated)
        writeBools(&manual, 1);


    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writeLodUsageManual(const MeshLodUsage& usage)
    {
        // Header
        size_t size = STREAM_OVERHEAD_SIZE;
        size_t manualSize = STREAM_OVERHEAD_SIZE;
        // float lodValue;
        size += sizeof(float);
        // Manual part size

        // String manualMeshName;
        manualSize += usage.manualName.length() + 1;

        size += manualSize;

        writeChunkHeader(M_MESH_LOD_USAGE, size);
        writeFloats(&(usage.userValue), 1);

        writeChunkHeader(M_MESH_LOD_MANUAL, manualSize);
        writeString(usage.manualName);


    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writeLodUsageGenerated(const Mesh* pMesh, const MeshLodUsage& usage,
		unsigned short lodNum)
    {
		// Usage Header
        size_t size = STREAM_OVERHEAD_SIZE;
		unsigned short subidx;

        // float fromDepthSquared;
        size += sizeof(float);

        // Calc generated SubMesh sections size
		for(subidx = 0; subidx < pMesh->getNumSubMeshes(); ++subidx)
		{
			// header
			size += STREAM_OVERHEAD_SIZE;
			// unsigned int numFaces;
			size += sizeof(unsigned int);
			SubMesh* sm = pMesh->getSubMesh(subidx);
            const IndexData* indexData = sm->mLodFaceList[lodNum - 1];

            // bool indexes32Bit
			size += sizeof(bool);
			// unsigned short*/int* faceIndexes;
            if (!indexData->indexBuffer.isNull() &&
				indexData->indexBuffer->getType() == HardwareIndexBuffer::IT_32BIT)
            {
			    size += static_cast<unsigned long>(
                    sizeof(unsigned int) * indexData->indexCount);
            }
            else
            {
			    size += static_cast<unsigned long>(
                    sizeof(unsigned short) * indexData->indexCount);
            }

		}

        writeChunkHeader(M_MESH_LOD_USAGE, size);
        writeFloats(&(usage.userValue), 1);

		// Now write sections
        // Calc generated SubMesh sections size
		for(subidx = 0; subidx < pMesh->getNumSubMeshes(); ++subidx)
		{
			size = STREAM_OVERHEAD_SIZE;
			// unsigned int numFaces;
			size += sizeof(unsigned int);
			SubMesh* sm = pMesh->getSubMesh(subidx);
            const IndexData* indexData = sm->mLodFaceList[lodNum - 1];
            // bool indexes32Bit
			size += sizeof(bool);
			// Lock index buffer to write
			HardwareIndexBufferSharedPtr ibuf = indexData->indexBuffer;
			// bool indexes32bit
			bool idx32 = (!ibuf.isNull() && ibuf->getType() == HardwareIndexBuffer::IT_32BIT);
			// unsigned short*/int* faceIndexes;
            if (idx32)
            {
			    size += static_cast<unsigned long>(
                    sizeof(unsigned int) * indexData->indexCount);
            }
            else
            {
			    size += static_cast<unsigned long>(
                    sizeof(unsigned short) * indexData->indexCount);
            }

			writeChunkHeader(M_MESH_LOD_GENERATED, size);
			unsigned int idxCount = static_cast<unsigned int>(indexData->indexCount);
			writeInts(&idxCount, 1);
			writeBools(&idx32, 1);

			if (idxCount > 0)
			{
				if (idx32)
				{
					unsigned int* pIdx = static_cast<unsigned int*>(
						ibuf->lock(HardwareBuffer::HBL_READ_ONLY));
					writeInts(pIdx, indexData->indexCount);
					ibuf->unlock();
				}
				else
				{
					unsigned short* pIdx = static_cast<unsigned short*>(
						ibuf->lock(HardwareBuffer::HBL_READ_ONLY));
					writeShorts(pIdx, indexData->indexCount);
					ibuf->unlock();
				}
			}
		}


    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writeBoundsInfo(const Mesh* pMesh)
    {
		// Usage Header
        unsigned long size = STREAM_OVERHEAD_SIZE;

        size += sizeof(float) * 7;
        writeChunkHeader(M_MESH_BOUNDS, size);

        // float minx, miny, minz
        const Vector3& min = pMesh->mAABB.getMinimum();
        const Vector3& max = pMesh->mAABB.getMaximum();
        writeFloats(&min.x, 1);
        writeFloats(&min.y, 1);
        writeFloats(&min.z, 1);
        // float maxx, maxy, maxz
        writeFloats(&max.x, 1);
        writeFloats(&max.y, 1);
        writeFloats(&max.z, 1);
        // float radius
        writeFloats(&pMesh->mBoundRadius, 1);

    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::readBoundsInfo(DataStreamPtr& stream, Mesh* pMesh)
    {
        Vector3 min, max;
        // float minx, miny, minz
        readFloats(stream, &min.x, 1);
        readFloats(stream, &min.y, 1);
        readFloats(stream, &min.z, 1);
        // float maxx, maxy, maxz
        readFloats(stream, &max.x, 1);
        readFloats(stream, &max.y, 1);
        readFloats(stream, &max.z, 1);
        AxisAlignedBox box(min, max);
        pMesh->_setBounds(box, true);
        // float radius
        float radius;
        readFloats(stream, &radius, 1);
        pMesh->_setBoundingSphereRadius(radius);



    }
    //---------------------------------------------------------------------
	void MeshSerializerImpl::readMeshLodInfo(DataStreamPtr& stream, Mesh* pMesh)
	{
		unsigned short streamID, i;

        // Read the strategy to be used for this mesh
        String strategyName = readString(stream);
        LodStrategy *strategy = LodStrategyManager::getSingleton().getStrategy(strategyName);
        pMesh->setLodStrategy(strategy);

        // unsigned short numLevels;
		readShorts(stream, &(pMesh->mNumLods), 1);
        // bool manual;  (true for manual alternate meshes, false for generated)
		readBools(stream, &(pMesh->mIsLodManual), 1);

		// Preallocate submesh lod face data if not manual
		if (!pMesh->mIsLodManual)
		{
			unsigned short numsubs = pMesh->getNumSubMeshes();
			for (i = 0; i < numsubs; ++i)
			{
				SubMesh* sm = pMesh->getSubMesh(i);
				sm->mLodFaceList.resize(pMesh->mNumLods-1);
			}
		}

		// Loop from 1 rather than 0 (full detail index is not in file)
		for (i = 1; i < pMesh->mNumLods; ++i)
		{
			streamID = readChunk(stream);
			if (streamID != M_MESH_LOD_USAGE)
			{
				OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND,
					"Missing M_MESH_LOD_USAGE stream in " + pMesh->getName(),
					"MeshSerializerImpl::readMeshLodInfo");
			}
			// Read depth
			MeshLodUsage usage;
			readFloats(stream, &(usage.userValue), 1);

			if (pMesh->isLodManual())
			{
				readMeshLodUsageManual(stream, pMesh, i, usage);
			}
			else //(!pMesh->isLodManual)
			{
				readMeshLodUsageGenerated(stream, pMesh, i, usage);
			}
            usage.edgeData = NULL;

			// Save usage
			pMesh->mMeshLodUsageList.push_back(usage);
		}


	}
    //---------------------------------------------------------------------
	void MeshSerializerImpl::readMeshLodUsageManual(DataStreamPtr& stream,
        Mesh* pMesh, unsigned short lodNum, MeshLodUsage& usage)
	{
		unsigned long streamID;
		// Read detail stream
		streamID = readChunk(stream);
		if (streamID != M_MESH_LOD_MANUAL)
		{
			OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND,
				"Missing M_MESH_LOD_MANUAL stream in " + pMesh->getName(),
				"MeshSerializerImpl::readMeshLodUsageManual");
		}

		usage.manualName = readString(stream);
		usage.manualMesh.setNull(); // will trigger load later
	}
    //---------------------------------------------------------------------
	void MeshSerializerImpl::readMeshLodUsageGenerated(DataStreamPtr& stream,
        Mesh* pMesh, unsigned short lodNum, MeshLodUsage& usage)
	{
		usage.manualName = "";
		usage.manualMesh.setNull();

		// Get one set of detail per SubMesh
		unsigned short numSubs, i;
		unsigned long streamID;
		numSubs = pMesh->getNumSubMeshes();
		for (i = 0; i < numSubs; ++i)
		{
			streamID = readChunk(stream);
			if (streamID != M_MESH_LOD_GENERATED)
			{
				OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND,
					"Missing M_MESH_LOD_GENERATED stream in " + pMesh->getName(),
					"MeshSerializerImpl::readMeshLodUsageGenerated");
			}

			SubMesh* sm = pMesh->getSubMesh(i);
			// lodNum - 1 because SubMesh doesn't store full detail LOD
            sm->mLodFaceList[lodNum - 1] = OGRE_NEW IndexData();
			IndexData* indexData = sm->mLodFaceList[lodNum - 1];
            // unsigned int numIndexes
            unsigned int numIndexes;
			readInts(stream, &numIndexes, 1);
            indexData->indexCount = static_cast<size_t>(numIndexes);
            // bool indexes32Bit
            bool idx32Bit;
            readBools(stream, &idx32Bit, 1);
            // unsigned short*/int* faceIndexes;  ((v1, v2, v3) * numFaces)
            if (idx32Bit)
            {
                indexData->indexBuffer = HardwareBufferManager::getSingleton().
                    createIndexBuffer(HardwareIndexBuffer::IT_32BIT, indexData->indexCount,
                    pMesh->mIndexBufferUsage, pMesh->mIndexBufferShadowBuffer);
                unsigned int* pIdx = static_cast<unsigned int*>(
                    indexData->indexBuffer->lock(
                        0,
                        indexData->indexBuffer->getSizeInBytes(),
                        HardwareBuffer::HBL_DISCARD) );

			    readInts(stream, pIdx, indexData->indexCount);
                indexData->indexBuffer->unlock();

            }
            else
            {
                indexData->indexBuffer = HardwareBufferManager::getSingleton().
                    createIndexBuffer(HardwareIndexBuffer::IT_16BIT, indexData->indexCount,
                    pMesh->mIndexBufferUsage, pMesh->mIndexBufferShadowBuffer);
                unsigned short* pIdx = static_cast<unsigned short*>(
                    indexData->indexBuffer->lock(
                        0,
                        indexData->indexBuffer->getSizeInBytes(),
                        HardwareBuffer::HBL_DISCARD) );
			    readShorts(stream, pIdx, indexData->indexCount);
                indexData->indexBuffer->unlock();

            }

		}
	}
    //---------------------------------------------------------------------
    void MeshSerializerImpl::flipFromLittleEndian(void* pData, size_t vertexCount,
        size_t vertexSize, const VertexDeclaration::VertexElementList& elems)
	{
		if (mFlipEndian)
		{
	        flipEndian(pData, vertexCount, vertexSize, elems);
		}
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::flipToLittleEndian(void* pData, size_t vertexCount,
			size_t vertexSize, const VertexDeclaration::VertexElementList& elems)
	{
		if (mFlipEndian)
		{
	        flipEndian(pData, vertexCount, vertexSize, elems);
		}
	}
    //---------------------------------------------------------------------
    void MeshSerializerImpl::flipEndian(void* pData, size_t vertexCount,
        size_t vertexSize, const VertexDeclaration::VertexElementList& elems)
	{
		void *pBase = pData;
		for (size_t v = 0; v < vertexCount; ++v)
		{
			VertexDeclaration::VertexElementList::const_iterator ei, eiend;
			eiend = elems.end();
			for (ei = elems.begin(); ei != eiend; ++ei)
			{
				void *pElem;
				// re-base pointer to the element
				(*ei).baseVertexPointerToElement(pBase, &pElem);
				// Flip the endian based on the type
				size_t typeSize = 0;
				switch (VertexElement::getBaseType((*ei).getType()))
				{
					case VET_FLOAT1:
						typeSize = sizeof(float);
						break;
					case VET_SHORT1:
						typeSize = sizeof(short);
						break;
					case VET_COLOUR:
					case VET_COLOUR_ABGR:
					case VET_COLOUR_ARGB:
						typeSize = sizeof(RGBA);
						break;
					case VET_UBYTE4:
						typeSize = 0; // NO FLIPPING
						break;
					default:
						assert(false); // Should never happen
				};
                Serializer::flipEndian(pElem, typeSize,
					VertexElement::getTypeCount((*ei).getType()));

			}

			pBase = static_cast<void*>(
				static_cast<unsigned char*>(pBase) + vertexSize);

		}
	}
    //---------------------------------------------------------------------
	size_t MeshSerializerImpl::calcEdgeListSize(const Mesh* pMesh)
	{
        size_t size = STREAM_OVERHEAD_SIZE;

        for (ushort i = 0; i < pMesh->getNumLodLevels(); ++i)
        {

            const EdgeData* edgeData = pMesh->getEdgeList(i);
            bool isManual = pMesh->isLodManual() && (i > 0);

            size += calcEdgeListLodSize(edgeData, isManual);

        }

        return size;
	}
    //---------------------------------------------------------------------
    size_t MeshSerializerImpl::calcEdgeListLodSize(const EdgeData* edgeData, bool isManual)
    {
        size_t size = STREAM_OVERHEAD_SIZE;

        // unsigned short lodIndex
        size += sizeof(uint16);

        // bool isManual			// If manual, no edge data here, loaded from manual mesh
        size += sizeof(bool);
        if (!isManual)
        {
            // bool isClosed
            size += sizeof(bool);
            // unsigned long numTriangles
            size += sizeof(uint32);
            // unsigned long numEdgeGroups
            size += sizeof(uint32);
            // Triangle* triangleList
            size_t triSize = 0;
            // unsigned long indexSet
            // unsigned long vertexSet
            // unsigned long vertIndex[3]
            // unsigned long sharedVertIndex[3]
            // float normal[4]
            triSize += sizeof(uint32) * 8
                    + sizeof(float) * 4;

            size += triSize * edgeData->triangles.size();
            // Write the groups
            for (EdgeData::EdgeGroupList::const_iterator gi = edgeData->edgeGroups.begin();
                gi != edgeData->edgeGroups.end(); ++gi)
            {
                const EdgeData::EdgeGroup& edgeGroup = *gi;
                size += calcEdgeGroupSize(edgeGroup);
            }

        }

        return size;
    }
    //---------------------------------------------------------------------
    size_t MeshSerializerImpl::calcEdgeGroupSize(const EdgeData::EdgeGroup& group)
    {
        size_t size = STREAM_OVERHEAD_SIZE;

        // unsigned long vertexSet
        size += sizeof(uint32);
        // unsigned long triStart
        size += sizeof(uint32);
        // unsigned long triCount
        size += sizeof(uint32);
        // unsigned long numEdges
        size += sizeof(uint32);
        // Edge* edgeList
        size_t edgeSize = 0;
        // unsigned long  triIndex[2]
        // unsigned long  vertIndex[2]
        // unsigned long  sharedVertIndex[2]
        // bool degenerate
        edgeSize += sizeof(uint32) * 6 + sizeof(bool);
        size += edgeSize * group.edges.size();

        return size;
    }
    //---------------------------------------------------------------------
	void MeshSerializerImpl::writeEdgeList(const Mesh* pMesh)
	{
        writeChunkHeader(M_EDGE_LISTS, calcEdgeListSize(pMesh));

        for (ushort i = 0; i < pMesh->getNumLodLevels(); ++i)
        {
            const EdgeData* edgeData = pMesh->getEdgeList(i);
            bool isManual = pMesh->isLodManual() && (i > 0);
            writeChunkHeader(M_EDGE_LIST_LOD, calcEdgeListLodSize(edgeData, isManual));

            // unsigned short lodIndex
            writeShorts(&i, 1);

            // bool isManual			// If manual, no edge data here, loaded from manual mesh
            writeBools(&isManual, 1);
            if (!isManual)
            {
                // bool isClosed
                writeBools(&edgeData->isClosed, 1);
                // unsigned long  numTriangles
                uint32 count = static_cast<uint32>(edgeData->triangles.size());
                writeInts(&count, 1);
                // unsigned long numEdgeGroups
                count = static_cast<uint32>(edgeData->edgeGroups.size());
                writeInts(&count, 1);
                // Triangle* triangleList
                // Iterate rather than writing en-masse to allow endian conversion
                EdgeData::TriangleList::const_iterator t = edgeData->triangles.begin();
                EdgeData::TriangleFaceNormalList::const_iterator fni = edgeData->triangleFaceNormals.begin();
                for ( ; t != edgeData->triangles.end(); ++t, ++fni)
                {
                    const EdgeData::Triangle& tri = *t;
                    // unsigned long indexSet;
                    uint32 tmp[3];
                    tmp[0] = tri.indexSet;
                    writeInts(tmp, 1);
                    // unsigned long vertexSet;
                    tmp[0] = tri.vertexSet;
                    writeInts(tmp, 1);
                    // unsigned long vertIndex[3];
                    tmp[0] = tri.vertIndex[0];
                    tmp[1] = tri.vertIndex[1];
                    tmp[2] = tri.vertIndex[2];
                    writeInts(tmp, 3);
                    // unsigned long sharedVertIndex[3];
                    tmp[0] = tri.sharedVertIndex[0];
                    tmp[1] = tri.sharedVertIndex[1];
                    tmp[2] = tri.sharedVertIndex[2];
                    writeInts(tmp, 3);
                    // float normal[4];
                    writeFloats(&(fni->x), 4);

                }
                // Write the groups
                for (EdgeData::EdgeGroupList::const_iterator gi = edgeData->edgeGroups.begin();
                    gi != edgeData->edgeGroups.end(); ++gi)
                {
                    const EdgeData::EdgeGroup& edgeGroup = *gi;
                    writeChunkHeader(M_EDGE_GROUP, calcEdgeGroupSize(edgeGroup));
                    // unsigned long vertexSet
                    uint32 vertexSet = static_cast<uint32>(edgeGroup.vertexSet);
                    writeInts(&vertexSet, 1);
                    // unsigned long triStart
                    uint32 triStart = static_cast<uint32>(edgeGroup.triStart);
                    writeInts(&triStart, 1);
                    // unsigned long triCount
                    uint32 triCount = static_cast<uint32>(edgeGroup.triCount);
                    writeInts(&triCount, 1);
                    // unsigned long numEdges
                    count = static_cast<uint32>(edgeGroup.edges.size());
                    writeInts(&count, 1);
                    // Edge* edgeList
                    // Iterate rather than writing en-masse to allow endian conversion
                    for (EdgeData::EdgeList::const_iterator ei = edgeGroup.edges.begin();
                        ei != edgeGroup.edges.end(); ++ei)
                    {
                        const EdgeData::Edge& edge = *ei;
                        uint32 tmp[2];
                        // unsigned long  triIndex[2]
                        tmp[0] = edge.triIndex[0];
                        tmp[1] = edge.triIndex[1];
                        writeInts(tmp, 2);
                        // unsigned long  vertIndex[2]
                        tmp[0] = edge.vertIndex[0];
                        tmp[1] = edge.vertIndex[1];
                        writeInts(tmp, 2);
                        // unsigned long  sharedVertIndex[2]
                        tmp[0] = edge.sharedVertIndex[0];
                        tmp[1] = edge.sharedVertIndex[1];
                        writeInts(tmp, 2);
                        // bool degenerate
                        writeBools(&(edge.degenerate), 1);
                    }

                }

            }

        }
	}
    //---------------------------------------------------------------------
	void MeshSerializerImpl::readEdgeList(DataStreamPtr& stream, Mesh* pMesh)
	{
        unsigned short streamID;

        if (!stream->eof())
        {
            streamID = readChunk(stream);
            while(!stream->eof() &&
                streamID == M_EDGE_LIST_LOD)
            {
                // Process single LOD

                // unsigned short lodIndex
                unsigned short lodIndex;
                readShorts(stream, &lodIndex, 1);

                // bool isManual			// If manual, no edge data here, loaded from manual mesh
                bool isManual;
                readBools(stream, &isManual, 1);
                // Only load in non-manual levels; others will be connected up by Mesh on demand
                if (!isManual)
                {
                    MeshLodUsage& usage = const_cast<MeshLodUsage&>(pMesh->getLodLevel(lodIndex));

                    usage.edgeData = OGRE_NEW EdgeData();

                    // Read detail information of the edge list
                    readEdgeListLodInfo(stream, usage.edgeData);

                    // Postprocessing edge groups
                    EdgeData::EdgeGroupList::iterator egi, egend;
                    egend = usage.edgeData->edgeGroups.end();
                    for (egi = usage.edgeData->edgeGroups.begin(); egi != egend; ++egi)
                    {
                        EdgeData::EdgeGroup& edgeGroup = *egi;
                        // Populate edgeGroup.vertexData pointers
                        // If there is shared vertex data, vertexSet 0 is that,
                        // otherwise 0 is first dedicated
                        if (pMesh->sharedVertexData)
                        {
                            if (edgeGroup.vertexSet == 0)
                            {
                                edgeGroup.vertexData = pMesh->sharedVertexData;
                            }
                            else
                            {
                                edgeGroup.vertexData = pMesh->getSubMesh(
                                    edgeGroup.vertexSet-1)->vertexData;
                            }
                        }
                        else
                        {
                            edgeGroup.vertexData = pMesh->getSubMesh(
                                edgeGroup.vertexSet)->vertexData;
                        }
                    }
                }

                if (!stream->eof())
                {
                    streamID = readChunk(stream);
                }

            }
            if (!stream->eof())
            {
                // Backpedal back to start of stream
                stream->skip(-STREAM_OVERHEAD_SIZE);
            }
        }

        pMesh->mEdgeListsBuilt = true;
	}
	//---------------------------------------------------------------------
    void MeshSerializerImpl::readEdgeListLodInfo(DataStreamPtr& stream,
        EdgeData* edgeData)
    {
        // bool isClosed
        readBools(stream, &edgeData->isClosed, 1);
        // unsigned long numTriangles
        uint32 numTriangles;
        readInts(stream, &numTriangles, 1);
        // Allocate correct amount of memory
        edgeData->triangles.resize(numTriangles);
        edgeData->triangleFaceNormals.resize(numTriangles);
        edgeData->triangleLightFacings.resize(numTriangles);
        // unsigned long numEdgeGroups
        uint32 numEdgeGroups;
        readInts(stream, &numEdgeGroups, 1);
        // Allocate correct amount of memory
        edgeData->edgeGroups.resize(numEdgeGroups);
        // Triangle* triangleList
        uint32 tmp[3];
        for (size_t t = 0; t < numTriangles; ++t)
        {
            EdgeData::Triangle& tri = edgeData->triangles[t];
            // unsigned long indexSet
            readInts(stream, tmp, 1);
            tri.indexSet = tmp[0];
            // unsigned long vertexSet
            readInts(stream, tmp, 1);
            tri.vertexSet = tmp[0];
            // unsigned long vertIndex[3]
            readInts(stream, tmp, 3);
            tri.vertIndex[0] = tmp[0];
            tri.vertIndex[1] = tmp[1];
            tri.vertIndex[2] = tmp[2];
            // unsigned long sharedVertIndex[3]
            readInts(stream, tmp, 3);
            tri.sharedVertIndex[0] = tmp[0];
            tri.sharedVertIndex[1] = tmp[1];
            tri.sharedVertIndex[2] = tmp[2];
            // float normal[4]
            readFloats(stream, &(edgeData->triangleFaceNormals[t].x), 4);

        }

        for (uint32 eg = 0; eg < numEdgeGroups; ++eg)
        {
            unsigned short streamID = readChunk(stream);
            if (streamID != M_EDGE_GROUP)
            {
                OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                    "Missing M_EDGE_GROUP stream",
                    "MeshSerializerImpl::readEdgeListLodInfo");
            }
            EdgeData::EdgeGroup& edgeGroup = edgeData->edgeGroups[eg];

            // unsigned long vertexSet
            readInts(stream, tmp, 1);
            edgeGroup.vertexSet = tmp[0];
            // unsigned long triStart
            readInts(stream, tmp, 1);
            edgeGroup.triStart = tmp[0];
            // unsigned long triCount
            readInts(stream, tmp, 1);
            edgeGroup.triCount = tmp[0];
            // unsigned long numEdges
            uint32 numEdges;
            readInts(stream, &numEdges, 1);
            edgeGroup.edges.resize(numEdges);
            // Edge* edgeList
            for (uint32 e = 0; e < numEdges; ++e)
            {
                EdgeData::Edge& edge = edgeGroup.edges[e];
                // unsigned long  triIndex[2]
                readInts(stream, tmp, 2);
                edge.triIndex[0] = tmp[0];
                edge.triIndex[1] = tmp[1];
                // unsigned long  vertIndex[2]
                readInts(stream, tmp, 2);
                edge.vertIndex[0] = tmp[0];
                edge.vertIndex[1] = tmp[1];
                // unsigned long  sharedVertIndex[2]
                readInts(stream, tmp, 2);
                edge.sharedVertIndex[0] = tmp[0];
                edge.sharedVertIndex[1] = tmp[1];
                // bool degenerate
                readBools(stream, &(edge.degenerate), 1);
            }
        }
    }
	//---------------------------------------------------------------------
	size_t MeshSerializerImpl::calcAnimationsSize(const Mesh* pMesh)
	{
		size_t size = STREAM_OVERHEAD_SIZE;

		for (unsigned short a = 0; a < pMesh->getNumAnimations(); ++a)
		{
			Animation* anim = pMesh->getAnimation(a);
			size += calcAnimationSize(anim);
		}
		return size;

	}
	//---------------------------------------------------------------------
	size_t MeshSerializerImpl::calcAnimationSize(const Animation* anim)
	{
		size_t size = STREAM_OVERHEAD_SIZE;
		// char* name
		size += anim->getName().length() + 1;

		// float length
		size += sizeof(float);

		Animation::VertexTrackIterator trackIt = anim->getVertexTrackIterator();
		while (trackIt.hasMoreElements())
		{
			VertexAnimationTrack* vt = trackIt.getNext();
			size += calcAnimationTrackSize(vt);
		}

		return size;
	}
	//---------------------------------------------------------------------
	size_t MeshSerializerImpl::calcAnimationTrackSize(const VertexAnimationTrack* track)
	{
		size_t size = STREAM_OVERHEAD_SIZE;
		// uint16 type
		size += sizeof(uint16);
		// unsigned short target		// 0 for shared geometry,
		size += sizeof(unsigned short);

		if (track->getAnimationType() == VAT_MORPH)
		{
			for (unsigned short i = 0; i < track->getNumKeyFrames(); ++i)
			{
				VertexMorphKeyFrame* kf = track->getVertexMorphKeyFrame(i);
				size += calcMorphKeyframeSize(kf, track->getAssociatedVertexData()->vertexCount);
			}
		}
		else
		{
			for (unsigned short i = 0; i < track->getNumKeyFrames(); ++i)
			{
				VertexPoseKeyFrame* kf = track->getVertexPoseKeyFrame(i);
				size += calcPoseKeyframeSize(kf);
			}
		}
		return size;
	}
	//---------------------------------------------------------------------
	size_t MeshSerializerImpl::calcMorphKeyframeSize(const VertexMorphKeyFrame* kf,
		size_t vertexCount)
	{
		size_t size = STREAM_OVERHEAD_SIZE;
		// float time
		size += sizeof(float);
		// bool isOriginalGeometry  // point at original geometry?
		size += sizeof(bool);
		// float x,y,z
		size += sizeof(float) * 3 * vertexCount;

		return size;
	}
	//---------------------------------------------------------------------
	size_t MeshSerializerImpl::calcPoseKeyframeSize(const VertexPoseKeyFrame* kf)
	{
		size_t size = STREAM_OVERHEAD_SIZE;

		// float time
		size += sizeof(float);

		size += calcPoseKeyframePoseRefSize() * kf->getPoseReferences().size();

		return size;

	}
	//---------------------------------------------------------------------
	size_t MeshSerializerImpl::calcPoseKeyframePoseRefSize(void)
	{
		size_t size = STREAM_OVERHEAD_SIZE;
		// unsigned short poseIndex
		size += sizeof(uint16);
		// float influence
		size += sizeof(float);

		return size;

	}
	//---------------------------------------------------------------------
	size_t MeshSerializerImpl::calcPosesSize(const Mesh* pMesh)
	{
		size_t size = STREAM_OVERHEAD_SIZE;

		Mesh::ConstPoseIterator poseIt = pMesh->getPoseIterator();
		while (poseIt.hasMoreElements())
		{
			size += calcPoseSize(poseIt.getNext());
		}
		return size;
	}
	//---------------------------------------------------------------------
	size_t MeshSerializerImpl::calcPoseSize(const Pose* pose)
	{
		size_t size = STREAM_OVERHEAD_SIZE;

		// char* name (may be blank)
		size += pose->getName().length() + 1;
		// unsigned short target
		size += sizeof(uint16);

		// vertex offsets
		size += pose->getVertexOffsets().size() * calcPoseVertexSize();

		return size;

	}
	//---------------------------------------------------------------------
	size_t MeshSerializerImpl::calcPoseVertexSize(void)
	{
		size_t size = STREAM_OVERHEAD_SIZE;
		// unsigned long vertexIndex
		size += sizeof(uint32);
		// float xoffset, yoffset, zoffset
		size += sizeof(float) * 3;

		return size;
	}
	//---------------------------------------------------------------------
	void MeshSerializerImpl::writePoses(const Mesh* pMesh)
	{
		Mesh::ConstPoseIterator poseIterator = pMesh->getPoseIterator();
		if (poseIterator.hasMoreElements())
		{
			writeChunkHeader(M_POSES, calcPosesSize(pMesh));
			while (poseIterator.hasMoreElements())
			{
				writePose(poseIterator.getNext());
			}
		}

	}
	//---------------------------------------------------------------------
	void MeshSerializerImpl::writePose(const Pose* pose)
	{
		writeChunkHeader(M_POSE, calcPoseSize(pose));

		// char* name (may be blank)
		writeString(pose->getName());

		// unsigned short target
		ushort val = pose->getTarget();
		writeShorts(&val, 1);

		size_t vertexSize = calcPoseVertexSize();
		Pose::ConstVertexOffsetIterator vit = pose->getVertexOffsetIterator();
		while (vit.hasMoreElements())
		{
			uint32 vertexIndex = (uint32)vit.peekNextKey();
			Vector3 offset = vit.getNext();
			writeChunkHeader(M_POSE_VERTEX, vertexSize);
			// unsigned long vertexIndex
			writeInts(&vertexIndex, 1);
			// float xoffset, yoffset, zoffset
			writeFloats(offset.ptr(), 3);
		}


	}
	//---------------------------------------------------------------------
	void MeshSerializerImpl::writeAnimations(const Mesh* pMesh)
	{
		writeChunkHeader(M_ANIMATIONS, calcAnimationsSize(pMesh));

		for (unsigned short a = 0; a < pMesh->getNumAnimations(); ++a)
		{
			Animation* anim = pMesh->getAnimation(a);
			LogManager::getSingleton().logMessage("Exporting animation " + anim->getName());
			writeAnimation(anim);
			LogManager::getSingleton().logMessage("Animation exported.");
		}
	}
	//---------------------------------------------------------------------
	void MeshSerializerImpl::writeAnimation(const Animation* anim)
	{
		writeChunkHeader(M_ANIMATION, calcAnimationSize(anim));
		// char* name
		writeString(anim->getName());
		// float length
		float len = anim->getLength();
		writeFloats(&len, 1);
		Animation::VertexTrackIterator trackIt = anim->getVertexTrackIterator();
		while (trackIt.hasMoreElements())
		{
			VertexAnimationTrack* vt = trackIt.getNext();
			writeAnimationTrack(vt);
		}


	}
    //---------------------------------------------------------------------
	void MeshSerializerImpl::writeAnimationTrack(const VertexAnimationTrack* track)
	{
		writeChunkHeader(M_ANIMATION_TRACK, calcAnimationTrackSize(track));
		// unsigned short type			// 1 == morph, 2 == pose
		uint16 animType = (uint16)track->getAnimationType();
		writeShorts(&animType, 1);
		// unsigned short target
		uint16 target = track->getHandle();
		writeShorts(&target, 1);

		if (track->getAnimationType() == VAT_MORPH)
		{
			for (unsigned short i = 0; i < track->getNumKeyFrames(); ++i)
			{
				VertexMorphKeyFrame* kf = track->getVertexMorphKeyFrame(i);
				writeMorphKeyframe(kf, track->getAssociatedVertexData()->vertexCount);
			}
		}
		else // VAT_POSE
		{
			for (unsigned short i = 0; i < track->getNumKeyFrames(); ++i)
			{
				VertexPoseKeyFrame* kf = track->getVertexPoseKeyFrame(i);
				writePoseKeyframe(kf);
			}
		}

	}
	//---------------------------------------------------------------------
	void MeshSerializerImpl::writeMorphKeyframe(const VertexMorphKeyFrame* kf, size_t vertexCount)
	{
		writeChunkHeader(M_ANIMATION_MORPH_KEYFRAME, calcMorphKeyframeSize(kf, vertexCount));
		// float time
		float timePos = kf->getTime();
		writeFloats(&timePos, 1);
		// float x,y,z			// repeat by number of vertices in original geometry
		float* pSrc = static_cast<float*>(
			kf->getVertexBuffer()->lock(HardwareBuffer::HBL_READ_ONLY));
		writeFloats(pSrc, vertexCount * 3);
		kf->getVertexBuffer()->unlock();
	}
	//---------------------------------------------------------------------
	void MeshSerializerImpl::writePoseKeyframe(const VertexPoseKeyFrame* kf)
	{
		writeChunkHeader(M_ANIMATION_POSE_KEYFRAME, calcPoseKeyframeSize(kf));
		// float time
		float timePos = kf->getTime();
		writeFloats(&timePos, 1);

		// pose references
		VertexPoseKeyFrame::ConstPoseRefIterator poseRefIt =
			kf->getPoseReferenceIterator();
		while (poseRefIt.hasMoreElements())
		{
			writePoseKeyframePoseRef(poseRefIt.getNext());
		}



	}
	//---------------------------------------------------------------------
	void MeshSerializerImpl::writePoseKeyframePoseRef(
		const VertexPoseKeyFrame::PoseRef& poseRef)
	{
		writeChunkHeader(M_ANIMATION_POSE_REF, calcPoseKeyframePoseRefSize());
		// unsigned short poseIndex
		writeShorts(&(poseRef.poseIndex), 1);
		// float influence
		writeFloats(&(poseRef.influence), 1);
	}
	//---------------------------------------------------------------------
	void MeshSerializerImpl::readPoses(DataStreamPtr& stream, Mesh* pMesh)
	{
		unsigned short streamID;

		// Find all substreams
		if (!stream->eof())
		{
			streamID = readChunk(stream);
			while(!stream->eof() &&
				(streamID == M_POSE))
			{
				switch(streamID)
				{
				case M_POSE:
					readPose(stream, pMesh);
					break;

				}

				if (!stream->eof())
				{
					streamID = readChunk(stream);
				}

			}
			if (!stream->eof())
			{
				// Backpedal back to start of stream
				stream->skip(-STREAM_OVERHEAD_SIZE);
			}
		}
	}
	//---------------------------------------------------------------------
	void MeshSerializerImpl::readPose(DataStreamPtr& stream, Mesh* pMesh)
	{
		// char* name (may be blank)
		String name = readString(stream);
		// unsigned short target
		unsigned short target;
		readShorts(stream, &target, 1);

		Pose* pose = pMesh->createPose(target, name);

		// Find all substreams
		unsigned short streamID;
		if (!stream->eof())
		{
			streamID = readChunk(stream);
			while(!stream->eof() &&
				(streamID == M_POSE_VERTEX))
			{
				switch(streamID)
				{
				case M_POSE_VERTEX:
					// create vertex offset
					uint32 vertIndex;
					Vector3 offset;
					// unsigned long vertexIndex
					readInts(stream, &vertIndex, 1);
					// float xoffset, yoffset, zoffset
					readFloats(stream, offset.ptr(), 3);

					pose->addVertex(vertIndex, offset);
					break;

				}

				if (!stream->eof())
				{
					streamID = readChunk(stream);
				}

			}
			if (!stream->eof())
			{
				// Backpedal back to start of stream
				stream->skip(-STREAM_OVERHEAD_SIZE);
			}
		}

	}
	//---------------------------------------------------------------------
	void MeshSerializerImpl::readAnimations(DataStreamPtr& stream, Mesh* pMesh)
	{
		unsigned short streamID;

		// Find all substreams
		if (!stream->eof())
		{
			streamID = readChunk(stream);
			while(!stream->eof() &&
				(streamID == M_ANIMATION))
			{
				switch(streamID)
				{
				case M_ANIMATION:
					readAnimation(stream, pMesh);
					break;

				}

				if (!stream->eof())
				{
					streamID = readChunk(stream);
				}

			}
			if (!stream->eof())
			{
				// Backpedal back to start of stream
				stream->skip(-STREAM_OVERHEAD_SIZE);
			}
		}


	}
	//---------------------------------------------------------------------
	void MeshSerializerImpl::readAnimation(DataStreamPtr& stream, Mesh* pMesh)
	{

		// char* name
		String name = readString(stream);
		// float length
		float len;
		readFloats(stream, &len, 1);

		Animation* anim = pMesh->createAnimation(name, len);

		// tracks
		unsigned short streamID;

		if (!stream->eof())
		{
			streamID = readChunk(stream);
			while(!stream->eof() &&
				streamID == M_ANIMATION_TRACK)
			{
				switch(streamID)
				{
				case M_ANIMATION_TRACK:
					readAnimationTrack(stream, anim, pMesh);
					break;
				};
				if (!stream->eof())
				{
					streamID = readChunk(stream);
				}

			}
			if (!stream->eof())
			{
				// Backpedal back to start of stream
				stream->skip(-STREAM_OVERHEAD_SIZE);
			}
		}
	}
	//---------------------------------------------------------------------
	void MeshSerializerImpl::readAnimationTrack(DataStreamPtr& stream,
		Animation* anim, Mesh* pMesh)
	{
		// ushort type
		uint16 inAnimType;
		readShorts(stream, &inAnimType, 1);
		VertexAnimationType animType = (VertexAnimationType)inAnimType;

		// unsigned short target
		uint16 target;
		readShorts(stream, &target, 1);

		VertexAnimationTrack* track = anim->createVertexTrack(target,
			pMesh->getVertexDataByTrackHandle(target), animType);

		// keyframes
		unsigned short streamID;

		if (!stream->eof())
		{
			streamID = readChunk(stream);
			while(!stream->eof() &&
				(streamID == M_ANIMATION_MORPH_KEYFRAME ||
				 streamID == M_ANIMATION_POSE_KEYFRAME))
			{
				switch(streamID)
				{
				case M_ANIMATION_MORPH_KEYFRAME:
					readMorphKeyFrame(stream, track);
					break;
				case M_ANIMATION_POSE_KEYFRAME:
					readPoseKeyFrame(stream, track);
					break;
				};
				if (!stream->eof())
				{
					streamID = readChunk(stream);
				}

			}
			if (!stream->eof())
			{
				// Backpedal back to start of stream
				stream->skip(-STREAM_OVERHEAD_SIZE);
			}
		}

	}
	//---------------------------------------------------------------------
	void MeshSerializerImpl::readMorphKeyFrame(DataStreamPtr& stream, VertexAnimationTrack* track)
	{
		// float time
		float timePos;
		readFloats(stream, &timePos, 1);

		VertexMorphKeyFrame* kf = track->createVertexMorphKeyFrame(timePos);

		// Create buffer, allow read and use shadow buffer
		size_t vertexCount = track->getAssociatedVertexData()->vertexCount;
		HardwareVertexBufferSharedPtr vbuf =
			HardwareBufferManager::getSingleton().createVertexBuffer(
				VertexElement::getTypeSize(VET_FLOAT3), vertexCount,
				HardwareBuffer::HBU_STATIC, true);
		// float x,y,z			// repeat by number of vertices in original geometry
		float* pDst = static_cast<float*>(
			vbuf->lock(HardwareBuffer::HBL_DISCARD));
		readFloats(stream, pDst, vertexCount * 3);
		vbuf->unlock();
		kf->setVertexBuffer(vbuf);

	}
	//---------------------------------------------------------------------
	void MeshSerializerImpl::readPoseKeyFrame(DataStreamPtr& stream, VertexAnimationTrack* track)
	{
		// float time
		float timePos;
		readFloats(stream, &timePos, 1);

		// Create keyframe
		VertexPoseKeyFrame* kf = track->createVertexPoseKeyFrame(timePos);

		unsigned short streamID;

		if (!stream->eof())
		{
			streamID = readChunk(stream);
			while(!stream->eof() &&
				streamID == M_ANIMATION_POSE_REF)
			{
				switch(streamID)
				{
				case M_ANIMATION_POSE_REF:
					uint16 poseIndex;
					float influence;
					// unsigned short poseIndex
					readShorts(stream, &poseIndex, 1);
					// float influence
					readFloats(stream, &influence, 1);

					kf->addPoseReference(poseIndex, influence);

					break;
				};
				if (!stream->eof())
				{
					streamID = readChunk(stream);
				}

			}
			if (!stream->eof())
			{
				// Backpedal back to start of stream
				stream->skip(-STREAM_OVERHEAD_SIZE);
			}
		}

	}
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    MeshSerializerImpl_v1_4::MeshSerializerImpl_v1_4()
    {
        // Version number
        mVersion = "[MeshSerializer_v1.40]";
    }
    //---------------------------------------------------------------------
    MeshSerializerImpl_v1_4::~MeshSerializerImpl_v1_4()
    {
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl_v1_4::writeLodSummary(unsigned short numLevels, bool manual, const LodStrategy *strategy)
    {
        // Header
        size_t size = STREAM_OVERHEAD_SIZE;
        // unsigned short numLevels;
        size += sizeof(unsigned short);
        // bool manual;  (true for manual alternate meshes, false for generated)
        size += sizeof(bool);
        writeChunkHeader(M_MESH_LOD, size);

        // Details
        // unsigned short numLevels;
        writeShorts(&numLevels, 1);
        // bool manual;  (true for manual alternate meshes, false for generated)
        writeBools(&manual, 1);


    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl_v1_4::readMeshLodInfo(DataStreamPtr& stream, Mesh* pMesh)
    {
        unsigned short streamID, i;

        // Use the old strategy for this mesh
        LodStrategy *strategy = DistanceLodStrategy::getSingletonPtr();
        pMesh->setLodStrategy(strategy);

        // unsigned short numLevels;
        readShorts(stream, &(pMesh->mNumLods), 1);
        // bool manual;  (true for manual alternate meshes, false for generated)
        readBools(stream, &(pMesh->mIsLodManual), 1);

        // Preallocate submesh lod face data if not manual
        if (!pMesh->mIsLodManual)
        {
            unsigned short numsubs = pMesh->getNumSubMeshes();
            for (i = 0; i < numsubs; ++i)
            {
                SubMesh* sm = pMesh->getSubMesh(i);
                sm->mLodFaceList.resize(pMesh->mNumLods-1);
            }
        }

        // Loop from 1 rather than 0 (full detail index is not in file)
        for (i = 1; i < pMesh->mNumLods; ++i)
        {
            streamID = readChunk(stream);
            if (streamID != M_MESH_LOD_USAGE)
            {
                OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND,
                    "Missing M_MESH_LOD_USAGE stream in " + pMesh->getName(),
                    "MeshSerializerImpl::readMeshLodInfo");
            }
            // Read depth
            MeshLodUsage usage;
            readFloats(stream, &(usage.value), 1);
            usage.userValue = Math::Sqrt(usage.value);

            if (pMesh->isLodManual())
            {
                readMeshLodUsageManual(stream, pMesh, i, usage);
            }
            else //(!pMesh->isLodManual)
            {
                readMeshLodUsageGenerated(stream, pMesh, i, usage);
            }
            usage.edgeData = NULL;

            // Save usage
            pMesh->mMeshLodUsageList.push_back(usage);
        }


	}
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    MeshSerializerImpl_v1_3::MeshSerializerImpl_v1_3()
    {
        // Version number
        mVersion = "[MeshSerializer_v1.30]";
    }
    //---------------------------------------------------------------------
    MeshSerializerImpl_v1_3::~MeshSerializerImpl_v1_3()
    {
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl_v1_3::readEdgeListLodInfo(DataStreamPtr& stream,
        EdgeData* edgeData)
    {
        // unsigned long numTriangles
        uint32 numTriangles;
        readInts(stream, &numTriangles, 1);
        // Allocate correct amount of memory
        edgeData->triangles.resize(numTriangles);
        edgeData->triangleFaceNormals.resize(numTriangles);
        edgeData->triangleLightFacings.resize(numTriangles);
        // unsigned long numEdgeGroups
        uint32 numEdgeGroups;
        readInts(stream, &numEdgeGroups, 1);
        // Allocate correct amount of memory
        edgeData->edgeGroups.resize(numEdgeGroups);
        // Triangle* triangleList
        uint32 tmp[3];
        for (size_t t = 0; t < numTriangles; ++t)
        {
            EdgeData::Triangle& tri = edgeData->triangles[t];
            // unsigned long indexSet
            readInts(stream, tmp, 1);
            tri.indexSet = tmp[0];
            // unsigned long vertexSet
            readInts(stream, tmp, 1);
            tri.vertexSet = tmp[0];
            // unsigned long vertIndex[3]
            readInts(stream, tmp, 3);
            tri.vertIndex[0] = tmp[0];
            tri.vertIndex[1] = tmp[1];
            tri.vertIndex[2] = tmp[2];
            // unsigned long sharedVertIndex[3]
            readInts(stream, tmp, 3);
            tri.sharedVertIndex[0] = tmp[0];
            tri.sharedVertIndex[1] = tmp[1];
            tri.sharedVertIndex[2] = tmp[2];
            // float normal[4]
            readFloats(stream, &(edgeData->triangleFaceNormals[t].x), 4);

        }

        // Assume the mesh is closed, it will update later
        edgeData->isClosed = true;

        for (uint32 eg = 0; eg < numEdgeGroups; ++eg)
        {
            unsigned short streamID = readChunk(stream);
            if (streamID != M_EDGE_GROUP)
            {
                OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                    "Missing M_EDGE_GROUP stream",
                    "MeshSerializerImpl_v1_3::readEdgeListLodInfo");
            }
            EdgeData::EdgeGroup& edgeGroup = edgeData->edgeGroups[eg];

            // unsigned long vertexSet
            readInts(stream, tmp, 1);
            edgeGroup.vertexSet = tmp[0];
            // unsigned long numEdges
            uint32 numEdges;
            readInts(stream, &numEdges, 1);
            edgeGroup.edges.resize(numEdges);
            // Edge* edgeList
            for (uint32 e = 0; e < numEdges; ++e)
            {
                EdgeData::Edge& edge = edgeGroup.edges[e];
                // unsigned long  triIndex[2]
                readInts(stream, tmp, 2);
                edge.triIndex[0] = tmp[0];
                edge.triIndex[1] = tmp[1];
                // unsigned long  vertIndex[2]
                readInts(stream, tmp, 2);
                edge.vertIndex[0] = tmp[0];
                edge.vertIndex[1] = tmp[1];
                // unsigned long  sharedVertIndex[2]
                readInts(stream, tmp, 2);
                edge.sharedVertIndex[0] = tmp[0];
                edge.sharedVertIndex[1] = tmp[1];
                // bool degenerate
                readBools(stream, &(edge.degenerate), 1);

                // The mesh is closed only if no degenerate edge here
                if (edge.degenerate)
                {
                    edgeData->isClosed = false;
                }
            }
        }

        reorganiseTriangles(edgeData);
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl_v1_3::reorganiseTriangles(EdgeData* edgeData)
    {
        size_t numTriangles = edgeData->triangles.size();

        if (edgeData->edgeGroups.size() == 1)
        {
            // Special case for only one edge group in the edge list, which occuring
            // most time. In this case, all triangles belongs to that group.
            edgeData->edgeGroups.front().triStart = 0;
            edgeData->edgeGroups.front().triCount = numTriangles;
        }
        else
        {
            EdgeData::EdgeGroupList::iterator egi, egend;
            egend = edgeData->edgeGroups.end();

            // Calculate number of triangles for edge groups

            for (egi = edgeData->edgeGroups.begin(); egi != egend; ++egi)
            {
                egi->triStart = 0;
                egi->triCount = 0;
            }

            bool isGrouped = true;
            EdgeData::EdgeGroup* lastEdgeGroup = 0;
            for (size_t t = 0; t < numTriangles; ++t)
            {
                // Gets the edge group that the triangle belongs to
                const EdgeData::Triangle& tri = edgeData->triangles[t];
                EdgeData::EdgeGroup* edgeGroup = &edgeData->edgeGroups[tri.vertexSet];

                // Does edge group changes from last edge group?
                if (isGrouped && edgeGroup != lastEdgeGroup)
                {
                    // Remember last edge group
                    lastEdgeGroup = edgeGroup;

                    // Is't first time encounter this edge group?
                    if (!edgeGroup->triCount && !edgeGroup->triStart)
                    {
                        // setup first triangle of this edge group
                        edgeGroup->triStart = t;
                    }
                    else
                    {
                        // original triangles doesn't grouping by edge group
                        isGrouped = false;
                    }
                }

                // Count number of triangles for this edge group
                ++edgeGroup->triCount;
            }

            //
            // Note that triangles has been sorted by vertex set for a long time,
            // but never stored to old version mesh file.
            //
            // Adopt this fact to avoid remap triangles here.
            //

            // Does triangles grouped by vertex set?
            if (!isGrouped)
            {
                // Ok, the triangles of this edge list isn't grouped by vertex set
                // perfectly, seems ancient mesh file.
                //
                // We need work hardly to group triangles by vertex set.
                //

                // Calculate triStart and reset triCount to zero for each edge group first
                size_t triStart = 0;
                for (egi = edgeData->edgeGroups.begin(); egi != egend; ++egi)
                {
                    egi->triStart = triStart;
                    triStart += egi->triCount;
                    egi->triCount = 0;
                }

                // The map used to mapping original triangle index to new index
                typedef vector<size_t>::type TriangleIndexRemap;
                TriangleIndexRemap triangleIndexRemap(numTriangles);

                // New triangles information that should be group by vertex set.
                EdgeData::TriangleList newTriangles(numTriangles);
                EdgeData::TriangleFaceNormalList newTriangleFaceNormals(numTriangles);

                // Calculate triangle index map and organise triangles information
                for (size_t t = 0; t < numTriangles; ++t)
                {
                    // Gets the edge group that the triangle belongs to
                    const EdgeData::Triangle& tri = edgeData->triangles[t];
                    EdgeData::EdgeGroup& edgeGroup = edgeData->edgeGroups[tri.vertexSet];

                    // Calculate new index
                    size_t newIndex = edgeGroup.triStart + edgeGroup.triCount;
                    ++edgeGroup.triCount;

                    // Setup triangle index mapping entry
                    triangleIndexRemap[t] = newIndex;

                    // Copy triangle info to new placement
                    newTriangles[newIndex] = tri;
                    newTriangleFaceNormals[newIndex] = edgeData->triangleFaceNormals[t];
                }

                // Replace with new triangles information
                edgeData->triangles.swap(newTriangles);
                edgeData->triangleFaceNormals.swap(newTriangleFaceNormals);

                // Now, update old triangle indices to new index
                for (egi = edgeData->edgeGroups.begin(); egi != egend; ++egi)
                {
                    EdgeData::EdgeList::iterator ei, eend;
                    eend = egi->edges.end();
                    for (ei = egi->edges.begin(); ei != eend; ++ei)
                    {
                        ei->triIndex[0] = triangleIndexRemap[ei->triIndex[0]];
                        if (!ei->degenerate)
                        {
                            ei->triIndex[1] = triangleIndexRemap[ei->triIndex[1]];
                        }
                    }
                }
            }
        }
    }
	//---------------------------------------------------------------------
	void MeshSerializerImpl::readExtremes(DataStreamPtr& stream, Mesh *pMesh)
	{
		unsigned short idx;
		readShorts(stream, &idx, 1);

		SubMesh *sm = pMesh->getSubMesh (idx);

		int n_floats = (mCurrentstreamLen - STREAM_OVERHEAD_SIZE -
						sizeof (unsigned short)) / sizeof (float);

        assert ((n_floats % 3) == 0);

        float *vert = OGRE_ALLOC_T(float, n_floats, MEMCATEGORY_GEOMETRY);
		readFloats(stream, vert, n_floats);

        for (int i = 0; i < n_floats; i += 3)
			sm->extremityPoints.push_back(Vector3(vert [i], vert [i + 1], vert [i + 2]));

        OGRE_FREE(vert, MEMCATEGORY_GEOMETRY);
	}
	//---------------------------------------------------------------------
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    MeshSerializerImpl_v1_2::MeshSerializerImpl_v1_2()
    {
        // Version number
        mVersion = "[MeshSerializer_v1.20]";
    }
    //---------------------------------------------------------------------
    MeshSerializerImpl_v1_2::~MeshSerializerImpl_v1_2()
    {
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl_v1_2::readMesh(DataStreamPtr& stream, Mesh* pMesh, MeshSerializerListener *listener)
    {
        MeshSerializerImpl::readMesh(stream, pMesh, listener);
        // Always automatically build edge lists for this version
        pMesh->mAutoBuildEdgeLists = true;

    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl_v1_2::readGeometry(DataStreamPtr& stream, Mesh* pMesh,
        VertexData* dest)
    {
        unsigned short texCoordSet = 0;

        unsigned short bindIdx = 0;

        dest->vertexStart = 0;

        unsigned int vertexCount = 0;
        readInts(stream, &vertexCount, 1);
        dest->vertexCount = vertexCount;

        // Vertex buffers

        readGeometryPositions(bindIdx, stream, pMesh, dest);
        ++bindIdx;

        // Find optional geometry streams
        if (!stream->eof())
        {
            unsigned short streamID = readChunk(stream);
            while(!stream->eof() &&
                (streamID == M_GEOMETRY_NORMALS ||
                 streamID == M_GEOMETRY_COLOURS ||
                 streamID == M_GEOMETRY_TEXCOORDS ))
            {
                switch (streamID)
                {
                case M_GEOMETRY_NORMALS:
                    readGeometryNormals(bindIdx++, stream, pMesh, dest);
                    break;
                case M_GEOMETRY_COLOURS:
                    readGeometryColours(bindIdx++, stream, pMesh, dest);
                    break;
                case M_GEOMETRY_TEXCOORDS:
                    readGeometryTexCoords(bindIdx++, stream, pMesh, dest, texCoordSet++);
                    break;
                }
                // Get next stream
                if (!stream->eof())
                {
                    streamID = readChunk(stream);
                }
            }
            if (!stream->eof())
            {
                // Backpedal back to start of non-submesh stream
                stream->skip(-STREAM_OVERHEAD_SIZE);
            }
        }
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl_v1_2::readGeometryPositions(unsigned short bindIdx,
        DataStreamPtr& stream, Mesh* pMesh, VertexData* dest)
    {
        float *pFloat = 0;
        HardwareVertexBufferSharedPtr vbuf;
        // float* pVertices (x, y, z order x numVertices)
        dest->vertexDeclaration->addElement(bindIdx, 0, VET_FLOAT3, VES_POSITION);
        vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(
            dest->vertexDeclaration->getVertexSize(bindIdx),
            dest->vertexCount,
            pMesh->mVertexBufferUsage,
			pMesh->mVertexBufferShadowBuffer);
        pFloat = static_cast<float*>(
            vbuf->lock(HardwareBuffer::HBL_DISCARD));
        readFloats(stream, pFloat, dest->vertexCount * 3);
        vbuf->unlock();
        dest->vertexBufferBinding->setBinding(bindIdx, vbuf);
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl_v1_2::readGeometryNormals(unsigned short bindIdx,
        DataStreamPtr& stream, Mesh* pMesh, VertexData* dest)
    {
        float *pFloat = 0;
        HardwareVertexBufferSharedPtr vbuf;
        // float* pNormals (x, y, z order x numVertices)
        dest->vertexDeclaration->addElement(bindIdx, 0, VET_FLOAT3, VES_NORMAL);
        vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(
            dest->vertexDeclaration->getVertexSize(bindIdx),
            dest->vertexCount,
            pMesh->mVertexBufferUsage,
			pMesh->mVertexBufferShadowBuffer);
        pFloat = static_cast<float*>(
            vbuf->lock(HardwareBuffer::HBL_DISCARD));
        readFloats(stream, pFloat, dest->vertexCount * 3);
        vbuf->unlock();
        dest->vertexBufferBinding->setBinding(bindIdx, vbuf);
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl_v1_2::readGeometryColours(unsigned short bindIdx,
        DataStreamPtr& stream, Mesh* pMesh, VertexData* dest)
    {
        RGBA* pRGBA = 0;
        HardwareVertexBufferSharedPtr vbuf;
        // unsigned long* pColours (RGBA 8888 format x numVertices)
        dest->vertexDeclaration->addElement(bindIdx, 0, VET_COLOUR, VES_DIFFUSE);
        vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(
            dest->vertexDeclaration->getVertexSize(bindIdx),
            dest->vertexCount,
            pMesh->mVertexBufferUsage,
			pMesh->mVertexBufferShadowBuffer);
        pRGBA = static_cast<RGBA*>(
            vbuf->lock(HardwareBuffer::HBL_DISCARD));
        readInts(stream, pRGBA, dest->vertexCount);
        vbuf->unlock();
        dest->vertexBufferBinding->setBinding(bindIdx, vbuf);
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl_v1_2::readGeometryTexCoords(unsigned short bindIdx,
        DataStreamPtr& stream, Mesh* pMesh, VertexData* dest, unsigned short texCoordSet)
    {
        float *pFloat = 0;
        HardwareVertexBufferSharedPtr vbuf;
        // unsigned short dimensions    (1 for 1D, 2 for 2D, 3 for 3D)
        unsigned short dim;
        readShorts(stream, &dim, 1);
        // float* pTexCoords  (u [v] [w] order, dimensions x numVertices)
        dest->vertexDeclaration->addElement(
            bindIdx,
            0,
            VertexElement::multiplyTypeCount(VET_FLOAT1, dim),
            VES_TEXTURE_COORDINATES,
            texCoordSet);
        vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(
            dest->vertexDeclaration->getVertexSize(bindIdx),
            dest->vertexCount,
            pMesh->mVertexBufferUsage,
			pMesh->mVertexBufferShadowBuffer);
        pFloat = static_cast<float*>(
            vbuf->lock(HardwareBuffer::HBL_DISCARD));
        readFloats(stream, pFloat, dest->vertexCount * dim);
        vbuf->unlock();
        dest->vertexBufferBinding->setBinding(bindIdx, vbuf);
    }
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    MeshSerializerImpl_v1_1::MeshSerializerImpl_v1_1()
    {
        // Version number
        mVersion = "[MeshSerializer_v1.10]";
    }
    //---------------------------------------------------------------------
    MeshSerializerImpl_v1_1::~MeshSerializerImpl_v1_1()
    {
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl_v1_1::readGeometryTexCoords(unsigned short bindIdx,
        DataStreamPtr& stream, Mesh* pMesh, VertexData* dest, unsigned short texCoordSet)
    {
        float *pFloat = 0;
        HardwareVertexBufferSharedPtr vbuf;
        // unsigned short dimensions    (1 for 1D, 2 for 2D, 3 for 3D)
        unsigned short dim;
        readShorts(stream, &dim, 1);
        // float* pTexCoords  (u [v] [w] order, dimensions x numVertices)
        dest->vertexDeclaration->addElement(
            bindIdx,
            0,
            VertexElement::multiplyTypeCount(VET_FLOAT1, dim),
            VES_TEXTURE_COORDINATES,
            texCoordSet);
        vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(
            dest->vertexDeclaration->getVertexSize(bindIdx),
            dest->vertexCount,
            pMesh->getVertexBufferUsage(),
			pMesh->isVertexBufferShadowed());
        pFloat = static_cast<float*>(
            vbuf->lock(HardwareBuffer::HBL_DISCARD));
        readFloats(stream, pFloat, dest->vertexCount * dim);

        // Adjust individual v values to (1 - v)
        if (dim == 2)
        {
            for (size_t i = 0; i < dest->vertexCount; ++i)
            {
                ++pFloat; // skip u
                *pFloat = 1.0 - *pFloat; // v = 1 - v
                ++pFloat;
            }

        }
        vbuf->unlock();
        dest->vertexBufferBinding->setBinding(bindIdx, vbuf);
    }
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------




}


