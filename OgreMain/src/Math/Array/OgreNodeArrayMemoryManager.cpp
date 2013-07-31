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

#include "Math/Array/OgreArrayMemoryManager.h"

#include "Math/Array/OgreTransform.h"

namespace Ogre
{
	const size_t NodeArrayMemoryManager::ElementsMemSize[NodeArrayMemoryManager::NumMemoryTypes] =
	{
		sizeof( Node** ),				//ArrayMemoryManager::Parent
		sizeof( Node** ),				//ArrayMemoryManager::Owner
		3 * sizeof( Ogre::Real ),		//ArrayMemoryManager::Position
		4 * sizeof( Ogre::Real ),		//ArrayMemoryManager::Orientation
		3 * sizeof( Ogre::Real ),		//ArrayMemoryManager::Scale
		3 * sizeof( Ogre::Real ),		//ArrayMemoryManager::DerivedPosition
		4 * sizeof( Ogre::Real ),		//ArrayMemoryManager::DerivedOrientation
		3 * sizeof( Ogre::Real ),		//ArrayMemoryManager::DerivedScale
		16 * sizeof( Ogre::Real ),		//ArrayMemoryManager::WorldMat
		sizeof( bool ),					//ArrayMemoryManager::InheritOrientation
		sizeof( bool )					//ArrayMemoryManager::InheritScale
	};
	//-----------------------------------------------------------------------------------
	NodeArrayMemoryManager::NodeArrayMemoryManager( uint16 depthLevel, size_t hintMaxNodes,
													Node *dummyNode, size_t cleanupThreshold,
													size_t maxHardLimit,
													RebaseListener *rebaseListener ) :
			ArrayMemoryManager( ArrayMemoryManager::NodeType, ElementsMemSize,
								sizeof( ElementsMemSize ) / sizeof( size_t ), depthLevel,
								hintMaxNodes, cleanupThreshold, maxHardLimit, rebaseListener ),
			m_dummyNode( dummyNode )
	{
	}
	//-----------------------------------------------------------------------------------
	void NodeArrayMemoryManager::slotsRecreated( size_t prevNumSlots )
	{
		ArrayMemoryManager::slotsRecreated( prevNumSlots );

		Node **nodesPtr = reinterpret_cast<Node**>( m_memoryPools[Parent] ) + prevNumSlots;
		for( size_t i=prevNumSlots; i<m_maxMemory; ++i )
			*nodesPtr++ = m_dummyNode;
	}
	//-----------------------------------------------------------------------------------
	void NodeArrayMemoryManager::createNewNode( Transform &outTransform )
	{
		const size_t nextSlot = createNewSlot();
		const unsigned char nextSlotIdx	= nextSlot % ARRAY_PACKED_REALS;
		const size_t nextSlotBase		= nextSlot - nextSlotIdx;

		//Set memory ptrs
		outTransform.mIndex = nextSlotIdx;
		outTransform.mParents			= reinterpret_cast<Node**>( m_memoryPools[Parent] +
												nextSlotBase * m_elementsMemSizes[Parent] );
		outTransform.mOwner				= reinterpret_cast<Node**>( m_memoryPools[Owner] +
												nextSlotBase * m_elementsMemSizes[Owner] );
		outTransform.mPosition			= reinterpret_cast<ArrayVector3*>( m_memoryPools[Position] +
												nextSlotBase * m_elementsMemSizes[Position] );
		outTransform.mOrientation		= reinterpret_cast<ArrayQuaternion*>(
												m_memoryPools[Orientation] +
												nextSlotBase * m_elementsMemSizes[Orientation] );
		outTransform.mScale				= reinterpret_cast<ArrayVector3*>( m_memoryPools[Scale] +
												nextSlotBase * m_elementsMemSizes[Scale] );
		outTransform.mDerivedPosition	= reinterpret_cast<ArrayVector3*>(
												m_memoryPools[DerivedPosition] +
												nextSlotBase * m_elementsMemSizes[DerivedPosition] );
		outTransform.mDerivedOrientation=reinterpret_cast<ArrayQuaternion*>(
												m_memoryPools[DerivedOrientation] +
												nextSlotBase * m_elementsMemSizes[DerivedOrientation] );
		outTransform.mDerivedScale		= reinterpret_cast<ArrayVector3*>( m_memoryPools[DerivedScale] +
												nextSlotBase * m_elementsMemSizes[DerivedScale] );
		outTransform.mDerivedTransform	= reinterpret_cast<Matrix4*>( m_memoryPools[WorldMat] +
												nextSlotBase * m_elementsMemSizes[WorldMat] );
		outTransform.mInheritOrientation= reinterpret_cast<bool*>( m_memoryPools[InheritOrientation] +
												nextSlotBase * m_elementsMemSizes[InheritOrientation] );
		outTransform.mInheritScale		= reinterpret_cast<bool*>( m_memoryPools[InheritScale] +
												nextSlotBase * m_elementsMemSizes[InheritScale] );

		//Set default values
		outTransform.mParents[nextSlotIdx] = m_dummyNode;
		outTransform.mOwner[nextSlotIdx] = 0;
		outTransform.mPosition->setFromVector3( Vector3::ZERO, nextSlotIdx );
		outTransform.mOrientation->setFromQuaternion( Quaternion::IDENTITY, nextSlotIdx );
		outTransform.mScale->setFromVector3( Vector3::UNIT_SCALE, nextSlotIdx );
		outTransform.mDerivedPosition->setFromVector3( Vector3::ZERO, nextSlotIdx );
		outTransform.mDerivedOrientation->setFromQuaternion( Quaternion::IDENTITY, nextSlotIdx );
		outTransform.mDerivedScale->setFromVector3( Vector3::UNIT_SCALE, nextSlotIdx );
		outTransform.mDerivedTransform[nextSlotIdx] = Matrix4::IDENTITY;
		outTransform.mInheritOrientation[nextSlotIdx]	= true;
		outTransform.mInheritScale[nextSlotIdx]			= true;
	}
	//-----------------------------------------------------------------------------------
	void NodeArrayMemoryManager::destroyNode( Transform &inOutTransform )
	{
		//Zero out important data that would lead to bugs (Remember SIMD SoA means even if
		//there's one object in scene, 4 objects are still parsed simultaneously)

		inOutTransform.mParents[inOutTransform.mIndex]	= m_dummyNode;
		inOutTransform.mOwner[inOutTransform.mIndex]	= 0;
		destroySlot( reinterpret_cast<char*>(inOutTransform.mParents), inOutTransform.mIndex );
		//Zero out all pointers
		inOutTransform = Transform();
	}
	//-----------------------------------------------------------------------------------
	size_t NodeArrayMemoryManager::getFirstNode( Transform &outTransform )
	{
		outTransform.mParents			= reinterpret_cast<Node**>( m_memoryPools[Parent] );
		outTransform.mOwner				= reinterpret_cast<Node**>( m_memoryPools[Owner] );
		outTransform.mPosition			= reinterpret_cast<ArrayVector3*>( m_memoryPools[Position] );
		outTransform.mOrientation		= reinterpret_cast<ArrayQuaternion*>(
														m_memoryPools[Orientation] );
		outTransform.mScale				= reinterpret_cast<ArrayVector3*>( m_memoryPools[Scale] );
		outTransform.mDerivedPosition	= reinterpret_cast<ArrayVector3*>(
														m_memoryPools[DerivedPosition] );
		outTransform.mDerivedOrientation= reinterpret_cast<ArrayQuaternion*>(
														m_memoryPools[DerivedOrientation] );
		outTransform.mDerivedScale		= reinterpret_cast<ArrayVector3*>( m_memoryPools[DerivedScale] );
		outTransform.mDerivedTransform	= reinterpret_cast<Matrix4*>( m_memoryPools[WorldMat] );
		outTransform.mInheritOrientation= reinterpret_cast<bool*>( m_memoryPools[InheritOrientation] );
		outTransform.mInheritScale		= reinterpret_cast<bool*>( m_memoryPools[InheritScale] );

		return m_usedMemory;
	}
}
