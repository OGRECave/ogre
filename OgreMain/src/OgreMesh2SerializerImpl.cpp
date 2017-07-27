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
#include "OgreStableHeaders.h"

#include "OgreMesh2SerializerImpl.h"
#include "OgreMeshFileFormat.h"
#include "OgreMesh2Serializer.h"
#include "OgreMesh2.h"
#include "OgreSubMesh2.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreHardwareBufferManager.h"
#include "OgreAnimation.h"
#include "OgreAnimationTrack.h"
#include "OgreRoot.h"
#include "OgreLodStrategyManager.h"
#include "OgreDistanceLodStrategy.h"
#include "OgreBitwise.h"

#include "Vao/OgreVaoManager.h"
#include "Vao/OgreMultiSourceVertexBufferPool.h"
#include "Vao/OgreVertexArrayObject.h"
#include "Vao/OgreIndexBufferPacked.h"
#include "Vao/OgreAsyncTicket.h"

#if OGRE_COMPILER == OGRE_COMPILER_MSVC
// Disable conversion warnings, we do a lot of them, intentionally
#   pragma warning (disable : 4267)
#endif


namespace Ogre {
    /// stream overhead = ID + size
    const long MSTREAM_OVERHEAD_SIZE = sizeof(uint16) + sizeof(uint32);
    //---------------------------------------------------------------------
    MeshSerializerImpl::MeshSerializerImpl( VaoManager *vaoManager ) :
        mVaoManager( vaoManager )
    {
        // Version number
        mVersion = "[MeshSerializer_v2.1 R2]";
    }
    //---------------------------------------------------------------------
    MeshSerializerImpl::~MeshSerializerImpl()
    {
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::exportMesh(const Mesh* pMesh, 
        DataStreamPtr stream, Endian endianMode)
    {
        LogManager::getSingleton().logMessage("MeshSerializer writing mesh data to stream " + stream->getName() + "...");

        // Decide on endian mode
        determineEndianness(endianMode);

        // Check that the mesh has it's bounds set

        if (pMesh->getAabb().mHalfSize == Vector3::ZERO || pMesh->getBoundingSphereRadius() == 0.0f)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "The Mesh you have supplied does not have its"
                " bounds completely defined. Define them first before exporting.",
                "MeshSerializerImpl::exportMesh");
        }
        mStream = stream;
        if (!mStream->isWriteable())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "Unable to use stream " + mStream->getName() + " for writing",
                "MeshSerializerImpl::exportMesh");
        }

        for (uint16 i = 0; i < pMesh->getNumSubMeshes(); ++i)
        {
            if( pMesh->getSubMesh(i)->mVao[VpNormal].empty() )
            {
                OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "The Mesh you have supplied does not have all"
                    " of its submeshes properfly initialized. Initialize their vertex buffers "
                    "before exporting.",
                    "MeshSerializerImpl::exportMesh");
            }
        }

        writeFileHeader();
        LogManager::getSingleton().logMessage("File header written.");


        LogManager::getSingleton().logMessage("Writing mesh data...");
        pushInnerChunk(mStream);
        writeMesh(pMesh);
        popInnerChunk(mStream);
        LogManager::getSingleton().logMessage("Mesh data exported.");

        LogManager::getSingleton().logMessage("MeshSerializer export successful.");
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::importMesh(DataStreamPtr& stream, Mesh* pMesh, MeshSerializerListener *listener)
    {
        // Determine endianness (must be the first thing we do!)
        determineEndianness(stream);

#if OGRE_SERIALIZER_VALIDATE_CHUNKSIZE
        enableValidation();
#endif
        // Check header
        readFileHeader(stream);
        pushInnerChunk(stream);
        uint16 streamID;
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
        popInnerChunk(stream);

        if( !pMesh->hasValidShadowMappingVaos() )
            pMesh->prepareForShadowMapping( false );
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writeMesh(const Mesh* pMesh)
    {
        exportedLodCount = 1; // generate edge data for original mesh

        LodLevelVertexBufferTableVec lodVertexTable;
        lodVertexTable.resize( pMesh->getNumSubMeshes() );

        for( size_t i=0; i<pMesh->getNumSubMeshes(); ++i )
        {
            const SubMesh *s = pMesh->getSubMesh( i );

            size_t numLodLevels = s->mVao[VpNormal].size();
            lodVertexTable[i].reserve( numLodLevels );
            lodVertexTable[i].push_back( 0 );

            for( uint8 lodLevel=1; lodLevel<numLodLevels; ++lodLevel )
            {
                for( uint8 j=0; j<lodLevel && lodVertexTable[i].size() == lodLevel; ++j )
                {
                    //Find if a previous LOD already uses these vertex buffers.
                    if( s->mVao[VpNormal][lodLevel]->getVertexBuffers() ==
                            s->mVao[VpNormal][j]->getVertexBuffers() )
                    {
                        lodVertexTable[i].push_back( j );
                    }
                }

                //Couldn't find a previous LOD sharing the vertex buffer with us.
                if( lodVertexTable[i].size() == lodLevel )
                    lodVertexTable[i].push_back( lodLevel );
            }
        }

        // Header
        writeChunkHeader(M_MESH, calcMeshSize(pMesh, lodVertexTable));
        {
        writeString(pMesh->getLodStrategyName()); // string strategyName;

        const uint8 numVaoPasses = pMesh->hasIndependentShadowMappingVaos() + 1;
        writeData( &numVaoPasses, 1, 1 );

            pushInnerChunk(mStream);

        // Write Submeshes
        for (uint16 i = 0; i < pMesh->getNumSubMeshes(); ++i)
        {
            LogManager::getSingleton().logMessage("Writing submesh...");
            writeSubMesh( pMesh->getSubMesh(i), lodVertexTable[i] );
            LogManager::getSingleton().logMessage("Submesh exported.");
        }

        // Write skeleton info if required
        if (pMesh->hasSkeleton())
        {
            LogManager::getSingleton().logMessage("Exporting skeleton link...");
            // Write skeleton link
            writeSkeletonLink(pMesh->getSkeletonName());
            LogManager::getSingleton().logMessage("Skeleton link exported.");
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
        /*if (pMesh->isEdgeListBuilt())
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
        }*/

            popInnerChunk(mStream);
        }
    }
    //---------------------------------------------------------------------
    // Added by DrEvil
    void MeshSerializerImpl::writeSubMeshNameTable(const Mesh* pMesh)
    {
        // Header
        writeChunkHeader(M_SUBMESH_NAME_TABLE, calcSubMeshNameTableSize(pMesh));

        // Loop through and save out the index and names.
        Mesh::SubMeshNameMap::const_iterator it = pMesh->mSubMeshNameMap.begin();
        pushInnerChunk(mStream);
        while(it != pMesh->mSubMeshNameMap.end())
        {
            // Header
            writeChunkHeader(M_SUBMESH_NAME_TABLE_ELEMENT, MSTREAM_OVERHEAD_SIZE +
                sizeof(uint16) + calcStringSize(it->first));

            // write the index
            writeShorts(&it->second, 1);
            // name
            writeString(it->first);

            ++it;
        }
        popInnerChunk(mStream);
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writeSubMesh( const SubMesh* s,
                                           const LodLevelVertexBufferTable &lodVertexTable )
    {
        // Header
        writeChunkHeader(M_SUBMESH, calcSubMeshSize(s, lodVertexTable));

        // char* materialName
        writeString(s->getMaterialName());

        const uint8 blendIndexToBoneIndexCount = s->mBlendIndexToBoneIndexMap.size();
        writeData( &blendIndexToBoneIndexCount, 1, 1 );
        writeShorts( s->mBlendIndexToBoneIndexMap.begin(), blendIndexToBoneIndexCount );

        uint8 numLodLevels = static_cast<uint8>( s->mVao[VpNormal].size() );
        writeData( &numLodLevels, 1, 1 );

        uint8 numVaoPasses = s->mParent->hasIndependentShadowMappingVaos() + 1;
        for( uint i=0; i<numVaoPasses; ++i )
        {
            for( uint8 lodLevel=0; lodLevel<numLodLevels; ++lodLevel )
                writeSubMeshLod( s->mVao[i][lodLevel], lodLevel, lodVertexTable[lodLevel] );
        }
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writeSubMeshLod( const VertexArrayObject *vao, uint8 lodLevel,
                                              uint8 lodSource )
    {
        const bool skipVertexBuffer = (lodLevel != lodSource);

        pushInnerChunk(mStream);
        writeChunkHeader( M_SUBMESH_LOD, calcSubMeshLodSize( vao, skipVertexBuffer ) );

        writeIndexes( vao->getIndexBuffer() );

        if( !skipVertexBuffer )
        {
            writeGeometry( vao->getVertexBuffers() );
        }
        else
        {
            pushInnerChunk(mStream);
            writeChunkHeader( M_SUBMESH_M_GEOMETRY_EXTERNAL_SOURCE,
                              MSTREAM_OVERHEAD_SIZE + sizeof(uint8) );
            writeData( &lodSource, 1, 1 );
            popInnerChunk(mStream);
        }

        if( vao->getOperationType() != OT_TRIANGLE_LIST )
            writeSubMeshLodOperation( vao );

        popInnerChunk(mStream);
    }
    //---------------------------------------------------------------------
    /*void MeshSerializerImpl::writeSubMeshTextureAliases(const SubMesh* s)
    {
        size_t chunkSize;
        AliasTextureNamePairList::const_iterator i;

        LogManager::getSingleton().logMessage("Exporting submesh texture aliases...");

        // iterate through texture aliases and write them out as a chunk
        for (i = s->mTextureAliases.begin(); i != s->mTextureAliases.end(); ++i)
        {
            // calculate chunk size based on string length + 1.  Add 1 for the line feed.
            chunkSize = MSTREAM_OVERHEAD_SIZE + calcStringSize(i->first) + calcStringSize(i->second);
            writeChunkHeader(M_SUBMESH_TEXTURE_ALIAS, chunkSize);
            // write out alias name
            writeString(i->first);
            // write out texture name
            writeString(i->second);
        }

        LogManager::getSingleton().logMessage("Submesh texture aliases exported.");
    }*/
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writeSubMeshLodOperation( const VertexArrayObject *vao )
    {
        // Header
        writeChunkHeader(M_SUBMESH_LOD_OPERATION, calcSubMeshLodOperationSize(vao));

        // uint16 operationType
        uint16 opType = static_cast<uint16>(vao->getOperationType());
        writeShorts(&opType, 1);
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writeIndexes( IndexBufferPacked *indexBuffer )
    {
        uint32 indexCount = 0;

        if( indexBuffer )
            indexCount = indexBuffer->getNumElements();

        writeInts(&indexCount, 1);

        if (indexCount > 0)
        {
            // bool indexes32Bit
            bool idx32bit = (indexBuffer && indexBuffer->getIndexType() == IndexBufferPacked::IT_32BIT);
            writeBools(&idx32bit, 1);

            // uint16* faceVertexIndices ((indexCount)
            AsyncTicketPtr asyncTicket = indexBuffer->readRequest( 0, indexCount );
            const void* pIdx = asyncTicket->map();
            if (idx32bit)
            {
                const uint32* pIdx32 = static_cast<const uint32*>(pIdx);
                writeInts(pIdx32, indexCount);
            }
            else
            {
                const uint16* pIdx16 = static_cast<const uint16*>(pIdx);
                writeShorts(pIdx16, indexCount);
            }
            asyncTicket->unmap();
        }
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writeGeometry( const VertexBufferPackedVec &vertexData )
    {
        // Header
        pushInnerChunk(mStream);
        writeChunkHeader(M_SUBMESH_M_GEOMETRY, calcGeometrySize(vertexData));

        uint32 vertexCount = 0;
        uint8 numSources = static_cast<uint8>( vertexData.size() );
        if( !vertexData.empty() )
            vertexCount = static_cast<uint32>( vertexData[VpNormal]->getNumElements() );

        writeInts(&vertexCount, 1);
        writeData(&numSources, 1, 1);
        {
            // Vertex declaration
            size_t vertexDeclChunkSize = calcVertexDeclSize( vertexData );

            pushInnerChunk(mStream);
            writeChunkHeader(M_SUBMESH_M_GEOMETRY_VERTEX_DECLARATION, vertexDeclChunkSize);

            {
                //Go through all the sources
                VertexBufferPackedVec::const_iterator itor = vertexData.begin();
                VertexBufferPackedVec::const_iterator end  = vertexData.end();

                while( itor != end )
                {
                    VertexElement2Vec vertexElements = (*itor)->getVertexElements();

                    //Number of elements in the declaration in this source.
                    uint8 numVertexElements = vertexElements.size();
                    writeData( &numVertexElements, 1, 1 );

                    VertexElement2Vec::const_iterator itElement = vertexElements.begin();
                    VertexElement2Vec::const_iterator enElement = vertexElements.end();

                    while( itElement != enElement )
                    {
                        uint8 type      = itElement->mType;
                        uint8 semantic  = itElement->mSemantic;
                        writeData( &type, 1, 1 );
                        writeData( &semantic, 1, 1 );

                        ++itElement;
                    }

                    ++itor;
                }
            }
            popInnerChunk(mStream);

            for( uint8 i=0; i<numSources; ++i )
            {
                size_t size = MSTREAM_OVERHEAD_SIZE + (sizeof(uint8)* 2) +
                                vertexData[i]->getTotalSizeBytes();

                pushInnerChunk(mStream);
                writeChunkHeader(M_SUBMESH_M_GEOMETRY_VERTEX_BUFFER, size);

                //Source
                writeData( &i, 1, 1 );

                // Per-vertex size, must agree with declaration at this source
                uint8 bytesPerVertex = vertexData[i]->getBytesPerElement();
                writeData( &bytesPerVertex, 1, 1 );

                AsyncTicketPtr asyncTicket = vertexData[i]->readRequest(
                                                    0, vertexData[i]->getNumElements() );

                const void* data = asyncTicket->map();

                if (mFlipEndian)
                {
                    // endian conversion
                    // Copy data
                    unsigned char* tempData = OGRE_ALLOC_T( unsigned char,
                                                            vertexData[i]->getTotalSizeBytes(),
                                                            MEMCATEGORY_GEOMETRY );
                    memcpy( tempData, data, vertexData[i]->getTotalSizeBytes() );

                    flipLittleEndian( tempData, vertexData[i] );

                    writeData( tempData, vertexData[i]->getBytesPerElement(),
                               vertexData[i]->getNumElements() );
                    OGRE_FREE( tempData, MEMCATEGORY_GEOMETRY );
                }
                else
                {
                    writeData( data, vertexData[i]->getBytesPerElement(),
                               vertexData[i]->getNumElements() );
                }

                asyncTicket->unmap();

                popInnerChunk(mStream);
            }
        }

        popInnerChunk(mStream);
    }
    //---------------------------------------------------------------------
    size_t MeshSerializerImpl::calcSubMeshNameTableSize(const Mesh* pMesh)
    {
        size_t size = MSTREAM_OVERHEAD_SIZE;
        // Figure out the size of the Name table.
        // Iterate through the subMeshList & add up the size of the indexes and names.
        Mesh::SubMeshNameMap::const_iterator it = pMesh->mSubMeshNameMap.begin();
        while(it != pMesh->mSubMeshNameMap.end())
        {
            // size of the index + header size for each element chunk
            size += MSTREAM_OVERHEAD_SIZE + sizeof(uint16);
            // name
            size += calcStringSize(it->first);

            ++it;
        }

        // size of the sub-mesh name table.
        return size;
    }
    //---------------------------------------------------------------------
    size_t MeshSerializerImpl::calcMeshSize( const Mesh* pMesh,
                                             const LodLevelVertexBufferTableVec &lodVertexTable )
    {
        size_t size = MSTREAM_OVERHEAD_SIZE;

        // bool hasSkeleton
        size += sizeof(bool);
        // string strategyName
        size += calcStringSize( pMesh->getLodStrategyName() );

        // Submeshes
        for (uint16 i = 0; i < pMesh->getNumSubMeshes(); ++i)
        {
            size += calcSubMeshSize( pMesh->getSubMesh(i), lodVertexTable[i] );
        }

        // Skeleton link
        if (pMesh->hasSkeleton())
        {
            size += calcSkeletonLinkSize(pMesh->getSkeletonName());
        }
        
        size += calcBoundsInfoSize(pMesh);

        // Submesh name table
        size += calcSubMeshNameTableSize(pMesh);

        // Edge list
        /*if (pMesh->isEdgeListBuilt())
        {
            size += calcEdgeListSize(pMesh);
        }

        // Morph animation
        size += calcPosesSize(pMesh);

        // Vertex animation
        if (pMesh->hasVertexAnimation())
        {
            size += calcAnimationsSize(pMesh);
        }*/

        return size;
    }
    //---------------------------------------------------------------------
    size_t MeshSerializerImpl::calcSubMeshSize( const SubMesh* pSub,
                                                const LodLevelVertexBufferTable &lodVertexTable )
    {
        size_t size = MSTREAM_OVERHEAD_SIZE;
        
        // Material name
        size += calcStringSize(pSub->getMaterialName());
        // uint8 numLodLevels
        size += sizeof(uint8);

        uint8 numVaoPasses = pSub->mParent->hasIndependentShadowMappingVaos() + 1;
        for( uint8 i=0; i<numVaoPasses; ++i )
        {
            for( uint8 lodLevel=0; lodLevel<pSub->mVao[i].size(); ++lodLevel )
                size += calcSubMeshLodSize( pSub->mVao[i][lodLevel], lodVertexTable[lodLevel] != lodLevel );
        }

        return size;
    }
    //---------------------------------------------------------------------
    size_t MeshSerializerImpl::calcSubMeshLodSize( const VertexArrayObject *vao, bool skipVertexBuffer )
    {
        size_t size = MSTREAM_OVERHEAD_SIZE;

        if( vao->getOperationType() != OT_TRIANGLE_LIST )
            size += calcSubMeshLodOperationSize( vao );

        // uint32 indexCount
        size += sizeof(uint32);
        // bool indexes32bit
        size += sizeof(bool);

        const IndexBufferPacked *indexBuffer = vao->getIndexBuffer();
        if( indexBuffer )
            size += indexBuffer->getTotalSizeBytes();

        if( !skipVertexBuffer )
        {
            size += calcGeometrySize( vao->getVertexBuffers() );
        }
        else
        {
            //uint8 lodSource
            size += sizeof(uint8);
        }

        return size;
    }
    //---------------------------------------------------------------------
    size_t MeshSerializerImpl::calcSubMeshLodOperationSize(const VertexArrayObject *vao)
    {
        return MSTREAM_OVERHEAD_SIZE + sizeof(uint16);
    }
    //---------------------------------------------------------------------
    size_t MeshSerializerImpl::calcGeometrySize(const VertexBufferPackedVec &vertexData)
    {
        size_t size = MSTREAM_OVERHEAD_SIZE;

        // Vertex count
        size += sizeof(uint32);

        // Number of sources = vertexData.size();
        size += sizeof(uint8);

        // Vertex declaration (all sources)
        {
            size += calcVertexDeclSize( vertexData );
        }

        // Buffers
        {
            size += vertexData.size() * (MSTREAM_OVERHEAD_SIZE + (sizeof(uint8)* 2));

            VertexBufferPackedVec::const_iterator itor = vertexData.begin();
            VertexBufferPackedVec::const_iterator end  = vertexData.end();

            while( itor != end )
            {
                size += (*itor)->getTotalSizeBytes();
                ++itor;
            }
        }

        return size;
    }
    //---------------------------------------------------------------------
    size_t MeshSerializerImpl::calcVertexDeclSize( const VertexBufferPackedVec &vertexData )
    {
        size_t size = MSTREAM_OVERHEAD_SIZE;

        VertexBufferPackedVec::const_iterator itor = vertexData.begin();
        VertexBufferPackedVec::const_iterator end  = vertexData.end();

        while( itor != end )
        {
            //Number of elements in the declaration in this source.
            size += sizeof(uint8);
            size += (*itor)->getVertexElements().size() * (sizeof(uint8) * 2);
            ++itor;
        }

        return size;
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::readSubMeshNameTable(DataStreamPtr& stream, Mesh* pMesh)
    {
        // The map for
        map<uint16, String>::type subMeshNames;
        uint16 streamID, subMeshIndex;

        // Need something to store the index, and the objects name
        // This table is a method that imported meshes can retain their naming
        // so that the names established in the modelling software can be used
        // to get the sub-meshes by name. The exporter must support exporting
        // the optional stream M_SUBMESH_NAME_TABLE.

        // Read in all the sub-streams. Each sub-stream should contain an index and Ogre::String for the name.
        if (!stream->eof())
        {
            pushInnerChunk(stream);
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
                backpedalChunkHeader(stream);
            }
            popInnerChunk(stream);
        }

        // Set all the submeshes names
        // ?

        // Loop through and save out the index and names.
        map<uint16, String>::type::const_iterator it = subMeshNames.begin();

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
        // Read the strategy to be used for this mesh
        // string strategyName;
        pMesh->setLodStrategyName( readString( stream ) );

        uint8 numVaoPasses = 1;
        readChar( stream, &numVaoPasses );
        assert( numVaoPasses == 1 || numVaoPasses == 2 );

        // Find all substreams
        if (!stream->eof())
        {
            pushInnerChunk(stream);
            uint16 streamID = readChunk(stream);
            while(!stream->eof() &&
                (streamID == M_SUBMESH ||
                 streamID == M_MESH_SKELETON_LINK ||
                 streamID == M_MESH_BOUNDS ||
                 streamID == M_SUBMESH_NAME_TABLE /*||
                 streamID == M_EDGE_LISTS ||
                 streamID == M_POSES ||
                 streamID == M_ANIMATIONS*/))
            {
                switch(streamID)
                {
                case M_SUBMESH:
                    readSubMesh(stream, pMesh, listener, numVaoPasses);
                    break;
                case M_MESH_SKELETON_LINK:
                    readSkeletonLink(stream, pMesh, listener);
                    break;
                /*case M_MESH_LOD_LEVEL:
                    readMeshLodLevel(stream, pMesh);
                    break;*/
                case M_MESH_BOUNDS:
                    readBoundsInfo(stream, pMesh);
                    break;
                case M_SUBMESH_NAME_TABLE:
                    readSubMeshNameTable(stream, pMesh);
                    break;
                /*case M_EDGE_LISTS:
                    readEdgeList(stream, pMesh);
                    break;
                case M_POSES:
                    readPoses(stream, pMesh);
                    break;
                case M_ANIMATIONS:
                    readAnimations(stream, pMesh);
                    break;*/
                }

                if (!stream->eof())
                {
                    streamID = readChunk(stream);
                }

            }
            if (!stream->eof())
            {
                // Backpedal back to start of stream
                backpedalChunkHeader(stream);
            }
            popInnerChunk(stream);
        }

    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::readSubMesh( DataStreamPtr& stream, Mesh* pMesh,
                                          MeshSerializerListener *listener, uint8 numVaoPasses )
    {
        SubMesh* sm = pMesh->createSubMesh();

        // char* materialName
        String materialName = readString(stream);
        if(listener)
            listener->processMaterialName(pMesh, &materialName);
        sm->setMaterialName( materialName );

        uint8 blendIndexToBoneIndexCount = 0;
        readChar( stream, &blendIndexToBoneIndexCount );
        if( blendIndexToBoneIndexCount )
        {
            sm->mBlendIndexToBoneIndexMap.resize( blendIndexToBoneIndexCount );
            readShorts( stream, sm->mBlendIndexToBoneIndexMap.begin(), blendIndexToBoneIndexCount );
        }

        uint8 numLodLevels = 0;
        readChar( stream, &numLodLevels );

        SubMeshLodVec totalSubmeshLods;
        totalSubmeshLods.reserve( numLodLevels * numVaoPasses );

        //M_SUBMESH_LOD
        pushInnerChunk(stream);
        try
        {
            SubMeshLodVec submeshLods;
            submeshLods.reserve( numLodLevels );

            for( uint8 i=0; i<numVaoPasses; ++i )
            {
                for( uint8 j=0; j<numLodLevels; ++j )
                {
                    uint16 streamID = readChunk(stream);
                    assert( streamID == M_SUBMESH_LOD && !stream->eof() );

                    totalSubmeshLods.push_back( SubMeshLod() );
                    const uint8 currentLod = static_cast<uint8>( submeshLods.size() );
                    readSubMeshLod( stream, pMesh, &totalSubmeshLods.back(), currentLod );

                    submeshLods.push_back( totalSubmeshLods.back() );
                }

                createSubMeshVao( sm, submeshLods, i );
                submeshLods.clear();
            }

            //Populate mBoneAssignments and mBlendIndexToBoneIndexMap;
            size_t indexSource = 0;
            size_t unusedVar = 0;

            const VertexElement2 *indexElement =
                    sm->mVao[VpNormal][0]->findBySemantic( VES_BLEND_INDICES, indexSource, unusedVar );
            if( indexElement )
            {
                const uint8 *vertexData = totalSubmeshLods[0].vertexBuffers[indexSource];
                sm->_buildBoneAssignmentsFromVertexData( vertexData );
            }
        }
        catch( Exception &e )
        {
            SubMeshLodVec::iterator itor = totalSubmeshLods.begin();
            SubMeshLodVec::iterator end  = totalSubmeshLods.end();

            while( itor != end )
            {
                Uint8Vec::iterator it = itor->vertexBuffers.begin();
                Uint8Vec::iterator en = itor->vertexBuffers.end();

                while( it != en )
                    OGRE_FREE_SIMD( *it++, MEMCATEGORY_GEOMETRY );

                itor->vertexBuffers.clear();

                if( itor->indexData )
                {
                    OGRE_FREE_SIMD( itor->indexData, MEMCATEGORY_GEOMETRY );
                    itor->indexData = 0;
                }

                ++itor;
            }

            //TODO: Delete created mVaos. Don't erase the data from those vaos?

            throw e;
        }

        popInnerChunk(stream);
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::createSubMeshVao( SubMesh *sm, SubMeshLodVec &submeshLods,
                                               uint8 casterPass )
    {
        sm->mVao[casterPass].reserve( submeshLods.size() );

        VertexBufferPackedVec vertexBuffers;
        for( size_t i=0; i<submeshLods.size(); ++i )
        {
            const SubMeshLod& subMeshLod = submeshLods[i];

            vertexBuffers.clear();
            vertexBuffers.reserve( subMeshLod.vertexBuffers.size() );

            assert( subMeshLod.vertexBuffers.size() == subMeshLod.vertexDeclarations.size() );

            if( subMeshLod.lodSource == i )
            {
                if( subMeshLod.vertexDeclarations.size() == 1 )
                {
                    VertexBufferPacked *vertexBuffer = mVaoManager->createVertexBuffer(
                        subMeshLod.vertexDeclarations[0], subMeshLod.numVertices, sm->mParent->getVertexBufferDefaultType(),
                        subMeshLod.vertexBuffers[0], sm->mParent->isVertexBufferShadowed() );

                    if( !sm->mParent->isVertexBufferShadowed() )
                    {
                        OGRE_FREE_SIMD( submeshLods[i].vertexBuffers[0], MEMCATEGORY_GEOMETRY );
                        submeshLods[i].vertexBuffers.erase( submeshLods[i].vertexBuffers.begin() );
                    }

                    vertexBuffers.push_back( vertexBuffer );
                }
                else
                {
                    OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED,
                                 "Meshes with multiple vertex buffer sources aren't yet supported."
                                 " Load it as v1 mesh and import it to a v2 mesh",
                                 "MeshSerializerImpl::createSubMeshVao" );
                    /*TODO: Support this, also grab an existing pool that can
                    handle our request before trying to create a new one.
                    MultiSourceVertexBufferPool *multiSourcePool = mVaoManager->
                            createMultiSourceVertexBufferPool(
                                subMeshLod.vertexDeclarations[0],
                                std::max( defaultMaxNumVertices, subMeshLod.numVertices ), BT_IMMUTABLE );

                    for( size_t j=0; j<subMeshLod.vertexBuffers.size(); ++j )
                    {
                        void * const *initialData = reinterpret_cast<void*const*>(
                                                        &subMeshLod.vertexBuffers[0] );
                        multiSourcePool->createVertexBuffers( vertexBuffers, subMeshLod.numVertices,
                                                              initialData, true );
                    }*/
                }
            }
            else
            {
                vertexBuffers = sm->mVao[casterPass][subMeshLod.lodSource]->getVertexBuffers();
            }

            IndexBufferPacked *indexBuffer = 0;
            if( subMeshLod.indexData )
            {
                indexBuffer = mVaoManager->createIndexBuffer(
                                    subMeshLod.index32Bit ? IndexBufferPacked::IT_32BIT :
                                                            IndexBufferPacked::IT_16BIT,
                                    subMeshLod.numIndices, sm->mParent->getIndexBufferDefaultType(),
                                    subMeshLod.indexData, sm->mParent->isIndexBufferShadowed() );

                if( !sm->mParent->isIndexBufferShadowed() )
                {
                    OGRE_FREE_SIMD( subMeshLod.indexData, MEMCATEGORY_GEOMETRY );
                    submeshLods[ i ].indexData = 0;
                }
            }

            VertexArrayObject *vao = mVaoManager->createVertexArrayObject( vertexBuffers, indexBuffer,
                                                                           subMeshLod.operationType );

            sm->mVao[casterPass].push_back( vao );
        }
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::readSubMeshLod( DataStreamPtr& stream, Mesh *pMesh,
                                             SubMeshLod *subLod, uint8 currentLod )
    {
        readIndexes( stream, subLod );

        pushInnerChunk(stream);

        subLod->operationType = OT_TRIANGLE_LIST;

        uint16 streamID = readChunk(stream);
        while( !stream->eof() &&
               (streamID == M_SUBMESH_M_GEOMETRY ||
                streamID == M_SUBMESH_M_GEOMETRY_EXTERNAL_SOURCE ||
                streamID == M_SUBMESH_LOD_OPERATION ) )
        {
            switch( streamID )
            {
            case M_SUBMESH_M_GEOMETRY:
                if( subLod->lodSource != currentLod )
                {
                    OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                        "Submesh contains both M_SUBMESH_M_GEOMETRY and "
                        "M_SUBMESH_M_GEOMETRY_EXTERNAL_SOURCE streams. They're mutually exclusive. " +
                         pMesh->getName(),
                        "MeshSerializerImpl::readSubMeshLod" );
                }
                readGeometry( stream, subLod );
                break;

            case M_SUBMESH_M_GEOMETRY_EXTERNAL_SOURCE:
                if( !subLod->vertexBuffers.empty() || !subLod->vertexDeclarations.empty() )
                {
                    OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                        "Submesh contains both M_SUBMESH_M_GEOMETRY_EXTERNAL_SOURCE "
                        "and M_SUBMESH_M_GEOMETRY streams. They're mutually exclusive. " +
                         pMesh->getName(),
                        "MeshSerializerImpl::readSubMeshLod" );
                }

                readChar( stream, &subLod->lodSource );
                break;

            case M_SUBMESH_LOD_OPERATION:
                readSubMeshLodOperation( stream, subLod );
                break;

            default:
                OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                    "Invalid stream in " + pMesh->getName(),
                    "MeshSerializerImpl::readSubMeshLod" );
                break;
            }

            // Get next stream
            streamID = readChunk(stream);
        }
        if( !stream->eof() )
        {
            // Backpedal back to start of non-submesh stream
            backpedalChunkHeader(stream);
        }

        popInnerChunk(stream);
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::readIndexes(DataStreamPtr& stream, SubMeshLod *subLod)
    {
        assert( !subLod->indexData );

        readInts( stream, &subLod->numIndices, 1 );

        if( subLod->numIndices > 0 )
        {
            readBools( stream, &subLod->index32Bit, 1 );

            if( subLod->index32Bit )
            {
                subLod->indexData = OGRE_MALLOC_SIMD( sizeof(uint32) * subLod->numIndices,
                                                      MEMCATEGORY_GEOMETRY );
                readInts(stream, reinterpret_cast<uint32*>(subLod->indexData), subLod->numIndices);
            }
            else
            {
                subLod->indexData = OGRE_MALLOC_SIMD( sizeof(uint16) * subLod->numIndices,
                                                      MEMCATEGORY_GEOMETRY );
                readShorts(stream, reinterpret_cast<uint16*>(subLod->indexData), subLod->numIndices);
            }
        }
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::readGeometry(DataStreamPtr& stream, SubMeshLod *subLod)
    {
        readInts( stream, &subLod->numVertices, 1 );

        uint8 numSources;
        readChar( stream, &numSources );

        subLod->vertexDeclarations.resize( numSources );
        subLod->vertexBuffers.resize( numSources );

        pushInnerChunk( stream );

        uint16 streamID = readChunk(stream);
        while( !stream->eof() &&
               (streamID == M_SUBMESH_M_GEOMETRY_VERTEX_DECLARATION ||
                streamID == M_SUBMESH_M_GEOMETRY_VERTEX_BUFFER) )
        {
            switch( streamID )
            {
            case M_SUBMESH_M_GEOMETRY_VERTEX_DECLARATION:
                readVertexDeclaration( stream, subLod );
                break;
            case M_SUBMESH_M_GEOMETRY_VERTEX_BUFFER:
                readVertexBuffer( stream, subLod );
                break;
            }
            // Get next stream
            streamID = readChunk(stream);
        }
        if( !stream->eof() )
        {
            // Backpedal back to start of non-submesh stream
            backpedalChunkHeader(stream);
        }

        popInnerChunk( stream );
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::readVertexDeclaration(DataStreamPtr& stream, SubMeshLod *subLod)
    {
        VertexElement2VecVec::iterator itor = subLod->vertexDeclarations.begin();
        VertexElement2VecVec::iterator end  = subLod->vertexDeclarations.end();

        while( itor != end )
        {
            uint8 numVertexElements;
            readChar( stream, &numVertexElements );

            itor->reserve( numVertexElements );

            for( uint8 i=0; i<numVertexElements; ++i )
            {
                uint8 type;
                readChar( stream, &type );
                uint8 semantic;
                readChar( stream, &semantic );

                itor->push_back( VertexElement2( static_cast<VertexElementType>( type ),
                                                 static_cast<VertexElementSemantic>( semantic ) ) );
            }

            ++itor;
        }
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::readVertexBuffer(DataStreamPtr& stream, SubMeshLod *subLod)
    {
        //Source
        uint8 source;
        readChar( stream, &source );

        // Per-vertex size, must agree with declaration at this source
        uint8 bytesPerVertex;
        readChar( stream, &bytesPerVertex );

        const VertexElement2Vec &vertexElements = subLod->vertexDeclarations[source];

        if( bytesPerVertex != VaoManager::calculateVertexSize( vertexElements ) )
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Buffer vertex size does not agree with vertex declaration",
                        "MeshSerializerImpl::readVertexBuffer");
        }

        if( subLod->vertexBuffers[source] )
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Two vertex buffer streams are assigned to the same source."
                        " This mesh is invalid.",
                        "MeshSerializerImpl::readVertexBuffer");
        }

        uint8 *vertexData = reinterpret_cast<uint8*>( OGRE_MALLOC_SIMD(
                                sizeof(uint8) * bytesPerVertex * subLod->numVertices,
                                MEMCATEGORY_GEOMETRY ) );
        subLod->vertexBuffers[source] = vertexData;

        stream->read( vertexData, bytesPerVertex * subLod->numVertices );

        // Endian conversion
        flipLittleEndian( vertexData, subLod->numVertices, bytesPerVertex,
                          vertexElements );
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::readSubMeshLodOperation( DataStreamPtr& stream,
                                                      SubMeshLod *subLod )
    {
        // uint16 operationType
        uint16 opType;
        readShorts(stream, &opType, 1);
        subLod->operationType = static_cast<OperationType>(opType);
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
        size_t size = MSTREAM_OVERHEAD_SIZE;

        size += calcStringSize(skelName);

        return size;

    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writeBoundsInfo(const Mesh* pMesh)
    {
        writeChunkHeader(M_MESH_BOUNDS, calcBoundsInfoSize(pMesh));

        // float centerX, centerY, centerZ
        Aabb aabb = pMesh->getAabb();
        writeFloats(&aabb.mCenter.x, 1);
        writeFloats(&aabb.mCenter.y, 1);
        writeFloats(&aabb.mCenter.z, 1);
        // float halfSizeX, halfSizeY, halfSizeZ
        writeFloats(&aabb.mHalfSize.x, 1);
        writeFloats(&aabb.mHalfSize.y, 1);
        writeFloats(&aabb.mHalfSize.z, 1);
        // float radius
        Real radius = pMesh->getBoundingSphereRadius();
        writeFloats(&radius, 1);

    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::readBoundsInfo(DataStreamPtr& stream, Mesh* pMesh)
    {
        Vector3 center, halfSize;
        // float centerX, centerY, centerZ
        readFloats(stream, &center.x, 1);
        readFloats(stream, &center.y, 1);
        readFloats(stream, &center.z, 1);
        // float halfSizeX, halfSizeY, halfSizeZ
        readFloats(stream, &halfSize.x, 1);
        readFloats(stream, &halfSize.y, 1);
        readFloats(stream, &halfSize.z, 1);

        Aabb aabb( center, halfSize );
        pMesh->_setBounds( aabb, false );
        // float radius
        float radius;
        readFloats(stream, &radius, 1);
        pMesh->_setBoundingSphereRadius(radius);
    }
    size_t MeshSerializerImpl::calcBoundsInfoSize(const Mesh* pMesh)
    {
        unsigned long size = MSTREAM_OVERHEAD_SIZE;
        size += sizeof(float) * 7;
        return size;
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::flipLittleEndian( void* pData, VertexBufferPacked *vertexBuffer )
    {
        flipLittleEndian( pData, vertexBuffer->getNumElements(), vertexBuffer->getBytesPerElement(),
                          vertexBuffer->getVertexElements() );
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::flipLittleEndian( void* pData, size_t numVertices, size_t bytesPerVertex,
                                               const VertexElement2Vec &vertexElements )
    {
        if (mFlipEndian)
        {
            VertexElement2Vec::const_iterator itElement = vertexElements.begin();
            VertexElement2Vec::const_iterator enElement = vertexElements.end();

            size_t accumulatedOffset = 0;
            while( itElement != enElement )
            {
                flipEndian( pData, numVertices, bytesPerVertex,
                            accumulatedOffset, itElement->mType );

                accumulatedOffset += v1::VertexElement::getTypeSize( itElement->mType );
                ++itElement;
            }
        }
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::flipEndian( void* pData, size_t vertexCount,
                                         size_t vertexSize, size_t baseOffset,
                                         const VertexElementType elementType )
    {
        void *pBase = static_cast<uint8*>(pData) + baseOffset;

        size_t typeCount                = v1::VertexElement::getTypeCount( elementType );
        size_t typeSingleMemberSize     = v1::VertexElement::getTypeSize( elementType ) / typeCount;

        if( elementType == VET_BYTE4 || elementType == VET_BYTE4_SNORM ||
            elementType == VET_UBYTE4 || elementType == VET_UBYTE4_NORM )
        {
            // NO FLIPPING
            return;
        }

        for( size_t v = 0; v < vertexCount; ++v )
        {
            Bitwise::bswapChunks( pBase, typeSingleMemberSize, typeCount );

            pBase = static_cast<void*>(
                static_cast<unsigned char*>(pBase) + vertexSize);
        }
    }
    //---------------------------------------------------------------------
    /*size_t MeshSerializerImpl::calcEdgeListSize(const Mesh* pMesh)
    {

        size_t size = MSTREAM_OVERHEAD_SIZE;

        for (ushort i = 0; i < exportedLodCount; ++i)
        {

            const EdgeData* edgeData = pMesh->getEdgeList(i);
            bool isManual = !pMesh->mMeshLodUsageList[i].manualName.empty();

            size += calcEdgeListLodSize(edgeData, isManual);

        }

        return size;
    }
    //---------------------------------------------------------------------
    size_t MeshSerializerImpl::calcEdgeListLodSize(const EdgeData* edgeData, bool isManual)
    {
        size_t size = MSTREAM_OVERHEAD_SIZE;

        // uint16 lodIndex
        size += sizeof(uint16);

        // bool isManual            // If manual, no edge data here, loaded from manual mesh
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
        size_t size = MSTREAM_OVERHEAD_SIZE;

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
        pushInnerChunk(mStream);
        {
        for (ushort i = 0; i < exportedLodCount; ++i)
        {
            const EdgeData* edgeData = pMesh->getEdgeList(i);
                bool isManual = !pMesh->mMeshLodUsageList[i].manualName.empty();
            writeChunkHeader(M_EDGE_LIST_LOD, calcEdgeListLodSize(edgeData, isManual));

            // uint16 lodIndex
            writeShorts(&i, 1);

            // bool isManual            // If manual, no edge data here, loaded from manual mesh
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
                    tmp[0] = static_cast<uint32>(tri.indexSet);
                    writeInts(tmp, 1);
                    // unsigned long vertexSet;
                    tmp[0] = static_cast<uint32>(tri.vertexSet);
                    writeInts(tmp, 1);
                    // unsigned long vertIndex[3];
                    tmp[0] = static_cast<uint32>(tri.vertIndex[0]);
                    tmp[1] = static_cast<uint32>(tri.vertIndex[1]);
                    tmp[2] = static_cast<uint32>(tri.vertIndex[2]);
                    writeInts(tmp, 3);
                    // unsigned long sharedVertIndex[3];
                    tmp[0] = static_cast<uint32>(tri.sharedVertIndex[0]);
                    tmp[1] = static_cast<uint32>(tri.sharedVertIndex[1]);
                    tmp[2] = static_cast<uint32>(tri.sharedVertIndex[2]);
                    writeInts(tmp, 3);
                    // float normal[4];
                    writeFloats(&(fni->x), 4);

                }
                    pushInnerChunk(mStream);
                    {
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
                        tmp[0] = static_cast<uint32>(edge.triIndex[0]);
                        tmp[1] = static_cast<uint32>(edge.triIndex[1]);
                        writeInts(tmp, 2);
                        // unsigned long  vertIndex[2]
                        tmp[0] = static_cast<uint32>(edge.vertIndex[0]);
                        tmp[1] = static_cast<uint32>(edge.vertIndex[1]);
                        writeInts(tmp, 2);
                        // unsigned long  sharedVertIndex[2]
                        tmp[0] = static_cast<uint32>(edge.sharedVertIndex[0]);
                        tmp[1] = static_cast<uint32>(edge.sharedVertIndex[1]);
                        writeInts(tmp, 2);
                        // bool degenerate
                        writeBools(&(edge.degenerate), 1);
                    }

                }
                    }
                    popInnerChunk(mStream);

                }

            }
        }
        popInnerChunk(mStream);
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::readEdgeList(DataStreamPtr& stream, Mesh* pMesh)
    {
        if (!stream->eof())
        {
            pushInnerChunk(stream);
            uint16 streamID = readChunk(stream);
            while(!stream->eof() &&
                streamID == M_EDGE_LIST_LOD)
            {
                // Process single LOD

                // uint16 lodIndex
                uint16 lodIndex;
                readShorts(stream, &lodIndex, 1);

                // bool isManual            // If manual, no edge data here, loaded from manual mesh
                bool isManual;
                readBools(stream, &isManual, 1);
                // Only load in non-manual levels; others will be connected up by Mesh on demand
#if OGRE_NO_MESHLOD
                // 
                if (!isManual)
                    if (lodIndex != 0) {
                        readEdgeListLodInfo(stream, NULL);
                    } else {
#else
                if (!isManual) {
#endif
                    MeshLodUsage& usage = pMesh->mMeshLodUsageList[lodIndex];

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
                        if (pMesh->sharedVertexData[VpNormal])
                        {
                            if (edgeGroup.vertexSet == 0)
                            {
                                edgeGroup.vertexData = pMesh->sharedVertexData;
                            }
                            else
                            {
                                edgeGroup.vertexData = pMesh->getSubMesh(
                                    (uint16)edgeGroup.vertexSet-1)->vertexData;
                            }
                        }
                        else
                        {
                            edgeGroup.vertexData = pMesh->getSubMesh(
                                (uint16)edgeGroup.vertexSet)->vertexData;
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
                backpedalChunkHeader(stream);
            }
            popInnerChunk(stream);
        }

        pMesh->mEdgeListsBuilt = true;
    }
    //---------------------------------------------------------------------
    size_t MeshSerializerImpl::calcAnimationSize(const Animation* anim)
    {
        size_t size = MSTREAM_OVERHEAD_SIZE;
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
        size_t size = MSTREAM_OVERHEAD_SIZE;
        // uint16 type
        size += sizeof(uint16);
        // uint16 target        // 0 for shared geometry,
        size += sizeof(uint16);

        if (track->getAnimationType() == VAT_MORPH)
        {
            for (uint16 i = 0; i < track->getNumKeyFrames(); ++i)
            {
                VertexMorphKeyFrame* kf = track->getVertexMorphKeyFrame(i);
                size += calcMorphKeyframeSize(kf, track->getAssociatedVertexData()->vertexCount);
            }
        }
        else
        {
            for (uint16 i = 0; i < track->getNumKeyFrames(); ++i)
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
        size_t size = MSTREAM_OVERHEAD_SIZE;
        // float time
        size += sizeof(float);
        // float x,y,z[,nx,ny,nz]
        bool includesNormals = kf->getVertexBuffer()->getVertexSize() > (sizeof(float) * 3);
        size += sizeof(float) * (includesNormals ? 6 : 3) * vertexCount;

        return size;
    }
    //---------------------------------------------------------------------
    size_t MeshSerializerImpl::calcPoseKeyframeSize(const VertexPoseKeyFrame* kf)
    {
        size_t size = MSTREAM_OVERHEAD_SIZE;

        // float time
        size += sizeof(float);

        size += calcPoseKeyframePoseRefSize() * kf->getPoseReferences().size();

        return size;

    }
    //---------------------------------------------------------------------
    size_t MeshSerializerImpl::calcPoseKeyframePoseRefSize(void)
    {
        size_t size = MSTREAM_OVERHEAD_SIZE;
        // uint16 poseIndex
        size += sizeof(uint16);
        // float influence
        size += sizeof(float);

        return size;

    }
    //---------------------------------------------------------------------
    size_t MeshSerializerImpl::calcPosesSize(const Mesh* pMesh)
    {
        size_t size = 0;
        Mesh::ConstPoseIterator poseIterator = pMesh->getPoseIterator();
        if (poseIterator.hasMoreElements())
        {
            size += MSTREAM_OVERHEAD_SIZE;
            while (poseIterator.hasMoreElements())
            {
                size += calcPoseSize(poseIterator.getNext());
            }
        }
        return size;
    }
    //---------------------------------------------------------------------
    size_t MeshSerializerImpl::calcPoseSize(const Pose* pose)
    {
        size_t size = MSTREAM_OVERHEAD_SIZE;

        // char* name (may be blank)
        size += pose->getName().length() + 1;
        // uint16 target
        size += sizeof(uint16);
        // bool includesNormals
        size += sizeof(bool);

        // vertex offsets
        size += pose->getVertexOffsets().size() * calcPoseVertexSize(pose);

        return size;

    }
    //---------------------------------------------------------------------
    size_t MeshSerializerImpl::calcPoseVertexSize(const Pose* pose)
    {
        size_t size = MSTREAM_OVERHEAD_SIZE;
        // unsigned long vertexIndex
        size += sizeof(uint32);
        // float xoffset, yoffset, zoffset
        size += sizeof(float) * 3;
        // optional normals
        if (!pose->getNormals().empty())
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
            pushInnerChunk(mStream);
            while (poseIterator.hasMoreElements())
            {
                writePose(poseIterator.getNext());
            }
            popInnerChunk(mStream);
        }

    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writePose(const Pose* pose)
    {
        writeChunkHeader(M_POSE, calcPoseSize(pose));

        // char* name (may be blank)
        writeString(pose->getName());

        // uint16 target
        ushort val = pose->getTarget();
        writeShorts(&val, 1);

        // bool includesNormals
        bool includesNormals = !pose->getNormals().empty();
        writeBools(&includesNormals, 1);
        pushInnerChunk(mStream);
        {
            size_t vertexSize = calcPoseVertexSize(pose);
            Pose::ConstVertexOffsetIterator vit = pose->getVertexOffsetIterator();
            Pose::ConstNormalsIterator nit = pose->getNormalsIterator();
            while (vit.hasMoreElements())
            {
                uint32 vertexIndex = (uint32)vit.peekNextKey();
                Vector3 offset = vit.getNext();
                writeChunkHeader(M_POSE_VERTEX, vertexSize);
                // unsigned long vertexIndex
                writeInts(&vertexIndex, 1);
                // float xoffset, yoffset, zoffset
                writeFloats(offset.ptr(), 3);
                if (includesNormals)
                {
                    Vector3 normal = nit.getNext();
                    // float xnormal, ynormal, znormal
                    writeFloats(normal.ptr(), 3);
                }
            }
        }
        popInnerChunk(mStream);
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writeAnimations(const Mesh* pMesh)
    {
        writeChunkHeader(M_ANIMATIONS, calcAnimationsSize(pMesh));
        pushInnerChunk(mStream);
        for (uint16 a = 0; a < pMesh->getNumAnimations(); ++a)
        {
            Animation* anim = pMesh->getAnimation(a);
            LogManager::getSingleton().logMessage("Exporting animation " + anim->getName());
            writeAnimation(anim);
            LogManager::getSingleton().logMessage("Animation exported.");
        }
        popInnerChunk(mStream);
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
        pushInnerChunk(mStream);
        if (anim->getUseBaseKeyFrame())
        {
            size_t size = MSTREAM_OVERHEAD_SIZE;
            // char* baseAnimationName (including terminator)
            size += anim->getBaseKeyFrameAnimationName().length() + 1;
            // float baseKeyFrameTime
            size += sizeof(float);
            
            writeChunkHeader(M_ANIMATION_BASEINFO, size);
            
            // char* baseAnimationName (blank for self)
            writeString(anim->getBaseKeyFrameAnimationName());
            
            // float baseKeyFrameTime
            float t = (float)anim->getBaseKeyFrameTime();
            writeFloats(&t, 1);
        }

        // tracks
        Animation::VertexTrackIterator trackIt = anim->getVertexTrackIterator();
        while (trackIt.hasMoreElements())
        {
            VertexAnimationTrack* vt = trackIt.getNext();
            writeAnimationTrack(vt);
        }
        popInnerChunk(mStream);

    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writeAnimationTrack(const VertexAnimationTrack* track)
    {
        writeChunkHeader(M_ANIMATION_TRACK, calcAnimationTrackSize(track));
        // uint16 type          // 1 == morph, 2 == pose
        uint16 animType = (uint16)track->getAnimationType();
        writeShorts(&animType, 1);
        // uint16 target
        uint16 target = track->getHandle();
        writeShorts(&target, 1);
        pushInnerChunk(mStream);
        {
            if (track->getAnimationType() == VAT_MORPH)
            {
                for (uint16 i = 0; i < track->getNumKeyFrames(); ++i)
                {
                    VertexMorphKeyFrame* kf = track->getVertexMorphKeyFrame(i);
                    writeMorphKeyframe(kf, track->getAssociatedVertexData()->vertexCount);
                }
            }
            else // VAT_POSE
            {
                for (uint16 i = 0; i < track->getNumKeyFrames(); ++i)
                {
                    VertexPoseKeyFrame* kf = track->getVertexPoseKeyFrame(i);
                    writePoseKeyframe(kf);
                }
            }
        }
        popInnerChunk(mStream);
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writeMorphKeyframe(const VertexMorphKeyFrame* kf, size_t vertexCount)
    {
        writeChunkHeader(M_ANIMATION_MORPH_KEYFRAME, calcMorphKeyframeSize(kf, vertexCount));
        // float time
        float timePos = kf->getTime();
        writeFloats(&timePos, 1);
        // bool includeNormals
        bool includeNormals = kf->getVertexBuffer()->getVertexSize() > (sizeof(float) * 3);
        writeBools(&includeNormals, 1);
        // float x,y,z          // repeat by number of vertices in original geometry
        float* pSrc = static_cast<float*>(
            kf->getVertexBuffer()->lock(HardwareBuffer::HBL_READ_ONLY));
        writeFloats(pSrc, vertexCount * (includeNormals ? 6 : 3));
        kf->getVertexBuffer()->unlock();
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writePoseKeyframe(const VertexPoseKeyFrame* kf)
    {
        writeChunkHeader(M_ANIMATION_POSE_KEYFRAME, calcPoseKeyframeSize(kf));
        // float time
        float timePos = kf->getTime();
        writeFloats(&timePos, 1);
        pushInnerChunk(mStream);
        // pose references
        VertexPoseKeyFrame::ConstPoseRefIterator poseRefIt =
            kf->getPoseReferenceIterator();
        while (poseRefIt.hasMoreElements())
        {
            writePoseKeyframePoseRef(poseRefIt.getNext());
        }
        popInnerChunk(mStream);
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writePoseKeyframePoseRef(
        const VertexPoseKeyFrame::PoseRef& poseRef)
    {
        writeChunkHeader(M_ANIMATION_POSE_REF, calcPoseKeyframePoseRefSize());
        // uint16 poseIndex
        writeShorts(&(poseRef.poseIndex), 1);
        // float influence
        writeFloats(&(poseRef.influence), 1);
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::readPoses(DataStreamPtr& stream, Mesh* pMesh)
    {
        // Find all substreams
        if (!stream->eof())
        {
            pushInnerChunk(stream);
            uint16 streamID = readChunk(stream);
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
                backpedalChunkHeader(stream);
            }
            popInnerChunk(stream);
        }
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::readPose(DataStreamPtr& stream, Mesh* pMesh)
    {
        // char* name (may be blank)
        String name = readString(stream);
        // uint16 target
        uint16 target;
        readShorts(stream, &target, 1);

        // bool includesNormals
        bool includesNormals;
        readBools(stream, &includesNormals, 1);
        
        Pose* pose = pMesh->createPose(target, name);

        // Find all substreams
        if (!stream->eof())
        {
            pushInnerChunk(stream);
            uint16 streamID = readChunk(stream);
            while(!stream->eof() &&
                (streamID == M_POSE_VERTEX))
            {
                switch(streamID)
                {
                case M_POSE_VERTEX:
                    // create vertex offset
                    uint32 vertIndex;
                    Vector3 offset, normal;
                    // unsigned long vertexIndex
                    readInts(stream, &vertIndex, 1);
                    // float xoffset, yoffset, zoffset
                    readFloats(stream, offset.ptr(), 3);
                    
                    if (includesNormals)
                    {
                        readFloats(stream, normal.ptr(), 3);
                        pose->addVertex(vertIndex, offset, normal);                     
                    }
                    else 
                    {
                        pose->addVertex(vertIndex, offset);
                    }


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
                backpedalChunkHeader(stream);
            }
            popInnerChunk(stream);
        }

    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::readAnimations(DataStreamPtr& stream, Mesh* pMesh)
    {
        // Find all substreams
        if (!stream->eof())
        {
            pushInnerChunk(stream);
            uint16 streamID = readChunk(stream);
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
                backpedalChunkHeader(stream);
            }
            popInnerChunk(stream);
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
        if (!stream->eof())
        {
            pushInnerChunk(stream);
            uint16 streamID = readChunk(stream);
            
            // Optional base info is possible
            if (streamID == M_ANIMATION_BASEINFO)
            {
                // char baseAnimationName
                String baseAnimName = readString(stream);
                // float baseKeyFrameTime
                float baseKeyTime;
                readFloats(stream, &baseKeyTime, 1);
                
                anim->setUseBaseKeyFrame(true, baseKeyTime, baseAnimName);
                
                if (!stream->eof())
                {
                    // Get next stream
                    streamID = readChunk(stream);
                }
            }
            
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
                backpedalChunkHeader(stream);
            }
            popInnerChunk(stream);
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

        // uint16 target
        uint16 target;
        readShorts(stream, &target, 1);

        VertexAnimationTrack* track = anim->createVertexTrack(target,
            pMesh->getVertexDataByTrackHandle(target), animType);

        // keyframes
        if (!stream->eof())
        {
            pushInnerChunk(stream);
            uint16 streamID = readChunk(stream);
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
                backpedalChunkHeader(stream);
            }
            popInnerChunk(stream);
        }

    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::readMorphKeyFrame(DataStreamPtr& stream, VertexAnimationTrack* track)
    {
        // float time
        float timePos;
        readFloats(stream, &timePos, 1);
        
        // bool includesNormals
        bool includesNormals;
        readBools(stream, &includesNormals, 1);

        VertexMorphKeyFrame* kf = track->createVertexMorphKeyFrame(timePos);

        // Create buffer, allow read and use shadow buffer
        size_t vertexCount = track->getAssociatedVertexData()->vertexCount;
        size_t vertexSize = sizeof(float) * (includesNormals ? 6 : 3);
        HardwareVertexBufferSharedPtr vbuf =
            HardwareBufferManager::getSingleton().createVertexBuffer(
                vertexSize, vertexCount,
                HardwareBuffer::HBU_STATIC, true);
        // float x,y,z          // repeat by number of vertices in original geometry
        float* pDst = static_cast<float*>(
            vbuf->lock(HardwareBuffer::HBL_DISCARD));
        readFloats(stream, pDst, vertexCount * (includesNormals ? 6 : 3));
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

        if (!stream->eof())
        {
            pushInnerChunk(stream);
            uint16 streamID = readChunk(stream);
            while(!stream->eof() &&
                streamID == M_ANIMATION_POSE_REF)
            {
                switch(streamID)
                {
                case M_ANIMATION_POSE_REF:
                    uint16 poseIndex;
                    float influence;
                    // uint16 poseIndex
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
                backpedalChunkHeader(stream);
            }
            popInnerChunk(stream);
        }

    }*/

    void MeshSerializerImpl::enableValidation()
    {
#if OGRE_SERIALIZER_VALIDATE_CHUNKSIZE
        mReportChunkErrors = true;
#endif
    }


    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    MeshSerializerImpl::SubMeshLod::SubMeshLod() :
        numVertices( 0 ),
        lodSource( 0 ),
        index32Bit( false ),
        numIndices( 0 ),
        indexData( 0 )
    {
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    MeshSerializerImpl_v2_1_R1::MeshSerializerImpl_v2_1_R1( VaoManager *vaoManager ) :
        MeshSerializerImpl( vaoManager )
    {
        // Version number
        mVersion = "[MeshSerializer_v2.1 R1]";
    }
    //---------------------------------------------------------------------
    MeshSerializerImpl_v2_1_R1::~MeshSerializerImpl_v2_1_R1()
    {
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl_v2_1_R1::readSubMesh( DataStreamPtr& stream, Mesh* pMesh,
                                                  MeshSerializerListener *listener, uint8 numVaoPasses )
    {
        SubMesh* sm = pMesh->createSubMesh();

        // char* materialName
        String materialName = readString(stream);
        if(listener)
            listener->processMaterialName(pMesh, &materialName);
        sm->setMaterialName( materialName );

        uint8 numLodLevels = 0;
        readChar( stream, &numLodLevels );

        SubMeshLodVec totalSubmeshLods;
        totalSubmeshLods.reserve( numLodLevels * numVaoPasses );

        //M_SUBMESH_LOD
        pushInnerChunk(stream);
        try
        {
            SubMeshLodVec submeshLods;
            submeshLods.reserve( numLodLevels );

            for( uint8 i=0; i<numVaoPasses; ++i )
            {
                for( uint8 j=0; j<numLodLevels; ++j )
                {
                    uint16 streamID = readChunk(stream);
                    assert( streamID == M_SUBMESH_LOD && !stream->eof() );

                    totalSubmeshLods.push_back( SubMeshLod() );
                    const uint8 currentLod = static_cast<uint8>( submeshLods.size() );
                    readSubMeshLod( stream, pMesh, &totalSubmeshLods.back(), currentLod );

                    submeshLods.push_back( totalSubmeshLods.back() );
                }

                createSubMeshVao( sm, submeshLods, i );
                submeshLods.clear();
            }
        }
        catch( Exception &e )
        {
            SubMeshLodVec::iterator itor = totalSubmeshLods.begin();
            SubMeshLodVec::iterator end  = totalSubmeshLods.end();

            while( itor != end )
            {
                Uint8Vec::iterator it = itor->vertexBuffers.begin();
                Uint8Vec::iterator en = itor->vertexBuffers.end();

                while( it != en )
                    OGRE_FREE_SIMD( *it++, MEMCATEGORY_GEOMETRY );

                itor->vertexBuffers.clear();

                if( itor->indexData )
                {
                    OGRE_FREE_SIMD( itor->indexData, MEMCATEGORY_GEOMETRY );
                    itor->indexData = 0;
                }

                ++itor;
            }

            //TODO: Delete created mVaos. Don't erase the data from those vaos?

            throw e;
        }

        popInnerChunk(stream);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    MeshSerializerImpl_v2_1_R0::MeshSerializerImpl_v2_1_R0( VaoManager *vaoManager ) :
        MeshSerializerImpl_v2_1_R1( vaoManager )
    {
        // Version number
        mVersion = "[MeshSerializer_v2.1]";
    }
    //---------------------------------------------------------------------
    MeshSerializerImpl_v2_1_R0::~MeshSerializerImpl_v2_1_R0()
    {
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl_v2_1_R0::readMesh(DataStreamPtr& stream, Mesh* pMesh, MeshSerializerListener *listener)
    {
        // bool skeletallyAnimated
        bool skeletallyAnimated;
        readBools(stream, &skeletallyAnimated, 1);

        // Read the strategy to be used for this mesh
        // string strategyName;
        pMesh->setLodStrategyName( readString( stream ) );

        // Find all substreams
        if (!stream->eof())
        {
            pushInnerChunk(stream);
            uint16 streamID = readChunk(stream);
            while(!stream->eof() &&
                (streamID == M_SUBMESH ||
                 streamID == M_MESH_SKELETON_LINK ||
                 streamID == M_MESH_BOUNDS ||
                 streamID == M_SUBMESH_NAME_TABLE ))
            {
                switch(streamID)
                {
                case M_SUBMESH:
                    readSubMesh(stream, pMesh, listener, 1);
                    break;
                case M_MESH_SKELETON_LINK:
                    readSkeletonLink(stream, pMesh, listener);
                    break;
                case M_MESH_BOUNDS:
                    readBoundsInfo(stream, pMesh);
                    break;
                case M_SUBMESH_NAME_TABLE:
                    readSubMeshNameTable(stream, pMesh);
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
                backpedalChunkHeader(stream);
            }
            popInnerChunk(stream);
        }

    }
}
