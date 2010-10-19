/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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
#include "OgreInstanceManager.h"
#include "OgreInstanceBatchShader.h"
#include "OgreInstanceBatchVTF.h"
#include "OgreMesh.h"
#include "OgreSubMesh.h"
#include "OgreMeshManager.h"
#include "OgreMaterialManager.h"
#include "OgreSceneManager.h"

namespace Ogre
{
	InstanceManager::InstanceManager( const String &customName, SceneManager *sceneManager,
										const String &meshName, const String &groupName,
										InstancingTechnique instancingTechnique, uint16 instancingFlags,
										size_t instancesPerBatch ) :
				m_name( customName ),
				m_idCount( 0 ),
				m_sceneManager( sceneManager ),
				m_instancesPerBatch( instancesPerBatch ),
				m_instancingTechnique( instancingTechnique ),
				m_instancingFlags( instancingFlags ),
				m_showBoundingBoxes( false )
	{
		m_meshReference = MeshManager::getSingleton().load( meshName, groupName );

		if( m_meshReference->hasSkeleton() && !m_meshReference->getSkeleton().isNull() )
			m_meshReference->getSubMesh(0)->_compileBoneAssignments();
	}

	InstanceManager::~InstanceManager()
	{
		//Remove all batches from all materials we created
		InstanceBatchMap::const_iterator itor = m_instanceBatches.begin();
		InstanceBatchMap::const_iterator end  = m_instanceBatches.end();

		while( itor != end )
		{
			InstanceBatchVec::const_iterator it = itor->second.begin();
			InstanceBatchVec::const_iterator en = itor->second.end();

			while( it != en )
				OGRE_DELETE *it++;

			++itor;
		}

		//Free the shared RenderOperation since it's our responsability
		if( m_sharedRenderOperation.vertexData )
			OGRE_DELETE m_sharedRenderOperation.vertexData;
		if( m_sharedRenderOperation.indexData )
			OGRE_DELETE m_sharedRenderOperation.indexData;
	}
	//----------------------------------------------------------------------
	void InstanceManager::setInstancesPerBatch( size_t instancesPerBatch )
	{
		if( !m_instanceBatches.empty() )
		{
			OGRE_EXCEPT(Exception::ERR_INVALID_STATE, "Instances per batch can only be changed before"
						" building the batch.", "InstanceManager::setInstancesPerBatch");
		}

		m_instancesPerBatch = instancesPerBatch;
	}
	//----------------------------------------------------------------------
	size_t InstanceManager::getMaxOrBestNumInstancesPerBatch( String materialName, size_t suggestedSize,
																uint16 flags )
	{
		//Get the material
		MaterialPtr mat = MaterialManager::getSingleton().getByName( materialName,
																	m_meshReference->getGroup() );
		InstanceBatch *batch = 0;

		//Base material couldn't be found
		if( mat.isNull() )
			return 0;

		switch( m_instancingTechnique )
		{
		case ShaderBased:
			batch = OGRE_NEW InstanceBatchShader( this, m_meshReference, mat, suggestedSize,
													0, m_name + "/TempBatch" );
			break;
		case TextureVTF:
			batch = OGRE_NEW ClonedGeometryInstanceBatchVTF( this, m_meshReference, mat, suggestedSize,
													0, m_name + "/TempBatch" );
			break;
		case HardwareInstancing:
			batch = OGRE_NEW HardwareInstanceBatchVFT( this, m_meshReference, mat, suggestedSize,
													0, m_name + "/TempBatch" );
			break;
		default:
			OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
					"Unimplemented instancing technique: " +
					StringConverter::toString(m_instancingTechnique),
					"InstanceBatch::getMaxOrBestNumInstancesPerBatches()");
		}

		const size_t retVal = batch->calculateMaxNumInstances( m_meshReference->getSubMesh(0), flags );

		OGRE_DELETE batch;

