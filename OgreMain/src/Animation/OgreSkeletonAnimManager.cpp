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

#include "Animation/OgreSkeletonAnimManager.h"
#include "Animation/OgreSkeletonDef.h"

namespace Ogre
{
	BySkeletonDef::BySkeletonDef( IdString defName, size_t threadCount ) :
		skeletonDefName( defName )
	{
		threadStarts.resize( threadCount + 1, 0 );
	}
	//-----------------------------------------------------------------------
	void BySkeletonDef::updateThreadStarts(void)
	{
		size_t lastStart = 0;
		size_t increments = std::min<size_t>( ARRAY_PACKED_REALS,
											  skeletons.size() / (threadStarts.size() - 1) );
		for( size_t i=0; i<threadStarts.size(); ++i )
		{
			threadStarts[i] = lastStart;
			lastStart += increments;
			lastStart = std::min( lastStart, skeletons.size() );

			while( lastStart < skeletons.size() &&
				   skeletons[lastStart]->_getMemoryBlock() ==
				   skeletons[lastStart-1]->_getMemoryBlock() )
			{
				++lastStart;
			}
		}
	}
	//-----------------------------------------------------------------------
	void BySkeletonDef::_updateBoneStartTransforms(void)
	{
		FastArray<SkeletonInstance*>::iterator itor = skeletons.begin();
		FastArray<SkeletonInstance*>::iterator end  = skeletons.end();

		while( itor != end )
		{
			(*itor)->_updateBoneStartTransforms();
			++itor;
		}
	}

	//-----------------------------------------------------------------------
	SkeletonInstance* SkeletonAnimManager::createSkeletonInstance( const SkeletonDef *skeletonDef,
																	size_t numWorkerThreads )
	{
		IdString defName( skeletonDef->getName() );
		BySkeletonDefList::iterator itor = std::find( bySkeletonDefs.begin(), bySkeletonDefs.end(),
														defName );
		if( itor == bySkeletonDefs.end() )
		{
			bySkeletonDefs.push_front( BySkeletonDef( defName, numWorkerThreads ) );
			bySkeletonDefs.front().boneMemoryManager.setBoneRebaseListener( &bySkeletonDefs.front() );
			itor = bySkeletonDefs.begin();
		}

		BySkeletonDef &bySkelDef = *itor;
		FastArray<SkeletonInstance*> &skeletonsArray = bySkelDef.skeletons;
		SkeletonInstance *newInstance = OGRE_NEW SkeletonInstance( skeletonDef,
																	&bySkelDef.boneMemoryManager );
		FastArray<SkeletonInstance*>::iterator it = std::lower_bound(
															skeletonsArray.begin(), skeletonsArray.end(),
															OrderSkeletonInstanceByMemory( newInstance ) );

		//If this assert triggers, two instances got the same memory slot. Most likely we forgot to
		//remove a previous instance and the slot was reused. Otherwise something nasty happened.
		assert( it == skeletonsArray.end() || (*it)->_getMemoryUniqueOffset() != newInstance );

		skeletonsArray.insert( it, newInstance );

		//Update the thread starts, they have changed.
		bySkelDef.updateThreadStarts();

		return newInstance;
	}
	//-----------------------------------------------------------------------
	void SkeletonAnimManager::destroySkeletonInstance( SkeletonInstance *skeletonInstance )
	{
		IdString defName( skeletonInstance->getDefinition()->getName() );
		BySkeletonDefList::iterator itor = std::find( bySkeletonDefs.begin(), bySkeletonDefs.end(),
														defName );

		if( itor != bySkeletonDefs.end() )
		{
			OGRE_EXCEPT( Exception::ERR_INVALID_STATE,
						 "Trying to remove a SkeletonInstance for which we have no definition for!",
						 "SceneManager::destroySkeletonInstance" );
		}

		BySkeletonDef &bySkelDef = *itor;
		FastArray<SkeletonInstance*> &skeletonsArray = bySkelDef.skeletons;

		FastArray<SkeletonInstance*>::iterator it = std::lower_bound(
														skeletonsArray.begin(), skeletonsArray.end(),
														OrderSkeletonInstanceByMemory( skeletonInstance ) );

		if( it == skeletonsArray.end() || *it != skeletonInstance )
		{
			OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
						 "Trying to remove a SkeletonInstance we don't have. Have you already removed it?",
						 "SceneManager::destroySkeletonInstance" );
		}

		OGRE_DELETE *it;
		skeletonsArray.erase( it );

		//Update the thread starts, they have changed.
		bySkelDef.updateThreadStarts();
	}
}
