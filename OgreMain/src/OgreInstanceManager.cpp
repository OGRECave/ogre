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
#include "OgreInstanceManager.h"
#include "OgreInstanceBatchHW.h"
#include "OgreInstanceBatchHW_VTF.h"
#include "OgreInstanceBatchShader.h"
#include "OgreInstanceBatchVTF.h"
#include "OgreMesh.h"
#include "OgreSubMesh.h"
#include "OgreMeshManager.h"
#include "OgreMaterialManager.h"
#include "OgreSceneManager.h"
#include "OgreHardwareBufferManager.h"
#include "OgreSceneNode.h"
#include "OgreIteratorWrappers.h"

namespace Ogre
{
namespace v1
{
    InstanceManager::InstanceManager( IdString customName, SceneManager *sceneManager,
                                        const String &meshName, const String &groupName,
                                        InstancingTechnique instancingTechnique, uint16 instancingFlags,
                                        size_t instancesPerBatch, unsigned short subMeshIdx,
                                        bool useBoneMatrixLookup ) :
                mName( customName ),
#if OGRE_DEBUG_MODE
                mIdCount( 0 ),
#endif
                mInstancesPerBatch( instancesPerBatch ),
                mInstancingTechnique( instancingTechnique ),
                mInstancingFlags( instancingFlags ),
                mSubMeshIdx( subMeshIdx ),
                mSceneManager( sceneManager ),
                mMaxLookupTableInstances(16),
                mNumCustomParams( 0 )
    {
        mMeshReference = MeshManager::getSingleton().load( meshName, groupName );

        if(mMeshReference->sharedVertexData[VpNormal])
            MeshManager::unshareVertices(mMeshReference.get());

        if( mMeshReference->hasSkeleton() && !mMeshReference->getSkeleton().isNull() )
            mMeshReference->getSubMesh(mSubMeshIdx)->_compileBoneAssignments();
    }
                
    InstanceManager::~InstanceManager()
    {
        //Remove all batches from all materials we created
        InstanceBatchMap::const_iterator itor = mInstanceBatches.begin();
        InstanceBatchMap::const_iterator end  = mInstanceBatches.end();

        while( itor != end )
        {
            InstanceBatchVec::const_iterator it = itor->second.begin();
            InstanceBatchVec::const_iterator en = itor->second.end();

            while( it != en )
                OGRE_DELETE *it++;

            ++itor;
        }
    }
    //----------------------------------------------------------------------
    void InstanceManager::setInstancesPerBatch( size_t instancesPerBatch )
    {
        if( !mInstanceBatches.empty() )
        {
            OGRE_EXCEPT(Exception::ERR_INVALID_STATE, "Instances per batch can only be changed before"
                        " building the batch.", "InstanceManager::setInstancesPerBatch");
        }

        mInstancesPerBatch = instancesPerBatch;
    }

    //----------------------------------------------------------------------
    void InstanceManager::setMaxLookupTableInstances( size_t maxLookupTableInstances )
    {
        if( !mInstanceBatches.empty() )
        {
            OGRE_EXCEPT(Exception::ERR_INVALID_STATE, "Instances per batch can only be changed before"
                " building the batch.", "InstanceManager::setMaxLookupTableInstances");
        }

        mMaxLookupTableInstances = maxLookupTableInstances;
    }
    
