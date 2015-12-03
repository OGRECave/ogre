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
#include "OgreInstanceBatchHW_VTF.h"
#include "OgreSubMesh.h"
#include "OgreHardwareBufferManager.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreInstancedEntity.h"
#include "OgreCamera.h"
#include "OgreViewport.h"
#include "OgreRoot.h"

namespace Ogre
{
namespace v1
{
    static const uint16 c_maxTexWidthHW = 4096;
    static const uint16 c_maxTexHeightHW    = 4096;

    InstanceBatchHW_VTF::InstanceBatchHW_VTF( 
        IdType id, ObjectMemoryManager *objectMemoryManager,
        InstanceManager *creator, MeshPtr &meshReference, 
        const MaterialPtr &material, size_t instancesPerBatch, 
        const Mesh::IndexMap *indexToBoneMap )
            : BaseInstanceBatchVTF( id, objectMemoryManager, creator, meshReference, material,
                                    instancesPerBatch, indexToBoneMap )
    {
    }
    //-----------------------------------------------------------------------
    InstanceBatchHW_VTF::~InstanceBatchHW_VTF()
    {
    }   
    //-----------------------------------------------------------------------
    void InstanceBatchHW_VTF::setupVertices( const SubMesh* baseSubMesh )
    {
        mRenderOperation.vertexData = OGRE_NEW VertexData();
        mRemoveOwnVertexData = true; //Raise flag to remove our own vertex data in the end (not always needed)

        VertexData *thisVertexData = mRenderOperation.vertexData;
        VertexData *baseVertexData = baseSubMesh->vertexData[VpNormal];

        thisVertexData->vertexStart = 0;
        thisVertexData->vertexCount = baseVertexData->vertexCount;
        mRenderOperation.numberOfInstances = mInstancesPerBatch;

        HardwareBufferManager::getSingleton().destroyVertexDeclaration(
                                                                    thisVertexData->vertexDeclaration );
        thisVertexData->vertexDeclaration = baseVertexData->vertexDeclaration->clone();

        //Reuse all vertex buffers
        VertexBufferBinding::VertexBufferBindingMap::const_iterator itor = baseVertexData->
                                                            vertexBufferBinding->getBindings().begin();
        VertexBufferBinding::VertexBufferBindingMap::const_iterator end  = baseVertexData->
                                                            vertexBufferBinding->getBindings().end();
        while( itor != end )
        {
            const unsigned short bufferIdx = itor->first;
            const HardwareVertexBufferSharedPtr vBuf = itor->second;
            thisVertexData->vertexBufferBinding->setBinding( bufferIdx, vBuf );
            ++itor;
        }

        //Remove the blend weights & indices
        HWBoneIdxVec hwBoneIdx;
        HWBoneWgtVec hwBoneWgt;

        //Blend weights may not be present because HW_VTF does not require to be skeletally animated
        const VertexElement *veWeights = baseVertexData->vertexDeclaration->
                                                        findElementBySemantic( VES_BLEND_WEIGHTS ); 
        if( veWeights )
            mWeightCount = forceOneWeight() ? 1 : veWeights->getSize() / sizeof(float);
        else
            mWeightCount = 1;

        hwBoneIdx.resize( baseVertexData->vertexCount * mWeightCount, 0 );

        if( mMeshReference->hasSkeleton() && !mMeshReference->getSkeleton().isNull() )
        {
            if(mWeightCount > 1)
            {
                hwBoneWgt.resize( baseVertexData->vertexCount * mWeightCount, 0 );
                retrieveBoneIdxWithWeights(baseVertexData, hwBoneIdx, hwBoneWgt);
            }
            else
            {
                retrieveBoneIdx( baseVertexData, hwBoneIdx );
            }

            const VertexElement* pElement = thisVertexData->vertexDeclaration->findElementBySemantic
                                                                                    (VES_BLEND_INDICES);
            if (pElement) 
            {
                unsigned short skelDataSource = pElement->getSource();
                thisVertexData->vertexDeclaration->removeElement( VES_BLEND_INDICES );
                thisVertexData->vertexDeclaration->removeElement( VES_BLEND_WEIGHTS );
                if (thisVertexData->vertexDeclaration->findElementsBySource(skelDataSource).empty())
                {
                    thisVertexData->vertexDeclaration->closeGapsInSource();
                    thisVertexData->vertexBufferBinding->unsetBinding(skelDataSource);
                    VertexBufferBinding::BindingIndexMap tmpMap;
                    thisVertexData->vertexBufferBinding->closeGaps(tmpMap);
                }
            }
        }

        createVertexTexture( baseSubMesh );
        createVertexSemantics( thisVertexData, baseVertexData, hwBoneIdx, hwBoneWgt);
    }
    //-----------------------------------------------------------------------
    void InstanceBatchHW_VTF::setupIndices( const SubMesh* baseSubMesh )
    {
        //We could use just a reference, but the InstanceManager will in the end attampt to delete
        //the pointer, and we can't give it something that doesn't belong to us.
        mRenderOperation.indexData = baseSubMesh->indexData[VpNormal]->clone( true );
        mRemoveOwnIndexData = true; //Raise flag to remove our own index data in the end (not always needed)
    }
    //-----------------------------------------------------------------------
    void InstanceBatchHW_VTF::createVertexSemantics( VertexData *thisVertexData,
                                                         VertexData *baseVertexData,
                                                         const HWBoneIdxVec &hwBoneIdx,
                                                         const HWBoneWgtVec& hwBoneWgt)
    {
        const float texWidth  = static_cast<float>(mMatrixTexture->getWidth());

        //Only one weight per vertex is supported. It would not only be complex, but prohibitively slow.
        //Put them in a new buffer, since it's 16 bytes aligned :-)
        unsigned short newSource = thisVertexData->vertexDeclaration->getMaxSource() + 1;

        size_t offset = 0;

        size_t maxFloatsPerVector = 4;

        //Can fit two dual quaternions in every float4, but only one 3x4 matrix
        for(size_t i = 0; i < mWeightCount; i += maxFloatsPerVector / mRowLength)
        {
            offset += thisVertexData->vertexDeclaration->addElement( newSource, offset, VET_FLOAT4, VES_TEXTURE_COORDINATES,
                                        thisVertexData->vertexDeclaration->getNextFreeTextureCoordinate() ).getSize();
        }

        //Add the weights (supports up to four, which is Ogre's limit)
        if(mWeightCount > 1)
        {
            thisVertexData->vertexDeclaration->addElement(newSource, offset, VET_FLOAT4, VES_BLEND_WEIGHTS,
                                        0 ).getSize();
        }
        
        //Create our own vertex buffer
        HardwareVertexBufferSharedPtr vertexBuffer =
            HardwareBufferManager::getSingleton().createVertexBuffer(
            thisVertexData->vertexDeclaration->getVertexSize(newSource),
            thisVertexData->vertexCount,
            HardwareBuffer::HBU_STATIC_WRITE_ONLY );
        thisVertexData->vertexBufferBinding->setBinding( newSource, vertexBuffer );

        float *thisFloat = static_cast<float*>(vertexBuffer->lock(HardwareBuffer::HBL_DISCARD));

        //Create the UVs to sample from the right bone/matrix
        for( size_t j=0; j < baseVertexData->vertexCount * mWeightCount; j += mWeightCount)
        {
            size_t numberOfMatricesInLine = 0;
            
            //Write the matrices, adding padding as needed
            for(size_t i = 0; i < mWeightCount; ++i)
            {
                //Write the matrix
                for( size_t k=0; k < mRowLength; ++k)
                {
                    //Only calculate U (not V) since all matrices are in the same row. We use the instanced
                    //(repeated) buffer to tell how much U & V we need to offset
                    size_t instanceIdx = hwBoneIdx[j+i] * mRowLength + k;
                    *thisFloat++ = instanceIdx / texWidth;
                }

                ++numberOfMatricesInLine;

                //If another matrix can't be fit, we're on another line, or if this is the last weight
                if((numberOfMatricesInLine + 1) * mRowLength > maxFloatsPerVector || (i+1) == mWeightCount)
                {
                    //Place zeroes in the remaining coordinates
                    for ( size_t k=mRowLength * numberOfMatricesInLine; k < maxFloatsPerVector; ++k)
                    {
                        *thisFloat++ = 0.0f;
                    }

                    numberOfMatricesInLine = 0;
                }
            }

            //Don't need to write weights if there is only one
            if(mWeightCount > 1)
            {
                //Write the weights
                for(size_t i = 0; i < mWeightCount; ++i)
                {
                    *thisFloat++ = hwBoneWgt[j+i];
                }

                //Write the empty space
                for(size_t i = mWeightCount; i < maxFloatsPerVector; ++i)
                {
                    *thisFloat++ = 0.0f;
                }
            }
        }

        vertexBuffer->unlock();

        //Now create the instance buffer that will be incremented per instance, contains UV offsets
        newSource = thisVertexData->vertexDeclaration->getMaxSource() + 1;
        offset = thisVertexData->vertexDeclaration->addElement( newSource, 0, VET_FLOAT2, VES_TEXTURE_COORDINATES,
                                    thisVertexData->vertexDeclaration->getNextFreeTextureCoordinate() ).getSize();
        if (useBoneMatrixLookup())
        {
            //if using bone matrix lookup we will need to add 3 more float4 to contain the matrix. containing
            //the personal world transform of each entity.
            offset += thisVertexData->vertexDeclaration->addElement( newSource, offset, VET_FLOAT4, VES_TEXTURE_COORDINATES,
                thisVertexData->vertexDeclaration->getNextFreeTextureCoordinate() ).getSize();
            offset += thisVertexData->vertexDeclaration->addElement( newSource, offset, VET_FLOAT4, VES_TEXTURE_COORDINATES,
                thisVertexData->vertexDeclaration->getNextFreeTextureCoordinate() ).getSize();
            thisVertexData->vertexDeclaration->addElement( newSource, offset, VET_FLOAT4, VES_TEXTURE_COORDINATES,
                thisVertexData->vertexDeclaration->getNextFreeTextureCoordinate() ).getSize();
            //Add two floats of padding here? or earlier?
            //If not using bone matrix lookup, is it ok that it is 8 bytes since divides evenly into 16
        }

        //Create our own vertex buffer
        mInstanceVertexBuffer = HardwareBufferManager::getSingleton().createVertexBuffer(
                                        thisVertexData->vertexDeclaration->getVertexSize(newSource),
                                        mInstancesPerBatch,
                                        useBoneMatrixLookup() ?
                                            HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE :
                                            HardwareBuffer::HBU_STATIC_WRITE_ONLY );
        thisVertexData->vertexBufferBinding->setBinding( newSource, mInstanceVertexBuffer );

        //Mark this buffer as instanced
        mInstanceVertexBuffer->setIsInstanceData( true );
        mInstanceVertexBuffer->setInstanceDataStepRate( 1 );

        if( !useBoneMatrixLookup() )
            fillVertexBufferOffsets();
    }
    //-----------------------------------------------------------------------
    void InstanceBatchHW_VTF::fillVertexBufferOffsets(void)
    {
        const float texWidth  = static_cast<float>(mMatrixTexture->getWidth());
        const float texHeight = static_cast<float>(mMatrixTexture->getHeight());

        //Calculate the texel offsets to correct them offline
        //Awkwardly enough, the offset is needed in OpenGL too
        Vector2 texelOffsets;
        //RenderSystem *renderSystem = Root::getSingleton().getRenderSystem();
        texelOffsets.x = /*renderSystem->getHorizontalTexelOffset()*/ -0.5f / texWidth;
        texelOffsets.y = /*renderSystem->getHorizontalTexelOffset()*/ -0.5f / texHeight;

        const size_t maxPixelsPerLine = std::min( mMatrixTexture->getWidth(), static_cast<uint32>(mMaxFloatsPerLine >> 2) );

        Vector2 *thisVec = static_cast<Vector2*>(
                                mInstanceVertexBuffer->lock( HardwareBuffer::HBL_DISCARD ) );

        //Calculate UV offsets, which change per instance
        for( size_t i=0; i<mInstancesPerBatch; ++i )
        {
            size_t instanceIdx = i * mMatricesPerInstance * mRowLength;
            thisVec->x = (instanceIdx % maxPixelsPerLine) / texWidth - (float)(texelOffsets.x);
            thisVec->y = (instanceIdx / maxPixelsPerLine) / texHeight - (float)(texelOffsets.x);
            ++thisVec;
        }

        mInstanceVertexBuffer->unlock();
    }
    //-----------------------------------------------------------------------
    void InstanceBatchHW_VTF::fillVertexBufferLUT( const MovableObjectArray *culledInstances )
    {
        //update the mTransformLookupNumber value in the entities if needed 
        updateSharedLookupIndexes();

        const float texWidth  = static_cast<float>(mMatrixTexture->getWidth());
        const float texHeight = static_cast<float>(mMatrixTexture->getHeight());

        //Calculate the texel offsets to correct them offline
        //Awkwardly enough, the offset is needed in OpenGL too
        Vector2 texelOffsets;
        //RenderSystem *renderSystem = Root::getSingleton().getRenderSystem();
        texelOffsets.x = /*renderSystem->getHorizontalTexelOffset()*/ -0.5f / texWidth;
        texelOffsets.y = /*renderSystem->getHorizontalTexelOffset()*/ -0.5f / texHeight;

        float *thisVec = static_cast<float*>(mInstanceVertexBuffer->lock(HardwareBuffer::HBL_DISCARD));

        const size_t maxPixelsPerLine = std::min( mMatrixTexture->getWidth(), static_cast<uint32>(mMaxFloatsPerLine >> 2) );

        MovableObjectArray::const_iterator itor = culledInstances->begin();
        MovableObjectArray::const_iterator end  = culledInstances->end();

        while( itor != end )
        {
            assert( dynamic_cast<InstancedEntity*>(*itor) );
            const InstancedEntity *entity = static_cast<InstancedEntity*>(*itor);
            size_t matrixIndex = entity->mTransformLookupNumber;
            size_t instanceIdx = matrixIndex * mMatricesPerInstance * mRowLength;
            *thisVec = ((instanceIdx % maxPixelsPerLine) / texWidth) - (float)(texelOffsets.x);
            *(thisVec + 1) = ((instanceIdx / maxPixelsPerLine) / texHeight) - (float)(texelOffsets.y);
            thisVec += 2;

            const Matrix4& mat =  entity->_getParentNodeFullTransform();
            *(thisVec)     = static_cast<float>( mat[0][0] );
            *(thisVec + 1) = static_cast<float>( mat[0][1] );
            *(thisVec + 2) = static_cast<float>( mat[0][2] );
            *(thisVec + 3) = static_cast<float>( mat[0][3] );
            *(thisVec + 4) = static_cast<float>( mat[1][0] );
            *(thisVec + 5) = static_cast<float>( mat[1][1] );
            *(thisVec + 6) = static_cast<float>( mat[1][2] );
            *(thisVec + 7) = static_cast<float>( mat[1][3] );
            *(thisVec + 8) = static_cast<float>( mat[2][0] );
            *(thisVec + 9) = static_cast<float>( mat[2][1] );
            *(thisVec + 10)= static_cast<float>( mat[2][2] );
            *(thisVec + 11)= static_cast<float>( mat[2][3] );

            //TODO: This hurts my eyes (reading back from write-combining memory)
            /*if(currentCamera && mManager->getCameraRelativeRendering()) // && useMatrixLookup
            {
                const Vector3 &cameraRelativePosition = currentCamera->getDerivedPosition();
                *(thisVec + 3) -= static_cast<float>( cameraRelativePosition.x );
                *(thisVec + 7) -= static_cast<float>( cameraRelativePosition.y );
                *(thisVec + 11) -=  static_cast<float>( cameraRelativePosition.z );
            }*/
            thisVec += 12;

            ++itor;
        }

        mInstanceVertexBuffer->unlock();
    }
    
