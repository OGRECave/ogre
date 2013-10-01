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

#include "Math/Array/OgreNodeMemoryManager.h"

namespace Ogre
{
	NodeMemoryManager::NodeMemoryManager() :
			m_dummyNode( 0 ),
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

		/*m_dummyTransformPtrs.mDerivedTransform	= reinterpret_cast<ArrayMatrix4*>( OGRE_MALLOC_SIMD(
												sizeof( ArrayMatrix4 ), MEMCATEGORY_SCENE_OBJECTS ) );
		m_dummyTransformPtrs.mInheritOrientation= OGRE_MALLOC_SIMD( sizeof( bool ) * ARRAY_PACKED_REALS,
																	MEMCATEGORY_SCENE_OBJECTS );
		m_dummyTransformPtrs.mInheritScale		= OGRE_MALLOC_SIMD( sizeof( bool ) * ARRAY_PACKED_REALS,
																	MEMCATEGORY_SCENE_OBJECTS );*/

		*m_dummyTransformPtrs.mDerivedPosition		= ArrayVector3::ZERO;
		*m_dummyTransformPtrs.mDerivedOrientation	= ArrayQuaternion::IDENTITY;
		*m_dummyTransformPtrs.mDerivedScale			= ArrayVector3::UNIT_SCALE;

