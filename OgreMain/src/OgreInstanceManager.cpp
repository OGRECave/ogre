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
#include "OgreMesh.h"
#include "OgreSubMesh.h"
#include "OgreMeshManager.h"
#include "OgreMaterialManager.h"
#include "OgreSceneManager.h"

namespace Ogre
{
	InstanceManager::InstanceManager( const String &customName, SceneManager *sceneManager,
										const String &meshName, const String &groupName ) :
				m_name( customName ),
				m_sceneManager( sceneManager ),
				m_instancesPerBatch( 80 )
	{
		m_meshReference = MeshManager::getSingleton().load( meshName, groupName );
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
	InstancedEntity* InstanceManager::createInstancedEntity( const String &materialName )
	{
		InstanceBatch *instanceBatch;

		if( m_instanceBatches.empty() )
			instanceBatch = buildFirstTime( materialName );
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
		return buildNewBatch( materialName );
	}
	//-----------------------------------------------------------------------
	InstanceBatch* InstanceManager::buildFirstTime( const String &materialName )
	{
		Mesh::IndexMap &idxMap = m_meshReference->getSubMesh(0)->blendIndexToBoneIndexMap;
		idxMap = idxMap.empty() ? m_meshReference->sharedBlendIndexToBoneIndexMap : idxMap;

		MaterialPtr mat = MaterialManager::getSingleton().getByName( materialName,
																	m_meshReference->getGroup() );

		InstanceBatchVec &materialInstanceBatch = m_instanceBatches[materialName];

		InstanceBatchShader *batch = OGRE_NEW InstanceBatchShader( m_meshReference, mat,
																	m_instancesPerBatch, &idxMap );

		m_instancesPerBatch = std::min( m_instancesPerBatch,
										batch->calculateMaxNumInstances( m_meshReference->getSubMesh(0) ) );
		batch->_setInstancesPerBatch( m_instancesPerBatch );

		//TODO: Create a "merge" function that merges all submeshes into one big submesh
		//instead of just sending submesh #0
		//Get the RenderOperation to be shared with further instances.
		m_sharedRenderOperation = batch->build( m_meshReference->getSubMesh(0) );

		//Batches need to be part of a scene node so that their renderable can be rendered
		SceneNode *sceneNode = m_sceneManager->getRootSceneNode()->createChildSceneNode();
		sceneNode->attachObject( batch );

		materialInstanceBatch.push_back( batch );

		return batch;
	}
	//-----------------------------------------------------------------------
	InstanceBatch* InstanceManager::buildNewBatch( const String &materialName )
	{
		Mesh::IndexMap &idxMap = m_meshReference->getSubMesh(0)->blendIndexToBoneIndexMap;
		idxMap = idxMap.empty() ? m_meshReference->sharedBlendIndexToBoneIndexMap : idxMap;

		MaterialPtr mat = MaterialManager::getSingleton().getByName( materialName,
																	m_meshReference->getGroup() );

		InstanceBatchVec &materialInstanceBatch = m_instanceBatches[materialName];

		InstanceBatchShader *batch = OGRE_NEW InstanceBatchShader( m_meshReference, mat,
																	m_instancesPerBatch, &idxMap );

		//TODO: Check different materials have the same m_instancesPerBatch upper limit
		//otherwise we can't share
		batch->buildFrom( m_sharedRenderOperation );

		//Batches need to be part of a scene node so that their renderable can be rendered
		SceneNode *sceneNode = m_sceneManager->getRootSceneNode()->createChildSceneNode();
		sceneNode->attachObject( batch );

		materialInstanceBatch.push_back( batch );

		return batch;
	}
}