		return retVal;
	}
	//----------------------------------------------------------------------
	InstancedEntity* InstanceManager::createInstancedEntity( const String &materialName )
	{
		InstanceBatch *instanceBatch;

		if( m_instanceBatches.empty() )
			instanceBatch = buildNewBatch( materialName, true );
		else
			instanceBatch = getFreeBatch( materialName );

		return instanceBatch->createInstancedEntity();
	}
	//-----------------------------------------------------------------------
	inline InstanceBatch* InstanceManager::getFreeBatch( const String &materialName )
	{
		InstanceBatchVec &batchVec = m_instanceBatches[materialName];

		InstanceBatchVec::const_reverse_iterator itor = batchVec.rbegin();
		InstanceBatchVec::const_reverse_iterator end  = batchVec.rend();

		while( itor != end )
		{
			if( !(*itor)->isBatchFull() )
				return *itor;
			++itor;
		}

		//None found, or they're all full
		return buildNewBatch( materialName, false );
	}
	//-----------------------------------------------------------------------
	InstanceBatch* InstanceManager::buildNewBatch( const String &materialName, bool firstTime )
	{
		//Get the bone to index map for the batches
		Mesh::IndexMap &idxMap = m_meshReference->getSubMesh(0)->blendIndexToBoneIndexMap;
		idxMap = idxMap.empty() ? m_meshReference->sharedBlendIndexToBoneIndexMap : idxMap;

		//Get the material
		MaterialPtr mat = MaterialManager::getSingleton().getByName( materialName,
																	m_meshReference->getGroup() );

		//Get the array of batches grouped by this material
		InstanceBatchVec &materialInstanceBatch = m_instanceBatches[materialName];

		InstanceBatch *batch = 0;

		switch( m_instancingTechnique )
		{
		case ShaderBased:
			batch = OGRE_NEW InstanceBatchShader( this, m_meshReference, mat, m_instancesPerBatch,
													&idxMap, m_name + "/InstanceBatch_" +
													StringConverter::toString(m_idCount++) );
			break;
		case TextureVTF:
			batch = OGRE_NEW ClonedGeometryInstanceBatchVTF( this, m_meshReference, mat, m_instancesPerBatch,
													&idxMap, m_name + "/InstanceBatch_" +
													StringConverter::toString(m_idCount++) );
			break;
		case HardwareInstancing:
			batch = OGRE_NEW HardwareInstanceBatchVFT( this, m_meshReference, mat, m_instancesPerBatch,
													&idxMap, m_name + "/InstanceBatch_" +
													StringConverter::toString(m_idCount++) );
			break;
		default:
			OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
					"Unimplemented instancing technique: " +
					StringConverter::toString(m_instancingTechnique),
					"InstanceBatch::buildNewBatch()");
		}


		if( !firstTime )
		{
			//TODO: Check different materials have the same m_instancesPerBatch upper limit
			//otherwise we can't share
			batch->buildFrom( m_meshReference->getSubMesh(0), m_sharedRenderOperation );
		}
		else
		{
			//Ensure we don't request more than we can
			m_instancesPerBatch = std::min(batch->calculateMaxNumInstances( m_meshReference->getSubMesh(0),
																			m_instancingFlags ),
																			m_instancesPerBatch );
			batch->_setInstancesPerBatch( m_instancesPerBatch );

			//TODO: Create a "merge" function that merges all submeshes into one big submesh
			//instead of just sending submesh #0

			//Get the RenderOperation to be shared with further instances.
			m_sharedRenderOperation = batch->build( m_meshReference->getSubMesh(0) );
		}

		//Batches need to be part of a scene node so that their renderable can be rendered
		SceneNode *sceneNode = m_sceneManager->getRootSceneNode()->createChildSceneNode();
		sceneNode->attachObject( batch );
		sceneNode->showBoundingBox( m_showBoundingBoxes );

		materialInstanceBatch.push_back( batch );

		return batch;
	}
	//-----------------------------------------------------------------------
	void InstanceManager::cleanupEmptyBatches(void)
	{
		//Do this now to avoid any dangling pointer inside m_dirtyBatches
		_updateDirtyBatches();

		InstanceBatchMap::iterator itor = m_instanceBatches.begin();
		InstanceBatchMap::iterator end  = m_instanceBatches.end();

		while( itor != end )
		{
			InstanceBatchVec::iterator it = itor->second.begin();
			InstanceBatchVec::iterator en = itor->second.end();

			while( it != en )
			{
				if( (*it)->isBatchUnused() )
				{
					OGRE_DELETE *it;
					//Remove it from the list swapping with the last element and popping back
					size_t idx = it - itor->second.begin();
					*it = itor->second.back();
					itor->second.pop_back();

					//Restore invalidated iterators
					it = itor->second.begin() + idx;
					en = itor->second.end();
				}
				else
					++it;
			}

			++itor;
		}

		//By this point it may happen that all m_instanceBatches' objects are also empty
		//however if we call m_instanceBatches.clear(), next time we'll create an InstancedObject
		//we'll end up calling buildFirstTime() instead of buildNewBatch(), which is not the idea
		//(takes more time and will leak the shared render operation)
	}
	//-----------------------------------------------------------------------
	void InstanceManager::defragmentBatches( bool optimizeCull,
												InstanceBatch::InstancedEntityVec &usedEntities,
												InstanceBatchVec &fragmentedBatches )
	{
		InstanceBatchVec::const_iterator itor = fragmentedBatches.begin();
		InstanceBatchVec::const_iterator end  = fragmentedBatches.end();

		while( itor != end && !usedEntities.empty() )
		{
			(*itor)->_defragmentBatch( optimizeCull, usedEntities );
			++itor;
		}

		const size_t remainingBatches = end - itor;

		while( itor != end )
		{
			//If we get here, this means we hit remaining batches which will be unused.
			//Destroy them
			//Call this to avoid freeing InstancedEntities that were just reparented
			(*itor)->_defragmentBatchDiscard();
			OGRE_DELETE *itor;
			++itor;
		}

		//Remove remaining batches all at once from the vector
		fragmentedBatches.resize( fragmentedBatches.size() - remainingBatches );
	}
	//-----------------------------------------------------------------------
	void InstanceManager::defragmentBatches( bool optimizeCulling )
	{
		//Do this now to avoid any dangling pointer inside m_dirtyBatches
		_updateDirtyBatches();

		//Do this for every material
		InstanceBatchMap::iterator itor = m_instanceBatches.begin();
		InstanceBatchMap::iterator end  = m_instanceBatches.end();

		while( itor != end )
		{
			InstanceBatch::InstancedEntityVec usedEntities;
			usedEntities.reserve( itor->second.size() * m_instancesPerBatch );

			//Collect all Instanced Entities being used by _all_ batches from this material
			InstanceBatchVec::iterator it = itor->second.begin();
			InstanceBatchVec::iterator en = itor->second.end();

			while( it != en )
			{
				(*it)->getInstancedEntitiesInUse( usedEntities );
				++it;
			}

			defragmentBatches( optimizeCulling, usedEntities, itor->second );

			++itor;
		}
	}
	//-----------------------------------------------------------------------
	void InstanceManager::showBoundingBoxes( bool bShow )
	{
		m_showBoundingBoxes = bShow;

		InstanceBatchMap::iterator itor = m_instanceBatches.begin();
		InstanceBatchMap::iterator end  = m_instanceBatches.end();

		while( itor != end )
		{
			InstanceBatchVec::iterator it = itor->second.begin();
			InstanceBatchVec::iterator en = itor->second.end();

			while( it != en )
			{
				(*it)->getParentSceneNode()->showBoundingBox( bShow );
				++it;
			}

			++itor;
		}
	}
	//-----------------------------------------------------------------------
	void InstanceManager::_addDirtyBatch( InstanceBatch *dirtyBatch )
	{
		if( m_dirtyBatches.empty() )
			m_sceneManager->_addDirtyInstanceManager( this );

		m_dirtyBatches.push_back( dirtyBatch );
	}
	//-----------------------------------------------------------------------
	void InstanceManager::_updateDirtyBatches(void)
	{
		InstanceBatchVec::const_iterator itor = m_dirtyBatches.begin();
		InstanceBatchVec::const_iterator end  = m_dirtyBatches.end();

		while( itor != end )
		{
			(*itor)->_updateBounds();
			++itor;
		}

		m_dirtyBatches.clear();
	}
}
