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
#include "OgreInstanceBatchVTF.h"
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
#include "OgreDualQuaternion.h"

namespace Ogre
{
	static const uint16 c_maxTexWidth	= 4096;
	static const uint16 c_maxTexHeight	= 4096;

	BaseInstanceBatchVTF::BaseInstanceBatchVTF( InstanceManager *creator, MeshPtr &meshReference,
										const MaterialPtr &material, size_t instancesPerBatch,
										const Mesh::IndexMap *indexToBoneMap, const String &batchName) :
				InstanceBatch( creator, meshReference, material, instancesPerBatch,
								indexToBoneMap, batchName ),
                mMatricesPerInstance(0),
				mNumWorldMatrices( instancesPerBatch ),
				mWidthFloatsPadding( 0 ),
				mMaxFloatsPerLine( std::numeric_limits<size_t>::max() ),
				mRowLength(3),
				mWeightCount(1),
				mTempTransformsArray3x4(0),
				mUseBoneMatrixLookup(false),
				mMaxLookupTableInstances(16),
				mUseBoneDualQuaternions(false),
				mForceOneWeight(false),
				mUseOneWeight(false)
	{
		cloneMaterial( mMaterial );
	}

	BaseInstanceBatchVTF::~BaseInstanceBatchVTF()
	{
		//Remove cloned caster materials (if any)
		Material::TechniqueIterator techItor = mMaterial->getTechniqueIterator();
		while( techItor.hasMoreElements() )
		{
			Technique *technique = techItor.getNext();

			if( !technique->getShadowCasterMaterial().isNull() )
				MaterialManager::getSingleton().remove( technique->getShadowCasterMaterial()->getName() );
		}

		//Remove cloned material
		MaterialManager::getSingleton().remove( mMaterial->getName() );

		//Remove the VTF texture
		if( !mMatrixTexture.isNull() )
			TextureManager::getSingleton().remove( mMatrixTexture->getName() );

		OGRE_FREE(mTempTransformsArray3x4, MEMCATEGORY_GENERAL);
	}

	//-----------------------------------------------------------------------
	void BaseInstanceBatchVTF::buildFrom( const SubMesh *baseSubMesh, const RenderOperation &renderOperation )
	{
		if (useBoneMatrixLookup())
		{
			//when using bone matrix lookup resource are not shared
			//
			//Future implementation: while the instance vertex buffer can't be shared
			//The texture can be.
			//
			build(baseSubMesh);
		}
		else
		{
			createVertexTexture( baseSubMesh );
			InstanceBatch::buildFrom( baseSubMesh, renderOperation );
		}
	}
	//-----------------------------------------------------------------------
	void BaseInstanceBatchVTF::cloneMaterial( const MaterialPtr &material )
	{
		//Used to track down shadow casters, so the same material caster doesn't get cloned twice
		typedef map<String, MaterialPtr>::type MatMap;
		MatMap clonedMaterials;

		//We need to clone the material so we can have different textures for each batch.
		mMaterial = material->clone( mName + "/VTFMaterial" );

		//Now do the same with the techniques which have a material shadow caster
		Material::TechniqueIterator techItor = material->getTechniqueIterator();
		while( techItor.hasMoreElements() )
		{
			Technique *technique = techItor.getNext();

			if( !technique->getShadowCasterMaterial().isNull() )
			{
				const MaterialPtr &casterMat	= technique->getShadowCasterMaterial();
				const String &casterName		= casterMat->getName();

				//Was this material already cloned?
				MatMap::const_iterator itor = clonedMaterials.find(casterName);

				if( itor == clonedMaterials.end() )
				{
					//No? Clone it and track it
					MaterialPtr cloned = casterMat->clone( mName + "/VTFMaterialCaster" +
													StringConverter::toString(clonedMaterials.size()) );
					technique->setShadowCasterMaterial( cloned );
					clonedMaterials[casterName] = cloned;
				}
				else
					technique->setShadowCasterMaterial( itor->second ); //Reuse the previously cloned mat
			}
		}
	}
	//-----------------------------------------------------------------------
	void BaseInstanceBatchVTF::retrieveBoneIdx( VertexData *baseVertexData, HWBoneIdxVec &outBoneIdx )
	{
		const VertexElement *ve = baseVertexData->vertexDeclaration->
															findElementBySemantic( VES_BLEND_INDICES );
		const VertexElement *veWeights = baseVertexData->vertexDeclaration->findElementBySemantic( VES_BLEND_WEIGHTS );
		
		HardwareVertexBufferSharedPtr buff = baseVertexData->vertexBufferBinding->getBuffer(ve->getSource());
		char const *baseBuffer = static_cast<char const*>(buff->lock( HardwareBuffer::HBL_READ_ONLY ));

		for( size_t i=0; i<baseVertexData->vertexCount; ++i )
		{
			float const *pWeights = reinterpret_cast<float const*>(baseBuffer + veWeights->getOffset());

			uint8 biggestWeightIdx = 0;
			for( size_t j=1; j< mWeightCount; ++j )
			{
				biggestWeightIdx = pWeights[biggestWeightIdx] < pWeights[j] ? j : biggestWeightIdx;
			}

			uint8 const *pIndex = reinterpret_cast<uint8 const*>(baseBuffer + ve->getOffset());
			outBoneIdx[i] = pIndex[biggestWeightIdx];

			baseBuffer += baseVertexData->vertexDeclaration->getVertexSize(ve->getSource());
		}

		buff->unlock();
	}