    //-----------------------------------------------------------------------
    bool InstanceBatchHW_VTF::checkSubMeshCompatibility( const SubMesh* baseSubMesh )
    {
        //Max number of texture coordinates is _usually_ 8, we need at least 2 available
        unsigned short neededTextureCoord = 2;
        if (useBoneMatrixLookup())
        {
            //we need another 3 for the unique world transform of each instanced entity
            neededTextureCoord += 3;
        }
        if( baseSubMesh->vertexData[VpNormal]->vertexDeclaration->
                getNextFreeTextureCoordinate() > 8 - neededTextureCoord )
        {
            OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, 
                    String("Given mesh must have at least ") + 
                    StringConverter::toString(neededTextureCoord) + "free TEXCOORDs",
                    "InstanceBatchHW_VTF::checkSubMeshCompatibility");
        }

        return InstanceBatch::checkSubMeshCompatibility( baseSubMesh );
    }
    //-----------------------------------------------------------------------
    size_t InstanceBatchHW_VTF::calculateMaxNumInstances( 
                    const SubMesh *baseSubMesh, uint16 flags ) const
    {
        size_t retVal = 0;

        RenderSystem *renderSystem = Root::getSingleton().getRenderSystem();
        const RenderSystemCapabilities *capabilities = renderSystem->getCapabilities();

        //VTF & HW Instancing must be supported
        if( capabilities->hasCapability( RSC_VERTEX_BUFFER_INSTANCE_DATA ) &&
            capabilities->hasCapability( RSC_VERTEX_TEXTURE_FETCH ) )
        {
            //TODO: Check PF_FLOAT32_RGBA is supported (should be, since it was the 1st one)
            const size_t numBones = std::max<size_t>( 1, baseSubMesh->blendIndexToBoneIndexMap.size() );

            const size_t maxUsableWidth = c_maxTexWidthHW - (c_maxTexWidthHW % (numBones * mRowLength));

            //See InstanceBatchHW::calculateMaxNumInstances for the 65535
            retVal = std::min<size_t>( 65535, maxUsableWidth * c_maxTexHeightHW / mRowLength / numBones );

            if( flags & IM_VTFBESTFIT )
            {
                size_t numUsedSkeletons = mInstancesPerBatch;
                if (flags & IM_VTFBONEMATRIXLOOKUP)
                    numUsedSkeletons = std::min<size_t>(getMaxLookupTableInstances(), numUsedSkeletons);
                const size_t instancesPerBatch = std::min( retVal, numUsedSkeletons );
                //Do the same as in createVertexTexture(), but changing c_maxTexWidthHW for maxUsableWidth
                const size_t numWorldMatrices = instancesPerBatch * numBones;

                size_t texWidth  = std::min<size_t>( numWorldMatrices * mRowLength, maxUsableWidth );
                size_t texHeight = numWorldMatrices * mRowLength / maxUsableWidth;

                const size_t remainder = (numWorldMatrices * mRowLength) % maxUsableWidth;

                if( remainder && texHeight > 0 )
                    retVal = static_cast<size_t>(texWidth * texHeight / (float)mRowLength / (float)(numBones));
            }
        }

        return retVal;
    }
    //-----------------------------------------------------------------------
    size_t InstanceBatchHW_VTF::updateVertexTexture( Camera *camera, const Camera *lodCamera )
    {
        MovableObjectArray *visibleObjects = 0;
        if( mManager->getInstancingThreadedCullingMethod() == INSTANCING_CULLING_SINGLETHREAD )
        {
            //Perform the culling now
            ObjectData objData;
            const size_t numObjs = mLocalObjectMemoryManager.getFirstObjectData( objData, 0 );

            visibleObjects = &mManager->_getTmpVisibleObjectsList()[0][mRenderQueueID];
            visibleObjects->clear();

            //TODO: Static batches aren't yet supported (camera ptr will be null and crash)
            MovableObject::cullFrustum( numObjs, objData, camera,
                        camera->getLastViewport()->getVisibilityMask()&mManager->getVisibilityMask(),
                        *visibleObjects, lodCamera );
        }
        else
        {
            //Get the results from the time the threaded version ran.
            visibleObjects = &mCulledInstances;
        }

        bool useMatrixLookup = useBoneMatrixLookup();
        if (useMatrixLookup)
        {
            //if we are using bone matrix look up we have to update the instance buffer for the 
            //vertex texture to be relevant
            fillVertexBufferLUT( visibleObjects );
        }

        //Now lock the texture and copy the 4x3 matrices!
        size_t floatsPerEntity = mMatricesPerInstance * mRowLength * 4;
        size_t entitiesPerPadding = (size_t)(mMaxFloatsPerLine / floatsPerEntity);

        mMatrixTexture->getBuffer()->lock( HardwareBuffer::HBL_DISCARD );
        const PixelBox &pixelBox = mMatrixTexture->getBuffer()->getCurrentLock();

        float *pDest = static_cast<float*>(pixelBox.data);

        if( mMeshReference->getSkeleton().isNull() )
        {
            //No animations, no anything (perhaps HW Basic is a better technique for this case)
            std::for_each( visibleObjects->begin(), visibleObjects->end(),
                            SendAllSingleTransformsToTexture( pDest, floatsPerEntity,
                                                entitiesPerPadding, mWidthFloatsPadding ) );
        }
        else
        {
            if( !useMatrixLookup && !mUseBoneDualQuaternions )
            {
                // Animations, normal
                std::for_each( visibleObjects->begin(), visibleObjects->end(),
                                SendAllAnimatedTransformsToTexture( pDest, floatsPerEntity,
                                                        entitiesPerPadding, mWidthFloatsPadding,
                                                        mIndexToBoneMap ) );
            }
            else if( mUseBoneDualQuaternions )
            {
                // Animations, Dual Quaternion Skinning
                std::for_each( visibleObjects->begin(), visibleObjects->end(),
                                SendAllDualQuatTexture( pDest, floatsPerEntity,
                                                        entitiesPerPadding,
                                                        mWidthFloatsPadding,
                                                        mIndexToBoneMap ) );
            }
            else
            {
                // Animations, LUT (lookup table)
                std::for_each( visibleObjects->begin(), visibleObjects->end(),
                                SendAllLUTToTexture( pDest, floatsPerEntity,
                                                     entitiesPerPadding, mWidthFloatsPadding,
                                                     mIndexToBoneMap,
                                                     getMaxLookupTableInstances() ) );
            }
        }

        mMatrixTexture->getBuffer()->unlock();

        return visibleObjects->size();
    }
    //-----------------------------------------------------------------------
    void InstanceBatchHW_VTF::createAllInstancedEntities(void)
    {
        mCulledInstances.reserve( mInstancesPerBatch );
        InstanceBatch::createAllInstancedEntities();
    }
    //-----------------------------------------------------------------------
    void InstanceBatchHW_VTF::_updateRenderQueue( RenderQueue* queue, Camera *camera,
                                                  const Camera *lodCamera )
    {
        //if( !mKeepStatic )
        /*{
            TODO: RENDER QUEUE
            //Completely override base functionality, since we don't cull on an "all-or-nothing" basis
            if( (mRenderOperation.numberOfInstances = updateVertexTexture( camera, lodCamera )) )
                queue->addRenderable( this, mRenderQueueID, mRenderQueuePriority );
        }*/
        /*else
        {
            if( mManager->getCameraRelativeRendering() )
            {
                OGRE_EXCEPT(Exception::ERR_INVALID_STATE, "Camera-relative rendering is incompatible"
                    " with Instancing's static batches. Disable at least one of them",
                    "InstanceBatch::_updateRenderQueue");
            }

            //Don't update when we're static
            if( mRenderOperation.numberOfInstances )
                queue->addRenderable( this, mRenderQueueID, mRenderQueuePriority );
        }*/
    }
    //-----------------------------------------------------------------------
    FORCEINLINE void InstanceBatchHW_VTF::SendAllSingleTransformsToTexture::operator ()
                                        ( const MovableObject *movableObject )
    {
        assert( dynamic_cast<const InstancedEntity*>(movableObject) );
        const InstancedEntity *instancedEntity = static_cast<const InstancedEntity*>(movableObject);

        float * RESTRICT_ALIAS pDest = mDest + mFloatsPerEntity * mInstancesWritten + 
                                (size_t)(mInstancesWritten / mEntitiesPerPadding) * mWidthFloatsPadding;

        //Write transform matrix
        instancedEntity->writeSingleTransform3x4( pDest );
        ++mInstancesWritten;
    }
    //-----------------------------------------------------------------------
    FORCEINLINE void InstanceBatchHW_VTF::SendAllAnimatedTransformsToTexture::operator ()
                                        ( const MovableObject *movableObject )
    {
        assert( dynamic_cast<const InstancedEntity*>(movableObject) );
        const InstancedEntity *instancedEntity = static_cast<const InstancedEntity*>(movableObject);

        float * RESTRICT_ALIAS pDest = mDest + mFloatsPerEntity * mInstancesWritten + 
                                (size_t)(mInstancesWritten / mEntitiesPerPadding) * mWidthFloatsPadding;

        //Write transform matrices from each bone
        instancedEntity->writeAnimatedTransform3x4( pDest, boneIdxStart, boneIdxEnd );
        ++mInstancesWritten;
    }
    //-----------------------------------------------------------------------
    FORCEINLINE void InstanceBatchHW_VTF::SendAllLUTToTexture::operator ()
                                        ( const MovableObject *movableObject )
    {
        assert( dynamic_cast<const InstancedEntity*>(movableObject) );
        const InstancedEntity *instancedEntity = static_cast<const InstancedEntity*>(movableObject);

        size_t lutIdx = instancedEntity->mTransformLookupNumber;
        if( !mWrittenPositions[lutIdx] )
        {
            float * RESTRICT_ALIAS pDest = mDest + mFloatsPerEntity * lutIdx +
                                    (size_t)(lutIdx / mEntitiesPerPadding) *
                                    mWidthFloatsPadding;

            //Write transform matrices from each bone
            instancedEntity->writeLutTransform3x4( pDest, boneIdxStart, boneIdxEnd );
            mWrittenPositions[lutIdx] = true;
        }
    }
    //-----------------------------------------------------------------------
    FORCEINLINE void InstanceBatchHW_VTF::SendAllDualQuatTexture::operator ()
                                        ( const MovableObject *movableObject )
    {
        assert( dynamic_cast<const InstancedEntity*>(movableObject) );
        const InstancedEntity *instancedEntity = static_cast<const InstancedEntity*>(movableObject);

        float * RESTRICT_ALIAS pDest = mDest + mFloatsPerEntity * mInstancesWritten + 
                                (size_t)(mInstancesWritten / mEntitiesPerPadding) * mWidthFloatsPadding;

        //Write transform matrices from each bone
        instancedEntity->writeDualQuatTransform( pDest, boneIdxStart, boneIdxEnd );
        ++mInstancesWritten;
    }
}
}
