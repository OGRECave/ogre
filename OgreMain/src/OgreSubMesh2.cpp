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
#include "OgreSubMesh2.h"
#include "OgreSubMesh.h"

//#include "OgreMesh.h"
#include "OgreMesh2.h"
#include "OgreException.h"
#include "OgreHardwareBufferManager.h"
#include "OgreLogManager.h"

#include "Vao/OgreVaoManager.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    SubMesh::SubMesh() :
        mParent( 0 ),
        mBoneAssignmentsOutOfDate( false )
    {
    }
    //-----------------------------------------------------------------------
    SubMesh::~SubMesh()
    {
        typedef set<VertexBufferPacked*>::type VertexBufferPackedSet;
        VertexBufferPackedSet destroyedBuffers;

        VertexArrayObjectArray::const_iterator itor = mVao.begin();
        VertexArrayObjectArray::const_iterator end  = mVao.end();
        while( itor != end )
        {
            VertexArrayObject *vao = *itor;

            VaoManager *vaoManager = mParent->mVaoManager;
            const VertexBufferPackedVec &vertexBuffers = vao->getVertexBuffers();
            VertexBufferPackedVec::const_iterator itBuffers = vertexBuffers.begin();
            VertexBufferPackedVec::const_iterator enBuffers = vertexBuffers.end();

            //If LOD share the same buffers, we'll try to destroy the buffers only once.
            while( itBuffers != enBuffers )
            {
                std::pair<VertexBufferPackedSet::iterator, bool> bufferNotSeenYet =
                                                        destroyedBuffers.insert( *itBuffers );
                if( bufferNotSeenYet.second )
                    vaoManager->destroyVertexBuffer( *itBuffers );

                ++itBuffers;
            }

            vaoManager->destroyIndexBuffer( vao->getIndexBuffer() );
            vaoManager->destroyVertexArrayObject( vao );

            ++itor;
        }
    }
    //-----------------------------------------------------------------------
    void SubMesh::addBoneAssignment(const VertexBoneAssignment& vertBoneAssign)
    {
        mBoneAssignments.push_back( vertBoneAssign );
        mBoneAssignmentsOutOfDate = true;
    }
    //-----------------------------------------------------------------------
    void SubMesh::clearBoneAssignments(void)
    {
        mBoneAssignments.clear();
        mBoneAssignmentsOutOfDate = true;
    }
    //-----------------------------------------------------------------------
    /*void SubMesh::_compileBoneAssignments(void)
    {
        uint8 maxBones = rationaliseBoneAssignments( vertexData->vertexCount );

        if (maxBones != 0)
        {
            mParent->compileBoneAssignments(mBoneAssignments, maxBones,
                blendIndexToBoneIndexMap, vertexData);
        }

        mBoneAssignmentsOutOfDate = false;
    }
    //---------------------------------------------------------------------
    uint8 SubMesh::rationaliseBoneAssignments( size_t vertexCount )
    {
        // Iterate through, finding the largest # bones per vertex
        uint8 maxBonesPerVertex         = 0;
        bool existsNonSkinnedVertices   = false;

        //Ensure bone assignments are sorted.
        std::sort( mBoneAssignments.begin(), mBoneAssignments.end() );

        VertexBoneAssignmentVec::const_iterator end = mBoneAssignments.end();

        for( size_t i=0; i < vertexCount; ++i )
        {
            uint8 bonesPerVertex = 0;
            VertexBoneAssignmentVec::iterator first = std::lower_bound( mBoneAssignments.begin(),
                                                                        mBoneAssignments.end(), i );
            VertexBoneAssignmentVec::iterator itor = first;

            while( itor != end && itor->vertexIndex == i )
                ++bonesPerVertex;

            maxBonesPerVertex = std::max( maxBonesPerVertex, bonesPerVertex );

            if( itor == end || itor->vertexIndex != i )
            {
                existsNonSkinnedVertices = true;
            }
            else
            {
                if( bonesPerVertex > OGRE_MAX_BLEND_WEIGHTS )
                {
                    //Trim the assignments that exceed limit allowed.
                    mBoneAssignments.erase( first + OGRE_MAX_BLEND_WEIGHTS, itor );
                    bonesPerVertex = OGRE_MAX_BLEND_WEIGHTS;
                }

                //Always normalize the weights.
                Real totalWeight = 0;

                itor = first;
                while( itor != end && (itor - first) < bonesPerVertex )
                    totalWeight += itor->weight;

                totalWeight = 1.0f / totalWeight;
                itor = first;
                while( itor != end && (itor - first) < bonesPerVertex )
                    itor->weight *= totalWeight;
            }
        }

        if( maxBonesPerVertex > OGRE_MAX_BLEND_WEIGHTS )
        {
            // Warn that we've reduced bone assignments
            LogManager::getSingleton().logMessage("WARNING: the mesh '" + mParent->mName + "' "
                "includes vertices with up to " + StringConverter::toString( maxBonesPerVertex ) +
                " bone assignments, which is more than the limit of " +
                StringConverter::toString(OGRE_MAX_BLEND_WEIGHTS) +
                "The lowest weighted assignments beyond this limit have been removed, so "
                "your animation may look slightly different. To eliminate this, reduce "
                "the number of bone assignments per vertex on your mesh to " +
                StringConverter::toString(OGRE_MAX_BLEND_WEIGHTS) + ".", LML_CRITICAL);

            // we've adjusted them down to the max
            maxBonesPerVertex = OGRE_MAX_BLEND_WEIGHTS;
        }

        if( existsNonSkinnedVertices )
        {
            // Warn that we've non-skinned vertices
            LogManager::getSingleton().logMessage("WARNING: the mesh '" + mParent->mName + "' "
                "includes vertices without bone assignments. Those vertices will "
                "transform to wrong position when skeletal animation enabled. "
                "To eliminate this, assign at least one bone assignment per vertex "
                "on your mesh.", LML_CRITICAL);
        }

        return maxBonesPerVertex;
    }
    //---------------------------------------------------------------------
    void SubMesh::buildIndexMap( IndexMap &boneIndexToBlendIndexMap, IndexMap &blendIndexToBoneIndexMap )
    {
        if( mBoneAssignments.empty() )
        {
            // Just in case
            boneIndexToBlendIndexMap.clear();
            blendIndexToBoneIndexMap.clear();
            return;
        }

        typedef set<unsigned short>::type BoneIndexSet;
        BoneIndexSet usedBoneIndices;

        // Collect actually used bones
        VertexBoneAssignmentVec itor = mBoneAssignments.begin();
        VertexBoneAssignmentVec end  = mBoneAssignments.end();
        while( itor != end )
        {
            usedBoneIndices.insert( itor->boneIndex );
            ++itor;
        }

        // Allocate space for index map
        blendIndexToBoneIndexMap.resize( usedBoneIndices.size() );
        boneIndexToBlendIndexMap.resize( *usedBoneIndices.rbegin() + 1 );

        // Make index map between bone index and blend index
        BoneIndexSet::const_iterator itBoneIndex = usedBoneIndices.begin();
        BoneIndexSet::const_iterator enBoneIndex = usedBoneIndices.end();
        unsigned short blendIndex = 0;

        while( itBoneIndex != enBoneIndex )
        {
            boneIndexToBlendIndexMap[*itBoneIndex] = blendIndex;
            blendIndexToBoneIndexMap[blendIndex] = *itBoneIndex;

            ++itBoneIndex;
            ++blendIndex;
        }
    }*/
    //---------------------------------------------------------------------
    /*SubMesh * SubMesh::clone(const String& newName, Mesh *parentMesh)
    {
        return 0;
    }*/
    //---------------------------------------------------------------------
    void SubMesh::importFromV1( v1::SubMesh *subMesh, bool halfPos, bool halfTexCoords )
    {
        struct SafeDelete
        {
            void *ptr;
            SafeDelete( void *_ptr ) : ptr( _ptr ) {}
            ~SafeDelete()
            {
                if( ptr )
                    OGRE_FREE_SIMD( ptr, MEMCATEGORY_GEOMETRY );
            }
        };

        const v1::SubMesh::VertexBoneAssignmentList& v1BoneAssignments = subMesh->getBoneAssignments();
        mBoneAssignments.reserve( v1BoneAssignments.size() );

        {
            v1::SubMesh::VertexBoneAssignmentList::const_iterator itor = v1BoneAssignments.begin();
            v1::SubMesh::VertexBoneAssignmentList::const_iterator end  = v1BoneAssignments.end();

            while( itor != end )
            {
                mBoneAssignments.push_back( VertexBoneAssignment( itor->second ) );
                ++itor;
            }
        }
        std::sort( mBoneAssignments.begin(), mBoneAssignments.end() );
        mBlendIndexToBoneIndexMap = subMesh->blendIndexToBoneIndexMap;
        mBoneAssignmentsOutOfDate = false;

        VertexElement2Vec vertexElements;
        char *data = arrangeEfficient( subMesh, halfPos, halfTexCoords, &vertexElements );

        //Wrap the ptrs around these, because the VaoManager's call
        //can throw thus causing a leak if we don't free them.
        SafeDelete dataPtrContainer( data );

        VaoManager *vaoManager = mParent->mVaoManager;
        VertexBufferPackedVec vertexBuffers;
        IndexBufferPacked *indexBuffer = 0;

        //Create the vertex buffer
        bool keepAsShadow = mParent->mVertexBufferShadowBuffer;
        VertexBufferPacked *vertexBuffer = vaoManager->createVertexBuffer( vertexElements,
                                                        subMesh->vertexData->vertexCount,
                                                        mParent->mVertexBufferDefaultType,
                                                        data, keepAsShadow );
        vertexBuffers.push_back( vertexBuffer );

        if( keepAsShadow ) //Don't free the pointer ourselves
            dataPtrContainer.ptr = 0;

        v1::IndexData *indexData = subMesh->indexData;
        if( indexData )
        {
            //Create & copy the index buffer
            keepAsShadow = mParent->mIndexBufferShadowBuffer;
            void *indexDataPtr = OGRE_MALLOC_SIMD( indexData->indexCount *
                                                   indexData->indexBuffer->getIndexSize(),
                                                   MEMCATEGORY_GEOMETRY );
            SafeDelete indexDataPtrContainer( indexDataPtr );
            IndexBufferPacked::IndexType indexType = static_cast<IndexBufferPacked::IndexType>(
                                                            indexData->indexBuffer->getType() );

            memcpy( indexDataPtr, indexData->indexBuffer->lock( v1::HardwareBuffer::HBL_READ_ONLY ),
                    indexData->indexBuffer->getIndexSize() * indexData->indexCount );
            indexData->indexBuffer->unlock();

            indexBuffer = vaoManager->createIndexBuffer( indexType, indexData->indexCount,
                                                         mParent->mIndexBufferDefaultType,
                                                         indexDataPtr, keepAsShadow );

            if( keepAsShadow ) //Don't free the pointer ourselves
                indexDataPtrContainer.ptr = 0;
        }

        mVao.push_back( vaoManager->createVertexArrayObject( vertexBuffers, indexBuffer,
                                                             subMesh->operationType ) );
    }
    //---------------------------------------------------------------------
    bool sortVertexElementsBySemantic2( const VertexElement2 &l, const VertexElement2 &r )
    {
        return l.mSemantic < r.mSemantic;
    }
    bool sortVertexElementsBySemantic( const v1::VertexElement &l, const v1::VertexElement &r )
    {
        return l.getSemantic() < r.getSemantic() && l.getIndex() < l.getIndex();
    }

    char* SubMesh::arrangeEfficient( v1::SubMesh *subMesh, bool halfPos, bool halfTexCoords,
                                     VertexElement2Vec *outVertexElements )
    {
        typedef FastArray<v1::VertexElement> VertexElementArray;

        VertexElement2Vec vertexElements;
        VertexElementArray srcElements;
        bool hasTangents = false;

        v1::VertexElement const *tangentElement  = 0;
        v1::VertexElement const *binormalElement = 0;

        v1::VertexData *vertexData = subMesh->vertexData;

        {
            //Get an AZDO-friendly vertex declaration out of the original declaration.
            const v1::VertexDeclaration::VertexElementList &origElements = vertexData->
                                                                vertexDeclaration->getElements();
            srcElements.reserve( origElements.size() );
            v1::VertexDeclaration::VertexElementList::const_iterator itor = origElements.begin();
            v1::VertexDeclaration::VertexElementList::const_iterator end  = origElements.end();

            while( itor != end )
            {
                const v1::VertexElement &origElement = *itor;

                if( origElement.getSemantic() == VES_TANGENT ||
                    origElement.getSemantic() == VES_BINORMAL )
                {
                    hasTangents = true;
                    if( origElement.getSemantic() == VES_TANGENT )
                        tangentElement = &origElement;
                    else
                        binormalElement = &origElement;
                }
                else
                {
                    vertexElements.push_back( VertexElement2( origElement.getType(),
                                                              origElement.getSemantic() ) );
                    srcElements.push_back( *itor );
                }

                //We can't convert to half if it wasn't in floating point
                //Also avoid converting 1 Float ==> 2 Half.
                if( origElement.getBaseType( origElement.getType() ) == VET_FLOAT1 &&
                    origElement.getTypeCount( origElement.getType() ) != 1 )
                {
                    if( (origElement.getSemantic() == VES_POSITION && halfPos) ||
                        (origElement.getSemantic() == VES_TEXTURE_COORDINATES && halfTexCoords) )
                    {
                        VertexElementType type = origElement.multiplyTypeCount(
                                                            VET_HALF2, origElement.getTypeCount(
                                                                            origElement.getType() ) );

                        VertexElement2 &lastInserted = vertexElements.back();
                        lastInserted.mType = type;
                    }
                }

                ++itor;
            }

            //If it has tangents, prepare the normal to hold QTangents.
            if( hasTangents == true )
            {
                VertexElement2Vec::iterator it = std::find( vertexElements.begin(),
                                                            vertexElements.end(),
                                                            VertexElement2( VET_FLOAT3,
                                                                            VES_NORMAL ) );
                if( it != vertexElements.end() )
                    it->mType = VET_SHORT4_SNORM;
            }
        }

        std::sort( vertexElements.begin(), vertexElements.end(), sortVertexElementsBySemantic2 );
        std::sort( srcElements.begin(), srcElements.end(), sortVertexElementsBySemantic );

        //Prepare for the transfer between buffers.
        size_t vertexSize = VaoManager::calculateVertexSize( vertexElements );
        char *data = static_cast<char*>( OGRE_MALLOC_SIMD( vertexSize * vertexData->vertexCount,
                                                           MEMCATEGORY_GEOMETRY ) );

        FastArray<char*> srcPtrs;
        FastArray<size_t> vertexBuffSizes;
        srcPtrs.reserve( vertexData->vertexBufferBinding->getBufferCount() );
        for( size_t i=0; i<vertexData->vertexBufferBinding->getBufferCount(); ++i )
        {
            const v1::HardwareVertexBufferSharedPtr &vBuffer = vertexData->vertexBufferBinding->
                                                                                getBuffer( i );
            srcPtrs.push_back( static_cast<char*>( vBuffer->lock( v1::HardwareBuffer::HBL_READ_ONLY ) ) );
            vertexBuffSizes.push_back( vBuffer->getVertexSize() );
        }

        char *dstData = data;

        //Perform the transfer. Note that vertexElements & srcElements do not match.
        //As vertexElements is modified for smaller types and may include padding
        //for alignment reasons.
        for( size_t i=0; i<vertexData->vertexCount; ++i )
        {
            size_t acumOffset = 0;
            VertexElement2Vec::const_iterator itor = vertexElements.begin();
            VertexElement2Vec::const_iterator end  = vertexElements.end();
            VertexElementArray::const_iterator itSrc = srcElements.begin();

            while( itor != end )
            {
                const VertexElement2 &vElement = *itor;
                size_t writeSize = v1::VertexElement::getTypeSize( vElement.mType );

                if( vElement.mSemantic == VES_NORMAL && hasTangents && vElement.mType != VET_FLOAT3 )
                {
                    size_t readSize = v1::VertexElement::getTypeSize( itSrc->getType() );

                    //Convert TBN matrix (between 6 to 9 floats, 24-36 bytes)
                    //to a QTangent (4 shorts, 8 bytes)
                    assert( readSize == sizeof(float) * 3 );
                    assert( tangentElement->getSize() <= sizeof(float) * 4 &&
                            tangentElement->getSize() >= sizeof(float) * 3 );

                    float normal[3];
                    float tangent[4];
                    tangent[3] = 1.0f;
                    memcpy( normal, srcPtrs[itSrc->getSource()] + itSrc->getOffset(), readSize );
                    memcpy( tangent,
                            srcPtrs[tangentElement->getSource()] + tangentElement->getOffset(),
                            tangentElement->getSize() );

                    Vector3 vNormal( normal[0], normal[1], normal[2] );
                    Vector3 vTangent( tangent[0], tangent[1], tangent[2] );

                    if( binormalElement )
                    {
                        assert( binormalElement->getSize() == sizeof(float) * 3 );
                        float binormal[3];
                        memcpy( binormal,
                                srcPtrs[binormalElement->getSource()] + binormalElement->getOffset(),
                                binormalElement->getSize() );

                        Vector3 vBinormal( binormal[0], binormal[1], binormal[2] );

                        //It is reflected.
                        Vector3 naturalBinormal = vTangent.crossProduct( vNormal );
                        if( naturalBinormal.dotProduct( vBinormal ) <= 0 )
                            tangent[3] = -1.0f;
                    }

                    Matrix3 tbn;
                    tbn.SetColumn( 0, vNormal );
                    tbn.SetColumn( 1, vTangent );
                    tbn.SetColumn( 2, vNormal.crossProduct( vTangent ) );

                    //See Spherical Skinning with Dual-Quaternions and QTangents,
                    //Ivo Zoltan Frey, SIGRAPH 2011 Vancounver.
                    //http://www.crytek.com/download/izfrey_siggraph2011.ppt

                    Quaternion qTangent( tbn );
                    qTangent.normalise();

                    //Bias = 1 / [2^(bits-1) - 1]
                    const Real bias = 1.0f / 32767.0f;

                    //Make sure QTangent is always positive
                    if( qTangent.w < 0 )
                        qTangent = -qTangent;

                    //Because '-0' sign information is lost when using integers,
                    //we need to apply a "bias"; while making sure the Quatenion
                    //stays normalized.
                    // ** Also our shaders assume qTangent.w is never 0. **
                    if( qTangent.w < bias )
                    {
                        Real normFactor = Math::Sqrt( 1 - bias * bias );
                        qTangent.w = bias;
                        qTangent.x *= normFactor;
                        qTangent.y *= normFactor;
                        qTangent.z *= normFactor;
                    }

                    //Now negate if we require reflection
                    if( tangent[3] < 0 )
                        qTangent = -qTangent;

                    uint16 *dstData16 = reinterpret_cast<uint16*>(dstData + acumOffset);

                    dstData16[0] = Math::Clamp( qTangent.x * 32767.0f, -32768.0f, 32767.0f );
                    dstData16[1] = Math::Clamp( qTangent.y * 32767.0f, -32768.0f, 32767.0f );
                    dstData16[2] = Math::Clamp( qTangent.z * 32767.0f, -32768.0f, 32767.0f );
                    dstData16[3] = Math::Clamp( qTangent.w * 32767.0f, -32768.0f, 32767.0f );
                }
                else if( v1::VertexElement::getBaseType( vElement.mType ) == VET_HALF2 &&
                         v1::VertexElement::getBaseType( itSrc->getType() ) == VET_FLOAT1 )
                {
                    size_t readSize = v1::VertexElement::getTypeSize( itSrc->getType() );

                    //Convert float to half.
                    float fpData[4];
                    fpData[0] = fpData[1] = fpData[2] = 0.0f;
                    fpData[3] = 1.0f;
                    memcpy( fpData, srcPtrs[itSrc->getSource()] + itSrc->getOffset(), readSize );

                    uint16 *dstData16 = reinterpret_cast<uint16*>(dstData + acumOffset);

                    for( size_t j=0; j<v1::VertexElement::getTypeCount( vElement.mType ); ++j )
                        dstData16[j] = Bitwise::floatToHalf( fpData[j] );
                }
                else
                {
                    //Raw. Transfer as is.
                    memcpy( dstData + acumOffset,
                            srcPtrs[itSrc->getSource()] + itSrc->getOffset(),
                            writeSize ); //writeSize = readSize
                }

                acumOffset += writeSize;
                ++itSrc;
                ++itor;
            }

            dstData += vertexSize;
            for( size_t j=0; j<srcPtrs.size(); ++j )
                srcPtrs[j] += vertexBuffSizes[j];
        }

        assert( dstData == data + vertexSize * vertexData->vertexCount );

        //Cleanup
        for( size_t i=0; i<vertexData->vertexBufferBinding->getBufferCount(); ++i )
            vertexData->vertexBufferBinding->getBuffer( i )->unlock();

        if( outVertexElements )
            outVertexElements->swap( vertexElements );

        return data;
    }
}