    //----------------------------------------------------------------------
    void InstanceManager::setNumCustomParams( unsigned char numCustomParams )
    {
        if( !mInstanceBatches.empty() )
        {
            OGRE_EXCEPT(Exception::ERR_INVALID_STATE, "setNumCustomParams can only be changed before"
                " building the batch.", "InstanceManager::setNumCustomParams");
        }

        mNumCustomParams = numCustomParams;
    }
    //----------------------------------------------------------------------
    size_t InstanceManager::getMaxOrBestNumInstancesPerBatch( const String &materialName,
                                                                size_t suggestedSize, uint16 flags )
    {
        //Get the material
        MaterialPtr mat = MaterialManager::getSingleton().getByName( materialName,
                                                                    mMeshReference->getGroup() );
        InstanceBatch *batch = 0;

        //Base material couldn't be found
        if( mat.isNull() )
            return 0;

        switch( mInstancingTechnique )
        {
        case ShaderBased:
            batch = OGRE_NEW InstanceBatchShader( -1, &mSceneManager->_getEntityMemoryManager( SCENE_DYNAMIC ),
                                                    this, mMeshReference, mat, suggestedSize, 0 );
            break;
        case TextureVTF:
            batch = OGRE_NEW InstanceBatchVTF( -1, &mSceneManager->_getEntityMemoryManager( SCENE_DYNAMIC ), this,
                                                mMeshReference, mat, suggestedSize, 0 );
            static_cast<InstanceBatchVTF*>(batch)->setBoneDualQuaternions((mInstancingFlags & IM_USEBONEDUALQUATERNIONS) != 0);
            static_cast<InstanceBatchVTF*>(batch)->setUseOneWeight((mInstancingFlags & IM_USEONEWEIGHT) != 0);
            static_cast<InstanceBatchVTF*>(batch)->setForceOneWeight((mInstancingFlags & IM_FORCEONEWEIGHT) != 0);
            break;
        case HWInstancingBasic:
            batch = OGRE_NEW InstanceBatchHW( -1, &mSceneManager->_getEntityMemoryManager( SCENE_DYNAMIC ), this,
                                                mMeshReference, mat, suggestedSize, 0 );
            break;
        case HWInstancingVTF:
            batch = OGRE_NEW InstanceBatchHW_VTF( -1, &mSceneManager->_getEntityMemoryManager( SCENE_DYNAMIC ), this,
                                                    mMeshReference, mat, suggestedSize, 0 );
            static_cast<InstanceBatchHW_VTF*>(batch)->setBoneMatrixLookup((mInstancingFlags & IM_VTFBONEMATRIXLOOKUP) != 0, mMaxLookupTableInstances);
            static_cast<InstanceBatchHW_VTF*>(batch)->setBoneDualQuaternions((mInstancingFlags & IM_USEBONEDUALQUATERNIONS) != 0);
            static_cast<InstanceBatchHW_VTF*>(batch)->setUseOneWeight((mInstancingFlags & IM_USEONEWEIGHT) != 0);
            static_cast<InstanceBatchHW_VTF*>(batch)->setForceOneWeight((mInstancingFlags & IM_FORCEONEWEIGHT) != 0);
            break;
        default:
            OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
                    "Unimplemented instancing technique: " +
                    StringConverter::toString(mInstancingTechnique),
                    "InstanceBatch::getMaxOrBestNumInstancesPerBatches()");
        }

        const size_t retVal = batch->calculateMaxNumInstances( mMeshReference->getSubMesh(mSubMeshIdx),
                                                                flags );

        OGRE_DELETE batch;