	//-----------------------------------------------------------------------
	void BaseInstanceBatchVTF::retrieveBoneIdxWithWeights(VertexData *baseVertexData, HWBoneIdxVec &outBoneIdx, HWBoneWgtVec &outBoneWgt)
	{
		const VertexElement *ve = baseVertexData->vertexDeclaration->findElementBySemantic( VES_BLEND_INDICES );
		const VertexElement *veWeights = baseVertexData->vertexDeclaration->findElementBySemantic( VES_BLEND_WEIGHTS );
		
		HardwareVertexBufferSharedPtr buff = baseVertexData->vertexBufferBinding->getBuffer(ve->getSource());
		char const *baseBuffer = static_cast<char const*>(buff->lock( HardwareBuffer::HBL_READ_ONLY ));

		for( size_t i=0; i<baseVertexData->vertexCount * mWeightCount; i += mWeightCount)
		{
			float const *pWeights = reinterpret_cast<float const*>(baseBuffer + veWeights->getOffset());
			uint8 const *pIndex = reinterpret_cast<uint8 const*>(baseBuffer + ve->getOffset());

			float weightMagnitude = 0.0f;
			for( size_t j=0; j < mWeightCount; ++j )
			{
				outBoneWgt[i+j] = pWeights[j];
				weightMagnitude += pWeights[j];
				outBoneIdx[i+j] = pIndex[j];
			}

			//Normalize the bone weights so they add to one
			for(size_t j=0; j < mWeightCount; ++j)
			{
				outBoneWgt[i+j] /= weightMagnitude;
			}

			baseBuffer += baseVertexData->vertexDeclaration->getVertexSize(ve->getSource());
		}

		buff->unlock();
	}
	
