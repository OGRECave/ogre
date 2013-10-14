/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd

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
#include "OgreRenderOperation.h"
#include "OgreHardwareBufferManager.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreInstancedEntity.h"
#include "OgreMaterial.h"
#include "OgreTechnique.h"
#include "OgreMaterialManager.h"
#include "OgreTexture.h"
#include "OgreTextureManager.h"
#include "OgreRoot.h"

namespace Ogre
{
	static const uint16 c_maxTexWidthHW	= 4096;
	static const uint16 c_maxTexHeightHW	= 4096;

	InstanceBatchHW_VTF::InstanceBatchHW_VTF( 
		InstanceManager *creator, MeshPtr &meshReference, 
		const MaterialPtr &material, size_t instancesPerBatch, 
		const Mesh::IndexMap *indexToBoneMap, const String &batchName )
			: BaseInstanceBatchVTF( creator, meshReference, material, 
									instancesPerBatch, indexToBoneMap, batchName),
			  mKeepStatic( false )
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
		VertexData *baseVertexData = baseSubMesh->vertexData;

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
		mRenderOperation.indexData = baseSubMesh->indexData->clone( true );
		mRemoveOwnIndexData = true;	//Raise flag to remove our own index data in the end (not always needed)
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
										HardwareBuffer::HBU_STATIC_WRITE_ONLY );
		thisVertexData->vertexBufferBinding->setBinding( newSource, mInstanceVertexBuffer );

		//Mark this buffer as instanced
		mInstanceVertexBuffer->setIsInstanceData( true );
		mInstanceVertexBuffer->setInstanceDataStepRate( 1 );

		updateInstanceDataBuffer(true, NULL);
	}

	//updates the vertex buffer containing the per instance data
	size_t InstanceBatchHW_VTF::updateInstanceDataBuffer(bool isFirstTime, Camera* currentCamera)
	{
		size_t visibleEntityCount = 0;
		bool useMatrixLookup = useBoneMatrixLookup();
		if (isFirstTime ^ useMatrixLookup)
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

			const size_t maxPixelsPerLine = std::min( static_cast<size_t>(mMatrixTexture->getWidth()), mMaxFloatsPerLine >> 2 );

			//Calculate UV offsets, which change per instance
			for( size_t i=0; i<mInstancesPerBatch; ++i )
			{
				InstancedEntity* entity = useMatrixLookup ? mInstancedEntities[i] : NULL;
				if  //Update if we are not using a lookup bone matrix method. In this case the function will 
					//be called only once
					(!useMatrixLookup || 
					//Update if we are in the visible range of the camera (for look up bone matrix method
					//and static mode).
					(entity->findVisible(currentCamera)))
				{
					size_t matrixIndex = useMatrixLookup ? entity->mTransformLookupNumber : i;
					size_t instanceIdx = matrixIndex * mMatricesPerInstance * mRowLength;
					*thisVec = ((instanceIdx % maxPixelsPerLine) / texWidth) - (float)(texelOffsets.x);
					*(thisVec + 1) = ((instanceIdx / maxPixelsPerLine) / texHeight) - (float)(texelOffsets.y);
					thisVec += 2;

					if (useMatrixLookup)
					{
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
						if(currentCamera && mManager->getCameraRelativeRendering()) // && useMatrixLookup
						{
							const Vector3 &cameraRelativePosition = currentCamera->getDerivedPosition();
							*(thisVec + 3) -= static_cast<float>( cameraRelativePosition.x );
							*(thisVec + 7) -= static_cast<float>( cameraRelativePosition.y );
							*(thisVec + 11) -=  static_cast<float>( cameraRelativePosition.z );
						}
						thisVec += 12;
					}
					++visibleEntityCount;
				}
			}

			mInstanceVertexBuffer->unlock();
		}
		else
		{
			visibleEntityCount = mInstancedEntities.size();
		}
		return visibleEntityCount;
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
		if( baseSubMesh->vertexData->vertexDeclaration->getNextFreeTextureCoordinate() > 8 - neededTextureCoord )
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
	size_t InstanceBatchHW_VTF::updateVertexTexture( Camera *currentCamera )
	{
		size_t renderedInstances = 0;
		bool useMatrixLookup = useBoneMatrixLookup();
		if (useMatrixLookup)
		{
			//if we are using bone matrix look up we have to update the instance buffer for the 
			//vertex texture to be relevant

			//also note that in this case the number of instances to render comes directly from the 
			//updateInstanceDataBuffer() function, not from this function.
			renderedInstances = updateInstanceDataBuffer(false, currentCamera);
		}

		
		mDirtyAnimation = false;

		//Now lock the texture and copy the 4x3 matrices!
		mMatrixTexture->getBuffer()->lock( HardwareBuffer::HBL_DISCARD );
		const PixelBox &pixelBox = mMatrixTexture->getBuffer()->getCurrentLock();

		float *pSource = static_cast<float*>(pixelBox.data);
		
		InstancedEntityVec::const_iterator itor = mInstancedEntities.begin();
		
		vector<bool>::type writtenPositions(getMaxLookupTableInstances(), false);

		size_t floatPerEntity = mMatricesPerInstance * mRowLength * 4;
		size_t entitiesPerPadding = (size_t)(mMaxFloatsPerLine / floatPerEntity);
		
		size_t instanceCount = mInstancedEntities.size();
		size_t updatedInstances = 0;

		float* transforms = NULL;
		//If using dual quaternions, write 3x4 matrices to a temporary buffer, then convert to dual quaternions
		if(mUseBoneDualQuaternions)
		{
			transforms = mTempTransformsArray3x4;
		}
		
		for(size_t i = 0 ; i < instanceCount ; ++i)
		{
			InstancedEntity* entity = mInstancedEntities[i];
			size_t textureLookupPosition = updatedInstances;
			if (useMatrixLookup)
			{
				textureLookupPosition = entity->mTransformLookupNumber;
			}
			//Check that we are not using a lookup matrix or that we have not already written
			//The bone data
			if (((!useMatrixLookup) || !writtenPositions[entity->mTransformLookupNumber]) &&
				//Cull on an individual basis, the less entities are visible, the less instances we draw.
				//No need to use null matrices at all!
				(entity->findVisible( currentCamera )))
			{
				float* pDest = pSource + floatPerEntity * textureLookupPosition + 
					(size_t)(textureLookupPosition / entitiesPerPadding) * mWidthFloatsPadding;

				if(!mUseBoneDualQuaternions)
				{
					transforms = pDest;
				}
				
				if( mMeshReference->hasSkeleton() )
					mDirtyAnimation |= entity->_updateAnimation();

				size_t floatsWritten = entity->getTransforms3x4( transforms );

				if( !useMatrixLookup && mManager->getCameraRelativeRendering() )
					makeMatrixCameraRelative3x4( transforms, floatsWritten );

				if(mUseBoneDualQuaternions)
				{
					convert3x4MatricesToDualQuaternions(transforms, floatsWritten / 12, pDest);
				}

				if (useMatrixLookup)
				{
					writtenPositions[entity->mTransformLookupNumber] = true;
				}
				else
				{
					++updatedInstances;
				}
			}

			++itor;
		}

		if (!useMatrixLookup)
		{
			renderedInstances = updatedInstances;
		}

		mMatrixTexture->getBuffer()->unlock();

		return renderedInstances;
	}
	//-----------------------------------------------------------------------
	void InstanceBatchHW_VTF::_boundsDirty(void)
	{
		//Don't update if we're static, but still mark we're dirty
		if( !mBoundsDirty && !mKeepStatic && mCreator)
			mCreator->_addDirtyBatch( this );
		mBoundsDirty = true;
	}
	//-----------------------------------------------------------------------
	void InstanceBatchHW_VTF::setStaticAndUpdate( bool bStatic )
	{
		//We were dirty but didn't update bounds. Do it now.
		if( mKeepStatic && mBoundsDirty )
			mCreator->_addDirtyBatch( this );

		mKeepStatic = bStatic;
		if( mKeepStatic )
		{
			//One final update, since there will be none from now on
			//(except further calls to this function). Pass NULL because
			//we want to include only those who were added to the scene
			//but we don't want to perform culling
			mRenderOperation.numberOfInstances = updateVertexTexture( 0 );
		}
	}
	//-----------------------------------------------------------------------
	void InstanceBatchHW_VTF::_updateRenderQueue( RenderQueue* queue )
	{
		if( !mKeepStatic )
		{
			//Completely override base functionality, since we don't cull on an "all-or-nothing" basis
			if( (mRenderOperation.numberOfInstances = updateVertexTexture( mCurrentCamera )) )
				queue->addRenderable( this, mRenderQueueID, mRenderQueuePriority );
		}
		else
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
		}
	}
}