        return retVal;
    }
    //----------------------------------------------------------------------
    InstancedEntity* InstanceManager::createInstancedEntity( const String &materialName,
                                                             SceneMemoryMgrTypes sceneType )
    {
        InstanceBatch *instanceBatch;

        if( mInstanceBatches.empty() )
            instanceBatch = buildNewBatch( materialName, sceneType, true );
        else
            instanceBatch = getFreeBatch( materialName, sceneType );

        return instanceBatch->createInstancedEntity();
    }
    //-----------------------------------------------------------------------
    inline InstanceBatch* InstanceManager::getFreeBatch( const String &materialName,
                                                         SceneMemoryMgrTypes sceneType )
    {
        IdString materialHash( materialName + StringConverter::toString( sceneType ) );
        InstanceBatchVec &batchVec = mInstanceBatches[materialHash];

        InstanceBatchVec::const_reverse_iterator itor = batchVec.rbegin();
        InstanceBatchVec::const_reverse_iterator end  = batchVec.rend();

        while( itor != end )
        {
            if( !(*itor)->isBatchFull() )
                return *itor;
            ++itor;
        }

        //None found, or they're all full
        return buildNewBatch( materialName, sceneType, false );
    }
    //-----------------------------------------------------------------------
    InstanceBatch* InstanceManager::buildNewBatch( const String &materialName,
                                                    SceneMemoryMgrTypes sceneType, bool firstTime )
    {
        IdString materialHashGeneric( materialName );
        IdString materialHash( materialName + StringConverter::toString( sceneType ) );

        //Get the bone to index map for the batches
        Mesh::IndexMap &idxMap = mMeshReference->getSubMesh(mSubMeshIdx)->blendIndexToBoneIndexMap;
        idxMap = idxMap.empty() ? mMeshReference->sharedBlendIndexToBoneIndexMap : idxMap;

        //Get the material
        MaterialPtr mat = MaterialManager::getSingleton().getByName( materialName,
                                                                    mMeshReference->getGroup() );

        //Get the array of batches grouped by this material
        InstanceBatchVec &materialInstanceBatch = mInstanceBatches[materialHash];

        InstanceBatch *batch = 0;

        switch( mInstancingTechnique )
        {
        case ShaderBased:
            batch = OGRE_NEW InstanceBatchShader( Id::generateNewId<InstanceBatch>(),
                                                    &mSceneManager->_getEntityMemoryManager(sceneType),
                                                    this, mMeshReference, mat, mInstancesPerBatch,
                                                    &idxMap );
            break;
        case TextureVTF:
            batch = OGRE_NEW InstanceBatchVTF( Id::generateNewId<InstanceBatch>(),
                                                    &mSceneManager->_getEntityMemoryManager(sceneType),
                                                    this, mMeshReference, mat, mInstancesPerBatch,
                                                    &idxMap );
            static_cast<InstanceBatchVTF*>(batch)->setBoneDualQuaternions((mInstancingFlags & IM_USEBONEDUALQUATERNIONS) != 0);
            static_cast<InstanceBatchVTF*>(batch)->setUseOneWeight((mInstancingFlags & IM_USEONEWEIGHT) != 0);
            static_cast<InstanceBatchVTF*>(batch)->setForceOneWeight((mInstancingFlags & IM_FORCEONEWEIGHT) != 0);
            break;
        case HWInstancingBasic:
            batch = OGRE_NEW InstanceBatchHW( Id::generateNewId<InstanceBatch>(),
                                                    &mSceneManager->_getEntityMemoryManager(sceneType), 
                                                    this, mMeshReference, mat, mInstancesPerBatch,
                                                    &idxMap );
            break;
        case HWInstancingVTF:
            batch = OGRE_NEW InstanceBatchHW_VTF( Id::generateNewId<InstanceBatch>(),
                                                    &mSceneManager->_getEntityMemoryManager(sceneType),
                                                    this, mMeshReference, mat, mInstancesPerBatch,
                                                    &idxMap );
            static_cast<InstanceBatchHW_VTF*>(batch)->setBoneMatrixLookup((mInstancingFlags & IM_VTFBONEMATRIXLOOKUP) != 0, mMaxLookupTableInstances);
            static_cast<InstanceBatchHW_VTF*>(batch)->setBoneDualQuaternions((mInstancingFlags & IM_USEBONEDUALQUATERNIONS) != 0);
            static_cast<InstanceBatchHW_VTF*>(batch)->setUseOneWeight((mInstancingFlags & IM_USEONEWEIGHT) != 0);
            static_cast<InstanceBatchHW_VTF*>(batch)->setForceOneWeight((mInstancingFlags & IM_FORCEONEWEIGHT) != 0);
            break;
        default:
            OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
                    "Unimplemented instancing technique: " +
                    StringConverter::toString(mInstancingTechnique),
                    "InstanceBatch::buildNewBatch()");
        }

#if OGRE_DEBUG_MODE
        batch->setName( mName.getFriendlyText() + "/InstanceBatch_" +
                            StringConverter::toString(mIdCount++) );
#endif

