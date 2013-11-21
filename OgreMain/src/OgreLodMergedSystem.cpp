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

#include "OgreLodMergedSystem.h"

namespace Ogre {
    //-----------------------------------------------------------------------
	LodMergedSystem::~LodMergedSystem()
	{
		map<IdString, LodMovableObjectArray*>::type::const_iterator itor = mLodMerged.begin();
		map<IdString, LodMovableObjectArray*>::type::const_iterator end  = mLodMerged.end();

		while( itor != end )
		{
			delete itor->second;
			++itor;
		}

		mLodMerged.clear();
    }
	//-----------------------------------------------------------------------
	const LodMovableObjectArray* LodMergedSystem::getLodMerged( IdString hash,
																const LodMeshPairVec &meshesLod )
    {
		LodMovableObjectArray const *retVal = 0;

		map<IdString, LodMovableObjectArray*>::type::const_iterator itor = mLodMerged.find( hash );
		if( itor != mLodMerged.end() )
		{
			retVal = itor->second;
		}
		else
		{
			LodMovableObjectArray *newLodMovable = new LodMovableObjectArray( meshesLod.size() );

			LodMeshPairVec::const_iterator itMesh = meshesLod.begin();
			LodMeshPairVec::const_iterator enMesh = meshesLod.end();

			while( itMesh != enMesh )
			{
				newLodMovable->push_back( FastArray<LodMerged>() );
				FastArray<LodMerged> &newLodMerged = newLodMovable->back();

				LodValueVec::const_iterator itMeshLod = itMesh->meshList->begin();
				LodValueVec::const_iterator enMeshLod = itMesh->meshList->end();
				LodValueVec::const_iterator itMatLod = itMesh->materialList->begin();
				LodValueVec::const_iterator enMatLod = itMesh->materialList->end();
				while( itMeshLod != enMeshLod && itMatLod != enMatLod )
				{
					LodMerged lodMerged;
					lodMerged.lodValue = std::min( *itMeshLod, *itMatLod );
					lodMerged.meshLodIndex		= itMeshLod - itMesh->meshList->begin();
					lodMerged.materialLodIndex	= itMatLod - itMesh->materialList->begin();
					newLodMerged.push_back( lodMerged );

					if( itMeshLod+1 == enMeshLod )
						++itMatLod;
					else if( itMatLod+1 == enMatLod )
						++itMeshLod;
					else
					{
						if( *(itMeshLod+1) <= *(itMatLod+1) )
							++itMeshLod;
						if( *(itMatLod+1) <= *(itMeshLod+1) )
							++itMatLod;
					}
				}

				++itMesh;
			}

			mLodMerged[hash] = newLodMovable;
			retVal = newLodMovable;
		}

		return retVal;
	}
}

