/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

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

#include "OgreException.h"

#include "Animation/OgreSkeletonAnimManager.h"
#include "Animation/OgreSkeletonDef.h"
#include "Animation/OgreSkeletonInstance.h"

namespace Ogre
{
    BySkeletonDef::BySkeletonDef( const SkeletonDef *_skeletonDef, size_t threadCount ) :
        skeletonDef( _skeletonDef ),
        skeletonDefName( _skeletonDef->getNameStr() )
    {
        threadStarts.resize( threadCount + 1, 0 );
    }
    //-----------------------------------------------------------------------
    void BySkeletonDef::initializeMemoryManager(void)
    {
        vector<size_t>::type bonesPerDepth;
        skeletonDef->getBonesPerDepth( bonesPerDepth );
        boneMemoryManager._growToDepth( bonesPerDepth );
        boneMemoryManager.setBoneRebaseListener( this );
    }
    //-----------------------------------------------------------------------
    void BySkeletonDef::updateThreadStarts(void)
    {
        size_t lastStart = 0;
        size_t increments = std::max<size_t>( ARRAY_PACKED_REALS,
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

        assert( threadStarts.back() <= skeletons.size() );
        threadStarts.back() = skeletons.size();
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
        IdString defName( skeletonDef->getNameStr() );
        BySkeletonDefList::iterator itor = std::find( bySkeletonDefs.begin(), bySkeletonDefs.end(),
                                                        defName );
        if( itor == bySkeletonDefs.end() )
        {
            bySkeletonDefs.push_front( BySkeletonDef( skeletonDef, numWorkerThreads ) );
            bySkeletonDefs.front().initializeMemoryManager();
            itor = bySkeletonDefs.begin();
        }

        BySkeletonDef &bySkelDef = *itor;
        FastArray<SkeletonInstance*> &skeletonsArray = bySkelDef.skeletons;
        SkeletonInstance *newInstance = OGRE_NEW SkeletonInstance( skeletonDef,
                                                                    &bySkelDef.boneMemoryManager );
        FastArray<SkeletonInstance*>::iterator it = std::lower_bound(
                                                            skeletonsArray.begin(), skeletonsArray.end(),
                                                            newInstance,
                                                            OrderSkeletonInstanceByMemory );

        //If this assert triggers, two instances got the same memory slot. Most likely we forgot to
        //remove a previous instance and the slot was reused. Otherwise something nasty happened.
        assert( it == skeletonsArray.end() || (*it)->_getMemoryUniqueOffset() != newInstance );

        skeletonsArray.insert( it, newInstance );

#if OGRE_DEBUG_MODE
        {
            //Check all depth levels respect the same ordering
            FastArray<SkeletonInstance*>::const_iterator itSkel = skeletonsArray.begin();
            FastArray<SkeletonInstance*>::const_iterator enSkel = skeletonsArray.end();
            while( itSkel != enSkel )
            {
                const TransformArray &transf1 = (*itSkel)-> _getTransformArray();
                FastArray<SkeletonInstance*>::const_iterator itSkel2 = itSkel+1;
                while( itSkel2 != enSkel )
                {
                    const TransformArray &transf2 = (*itSkel2)-> _getTransformArray();
                    for( size_t i=0; i<transf1.size(); ++i )
                    {
                        Bone **owner1 = transf1[i].mOwner + transf1[i].mIndex;
                        Bone **owner2 = transf2[i].mOwner + transf2[i].mIndex;
                        assert( owner1 < owner2 );
                    }

                    ++itSkel2;
                }

                ++itSkel;
            }
        }
#endif

        //Update the thread starts, they have changed.
        bySkelDef.updateThreadStarts();

        return newInstance;
    }
    //-----------------------------------------------------------------------
    void SkeletonAnimManager::destroySkeletonInstance( SkeletonInstance *skeletonInstance )
    {
        IdString defName( skeletonInstance->getDefinition()->getNameStr() );
        BySkeletonDefList::iterator itor = std::find( bySkeletonDefs.begin(), bySkeletonDefs.end(),
                                                        defName );

        if( itor == bySkeletonDefs.end() )
        {
            OGRE_EXCEPT( Exception::ERR_INVALID_STATE,
                         "Trying to remove a SkeletonInstance for which we have no definition for!",
                         "SceneManager::destroySkeletonInstance" );
        }

        BySkeletonDef &bySkelDef = *itor;
        FastArray<SkeletonInstance*> &skeletonsArray = bySkelDef.skeletons;

        FastArray<SkeletonInstance*>::iterator it = std::lower_bound(
                                                        skeletonsArray.begin(), skeletonsArray.end(),
                                                        skeletonInstance,
                                                        OrderSkeletonInstanceByMemory );

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
