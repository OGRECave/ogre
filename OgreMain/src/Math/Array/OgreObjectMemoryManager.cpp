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

#include "Math/Array/OgreObjectMemoryManager.h"

#include "OgreMovableObject.h"

namespace Ogre
{
	ObjectMemoryManager::ObjectMemoryManager() :
			m_totalObjects( 0 ),
			m_dummyNode( 0 ),
			m_dummyObject( 0 ),
			m_memoryManagerType( SCENE_DYNAMIC ),
			m_twinMemoryManager( 0 )
	{
		//Manually allocate the memory for the dummy scene nodes (since we can't pass ourselves
		//or yet another object) We only allocate what's needed to prevent access violations.
		/*m_dummyTransformPtrs.mPosition = reinterpret_cast<ArrayVector3*>( OGRE_MALLOC_SIMD(
												sizeof( ArrayVector3 ), MEMCATEGORY_SCENE_OBJECTS ) );
		m_dummyTransformPtrs.mOrientation = reinterpret_cast<ArrayQuaternion*>( OGRE_MALLOC_SIMD(
												sizeof( ArrayQuaternion ), MEMCATEGORY_SCENE_OBJECTS ) );
		m_dummyTransformPtrs.mScale = reinterpret_cast<ArrayVector3*>( OGRE_MALLOC_SIMD(
												sizeof( ArrayVector3 ), MEMCATEGORY_SCENE_OBJECTS ) );*/

		m_dummyTransformPtrs.mDerivedPosition	= reinterpret_cast<ArrayVector3*>( OGRE_MALLOC_SIMD(
												sizeof( ArrayVector3 ), MEMCATEGORY_SCENE_OBJECTS ) );
		m_dummyTransformPtrs.mDerivedOrientation= reinterpret_cast<ArrayQuaternion*>( OGRE_MALLOC_SIMD(
												sizeof( ArrayQuaternion ), MEMCATEGORY_SCENE_OBJECTS ) );
		m_dummyTransformPtrs.mDerivedScale		= reinterpret_cast<ArrayVector3*>( OGRE_MALLOC_SIMD(
												sizeof( ArrayVector3 ), MEMCATEGORY_SCENE_OBJECTS ) );

		m_dummyTransformPtrs.mDerivedTransform	= reinterpret_cast<Matrix4*>( OGRE_MALLOC_SIMD(
												sizeof( Matrix4 ) * ARRAY_PACKED_REALS,
												MEMCATEGORY_SCENE_OBJECTS ) );
		/*m_dummyTransformPtrs.mInheritOrientation= OGRE_MALLOC_SIMD( sizeof( bool ) * ARRAY_PACKED_REALS,
																	MEMCATEGORY_SCENE_OBJECTS );
		m_dummyTransformPtrs.mInheritScale		= OGRE_MALLOC_SIMD( sizeof( bool ) * ARRAY_PACKED_REALS,
																	MEMCATEGORY_SCENE_OBJECTS );*/

		*m_dummyTransformPtrs.mDerivedPosition		= ArrayVector3::ZERO;
		*m_dummyTransformPtrs.mDerivedOrientation	= ArrayQuaternion::IDENTITY;
		*m_dummyTransformPtrs.mDerivedScale			= ArrayVector3::UNIT_SCALE;
		for( size_t i=0; i<ARRAY_PACKED_REALS; ++i )
			m_dummyTransformPtrs.mDerivedTransform[i] = Matrix4::IDENTITY;

		m_dummyNode = new SceneNode( m_dummyTransformPtrs );
		m_dummyObject = new NullEntity();
	}
	//-----------------------------------------------------------------------------------
	ObjectMemoryManager::~ObjectMemoryManager()
	{
		ArrayMemoryManagerVec::iterator itor = m_memoryManagers.begin();
		ArrayMemoryManagerVec::iterator end  = m_memoryManagers.end();

		while( itor != end )
		{
			itor->destroy();
			++itor;
		}

		m_memoryManagers.clear();

		delete m_dummyNode;
		m_dummyNode = 0;

		delete m_dummyObject;
		m_dummyObject = 0;

		/*OGRE_FREE_SIMD( m_dummyTransformPtrs.mPosition, MEMCATEGORY_SCENE_OBJECTS );
		OGRE_FREE_SIMD( m_dummyTransformPtrs.mOrientation, MEMCATEGORY_SCENE_OBJECTS );
		OGRE_FREE_SIMD( m_dummyTransformPtrs.mScale, MEMCATEGORY_SCENE_OBJECTS );*/

		OGRE_FREE_SIMD( m_dummyTransformPtrs.mDerivedPosition, MEMCATEGORY_SCENE_OBJECTS );
		OGRE_FREE_SIMD( m_dummyTransformPtrs.mDerivedOrientation, MEMCATEGORY_SCENE_OBJECTS );
		OGRE_FREE_SIMD( m_dummyTransformPtrs.mDerivedScale, MEMCATEGORY_SCENE_OBJECTS );

		OGRE_FREE_SIMD( m_dummyTransformPtrs.mDerivedTransform, MEMCATEGORY_SCENE_OBJECTS );
		/*OGRE_FREE_SIMD( m_dummyTransformPtrs.mInheritOrientation, MEMCATEGORY_SCENE_OBJECTS );
		OGRE_FREE_SIMD( m_dummyTransformPtrs.mInheritScale, MEMCATEGORY_SCENE_OBJECTS );*/
		m_dummyTransformPtrs = Transform();
	}
	//-----------------------------------------------------------------------------------
	void ObjectMemoryManager::_setTwin( SceneMemoryMgrTypes memoryManagerType,
										ObjectMemoryManager *twinMemoryManager )
	{
		m_memoryManagerType = memoryManagerType;
		m_twinMemoryManager = twinMemoryManager;
	}
	//-----------------------------------------------------------------------------------
	void ObjectMemoryManager::growToDepth( size_t newDepth )
	{
		//TODO: (dark_sylinc) give a specialized hint for each depth
		while( newDepth >= m_memoryManagers.size() )
		{
			m_memoryManagers.push_back( ObjectDataArrayMemoryManager( m_memoryManagers.size(), 100,
											m_dummyNode, m_dummyObject, 100,
											ArrayMemoryManager::MAX_MEMORY_SLOTS, this ) );
			m_memoryManagers.back().initialize();
		}
	}
	//-----------------------------------------------------------------------------------
	void ObjectMemoryManager::objectCreated( ObjectData &outObjectData, size_t renderQueue )
	{
		growToDepth( renderQueue );

		ObjectDataArrayMemoryManager& mgr = m_memoryManagers[renderQueue];
		mgr.createNewNode( outObjectData );

		++m_totalObjects;
	}
	//-----------------------------------------------------------------------------------
	void ObjectMemoryManager::objectMoved( ObjectData &inOutObjectData, size_t oldRenderQueue,
											size_t newRenderQueue )
	{
		growToDepth( newRenderQueue );

		ObjectData tmp;
		m_memoryManagers[newRenderQueue].createNewNode( tmp );

		tmp.copy( inOutObjectData );

		ObjectDataArrayMemoryManager &mgr = m_memoryManagers[oldRenderQueue];
		mgr.destroyNode( inOutObjectData );

		inOutObjectData = tmp;
	}
	//-----------------------------------------------------------------------------------
	void ObjectMemoryManager::objectDestroyed( ObjectData &outObjectData, size_t renderQueue )
	{
		ObjectDataArrayMemoryManager &mgr = m_memoryManagers[renderQueue];
		mgr.destroyNode( outObjectData );

		--m_totalObjects;
	}
	//-----------------------------------------------------------------------------------
	void ObjectMemoryManager::migrateTo( ObjectData &inOutObjectData, size_t renderQueue,
										 ObjectMemoryManager *dstObjectMemoryManager )
	{
		ObjectData tmp;
		dstObjectMemoryManager->objectCreated( tmp, renderQueue );
		tmp.copy( inOutObjectData );
		this->objectDestroyed( inOutObjectData, renderQueue );
		inOutObjectData = tmp;
	}
	//-----------------------------------------------------------------------------------
	size_t ObjectMemoryManager::getNumRenderQueues() const
	{
		size_t retVal = -1;
		ArrayMemoryManagerVec::const_iterator begin= m_memoryManagers.begin();
		ArrayMemoryManagerVec::const_iterator itor = m_memoryManagers.begin();
		ArrayMemoryManagerVec::const_iterator end  = m_memoryManagers.end();

		while( itor != end )
		{
			if( itor->getUsedMemory() )
				retVal = itor - begin;
			++itor;
		}

		return retVal + 1;
	}
	//-----------------------------------------------------------------------------------
	size_t ObjectMemoryManager::getFirstObjectData( ObjectData &outObjectData, size_t renderQueue )
	{
		return m_memoryManagers[renderQueue].getFirstNode( outObjectData );
	}
	//-----------------------------------------------------------------------------------
	void ObjectMemoryManager::buildDiffList( ArrayMemoryManager::ManagerType managerType, uint16 level,
												const MemoryPoolVec &basePtrs,
												ArrayMemoryManager::PtrdiffVec &outDiffsList )
	{
		//We don't need to build the diff list as we've access to the MovableObject through mOwner
		//and access to the actual ObjectData with the right pointers.
		/*ObjectData objectData;
		const size_t numObjs = this->getFirstObjectData( objectData, level );

		for( size_t i=0; i<numObjs; i += ARRAY_PACKED_REALS )
		{
			for( size_t j=0; j<ARRAY_PACKED_REALS; ++j )
			{
				if( objectData.mOwner[j] )
				{
					outDiffsList.push_back( (objectData.mParents + objectData.mIndex) -
										(Ogre::Node**)basePtrs[ObjectDataArrayMemoryManager::Parent] );
				}
			}
			objectData.advancePack();
		}*/
	}
	//---------------------------------------------------------------------
	void ObjectMemoryManager::applyRebase( ArrayMemoryManager::ManagerType managerType, uint16 level,
											const MemoryPoolVec &newBasePtrs,
											const ArrayMemoryManager::PtrdiffVec &diffsList )
	{
		ObjectData objectData;
		const size_t numObjs = this->getFirstObjectData( objectData, level );

		for( size_t i=0; i<numObjs; i += ARRAY_PACKED_REALS )
		{
			for( size_t j=0; j<ARRAY_PACKED_REALS; ++j )
			{
				if( objectData.mOwner[j] )
				{
					objectData.mIndex = j;
					objectData.mOwner[j]->_getObjectData() = objectData;
				}
			}

			objectData.advancePack();
		}
	}
	//---------------------------------------------------------------------
	void ObjectMemoryManager::performCleanup( ArrayMemoryManager::ManagerType managerType, uint16 level,
										const MemoryPoolVec &basePtrs, size_t const *elementsMemSizes,
										size_t startInstance, size_t diffInstances )
	{
		ObjectData objectData;
		const size_t numObjs = this->getFirstObjectData( objectData, level );

		size_t roundedStart = startInstance / ARRAY_PACKED_REALS;

		objectData.advancePack( roundedStart );

		for( size_t i=roundedStart * ARRAY_PACKED_REALS; i<numObjs; i += ARRAY_PACKED_REALS )
		{
			for( size_t j=0; j<ARRAY_PACKED_REALS; ++j )
			{
				if( objectData.mOwner[j] )
				{
					objectData.mIndex = j;
					objectData.mOwner[j]->_getObjectData() = objectData;
				}
			}

			objectData.advancePack();
		}
	}
}
