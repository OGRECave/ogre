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

#include "Math/Array/OgreObjectMemoryManager.h"

#include "OgreMovableObject.h"

namespace Ogre
{
    ObjectMemoryManager::ObjectMemoryManager() :
            mTotalObjects( 0 ),
            mDummyNode( 0 ),
            mDummyObject( 0 ),
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
        /*mDummyTransformPtrs.mInheritOrientation= OGRE_MALLOC_SIMD( sizeof( bool ) * ARRAY_PACKED_REALS,
                                                                    MEMCATEGORY_SCENE_OBJECTS );
        mDummyTransformPtrs.mInheritScale       = OGRE_MALLOC_SIMD( sizeof( bool ) * ARRAY_PACKED_REALS,
                                                                    MEMCATEGORY_SCENE_OBJECTS );*/

        *mDummyTransformPtrs.mDerivedPosition       = ArrayVector3::ZERO;
        *mDummyTransformPtrs.mDerivedOrientation    = ArrayQuaternion::IDENTITY;
        *mDummyTransformPtrs.mDerivedScale          = ArrayVector3::UNIT_SCALE;
        for( size_t i=0; i<ARRAY_PACKED_REALS; ++i )
            mDummyTransformPtrs.mDerivedTransform[i] = Matrix4::IDENTITY;

        mDummyNode = new SceneNode( mDummyTransformPtrs );
        mDummyObject = new NullEntity();
    }
    //-----------------------------------------------------------------------------------
    ObjectMemoryManager::~ObjectMemoryManager()
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

        delete mDummyObject;
        mDummyObject = 0;

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
    void ObjectMemoryManager::_setTwin( SceneMemoryMgrTypes memoryManagerType,
                                        ObjectMemoryManager *twinMemoryManager )
    {
        mMemoryManagerType = memoryManagerType;
        mTwinMemoryManager = twinMemoryManager;
    }
    //-----------------------------------------------------------------------------------
    void ObjectMemoryManager::growToDepth( size_t newDepth )
    {
        //TODO: (dark_sylinc) give a specialized hint for each depth
        while( newDepth >= mMemoryManagers.size() )
        {
            mMemoryManagers.push_back( ObjectDataArrayMemoryManager( mMemoryManagers.size(), 100,
                                            mDummyNode, mDummyObject, 100,
                                            ArrayMemoryManager::MAX_MEMORY_SLOTS, this ) );
            mMemoryManagers.back().initialize();
        }
    }
    //-----------------------------------------------------------------------------------
    void ObjectMemoryManager::objectCreated( ObjectData &outObjectData, size_t renderQueue )
    {
        growToDepth( renderQueue );

        ObjectDataArrayMemoryManager& mgr = mMemoryManagers[renderQueue];
        mgr.createNewNode( outObjectData );

        ++mTotalObjects;
    }
    //-----------------------------------------------------------------------------------
    void ObjectMemoryManager::objectMoved( ObjectData &inOutObjectData, size_t oldRenderQueue,
                                            size_t newRenderQueue )
    {
        growToDepth( newRenderQueue );

        ObjectData tmp;
        mMemoryManagers[newRenderQueue].createNewNode( tmp );

        tmp.copy( inOutObjectData );

        ObjectDataArrayMemoryManager &mgr = mMemoryManagers[oldRenderQueue];
        mgr.destroyNode( inOutObjectData );

        inOutObjectData = tmp;
    }
    //-----------------------------------------------------------------------------------
    void ObjectMemoryManager::objectDestroyed( ObjectData &outObjectData, size_t renderQueue )
    {
        ObjectDataArrayMemoryManager &mgr = mMemoryManagers[renderQueue];
        mgr.destroyNode( outObjectData );

        --mTotalObjects;
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
    size_t ObjectMemoryManager::calculateTotalNumObjectDataIncludingFragmentedSlots() const
    {
        size_t retVal = 0;

        ArrayMemoryManagerVec::const_iterator itor = mMemoryManagers.begin();
        ArrayMemoryManagerVec::const_iterator end  = mMemoryManagers.end();

        while( itor != end )
        {
            retVal += itor->getNumUsedSlotsIncludingFragmented();
            ++itor;
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    size_t ObjectMemoryManager::getFirstObjectData( ObjectData &outObjectData, size_t renderQueue )
    {
        return mMemoryManagers[renderQueue].getFirstNode( outObjectData );
    }
    //-----------------------------------------------------------------------------------
    void ObjectMemoryManager::buildDiffList( uint16 level, const MemoryPoolVec &basePtrs,
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
    void ObjectMemoryManager::applyRebase( uint16 level, const MemoryPoolVec &newBasePtrs,
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
    void ObjectMemoryManager::performCleanup( uint16 level, const MemoryPoolVec &basePtrs,
                                              size_t const *elementsMemSizes,
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
