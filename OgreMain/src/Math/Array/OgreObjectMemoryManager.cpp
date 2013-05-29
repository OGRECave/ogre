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

namespace Ogre
{
	void ObjectMemoryManager::growToDepth( size_t newDepth )
	{
		//TODO: (dark_sylinc) give a specialized hint for each depth
		if( newDepth >= m_memoryManagers.size() )
			m_memoryManagers.push_back( ObjectDataArrayMemoryManager( newDepth, 100 ) );
	}
	//-----------------------------------------------------------------------------------
	void ObjectMemoryManager::objectCreated( ObjectData &outObjectData, size_t renderQueue )
	{
		growToDepth( renderQueue );

		ObjectDataArrayMemoryManager& mgr = m_memoryManagers[renderQueue];
		mgr.createNewNode( outObjectData );
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
	}
	//-----------------------------------------------------------------------------------
	size_t ObjectMemoryManager::getNumRenderQueues() const
	{
		size_t retVal = 0;
		ArrayMemoryManagerVec::const_iterator begin= m_memoryManagers.begin();
		ArrayMemoryManagerVec::const_iterator itor = m_memoryManagers.begin();
		ArrayMemoryManagerVec::const_iterator end  = m_memoryManagers.end();

		while( itor != end )
		{
			if( itor->getUsedMemory() )
				retVal = itor - begin;
		}

		return retVal;
	}
	//-----------------------------------------------------------------------------------
	size_t ObjectMemoryManager::getFirstObjectData( ObjectData &outObjectData, size_t renderQueue )
	{
		return m_memoryManagers[renderQueue].getFirstNode( outObjectData );
	}
}