	//-----------------------------------------------------------------------
	void BaseInstanceBatchVTF::setupMaterialToUseVTF( TextureType textureType, MaterialPtr &material )
	{
		Material::TechniqueIterator techItor = material->getTechniqueIterator();
		while( techItor.hasMoreElements() )
		{
			Technique *technique = techItor.getNext();
			Technique::PassIterator passItor = technique->getPassIterator();

			while( passItor.hasMoreElements() )
			{
				bool bTexUnitFound = false;

				Pass *pass = passItor.getNext();
				Pass::TextureUnitStateIterator texUnitItor = pass->getTextureUnitStateIterator();

				while( texUnitItor.hasMoreElements() && !bTexUnitFound )
				{
					TextureUnitState *texUnit = texUnitItor.getNext();

					if( texUnit->getName() == "InstancingVTF" )
					{
						texUnit->setTextureName( mMatrixTexture->getName(), textureType );
						texUnit->setTextureFiltering( TFO_NONE );
						texUnit->setBindingType( TextureUnitState::BT_VERTEX );
					}
				}
			}

			if( !technique->getShadowCasterMaterial().isNull() )
			{
				MaterialPtr matCaster = technique->getShadowCasterMaterial();
				setupMaterialToUseVTF( textureType, matCaster );
			}
		}
	}
	//-----------------------------------------------------------------------
	void BaseInstanceBatchVTF::createVertexTexture( const SubMesh* baseSubMesh )
	{
		/*
		TODO: Find a way to retrieve max texture resolution,
		http://www.ogre3d.org/forums/viewtopic.php?t=38305

		Currently assuming it's 4096x4096, which is a safe bet for any hardware with decent VTF*/
		
		size_t uniqueAnimations = mInstancesPerBatch;
		if (useBoneMatrixLookup())
		{
			uniqueAnimations = std::min<size_t>(getMaxLookupTableInstances(), uniqueAnimations);
		}
		mMatricesPerInstance = std::max<size_t>( 1, baseSubMesh->blendIndexToBoneIndexMap.size() );

		if(mUseBoneDualQuaternions && !mTempTransformsArray3x4)
		{
			mTempTransformsArray3x4 = OGRE_ALLOC_T(float, mMatricesPerInstance * 3 * 4, MEMCATEGORY_GENERAL);
		}
		
		mNumWorldMatrices = uniqueAnimations * mMatricesPerInstance;

		//Calculate the width & height required to hold all the matrices. Start by filling the width
		//first (i.e. 4096x1 4096x2 4096x3, etc)
		
		size_t texWidth			= std::min<size_t>( mNumWorldMatrices * mRowLength, c_maxTexWidth );
		size_t maxUsableWidth	= texWidth;
		if( matricesTogetherPerRow() )
		{
			//The technique requires all matrices from the same instance in the same row
			//i.e. 4094 -> 4095 -> skip 4096 -> 0 (next row) contains data from a new instance 
			mWidthFloatsPadding = texWidth % (mMatricesPerInstance * mRowLength);

			if( mWidthFloatsPadding )
			{
				mMaxFloatsPerLine = texWidth - mWidthFloatsPadding;

				maxUsableWidth = mMaxFloatsPerLine;

				//Values are in pixels, convert them to floats (1 pixel = 4 floats)
				mWidthFloatsPadding	*= 4;
				mMaxFloatsPerLine		*= 4;
			}
		}

		size_t texHeight = mNumWorldMatrices * mRowLength / maxUsableWidth;

		if( (mNumWorldMatrices * mRowLength) % maxUsableWidth )
			texHeight += 1;

		//Don't use 1D textures, as OGL goes crazy because the shader should be calling texture1D()...
		//TextureType texType = texHeight == 1 ? TEX_TYPE_1D : TEX_TYPE_2D;
		TextureType texType = TEX_TYPE_2D;

		mMatrixTexture = TextureManager::getSingleton().createManual(
										mName + "/VTF", mMeshReference->getGroup(), texType,
										(uint)texWidth, (uint)texHeight,
										0, PF_FLOAT32_RGBA, TU_DYNAMIC_WRITE_ONLY_DISCARDABLE );

		//Set our cloned material to use this custom texture!
		setupMaterialToUseVTF( texType, mMaterial );
	}

	//-----------------------------------------------------------------------
	size_t BaseInstanceBatchVTF::convert3x4MatricesToDualQuaternions(float* matrices, size_t numOfMatrices, float* outDualQuaternions)
	{
		DualQuaternion dQuat;
		Matrix4 matrix;
		size_t floatsWritten = 0;

		for (size_t m = 0; m < numOfMatrices; ++m)
		{
			for(int i = 0; i < 3; ++i)
			{
				for(int b = 0; b < 4; ++b)
				{
					matrix[i][b] = *matrices++;
				}
			}

			matrix[3][0] = 0;
			matrix[3][1] = 0;
			matrix[3][2] = 0;
			matrix[3][3] = 1;
			
			dQuat.fromTransformationMatrix(matrix);
			
			//Copy the 2x4 matrix
			for(int i = 0; i < 8; ++i)
			{
				*outDualQuaternions++ = static_cast<float>( dQuat[i] );
				++floatsWritten;
			}
		}

		return floatsWritten;
	}
	