        batch->_notifyManager( mSceneManager );

        if( !firstTime )
        {
            //TODO: Check different materials have the same mInstancesPerBatch upper limit
            //otherwise we can't share
            batch->buildFrom( mMeshReference->getSubMesh(mSubMeshIdx), mSharedRenderOperation );
        }
        else
        {
            //Ensure we don't request more than we can
            const size_t maxInstPerBatch = batch->calculateMaxNumInstances( mMeshReference->
                                                        getSubMesh(mSubMeshIdx), mInstancingFlags );
            mInstancesPerBatch = std::min( maxInstPerBatch, mInstancesPerBatch );
            batch->_setInstancesPerBatch( mInstancesPerBatch );

            //TODO: Create a "merge" function that merges all submeshes into one big submesh
            //instead of just sending submesh #0

            //Get the RenderOperation to be shared with further instances.
            mSharedRenderOperation = batch->build( mMeshReference->getSubMesh(mSubMeshIdx) );
        }

        const BatchSettings &batchSettings = mBatchSettings[materialHashGeneric];
        batch->setCastShadows( batchSettings.setting[CAST_SHADOWS] );

        batch->setStatic( sceneType == SCENE_STATIC );

        materialInstanceBatch.push_back( batch );

        return batch;
    }
    //-----------------------------------------------------------------------
    void InstanceManager::cleanupEmptyBatches(void)
    {
        //Do this now to avoid any dangling pointer inside mDirtyBatches
        _updateDirtyBatches();

        InstanceBatchMap::iterator itor = mInstanceBatches.begin();
        InstanceBatchMap::iterator end  = mInstanceBatches.end();

        while( itor != end )
        {
            InstanceBatchVec::iterator it = itor->second.begin();
            InstanceBatchVec::iterator en = itor->second.end();

            while( it != en )
            {
                if( (*it)->isBatchUnused() )
                {
                    OGRE_DELETE *it;
                    //Remove it from the list swapping with the last element and popping back
                    size_t idx = it - itor->second.begin();
                    *it = itor->second.back();
                    itor->second.pop_back();

                    //Restore invalidated iterators
                    it = itor->second.begin() + idx;
                    en = itor->second.end();
                }
                else
                    ++it;
            }

            ++itor;
        }

        //By this point it may happen that all mInstanceBatches' objects are also empty
        //however if we call mInstanceBatches.clear(), next time we'll create an InstancedObject
        //we'll end up calling buildFirstTime() instead of buildNewBatch(), which is not the idea
        //(takes more time and will leak the shared render operation)
    }
    //-----------------------------------------------------------------------
    void InstanceManager::defragmentBatches( bool optimizeCull,
                                                InstanceBatch::InstancedEntityVec &usedEntities,
                                                InstanceBatch::CustomParamsVec &usedParams,
                                                InstanceBatchVec &fragmentedBatches )
    {
        InstanceBatchVec::iterator itor = fragmentedBatches.begin();
        InstanceBatchVec::iterator end  = fragmentedBatches.end();

        while( itor != end && !usedEntities.empty() )
        {
            if( !(*itor)->isStatic() )
                (*itor)->_defragmentBatch( optimizeCull, usedEntities, usedParams );
            ++itor;
        }

        InstanceBatchVec::iterator lastImportantBatch = itor;

        while( itor != end )
        {
            if( !(*itor)->isStatic() )
            {
                //If we get here, this means we hit remaining batches which will be unused.
                //Destroy them
                //Call this to avoid freeing InstancedEntities that were just reparented
                (*itor)->_defragmentBatchDiscard();
                OGRE_DELETE *itor;
            }
            else
            {
                //This isn't a meaningless batch, move it forward so it doesn't get wipe
                //when we resize the container (faster than removing element by element)
                *lastImportantBatch++ = *itor;
            }

            ++itor;
        }

        //Remove remaining batches all at once from the vector
        const size_t remainingBatches = end - lastImportantBatch;
        fragmentedBatches.resize( fragmentedBatches.size() - remainingBatches );
    }
    //-----------------------------------------------------------------------
    void InstanceManager::defragmentBatches( bool optimizeCulling )
    {
        //Do this now to avoid any dangling pointer inside mDirtyBatches
        _updateDirtyBatches();

        //Do this for every material
        InstanceBatchMap::iterator itor = mInstanceBatches.begin();
        InstanceBatchMap::iterator end  = mInstanceBatches.end();

        while( itor != end )
        {
            InstanceBatch::InstancedEntityVec   usedEntities;
            InstanceBatch::CustomParamsVec      usedParams;
            usedEntities.reserve( itor->second.size() * mInstancesPerBatch );

            //Collect all Instanced Entities being used by _all_ batches from this material
            InstanceBatchVec::iterator it = itor->second.begin();
            InstanceBatchVec::iterator en = itor->second.end();

            while( it != en )
            {
                //Don't collect instances from static batches, we assume they're correctly set
                //Plus, we don't want to put InstancedEntities from non-static into static batches
                if( !(*it)->isStatic() )
                    (*it)->getInstancedEntitiesInUse( usedEntities, usedParams );
                ++it;
            }

            defragmentBatches( optimizeCulling, usedEntities, usedParams, itor->second );

            ++itor;
        }
    }
    //-----------------------------------------------------------------------
    void InstanceManager::setSetting( BatchSettingId id, bool value, IdString materialName )
    {
        assert( id < NUM_SETTINGS );

        if( materialName == IdString() )
        {
            //Setup all existing materials
            InstanceBatchMap::iterator itor = mInstanceBatches.begin();
            InstanceBatchMap::iterator end  = mInstanceBatches.end();

            while( itor != end )
            {
                mBatchSettings[itor->first].setting[id] = value;
                applySettingToBatches( id, value, itor->second );

                ++itor;
            }
        }
        else
        {
            //Setup a given material
            mBatchSettings[materialName].setting[id] = value;

            InstanceBatchMap::const_iterator itor = mInstanceBatches.find( materialName );
            //Don't crash or throw if the batch with that material hasn't been created yet
            if( itor != mInstanceBatches.end() )
                applySettingToBatches( id, value, itor->second );
        }
    }
    //-----------------------------------------------------------------------
    bool InstanceManager::getSetting( BatchSettingId id, IdString materialName ) const
    {
        assert( id < NUM_SETTINGS );

        BatchSettingsMap::const_iterator itor = mBatchSettings.find( materialName );
        if( itor != mBatchSettings.end() )
            return itor->second.setting[id]; //Return current setting

        //Return default
        return BatchSettings().setting[id];
    }
    //-----------------------------------------------------------------------
    void InstanceManager::applySettingToBatches( BatchSettingId id, bool value,
                                                 const InstanceBatchVec &container )
    {
        InstanceBatchVec::const_iterator itor = container.begin();
        InstanceBatchVec::const_iterator end  = container.end();

        while( itor != end )
        {
            switch( id )
            {
            case CAST_SHADOWS:
                (*itor)->setCastShadows( value );
                break;
            case SHOW_BOUNDINGBOX:
                //(*itor)->getParentSceneNode()->showBoundingBox( value );
                break;
            default:
                break;
            }
            ++itor;
        }
    }
    //-----------------------------------------------------------------------
    void InstanceManager::setBatchesAsStatic( bool bStatic )
    {
        InstanceBatchMap::iterator itor = mInstanceBatches.begin();
        InstanceBatchMap::iterator end  = mInstanceBatches.end();

        while( itor != end )
        {
            InstanceBatchVec::iterator it = itor->second.begin();
            InstanceBatchVec::iterator en = itor->second.end();

            while( it != en )
            {
                (*it)->setStatic( bStatic );
                ++it;
            }

            ++itor;
        }
    }
    //-----------------------------------------------------------------------
    void InstanceManager::_addToDynamicBatchList( InstanceBatch *dynamicBatch )
    {
        InstanceBatchVec::iterator itor = std::find( mDynamicBatches.begin(), mDynamicBatches.end(),
                                                     dynamicBatch );
        if( itor == mDynamicBatches.end() )
            mDynamicBatches.push_back( dynamicBatch );
    }
    //-----------------------------------------------------------------------
    void InstanceManager::_removeFromDynamicBatchList( InstanceBatch *batch )
    {
        InstanceBatchVec::iterator itor = std::find( mDynamicBatches.begin(), mDynamicBatches.end(),
                                                     batch );
        if( itor != mDynamicBatches.end() )
            efficientVectorRemove( mDynamicBatches, itor );
    }
    //-----------------------------------------------------------------------
    void InstanceManager::_addDirtyStaticBatch( InstanceBatch *dirtyBatch )
    {
        //If he needs to this very often, they're probably not static...
        //Note: Calling this more often will only affect performance for the next frame.
        //It won't crash and can be ignored
        assert( std::find( mDirtyStaticBatches.begin(), mDirtyStaticBatches.end(), dirtyBatch )
                == mDirtyStaticBatches.end() && "Only flag as dirty static batches once!" );

        mDirtyStaticBatches.push_back( dirtyBatch );
    }
    //-----------------------------------------------------------------------
