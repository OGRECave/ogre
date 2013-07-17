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
	}
	//-----------------------------------------------------------------------------------
	void NodeMemoryManager::growToDepth( size_t newDepth )
	{
		//TODO: (dark_sylinc) give a specialized hint for each depth
		while( newDepth >= m_memoryManagers.size() )
		{
			m_memoryManagers.push_back( NodeArrayMemoryManager( m_memoryManagers.size(), 100 ) );
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
	size_t NodeMemoryManager::getNumDepths() const
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
	size_t NodeMemoryManager::getFirstNode( Transform &outTransform, size_t depth )
	{
		return m_memoryManagers[depth].getFirstNode( outTransform );
	}
}
