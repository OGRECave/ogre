/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2011 Torus Knot Software Ltd

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
	static const uint16 c_maxTexWidth	= 4096;
	static const uint16 c_maxTexHeight	= 4096;

	InstanceBatchHW_VTF::InstanceBatchHW_VTF( 
		InstanceManager *creator, MeshPtr &meshReference, 
		const MaterialPtr &material, size_t instancesPerBatch, 
		const Mesh::IndexMap *indexToBoneMap, const String &batchName )
			: BaseInstanceBatchVTF( creator, meshReference, material, 
									instancesPerBatch, indexToBoneMap, batchName ),
			  m_keepStatic( false )
	{

	}
	//-----------------------------------------------------------------------
	InstanceBatchHW_VTF::~InstanceBatchHW_VTF()
	{

	}	
	//-----------------------------------------------------------------------
	void InstanceBatchHW_VTF::setupVertices( const SubMesh* baseSubMesh )
	{
		m_renderOperation.vertexData = OGRE_NEW VertexData();

		VertexData *thisVertexData = m_renderOperation.vertexData;
		VertexData *baseVertexData = baseSubMesh->vertexData;

		thisVertexData->vertexStart = 0;
		thisVertexData->vertexCount = baseVertexData->vertexCount;
		m_renderOperation.numberOfInstances = m_instancesPerBatch;

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
		hwBoneIdx.resize( baseVertexData->vertexCount, 0 );
		if( m_meshReference->hasSkeleton() && !m_meshReference->getSkeleton().isNull() )
		{
			retrieveBoneIdx( baseVertexData, hwBoneIdx );

			thisVertexData->vertexDeclaration->removeElement( VES_BLEND_INDICES );
			thisVertexData->vertexDeclaration->removeElement( VES_BLEND_WEIGHTS );
			thisVertexData->vertexDeclaration->closeGapsInSource();
		}

		createVertexTexture( baseSubMesh );
		createVertexSemantics( thisVertexData, baseVertexData, hwBoneIdx );
	}
	//-----------------------------------------------------------------------
	void InstanceBatchHW_VTF::setupIndices( const SubMesh* baseSubMesh )
	{
		//We could use just a reference, but the InstanceManager will in the end attampt to delete
		//the pointer, and we can't give it something that doesn't belong to us.
		m_renderOperation.indexData = baseSubMesh->indexData->clone( true );
	}
	//-----------------------------------------------------------------------
	void InstanceBatchHW_VTF::createVertexSemantics( VertexData *thisVertexData,
														 VertexData *baseVertexData,
														 const HWBoneIdxVec &hwBoneIdx )
	{
		const float texWidth  = static_cast<float>(m_matrixTexture->getWidth());
		const float texHeight = static_cast<float>(m_matrixTexture->getHeight());

		//Calculate the texel offsets to correct them offline
		//Akwardly enough, the offset is needed in OpenGL too
		Vector2 texelOffsets;
		//RenderSystem *renderSystem = Root::getSingleton().getRenderSystem();
		texelOffsets.x = /*renderSystem->getHorizontalTexelOffset()*/ -0.5f / texWidth;
		texelOffsets.y = /*renderSystem->getHorizontalTexelOffset()*/ -0.5f / texHeight;

		//Only one weight per vertex is supported. It would not only be complex, but prohibitively slow.
		//Put them in a new buffer, since it's 16 bytes aligned :-)
		unsigned short newSource = thisVertexData->vertexDeclaration->getMaxSource() + 1;
		thisVertexData->vertexDeclaration->addElement( newSource, 0, VET_FLOAT4, VES_TEXTURE_COORDINATES,
									thisVertexData->vertexDeclaration->getNextFreeTextureCoordinate() );

		//Create our own vertex buffer
		HardwareVertexBufferSharedPtr vertexBuffer =
			HardwareBufferManager::getSingleton().createVertexBuffer(
			thisVertexData->vertexDeclaration->getVertexSize(newSource),
			thisVertexData->vertexCount,
			HardwareBuffer::HBU_STATIC_WRITE_ONLY );
		thisVertexData->vertexBufferBinding->setBinding( newSource, vertexBuffer );

		float *thisFloat = static_cast<float*>(vertexBuffer->lock(HardwareBuffer::HBL_DISCARD));

		//Create the UVs to sample from the right bone/matrix
		for( size_t j=0; j<baseVertexData->vertexCount; ++j )
		{
			for( size_t k=0; k<3; ++k )
			{
				//Only calculate U (not V) since all matrices are in the same row. We use the instanced
				//(repeated) buffer to tell how much U & V we need to offset
				size_t instanceIdx = hwBoneIdx[j] * 3 + k;
				*thisFloat++ = instanceIdx / texWidth;
			}

			//Put a zero in the 4th U coordinate (it's not really used, but it's handy)
			*thisFloat++ = 0;
		}

		vertexBuffer->unlock();

		const size_t numBones = m_numWorldMatrices / m_instancesPerBatch;
		//Now create the instance buffer that will be incremented per instance, contains UV offsets
		newSource = thisVertexData->vertexDeclaration->getMaxSource() + 1;
		thisVertexData->vertexDeclaration->addElement( newSource, 0, VET_FLOAT2, VES_TEXTURE_COORDINATES,
									thisVertexData->vertexDeclaration->getNextFreeTextureCoordinate() );

		//Create our own vertex buffer
		vertexBuffer = HardwareBufferManager::getSingleton().createVertexBuffer(
										thisVertexData->vertexDeclaration->getVertexSize(newSource),
										m_instancesPerBatch,
										HardwareBuffer::HBU_STATIC_WRITE_ONLY );
		thisVertexData->vertexBufferBinding->setBinding( newSource, vertexBuffer );

		//Mark this buffer as instanced
		vertexBuffer->setIsInstanceData( true );
		vertexBuffer->setInstanceDataStepRate( 1 );

		Vector2 *thisVec = static_cast<Vector2*>(vertexBuffer->lock(HardwareBuffer::HBL_DISCARD));

		const size_t maxPixelsPerLine = std::min( m_matrixTexture->getWidth(), m_maxFloatsPerLine >> 2 );

		//Calculate UV offsets, which change per instance
		for( size_t i=0; i<m_instancesPerBatch; ++i )
		{
			size_t instanceIdx = i * numBones * 3;
			thisVec->x = (instanceIdx % maxPixelsPerLine) / texWidth;
			thisVec->y = (instanceIdx / maxPixelsPerLine) / texHeight;
			*thisVec = *thisVec - texelOffsets;
			++thisVec;
		}

		vertexBuffer->unlock();
	}
	//-----------------------------------------------------------------------
	bool InstanceBatchHW_VTF::checkSubMeshCompatibility( const SubMesh* baseSubMesh )
	{
		//Max number of texture coordinates is _usually_ 8, we need at least 2 available
		if( baseSubMesh->vertexData->vertexDeclaration->getNextFreeTextureCoordinate() > 8-1 )
		{
			OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Given mesh must have at "
														"least 2 free TEXCOORDs",
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

			const size_t maxUsableWidth = c_maxTexWidth - (c_maxTexWidth % (numBones * 3));

			//See InstanceBatchHW::calculateMaxNumInstances for the 65535
			retVal = std::min<size_t>( 65535, maxUsableWidth * c_maxTexHeight / 3 / numBones );

			if( flags & IM_VTFBESTFIT )
			{
				const size_t instancesPerBatch = std::min( retVal, m_instancesPerBatch );
				//Do the same as in createVertexTexture(), but changing c_maxTexWidth for maxUsableWidth
				const size_t numWorldMatrices = instancesPerBatch * numBones;

				size_t texWidth  = std::min<size_t>( numWorldMatrices * 3, maxUsableWidth );
				size_t texHeight = numWorldMatrices * 3 / maxUsableWidth;

				const size_t remainder = (numWorldMatrices * 3) % maxUsableWidth;

				if( remainder && texHeight > 0 )
					retVal = static_cast<size_t>(texWidth * texHeight / 3.0f / (float)(numBones));
			}
		}

		return retVal;
	}
	//-----------------------------------------------------------------------
	size_t InstanceBatchHW_VTF::updateVertexTexture( Camera *currentCamera )
	{
		size_t retVal = 0;

		m_dirtyAnimation = false;

		//Now lock the texture and copy the 4x3 matrices!
		m_matrixTexture->getBuffer()->lock( HardwareBuffer::HBL_DISCARD );
		const PixelBox &pixelBox = m_matrixTexture->getBuffer()->getCurrentLock();

		float *pDest = static_cast<float*>(pixelBox.data);

		InstancedEntityVec::const_iterator itor = m_instancedEntities.begin();
		InstancedEntityVec::const_iterator end  = m_instancedEntities.end();

		size_t currentPixel = 0; //Resets on each line

		while( itor != end )
		{
			//Cull on an individual basis, the less entities are visible, the less instances we draw.
			//No need to use null matrices at all!
			if( (*itor)->findVisible( currentCamera ) )
			{
				if( m_meshReference->hasSkeleton() )
					m_dirtyAnimation |= (*itor)->_updateAnimation();

				const size_t floatsWritten = (*itor)->getTransforms3x4( pDest );

				pDest		 += floatsWritten;
				currentPixel += floatsWritten;

				if( currentPixel >= m_maxFloatsPerLine )
				{
					currentPixel = 0;
					pDest       += m_widthFloatsPadding;
				}

				++retVal;
			}

			++itor;
		}

		m_matrixTexture->getBuffer()->unlock();

		return retVal;
	}
	//-----------------------------------------------------------------------
	void InstanceBatchHW_VTF::_boundsDirty(void)
	{
		//Don't update if we're static, but still mark we're dirty
		if( !m_boundsDirty && !m_keepStatic )
			m_creator->_addDirtyBatch( this );
		m_boundsDirty = true;
	}
	//-----------------------------------------------------------------------
	void InstanceBatchHW_VTF::setStaticAndUpdate( bool bStatic )
	{
		//We were dirty but didn't update bounds. Do it now.
		if( m_keepStatic && m_boundsDirty )
			m_creator->_addDirtyBatch( this );

		m_keepStatic = bStatic;
		if( m_keepStatic )
		{
			//One final update, since there will be none from now on
			//(except further calls to this function). Pass NULL because
			//we want to include only those who were added to the scene
			//but we don't want to perform culling
			m_renderOperation.numberOfInstances = updateVertexTexture( 0 );
		}
	}
	//-----------------------------------------------------------------------
	void InstanceBatchHW_VTF::_updateRenderQueue( RenderQueue* queue )
	{
		if( !m_keepStatic )
		{
			//Completely override base functionality, since we don't cull on an "all-or-nothing" basis
			if( (m_renderOperation.numberOfInstances = updateVertexTexture( m_currentCamera )) )
				queue->addRenderable( this );
		}
		else
		{
			//Don't update when we're static
			if( m_renderOperation.numberOfInstances )
				queue->addRenderable( this );
		}
	}
}