		m_dummyNode = new SceneNode( m_dummyTransformPtrs );
	}
	//-----------------------------------------------------------------------------------
	NodeMemoryManager::~NodeMemoryManager()
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

		/*OGRE_FREE_SIMD( m_dummyTransformPtrs.mPosition, MEMCATEGORY_SCENE_OBJECTS );
		OGRE_FREE_SIMD( m_dummyTransformPtrs.mOrientation, MEMCATEGORY_SCENE_OBJECTS );
		OGRE_FREE_SIMD( m_dummyTransformPtrs.mScale, MEMCATEGORY_SCENE_OBJECTS );*/

		OGRE_FREE_SIMD( m_dummyTransformPtrs.mDerivedPosition, MEMCATEGORY_SCENE_OBJECTS );
		OGRE_FREE_SIMD( m_dummyTransformPtrs.mDerivedOrientation, MEMCATEGORY_SCENE_OBJECTS );
		OGRE_FREE_SIMD( m_dummyTransformPtrs.mDerivedScale, MEMCATEGORY_SCENE_OBJECTS );

		/*OGRE_FREE_SIMD( m_dummyTransformPtrs.mDerivedTransform, MEMCATEGORY_SCENE_OBJECTS );
		OGRE_FREE_SIMD( m_dummyTransformPtrs.mInheritOrientation, MEMCATEGORY_SCENE_OBJECTS );
		OGRE_FREE_SIMD( m_dummyTransformPtrs.mInheritScale, MEMCATEGORY_SCENE_OBJECTS );*/
		m_dummyTransformPtrs = Transform();
	}
	//-----------------------------------------------------------------------------------
	void NodeMemoryManager::_setTwin( SceneMemoryMgrTypes memoryManagerType,
										NodeMemoryManager *twinMemoryManager )
	{
		m_memoryManagerType = memoryManagerType;
		m_twinMemoryManager = twinMemoryManager;
	}
	//-----------------------------------------------------------------------------------
	void NodeMemoryManager::growToDepth( size_t newDepth )
	{
		//TODO: (dark_sylinc) give a specialized hint for each depth
		while( newDepth >= m_memoryManagers.size() )
		{
			m_memoryManagers.push_back( NodeArrayMemoryManager( m_memoryManagers.size(), 100,
																m_dummyNode, 100,
																ArrayMemoryManager::MAX_MEMORY_SLOTS,
																this ) );
			m_memoryManagers.back().initialize();
		}
	}
	//-----------------------------------------------------------------------------------
	void NodeMemoryManager::nodeCreated( Transform &outTransform, size_t depth )
	{
		growToDepth( depth );

		NodeArrayMemoryManager& mgr = m_memoryManagers[depth];
		mgr.createNewNode( outTransform );
	}
	//-----------------------------------------------------------------------------------
	void NodeMemoryManager::nodeAttached( Transform &outTransform, size_t depth )
	{
		growToDepth( depth );

		Transform tmp;
		m_memoryManagers[depth].createNewNode( tmp );

		tmp.copy( outTransform );

		NodeArrayMemoryManager &mgr = m_memoryManagers[0];
		mgr.destroyNode( outTransform );

		outTransform = tmp;
	}
	//-----------------------------------------------------------------------------------
	void NodeMemoryManager::nodeDettached( Transform &outTransform, size_t depth )
	{
		Transform tmp;
		m_memoryManagers[0].createNewNode( tmp );

		tmp.copy( outTransform );
		tmp.mParents[tmp.mIndex] = m_dummyNode;

		NodeArrayMemoryManager &mgr = m_memoryManagers[depth];
		mgr.destroyNode( outTransform );

		outTransform = tmp;
	}
	//-----------------------------------------------------------------------------------
	void NodeMemoryManager::nodeDestroyed( Transform &outTransform, size_t depth )
	{
		NodeArrayMemoryManager &mgr = m_memoryManagers[depth];
		mgr.destroyNode( outTransform );
	}
	//-----------------------------------------------------------------------------------
	void NodeMemoryManager::migrateTo( Transform &inOutTransform, size_t depth,
										NodeMemoryManager *dstNodeMemoryManager )
	{
		Transform tmp;
		dstNodeMemoryManager->nodeCreated( tmp, depth );
		tmp.copy( inOutTransform );
		this->nodeDestroyed( inOutTransform, depth );
		inOutTransform = tmp;
	}
	//-----------------------------------------------------------------------------------
	size_t NodeMemoryManager::getNumDepths() const
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
	size_t NodeMemoryManager::getFirstNode( Transform &outTransform, size_t depth )
	{
		return m_memoryManagers[depth].getFirstNode( outTransform );
	}
	//-----------------------------------------------------------------------------------
	void NodeMemoryManager::buildDiffList( ArrayMemoryManager::ManagerType managerType, uint16 level,
											const MemoryPoolVec &basePtrs,
											ArrayMemoryManager::PtrdiffVec &outDiffsList )
	{
		//We don't need to build the diff list as we've access to the Node through mOwner
		//and access to the actual Node with the right pointers.
		/*Transform transform;
		const size_t numNodes = this->getFirstNode( transform, level );

		for( size_t i=0; i<numNodes; i += ARRAY_PACKED_REALS )
		{
			for( size_t j=0; j<ARRAY_PACKED_REALS; ++j )
			{
				if( transform.mOwner[j] )
				{
					outDiffsList.push_back( (transform.mParents + transform.mIndex) -
										(Ogre::Node**)basePtrs[NodeArrayMemoryManager::Parent] );
				}
			}
			transform.advancePack();
		}*/
	}
	//---------------------------------------------------------------------
	void NodeMemoryManager::applyRebase( ArrayMemoryManager::ManagerType managerType, uint16 level,
											const MemoryPoolVec &newBasePtrs,
											const ArrayMemoryManager::PtrdiffVec &diffsList )
	{
		Transform transform;
		const size_t numNodes = this->getFirstNode( transform, level );

		for( size_t i=0; i<numNodes; i += ARRAY_PACKED_REALS )
		{
			for( size_t j=0; j<ARRAY_PACKED_REALS; ++j )
			{
				if( transform.mOwner[j] )
				{
					transform.mIndex = j;
					transform.mOwner[j]->_getTransform() = transform;
				}
			}

			transform.advancePack();
		}
	}
	//---------------------------------------------------------------------
	void NodeMemoryManager::performCleanup( ArrayMemoryManager::ManagerType managerType, uint16 level,
										const MemoryPoolVec &basePtrs, size_t const *elementsMemSizes,
										size_t startInstance, size_t diffInstances )
	{
		Transform transform;
		const size_t numNodes = this->getFirstNode( transform, level );

		size_t roundedStart = startInstance / ARRAY_PACKED_REALS;

		transform.advancePack( roundedStart );

		for( size_t i=roundedStart * ARRAY_PACKED_REALS; i<numNodes; i += ARRAY_PACKED_REALS )
		{
			for( size_t j=0; j<ARRAY_PACKED_REALS; ++j )
			{
				if( transform.mOwner[j] )
				{
					transform.mIndex = j;
					transform.mOwner[j]->_getTransform() = transform;
				}
			}

			transform.advancePack();
		}
	}
}