	//-----------------------------------------------------------------------
	void BaseInstanceBatchVTF::updateVertexTexture(void)
	{
		//Now lock the texture and copy the 4x3 matrices!
		mMatrixTexture->getBuffer()->lock( HardwareBuffer::HBL_DISCARD );
		const PixelBox &pixelBox = mMatrixTexture->getBuffer()->getCurrentLock();

		float *pDest = static_cast<float*>(pixelBox.data);

		InstancedEntityVec::const_iterator itor = mInstancedEntities.begin();
		InstancedEntityVec::const_iterator end  = mInstancedEntities.end();

		float* transforms;

		//If using dual quaternion skinning, write the transforms to a temporary buffer,
		//then convert to dual quaternions, then later write to the pixel buffer
		//Otherwise simply write the transforms to the pixel buffer directly
		if(mUseBoneDualQuaternions)
		{
			transforms = mTempTransformsArray3x4;
		}
		else
		{
			transforms = pDest;
		}

		
		while( itor != end )
		{
			size_t floatsWritten = (*itor)->getTransforms3x4( transforms );

			if( mManager->getCameraRelativeRendering() )
				makeMatrixCameraRelative3x4( transforms, floatsWritten );

			if(mUseBoneDualQuaternions)
			{
				floatsWritten = convert3x4MatricesToDualQuaternions(transforms, floatsWritten / 12, pDest);
				pDest += floatsWritten;
			}
			else
			{
				transforms += floatsWritten;
			}
			
			++itor;
		}
		
		mMatrixTexture->getBuffer()->unlock();
	}
	/** update the lookup numbers for entities with shared transforms */
	void BaseInstanceBatchVTF::updateSharedLookupIndexes()
	{
		if (mTransformSharingDirty)
		{
			if (useBoneMatrixLookup())
			{
				//In each entity update the "transform lookup number" so that:
				// 1. All entities sharing the same transformation will share the same unique number
				// 2. "transform lookup number" will be numbered from 0 up to getMaxLookupTableInstances
				size_t lookupCounter = 0;
				typedef map<Matrix4*,size_t>::type MapTransformId;
				MapTransformId transformToId;
				InstancedEntityVec::const_iterator itEnt = mInstancedEntities.begin(),
					itEntEnd = mInstancedEntities.end();
				for(;itEnt != itEntEnd ; ++itEnt)
				{
					if ((*itEnt)->isInScene())
					{
						Matrix4* transformUniqueId = (*itEnt)->mBoneMatrices;
						MapTransformId::iterator itLu = transformToId.find(transformUniqueId);
						if (itLu == transformToId.end())
						{
							itLu = transformToId.insert(MapTransformId::value_type(transformUniqueId,lookupCounter)).first;
							++lookupCounter;
						}
						(*itEnt)->setTransformLookupNumber(itLu->second);
					}
					else 
					{
						(*itEnt)->setTransformLookupNumber(0);
					}
				}

				if (lookupCounter > getMaxLookupTableInstances())
				{
					OGRE_EXCEPT(Exception::ERR_INVALID_STATE,"Number of unique bone matrix states exceeds current limitation.","BaseInstanceBatchVTF::updateSharedLookupIndexes()");
				}
			}

			mTransformSharingDirty = false;
		}
	}

	//-----------------------------------------------------------------------
	InstancedEntity* BaseInstanceBatchVTF::generateInstancedEntity(size_t num)
	{
		InstancedEntity* sharedTransformEntity = NULL;
		if ((useBoneMatrixLookup()) && (num >= getMaxLookupTableInstances()))
		{
			sharedTransformEntity = mInstancedEntities[num % getMaxLookupTableInstances()];
			if (sharedTransformEntity->mSharedTransformEntity)
			{
				sharedTransformEntity = sharedTransformEntity->mSharedTransformEntity;
			}
		}

		return OGRE_NEW InstancedEntity(this, static_cast<uint32>(num), sharedTransformEntity);
	}


