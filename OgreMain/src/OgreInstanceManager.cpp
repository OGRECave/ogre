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
#include "OgreInstanceManager.h"
#include "OgreInstanceBatchHW.h"
#include "OgreInstanceBatchHW_VTF.h"
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
										size_t instancesPerBatch, unsigned short subMeshIdx ) :
				m_name( customName ),
				m_idCount( 0 ),
				m_instancesPerBatch( instancesPerBatch ),
				m_instancingTechnique( instancingTechnique ),
				m_instancingFlags( instancingFlags ),
				m_subMeshIdx( subMeshIdx ),
                m_sceneManager( sceneManager )
	{
		m_meshReference = MeshManager::getSingleton().load( meshName, groupName );

		if( m_meshReference->hasSkeleton() && !m_meshReference->getSkeleton().isNull() )
			m_meshReference->getSubMesh(m_subMeshIdx)->_compileBoneAssignments();
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

		//Free the shared RenderOperation since it's our responsibility
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
			batch = OGRE_NEW InstanceBatchVTF( this, m_meshReference, mat, suggestedSize,
													0, m_name + "/TempBatch" );
			break;
		case HWInstancingBasic:
			batch = OGRE_NEW InstanceBatchHW( this, m_meshReference, mat, suggestedSize,
													0, m_name + "/TempBatch" );
			break;
		case HWInstancingVTF:
			batch = OGRE_NEW InstanceBatchHW_VTF( this, m_meshReference, mat, suggestedSize,
													0, m_name + "/TempBatch" );
			break;
		default:
			OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
					"Unimplemented instancing technique: " +
					StringConverter::toString(m_instancingTechnique),
					"InstanceBatch::getMaxOrBestNumInstancesPerBatches()");
		}

		const size_t retVal = batch->calculateMaxNumInstances( m_meshReference->getSubMesh(m_subMeshIdx),
																flags );

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
		Mesh::IndexMap &idxMap = m_meshReference->getSubMesh(m_subMeshIdx)->blendIndexToBoneIndexMap;
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
			batch = OGRE_NEW InstanceBatchVTF( this, m_meshReference, mat, m_instancesPerBatch,
													&idxMap, m_name + "/InstanceBatch_" +
													StringConverter::toString(m_idCount++) );
			break;
		case HWInstancingBasic:
			batch = OGRE_NEW InstanceBatchHW( this, m_meshReference, mat, m_instancesPerBatch,
													&idxMap, m_name + "/InstanceBatch_" +
													StringConverter::toString(m_idCount++) );
			break;
		case HWInstancingVTF:
			batch = OGRE_NEW InstanceBatchHW_VTF( this, m_meshReference, mat, m_instancesPerBatch,
													&idxMap, m_name + "/InstanceBatch_" +
													StringConverter::toString(m_idCount++) );
			break;
		default:
			OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
					"Unimplemented instancing technique: " +
					StringConverter::toString(m_instancingTechnique),
					"InstanceBatch::buildNewBatch()");
		}

		batch->_notifyManager( m_sceneManager );


		if( !firstTime )
		{
			//TODO: Check different materials have the same m_instancesPerBatch upper limit
			//otherwise we can't share
			batch->buildFrom( m_meshReference->getSubMesh(m_subMeshIdx), m_sharedRenderOperation );
		}
		else
		{
			//Ensure we don't request more than we can
			const size_t maxInstPerBatch = batch->calculateMaxNumInstances( m_meshReference->
														getSubMesh(m_subMeshIdx), m_instancingFlags );
			m_instancesPerBatch = std::min( maxInstPerBatch, m_instancesPerBatch );
			batch->_setInstancesPerBatch( m_instancesPerBatch );

			//TODO: Create a "merge" function that merges all submeshes into one big submesh
			//instead of just sending submesh #0

			//Get the RenderOperation to be shared with further instances.
			m_sharedRenderOperation = batch->build( m_meshReference->getSubMesh(m_subMeshIdx) );
		}

		const BatchSettings &batchSettings = m_batchSettings[materialName];
		batch->setCastShadows( batchSettings.setting[CAST_SHADOWS] );

		//Batches need to be part of a scene node so that their renderable can be rendered
		SceneNode *sceneNode = m_sceneManager->getRootSceneNode()->createChildSceneNode();
		sceneNode->attachObject( batch );
		sceneNode->showBoundingBox( batchSettings.setting[SHOW_BOUNDINGBOX] );

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
		InstanceBatchVec::iterator itor = fragmentedBatches.begin();
		InstanceBatchVec::iterator end  = fragmentedBatches.end();

		while( itor != end && !usedEntities.empty() )
		{
			if( !(*itor)->isStatic() )
				(*itor)->_defragmentBatch( optimizeCull, usedEntities );
			++itor;
		}

		InstanceBatchVec::iterator lastImportantBatch = itor;

		while( itor != end )
		{
			if( !(*itor)->isStatic() )
			{
				//If we get here, this means we hit remaining batches which will be unused.
				//Destroy them
				//Call this to avoid freeing InstancedEntities that were just reparented
				(*itor)->_defragmentBatchDiscard();
				OGRE_DELETE *itor;
			}
			else
			{
				//This isn't a meaningless batch, move it forward so it doesn't get wipe
				//when we resize the container (faster than removing element by element)
				*lastImportantBatch++ = *itor;
			}

			++itor;
		}

		//Remove remaining batches all at once from the vector
		const size_t remainingBatches = end - lastImportantBatch;
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
				//Don't collect instances from static batches, we assume they're correctly set
				//Plus, we don't want to put InstancedEntities from non-static into static batches
				if( !(*it)->isStatic() )
					(*it)->getInstancedEntitiesInUse( usedEntities );
				++it;
			}

			defragmentBatches( optimizeCulling, usedEntities, itor->second );

			++itor;
		}
	}
	//-----------------------------------------------------------------------
	void InstanceManager::setSetting( BatchSettingId id, bool value, const String &materialName )
	{
		assert( id < NUM_SETTINGS );

		if( materialName == StringUtil::BLANK )
		{
			//Setup all existing materials
			InstanceBatchMap::iterator itor = m_instanceBatches.begin();
			InstanceBatchMap::iterator end  = m_instanceBatches.end();

			while( itor != end )
			{
				m_batchSettings[itor->first].setting[id] = value;
				applySettingToBatches( id, value, itor->second );

				++itor;
			}
		}
		else
		{
			//Setup a given material
			m_batchSettings[materialName].setting[id] = value;

			InstanceBatchMap::const_iterator itor = m_instanceBatches.find( materialName );
			//Don't crash or throw if the batch with that material hasn't been created yet
			if( itor != m_instanceBatches.end() )
				applySettingToBatches( id, value, itor->second );
		}
	}
	//-----------------------------------------------------------------------
	bool InstanceManager::getSetting( BatchSettingId id, const String &materialName ) const
	{
		assert( id < NUM_SETTINGS );

		BatchSettingsMap::const_iterator itor = m_batchSettings.find( materialName );
		if( itor != m_batchSettings.end() )
			return itor->second.setting[id]; //Return current setting

		//Return default
		return BatchSettings().setting[id];
	}
	//-----------------------------------------------------------------------
	void InstanceManager::applySettingToBatches( BatchSettingId id, bool value,
												 const InstanceBatchVec &container )
	{
		InstanceBatchVec::const_iterator itor = container.begin();
		InstanceBatchVec::const_iterator end  = container.end();

		while( itor != end )
		{
			switch( id )
			{
			case CAST_SHADOWS:
				(*itor)->setCastShadows( value );
				break;
			case SHOW_BOUNDINGBOX:
				(*itor)->getParentSceneNode()->showBoundingBox( value );
				break;
			}
			++itor;
		}
	}
	//-----------------------------------------------------------------------
	void InstanceManager::setBatchesAsStaticAndUpdate( bool bStatic )
	{
		InstanceBatchMap::iterator itor = m_instanceBatches.begin();
		InstanceBatchMap::iterator end  = m_instanceBatches.end();

		while( itor != end )
		{
			InstanceBatchVec::iterator it = itor->second.begin();
			InstanceBatchVec::iterator en = itor->second.end();

			while( it != en )
			{
				(*it)->setStaticAndUpdate( bStatic );
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
