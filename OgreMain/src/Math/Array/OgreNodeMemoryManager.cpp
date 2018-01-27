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

#include "Math/Array/OgreNodeMemoryManager.h"

#include "OgreSceneNode.h"

namespace Ogre
{
    NodeMemoryManager::NodeMemoryManager() :
            mDummyNode( 0 ),
            mMemoryManagerType( SCENE_DYNAMIC ),
            mTwinMemoryManager( 0 )
    {
        //Manually allocate the memory for the dummy scene nodes (since we can't pass ourselves
        //or yet another object) We only allocate what's needed to prevent access violations.
        /*mDummyTransformPtrs.mPosition = reinterpret_cast<ArrayVector3*>( OGRE_MALLOC_SIMD(
                                                sizeof( ArrayVector3 ), MEMCATEGORY_SCENE_OBJECTS ) );
        mDummyTransformPtrs.mOrientation = reinterpret_cast<ArrayQuaternion*>( OGRE_MALLOC_SIMD(
                                                sizeof( ArrayQuaternion ), MEMCATEGORY_SCENE_OBJECTS ) );
        mDummyTransformPtrs.mScale = reinterpret_cast<ArrayVector3*>( OGRE_MALLOC_SIMD(
                                                sizeof( ArrayVector3 ), MEMCATEGORY_SCENE_OBJECTS ) );*/

        mDummyTransformPtrs.mDerivedPosition    = reinterpret_cast<ArrayVector3*>( OGRE_MALLOC_SIMD(
                                                sizeof( ArrayVector3 ), MEMCATEGORY_SCENE_OBJECTS ) );
        mDummyTransformPtrs.mDerivedOrientation= reinterpret_cast<ArrayQuaternion*>( OGRE_MALLOC_SIMD(
                                                sizeof( ArrayQuaternion ), MEMCATEGORY_SCENE_OBJECTS ) );
        mDummyTransformPtrs.mDerivedScale       = reinterpret_cast<ArrayVector3*>( OGRE_MALLOC_SIMD(
                                                sizeof( ArrayVector3 ), MEMCATEGORY_SCENE_OBJECTS ) );
        mDummyTransformPtrs.mDerivedTransform   = reinterpret_cast<Matrix4*>( OGRE_MALLOC_SIMD(
                                                sizeof( Matrix4 ) * ARRAY_PACKED_REALS,
                                                MEMCATEGORY_SCENE_OBJECTS ) );

        /*mDummyTransformPtrs.mDerivedTransform = reinterpret_cast<ArrayMatrix4*>( OGRE_MALLOC_SIMD(
                                                sizeof( ArrayMatrix4 ), MEMCATEGORY_SCENE_OBJECTS ) );
        mDummyTransformPtrs.mInheritOrientation= OGRE_MALLOC_SIMD( sizeof( bool ) * ARRAY_PACKED_REALS,
                                                                    MEMCATEGORY_SCENE_OBJECTS );
        mDummyTransformPtrs.mInheritScale       = OGRE_MALLOC_SIMD( sizeof( bool ) * ARRAY_PACKED_REALS,
                                                                    MEMCATEGORY_SCENE_OBJECTS );*/

        *mDummyTransformPtrs.mDerivedPosition       = ArrayVector3::ZERO;
        *mDummyTransformPtrs.mDerivedOrientation    = ArrayQuaternion::IDENTITY;
        *mDummyTransformPtrs.mDerivedScale          = ArrayVector3::UNIT_SCALE;
        for( int i=0; i<ARRAY_PACKED_REALS; ++i )
            mDummyTransformPtrs.mDerivedTransform[i] = Matrix4::IDENTITY;

        mDummyNode = new SceneNode( mDummyTransformPtrs );
    }
    //-----------------------------------------------------------------------------------
    NodeMemoryManager::~NodeMemoryManager()
    {
        ArrayMemoryManagerVec::iterator itor = mMemoryManagers.begin();
        ArrayMemoryManagerVec::iterator end  = mMemoryManagers.end();

        while( itor != end )
        {
            itor->destroy();
            ++itor;
        }

        mMemoryManagers.clear();

        delete mDummyNode;
        mDummyNode = 0;

        /*OGRE_FREE_SIMD( mDummyTransformPtrs.mPosition, MEMCATEGORY_SCENE_OBJECTS );
        OGRE_FREE_SIMD( mDummyTransformPtrs.mOrientation, MEMCATEGORY_SCENE_OBJECTS );
        OGRE_FREE_SIMD( mDummyTransformPtrs.mScale, MEMCATEGORY_SCENE_OBJECTS );*/

        OGRE_FREE_SIMD( mDummyTransformPtrs.mDerivedPosition, MEMCATEGORY_SCENE_OBJECTS );
        OGRE_FREE_SIMD( mDummyTransformPtrs.mDerivedOrientation, MEMCATEGORY_SCENE_OBJECTS );
        OGRE_FREE_SIMD( mDummyTransformPtrs.mDerivedScale, MEMCATEGORY_SCENE_OBJECTS );

        OGRE_FREE_SIMD( mDummyTransformPtrs.mDerivedTransform, MEMCATEGORY_SCENE_OBJECTS );
        /*OGRE_FREE_SIMD( mDummyTransformPtrs.mInheritOrientation, MEMCATEGORY_SCENE_OBJECTS );
        OGRE_FREE_SIMD( mDummyTransformPtrs.mInheritScale, MEMCATEGORY_SCENE_OBJECTS );*/
        mDummyTransformPtrs = Transform();
    }
    //-----------------------------------------------------------------------------------
    void NodeMemoryManager::_setTwin( SceneMemoryMgrTypes memoryManagerType,
                                        NodeMemoryManager *twinMemoryManager )
    {
        mMemoryManagerType = memoryManagerType;
        mTwinMemoryManager = twinMemoryManager;
    }
    //-----------------------------------------------------------------------------------
    void NodeMemoryManager::growToDepth( size_t newDepth )
    {
        //TODO: (dark_sylinc) give a specialized hint for each depth
        while( newDepth >= mMemoryManagers.size() )
        {
            mMemoryManagers.push_back( NodeArrayMemoryManager( mMemoryManagers.size(), 100,
                                                                mDummyNode, 100,
                                                                ArrayMemoryManager::MAX_MEMORY_SLOTS,
                                                                this ) );
            mMemoryManagers.back().initialize();
        }
    }
    //-----------------------------------------------------------------------------------
    void NodeMemoryManager::nodeCreated( Transform &outTransform, size_t depth )
    {
        growToDepth( depth );

        NodeArrayMemoryManager& mgr = mMemoryManagers[depth];
        mgr.createNewNode( outTransform );
    }
    //-----------------------------------------------------------------------------------
    void NodeMemoryManager::nodeAttached( Transform &outTransform, size_t depth )
    {
        this->nodeMoved( outTransform, 0, depth );
    }
    //-----------------------------------------------------------------------------------
    void NodeMemoryManager::nodeDettached( Transform &outTransform, size_t depth )
    {
        Transform tmp;
        mMemoryManagers[0].createNewNode( tmp );

        tmp.copy( outTransform );
        tmp.mParents[tmp.mIndex] = mDummyNode;

        NodeArrayMemoryManager &mgr = mMemoryManagers[depth];
        mgr.destroyNode( outTransform );

        outTransform = tmp;
    }
    //-----------------------------------------------------------------------------------
    void NodeMemoryManager::nodeDestroyed( Transform &outTransform, size_t depth )
    {
        NodeArrayMemoryManager &mgr = mMemoryManagers[depth];
        mgr.destroyNode( outTransform );
    }
    //-----------------------------------------------------------------------------------
    void NodeMemoryManager::nodeMoved( Transform &inOutTransform, size_t oldDepth, size_t newDepth )
    {
        growToDepth( newDepth );

        Transform tmp;
        mMemoryManagers[newDepth].createNewNode( tmp );

        tmp.copy( inOutTransform );

        NodeArrayMemoryManager &mgr = mMemoryManagers[oldDepth];
        mgr.destroyNode( inOutTransform );

        inOutTransform = tmp;
    }
    //-----------------------------------------------------------------------------------
    void NodeMemoryManager::migrateTo( Transform &inOutTransform, size_t depth,
                                        NodeMemoryManager *dstNodeMemoryManager )
    {
        migrateTo( inOutTransform, depth, depth, dstNodeMemoryManager );
    }
    //-----------------------------------------------------------------------------------
    void NodeMemoryManager::migrateTo( Transform &inOutTransform, size_t oldDepth, size_t newDepth,
                                       NodeMemoryManager *dstNodeMemoryManager )
    {
        assert( (newDepth == oldDepth || newDepth != 0) &&
                "When newDepth = 0, oldDepth must be 0 too!" );

        Transform tmp;
        dstNodeMemoryManager->nodeCreated( tmp, newDepth );
        tmp.copy( inOutTransform );
        if( newDepth == 0 )
            tmp.mParents[tmp.mIndex] = dstNodeMemoryManager->mDummyNode;
        this->nodeDestroyed( inOutTransform, oldDepth );
        inOutTransform = tmp;
    }
    //-----------------------------------------------------------------------------------
    void NodeMemoryManager::migrateToAndAttach( Transform &inOutTransform, size_t depth,
                                                NodeMemoryManager *dstNodeMemoryManager )
    {
        migrateTo( inOutTransform, 0, depth, dstNodeMemoryManager );
    }
    //-----------------------------------------------------------------------------------
    void NodeMemoryManager::migrateToAndDetach( Transform &outTransform, size_t depth,
                                                NodeMemoryManager *dstNodeMemoryManager )
    {
        Transform tmp;
        dstNodeMemoryManager->mMemoryManagers[0].createNewNode( tmp );

        tmp.copy( outTransform );
        tmp.mParents[tmp.mIndex] = dstNodeMemoryManager->mDummyNode;

        NodeArrayMemoryManager &mgr = mMemoryManagers[depth];
        mgr.destroyNode( outTransform );

        outTransform = tmp;
    }
    //-----------------------------------------------------------------------------------
    size_t NodeMemoryManager::getNumDepths() const
    {
        size_t retVal = -1;
        ArrayMemoryManagerVec::const_iterator begin= mMemoryManagers.begin();
        ArrayMemoryManagerVec::const_iterator itor = mMemoryManagers.begin();
        ArrayMemoryManagerVec::const_iterator end  = mMemoryManagers.end();

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
        return mMemoryManagers[depth].getFirstNode( outTransform );
    }
    //-----------------------------------------------------------------------------------
    void NodeMemoryManager::buildDiffList( uint16 level, const MemoryPoolVec &basePtrs,
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
    void NodeMemoryManager::applyRebase( uint16 level, const MemoryPoolVec &newBasePtrs,
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
                    transform.mOwner[j]->_callMemoryChangeListeners();
                }
            }

            transform.advancePack();
        }
    }
    //---------------------------------------------------------------------
    void NodeMemoryManager::performCleanup( uint16 level, const MemoryPoolVec &basePtrs,
                                            size_t const *elementsMemSizes,
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
                    transform.mOwner[j]->_callMemoryChangeListeners();
                }
            }

            transform.advancePack();
        }
    }
}