	//-----------------------------------------------------------------------
	void BaseInstanceBatchVTF::getWorldTransforms( Matrix4* xform ) const
	{
		*xform = Matrix4::IDENTITY;
	}
	//-----------------------------------------------------------------------
	unsigned short BaseInstanceBatchVTF::getNumWorldTransforms(void) const
	{
		return 1;
	}
	//-----------------------------------------------------------------------
	void BaseInstanceBatchVTF::_updateRenderQueue(RenderQueue* queue)
	{
		InstanceBatch::_updateRenderQueue( queue );

		if( mBoundsUpdated || mDirtyAnimation || mManager->getCameraRelativeRendering() )
			updateVertexTexture();

		mBoundsUpdated = false;
	}
	//-----------------------------------------------------------------------
	// InstanceBatchVTF
	//-----------------------------------------------------------------------
	InstanceBatchVTF::InstanceBatchVTF( 
		InstanceManager *creator, MeshPtr &meshReference, 
		const MaterialPtr &material, size_t instancesPerBatch, 
		const Mesh::IndexMap *indexToBoneMap, const String &batchName )
			: BaseInstanceBatchVTF (creator, meshReference, material, 
									instancesPerBatch, indexToBoneMap, batchName)
	{

	}
	//-----------------------------------------------------------------------
	InstanceBatchVTF::~InstanceBatchVTF()
	{
	}	
	//-----------------------------------------------------------------------
	void InstanceBatchVTF::setupVertices( const SubMesh* baseSubMesh )
	{
		mRenderOperation.vertexData = OGRE_NEW VertexData();
		mRemoveOwnVertexData = true; //Raise flag to remove our own vertex data in the end (not always needed)

		VertexData *thisVertexData = mRenderOperation.vertexData;
		VertexData *baseVertexData = baseSubMesh->vertexData;

		thisVertexData->vertexStart = 0;
		thisVertexData->vertexCount = baseVertexData->vertexCount * mInstancesPerBatch;

		HardwareBufferManager::getSingleton().destroyVertexDeclaration( thisVertexData->vertexDeclaration );
		thisVertexData->vertexDeclaration = baseVertexData->vertexDeclaration->clone();

		HWBoneIdxVec hwBoneIdx;
		HWBoneWgtVec hwBoneWgt;

		//Blend weights may not be present because HW_VTF does not require to be skeletally animated
		const VertexElement *veWeights = baseVertexData->vertexDeclaration->
													findElementBySemantic( VES_BLEND_WEIGHTS );
		if( veWeights )
		{
			//One weight is recommended for VTF
			mWeightCount = (forceOneWeight() || useOneWeight()) ?
								1 : veWeights->getSize() / sizeof(float);
		}
		else
		{
			mWeightCount = 1;
		}

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
				thisVertexData->vertexDeclaration->removeElement( VES_BLEND_INDICES );
				thisVertexData->vertexDeclaration->removeElement( VES_BLEND_WEIGHTS );

				thisVertexData->vertexDeclaration->closeGapsInSource();
			}

		}

		for( unsigned short i=0; i<thisVertexData->vertexDeclaration->getMaxSource()+1; ++i )
		{
			//Create our own vertex buffer
			HardwareVertexBufferSharedPtr vertexBuffer =
				HardwareBufferManager::getSingleton().createVertexBuffer(
				thisVertexData->vertexDeclaration->getVertexSize(i),
				thisVertexData->vertexCount,
				HardwareBuffer::HBU_STATIC_WRITE_ONLY );
			thisVertexData->vertexBufferBinding->setBinding( i, vertexBuffer );

			//Grab the base submesh data
			HardwareVertexBufferSharedPtr baseVertexBuffer =
				baseVertexData->vertexBufferBinding->getBuffer(i);

			char* thisBuf = static_cast<char*>(vertexBuffer->lock(HardwareBuffer::HBL_DISCARD));
			char* baseBuf = static_cast<char*>(baseVertexBuffer->lock(HardwareBuffer::HBL_READ_ONLY));

			//Copy and repeat
			for( size_t j=0; j<mInstancesPerBatch; ++j )
			{
				const size_t sizeOfBuffer = baseVertexData->vertexCount *
					baseVertexData->vertexDeclaration->getVertexSize(i);
				memcpy( thisBuf + j * sizeOfBuffer, baseBuf, sizeOfBuffer );
			}

			baseVertexBuffer->unlock();
			vertexBuffer->unlock();
		}

		createVertexTexture( baseSubMesh );
		createVertexSemantics( thisVertexData, baseVertexData, hwBoneIdx, hwBoneWgt);
	}
	//-----------------------------------------------------------------------
	void InstanceBatchVTF::setupIndices( const SubMesh* baseSubMesh )
	{
		mRenderOperation.indexData = OGRE_NEW IndexData();
		mRemoveOwnIndexData = true;	//Raise flag to remove our own index data in the end (not always needed)

		IndexData *thisIndexData = mRenderOperation.indexData;
		IndexData *baseIndexData = baseSubMesh->indexData;

		thisIndexData->indexStart = 0;
		thisIndexData->indexCount = baseIndexData->indexCount * mInstancesPerBatch;

		//TODO: Check numVertices is below max supported by GPU
		HardwareIndexBuffer::IndexType indexType = HardwareIndexBuffer::IT_16BIT;
		if( mRenderOperation.vertexData->vertexCount > 65535 )
			indexType = HardwareIndexBuffer::IT_32BIT;
		thisIndexData->indexBuffer = HardwareBufferManager::getSingleton().createIndexBuffer(
			indexType, thisIndexData->indexCount,
			HardwareBuffer::HBU_STATIC_WRITE_ONLY );

		void *buf			= thisIndexData->indexBuffer->lock( HardwareBuffer::HBL_DISCARD );
		void const *baseBuf	= baseIndexData->indexBuffer->lock( HardwareBuffer::HBL_READ_ONLY );

		uint16 *thisBuf16 = static_cast<uint16*>(buf);
		uint32 *thisBuf32 = static_cast<uint32*>(buf);

		for( size_t i=0; i<mInstancesPerBatch; ++i )
		{
			const size_t vertexOffset = i * mRenderOperation.vertexData->vertexCount / mInstancesPerBatch;

			uint16 const *initBuf16 = static_cast<uint16 const *>(baseBuf);
			uint32 const *initBuf32 = static_cast<uint32 const *>(baseBuf);

			for( size_t j=0; j<baseIndexData->indexCount; ++j )
			{
				uint32 originalVal;
				if( baseSubMesh->indexData->indexBuffer->getType() == HardwareIndexBuffer::IT_16BIT )
					originalVal = *initBuf16++;
				else
					originalVal = *initBuf32++;

				if( indexType == HardwareIndexBuffer::IT_16BIT )
					*thisBuf16++ = static_cast<uint16>(originalVal) + vertexOffset;
				else
					*thisBuf32++ = static_cast<uint32>(originalVal + vertexOffset);
			}
		}

		baseIndexData->indexBuffer->unlock();
		thisIndexData->indexBuffer->unlock();
	}
	//-----------------------------------------------------------------------
	void InstanceBatchVTF::createVertexSemantics( 
		VertexData *thisVertexData, VertexData *baseVertexData, const HWBoneIdxVec &hwBoneIdx, const HWBoneWgtVec &hwBoneWgt)
	{
		const size_t texWidth  = mMatrixTexture->getWidth();
		const size_t texHeight = mMatrixTexture->getHeight();

		//Calculate the texel offsets to correct them offline
		//Akwardly enough, the offset is needed in OpenGL too
		Vector2 texelOffsets;
		//RenderSystem *renderSystem = Root::getSingleton().getRenderSystem();
		texelOffsets.x = /*renderSystem->getHorizontalTexelOffset()*/ -0.5f / (float)texWidth;
		texelOffsets.y = /*renderSystem->getVerticalTexelOffset()*/ -0.5f / (float)texHeight;

		//Only one weight per vertex is supported. It would not only be complex, but prohibitively slow.
		//Put them in a new buffer, since it's 32 bytes aligned :-)
		const unsigned short newSource = thisVertexData->vertexDeclaration->getMaxSource() + 1;
		size_t maxFloatsPerVector = 4;
		size_t offset = 0;

		for(size_t i = 0; i < mWeightCount; i += maxFloatsPerVector / mRowLength)
		{
			offset += thisVertexData->vertexDeclaration->addElement( newSource, offset, VET_FLOAT4, VES_TEXTURE_COORDINATES,
				thisVertexData->vertexDeclaration->
				getNextFreeTextureCoordinate() ).getSize();
			offset += thisVertexData->vertexDeclaration->addElement( newSource, offset, VET_FLOAT4, VES_TEXTURE_COORDINATES,
				thisVertexData->vertexDeclaration->
				getNextFreeTextureCoordinate() ).getSize();
		}

		//Add the weights (supports up to four, which is Ogre's limit)
		if(mWeightCount > 1)
		{
			thisVertexData->vertexDeclaration->addElement(newSource, offset, VET_FLOAT4, VES_BLEND_WEIGHTS,
										thisVertexData->vertexDeclaration->getNextFreeTextureCoordinate() ).getSize();
		}

		//Create our own vertex buffer
		HardwareVertexBufferSharedPtr vertexBuffer =
			HardwareBufferManager::getSingleton().createVertexBuffer(
			thisVertexData->vertexDeclaration->getVertexSize(newSource),
			thisVertexData->vertexCount,
			HardwareBuffer::HBU_STATIC_WRITE_ONLY );
		thisVertexData->vertexBufferBinding->setBinding( newSource, vertexBuffer );

		float *thisFloat = static_cast<float*>(vertexBuffer->lock(HardwareBuffer::HBL_DISCARD));
		
		//Copy and repeat
		for( size_t i=0; i<mInstancesPerBatch; ++i )
		{
			for( size_t j=0; j<baseVertexData->vertexCount * mWeightCount; j += mWeightCount )
			{
				size_t numberOfMatricesInLine = 0;

				for(size_t wgtIdx = 0; wgtIdx < mWeightCount; ++wgtIdx)
				{
					for( size_t k=0; k < mRowLength; ++k)
					{
						size_t instanceIdx = (hwBoneIdx[j+wgtIdx] + i * mMatricesPerInstance) * mRowLength + k;
						//x
						*thisFloat++ = ((instanceIdx % texWidth) / (float)texWidth) - (float)texelOffsets.x;
						//y
						*thisFloat++ = ((instanceIdx / texWidth) / (float)texHeight) - (float)texelOffsets.y;
					}

					++numberOfMatricesInLine;

					//If another matrix can't be fit, we're on another line, or if this is the last weight
					if((numberOfMatricesInLine + 1) * mRowLength > maxFloatsPerVector || (wgtIdx+1) == mWeightCount)
					{
						//Place zeroes in the remaining coordinates
						for ( size_t k=mRowLength * numberOfMatricesInLine; k < maxFloatsPerVector; ++k)
						{
							*thisFloat++ = 0.0f;
							*thisFloat++ = 0.0f;
						}

						numberOfMatricesInLine = 0;
					}
				}

				//Don't need to write weights if there is only one
				if(mWeightCount > 1)
				{
					//Write the weights
					for(size_t wgtIdx = 0; wgtIdx < mWeightCount; ++wgtIdx)
					{
						*thisFloat++ = hwBoneWgt[j+wgtIdx];
					}

					//Fill the rest of the line with zeros
					for(size_t wgtIdx = mWeightCount; wgtIdx < maxFloatsPerVector; ++wgtIdx)
					{
						*thisFloat++ = 0.0f;
					}
				}
			}
		}

		vertexBuffer->unlock();

	}
	//-----------------------------------------------------------------------
	size_t InstanceBatchVTF::calculateMaxNumInstances( 
					const SubMesh *baseSubMesh, uint16 flags ) const
	{
		size_t retVal = 0;

		RenderSystem *renderSystem = Root::getSingleton().getRenderSystem();
		const RenderSystemCapabilities *capabilities = renderSystem->getCapabilities();

		//VTF must be supported
		if( capabilities->hasCapability( RSC_VERTEX_TEXTURE_FETCH ) )
		{
			//TODO: Check PF_FLOAT32_RGBA is supported (should be, since it was the 1st one)
			const size_t numBones = std::max<size_t>( 1, baseSubMesh->blendIndexToBoneIndexMap.size() );
			retVal = c_maxTexWidth * c_maxTexHeight / mRowLength / numBones;

			if( flags & IM_USE16BIT )
			{
				if( baseSubMesh->vertexData->vertexCount * retVal > 0xFFFF )
					retVal = 0xFFFF / baseSubMesh->vertexData->vertexCount;
			}

			if( flags & IM_VTFBESTFIT )
			{
				const size_t instancesPerBatch = std::min( retVal, mInstancesPerBatch );
				//Do the same as in createVertexTexture()
				const size_t numWorldMatrices = instancesPerBatch * numBones;

				size_t texWidth  = std::min<size_t>( numWorldMatrices * mRowLength, c_maxTexWidth );
				size_t texHeight = numWorldMatrices * mRowLength / c_maxTexWidth;

				const size_t remainder = (numWorldMatrices * mRowLength) % c_maxTexWidth;

				if( remainder && texHeight > 0 )
					retVal = static_cast<size_t>(texWidth * texHeight / (float)mRowLength / (float)(numBones));
			}
		}

		return retVal;

	}
}