#ifdef OGRE_LEGACY_ANIMATIONS
    void InstanceManager::_updateAnimations(void)
    {
        InstanceBatchVec::const_iterator itor = mDynamicBatches.begin();
        InstanceBatchVec::const_iterator end  = mDynamicBatches.end();

        while( itor != end )
        {
            (*itor)->_updateAnimations();
            ++itor;
        }

        itor = mDirtyStaticBatches.begin();
        end  = mDirtyStaticBatches.end();

        while( itor != end )
        {
            (*itor)->_updateAnimations();
            ++itor;
        }

        //_updateDirtyBatches will be called after us, and will do that job.
        //mDirtyStaticBatches.clear();
    }
#endif
    //-----------------------------------------------------------------------
    void InstanceManager::_updateDirtyBatchesThread( size_t threadIdx )
    {
        InstanceBatchVec::const_iterator itor = mDynamicBatches.begin();
        InstanceBatchVec::const_iterator end  = mDynamicBatches.end();

        while( itor != end )
        {
            (*itor)->_updateEntitiesBoundsThread( threadIdx );
            ++itor;
        }

        itor = mDirtyStaticBatches.begin();
        end  = mDirtyStaticBatches.end();

        while( itor != end )
        {
            (*itor)->_updateEntitiesBoundsThread( threadIdx );
            ++itor;
        }
    }
    //-----------------------------------------------------------------------
    void InstanceManager::_updateDirtyBatches(void)
    {
        InstanceBatchVec::const_iterator itor = mDynamicBatches.begin();
        InstanceBatchVec::const_iterator end  = mDynamicBatches.end();

        while( itor != end )
        {
            (*itor)->_updateBounds();
            ++itor;
        }

        itor = mDirtyStaticBatches.begin();
        end  = mDirtyStaticBatches.end();

        while( itor != end )
        {
            (*itor)->_updateBounds();
            ++itor;
        }

        mDirtyStaticBatches.clear();
    }
    //-----------------------------------------------------------------------
    InstanceManager::InstanceBatchIterator InstanceManager::getInstanceBatchIterator(
                                        const String &materialName, SceneMemoryMgrTypes sceneType ) const
    {
        IdString materialHash( materialName + StringConverter::toString( sceneType ) );
        InstanceBatchMap::const_iterator it = mInstanceBatches.find( materialHash );
        if( it == mInstanceBatches.end() )
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND,
                        "Can't find instance batches with material '" + materialName + "'",
                        "InstanceManager::getInstanceBatchIterator");
        }
        return InstanceBatchIterator( it->second.begin(), it->second.end() );
    }
}
}
