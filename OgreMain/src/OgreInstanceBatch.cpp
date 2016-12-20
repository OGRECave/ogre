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
#include "OgreInstanceBatch.h"
#include "OgreSubMesh.h"
#include "OgreInstancedEntity.h"
#include "OgreSceneNode.h"
#include "OgreSceneManager.h"
#include "OgreCamera.h"
#include "OgreException.h"
#include "OgreRenderQueue.h"

namespace Ogre
{
    using namespace VisibilityFlags;
namespace v1
{
    InstanceBatch::InstanceBatch( IdType id, ObjectMemoryManager *objectMemoryManager,
                                    InstanceManager *creator, MeshPtr &meshReference,
                                    const MaterialPtr &material, size_t instancesPerBatch,
                                    const Mesh::IndexMap *indexToBoneMap ) :
                Renderable(),
                MovableObject( id, objectMemoryManager, (SceneManager*)0, 1 ),
                mInstancesPerBatch( instancesPerBatch ),
                mCreator( creator ),
                mMeshReference( meshReference ),
                mIndexToBoneMap( indexToBoneMap ),
                mTechnSupportsSkeletal( SKELETONS_SUPPORTED ),
                mCachedCamera( 0 ),
                mTransformSharingDirty(true),
                mStaticDirty( false ),
                mRemoveOwnVertexData(false),
                mRemoveOwnIndexData(false)
    {
        assert( mInstancesPerBatch );

        mThreadAabbs.resize( mCreator->getSceneManager()->getNumWorkerThreads(), Aabb::BOX_INFINITE );

        //No twin, but we have the same scene type as our creator
        mLocalObjectMemoryManager._setTwin( mObjectMemoryManager->getMemoryManagerType(), 0 );

        //Force batch visibility to be always visible. The instanced entities
        //have individual visibility flags. If none matches the scene's current,
        //then this batch won't rendered.
        setVisibilityFlags( std::numeric_limits<Ogre::uint32>::max() );

        if( indexToBoneMap )
        {
            assert( !(meshReference->hasSkeleton() && indexToBoneMap->empty()) );
        }

        this->setMaterial( material );
        mLodMesh = meshReference->_getLodValueArray();
        mLodMaterial = material->_getLodValues();
        mRenderables.resize( 1, this );

        mCustomParams.resize( mCreator->getNumCustomParams() * mInstancesPerBatch, Ogre::Vector4::ZERO );

        //We need some sort of node so that some MovableObject functions don't crash.
        //We don't conventionally attach the object. The dummy node doesn't know we're
        //attached to it. InstanceBatch could be nodeless with a very large refactor.
        //It's the InstancedEntity(s) that need a Node. The user doesn't need access
        //to the batch's node (Aka. hack)
        //We used to use the Root node, but doesn't play nice with SceneManager::setRelativeOrigin
        //(the offset is applied twice, but only the AABB is displaced, causing culling errors)
        mParentNode = mObjectData.mParents[mObjectData.mIndex];
        setVisible( true );
    }

    InstanceBatch::~InstanceBatch()
    {
        deleteAllInstancedEntities();

        //Remove the hacked parent scene node before MovableObject's destructor kicks in
        mParentNode = 0;

        if( mRemoveOwnVertexData )
            OGRE_DELETE mRenderOperation.vertexData;
        if( mRemoveOwnIndexData )
            OGRE_DELETE mRenderOperation.indexData;
    }

    void InstanceBatch::_setInstancesPerBatch( size_t instancesPerBatch )
    {
        if( !mInstancedEntities.empty() )
        {
            OGRE_EXCEPT(Exception::ERR_INVALID_STATE, "Instances per batch can only be changed before"
                        " building the batch.", "InstanceBatch::_setInstancesPerBatch");
        }

        mInstancesPerBatch = instancesPerBatch;
    }
    //-----------------------------------------------------------------------
    bool InstanceBatch::checkSubMeshCompatibility( const SubMesh* baseSubMesh )
    {
        if( baseSubMesh->operationType != OT_TRIANGLE_LIST )
        {
            OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Only meshes with OT_TRIANGLE_LIST are supported",
                        "InstanceBatch::checkSubMeshCompatibility");
        }

        if( !mCustomParams.empty() && mCreator->getInstancingTechnique() != InstanceManager::HWInstancingBasic )
        {
            //Implementing this for ShaderBased is impossible. All other variants can be.
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Custom parameters not supported for this "
                                                        "technique. Do you dare implementing it?"
                                                        "See InstanceManager::setNumCustomParams "
                                                        "documentation.",
                        "InstanceBatch::checkSubMeshCompatibility");
        }

        return true;
    }
    //-----------------------------------------------------------------------
#ifdef OGRE_LEGACY_ANIMATIONS
    void InstanceBatch::_updateAnimations(void)
    {
        InstancedEntityArray::const_iterator itor = mAnimatedEntities.begin();
        InstancedEntityArray::const_iterator end  = mAnimatedEntities.end();

        while( itor != end )
        {
            (*itor)->_updateAnimation();
            ++itor;
        }
    }
#endif
    //-----------------------------------------------------------------------
    void InstanceBatch::_updateEntitiesBoundsThread( size_t threadIdx )
    {
        const size_t numWorkerThreads = mThreadAabbs.size();

        //Update all bounds from our objects (our share only)
        ObjectData objData;
        const size_t totalObjs = mLocalObjectMemoryManager.getFirstObjectData( objData, 0 );

        //Distribute the work evenly across all threads (not perfect), taking into
        //account we need to distribute in multiples of ARRAY_PACKED_REALS
        size_t numObjs  = ( totalObjs + (numWorkerThreads-1) ) / numWorkerThreads;
        numObjs         = ( (numObjs + ARRAY_PACKED_REALS - 1) / ARRAY_PACKED_REALS ) *
                            ARRAY_PACKED_REALS;

        const size_t toAdvance = std::min( threadIdx * numObjs, totalObjs );

        //Prevent going out of bounds (usually in the last threadIdx, or
        //when there are less entities than ARRAY_PACKED_REALS
        numObjs = std::min( numObjs, totalObjs - toAdvance );
        objData.advancePack( toAdvance / ARRAY_PACKED_REALS );

        MovableObject::updateAllBounds( numObjs, objData );

        //Now merge the bounds to ours
        //ArrayReal maxWorldRadius = ARRAY_REAL_ZERO;
        ArrayVector3 vMinBounds( Mathlib::MAX_POS, Mathlib::MAX_POS, Mathlib::MAX_POS );
        ArrayVector3 vMaxBounds( Mathlib::MAX_NEG, Mathlib::MAX_NEG, Mathlib::MAX_NEG );

        for( size_t i=0; i<numObjs; i += ARRAY_PACKED_REALS )
        {
            ArrayInt * RESTRICT_ALIAS visibilityFlags = reinterpret_cast<ArrayInt*RESTRICT_ALIAS>
                                                                        (objData.mVisibilityFlags);
            ArrayMaskR inUse = CastIntToReal(Mathlib::TestFlags4( *visibilityFlags,
                                                                 Mathlib::SetAll( LAYER_VISIBILITY ) ));

            //Merge with bounds only if they're in use (and not explicitly hidden,
            //but may be invisible for some cameras or out of frustum)
            ArrayVector3 oldVal( vMinBounds );
            vMinBounds.makeFloor( objData.mWorldAabb->mCenter - objData.mWorldAabb->mHalfSize );
            vMinBounds.CmovRobust( inUse, oldVal );

            oldVal = vMaxBounds;
            vMaxBounds.makeCeil( objData.mWorldAabb->mCenter + objData.mWorldAabb->mHalfSize );
            vMaxBounds.CmovRobust( inUse, oldVal );

            //maxWorldRadius = Mathlib::Max( maxWorldRadius, *worldRadius );

            objData.advanceDirtyInstanceMgr();
        }

        //We've been merging and processing in bulks, but we now need to join all simd results
        //Real maxRadius = Mathlib::CollapseMax( maxWorldRadius );
        Vector3 vMin = vMinBounds.collapseMin();
        Vector3 vMax = vMaxBounds.collapseMax();

        //Don't use newFromExtents on purpose because min > max is valid. Because we're
        //threaded, we might've processed a full chunk of non-visible objects.
        //Aabb aabb = Aabb::newFromExtents( vMin - maxRadius, vMax + maxRadius );
        Aabb aabb;
        aabb.mCenter    = (vMax + vMin) * 0.5f;
        aabb.mHalfSize  = (vMax - vMin) * 0.5f;
        mThreadAabbs[threadIdx] = aabb;
    }
    //-----------------------------------------------------------------------
    void InstanceBatch::_updateBounds(void)
    {
        //If this assert triggers, then we did not properly remove ourselves from
        //the Manager's update list (it's a performance optimization warning)
        assert( mUnusedEntities.size() != mInstancedEntities.size() );

        //Collect Aabbs from the multiple threads
        Aabb aabb = Aabb::BOX_NULL;
        vector<Aabb>::type::const_iterator itor = mThreadAabbs.begin();
        vector<Aabb>::type::const_iterator end  = mThreadAabbs.end();
        while( itor != end )
        {
            aabb.merge( *itor );
            ++itor;
        }

        mObjectData.mLocalAabb->setFromAabb( aabb, mObjectData.mIndex );
        mObjectData.mLocalRadius[mObjectData.mIndex] = aabb.getRadius();

        mStaticDirty = false;
    }
    //-----------------------------------------------------------------------
    void InstanceBatch::createAllInstancedEntities()
    {
        mInstancedEntities.reserve( mInstancesPerBatch );
        mUnusedEntities.reserve( mInstancesPerBatch );
#ifdef OGRE_LEGACY_ANIMATIONS
        mAnimatedEntities.reserve( mInstancesPerBatch );
#endif

        for( size_t i=0; i<mInstancesPerBatch; ++i )
        {
            InstancedEntity *instance = generateInstancedEntity(i);
            mInstancedEntities.push_back( instance );
            mUnusedEntities.push_back( instance );
        }
    }
    //-----------------------------------------------------------------------
    InstancedEntity* InstanceBatch::generateInstancedEntity(size_t num)
    {
        return OGRE_NEW InstancedEntity( Id::generateNewId<InstancedEntity>(),
                                         &mLocalObjectMemoryManager, this, static_cast<uint32>(num)
                                 #ifndef OGRE_LEGACY_ANIMATIONS
                                         , 0
                                 #endif
                                         );
    }
    //-----------------------------------------------------------------------
    void InstanceBatch::deleteAllInstancedEntities()
    {
        //Destroy in the reverse order they were created (LIFO!)
        InstancedEntityVec::const_reverse_iterator ritor = mInstancedEntities.rbegin();
        InstancedEntityVec::const_reverse_iterator rend  = mInstancedEntities.rend();

        while( ritor != rend )
        {
            if( (*ritor)->getParentSceneNode() )
                (*ritor)->getParentSceneNode()->detachObject( (*ritor) );

            OGRE_DELETE *ritor++;
        }
    }
    //-----------------------------------------------------------------------
    void InstanceBatch::deleteUnusedInstancedEntities()
    {
        InstancedEntityVec::const_iterator itor = mUnusedEntities.begin();
        InstancedEntityVec::const_iterator end  = mUnusedEntities.end();

        while( itor != end )
            OGRE_DELETE *itor++;

        mUnusedEntities.clear();
    }
    //-----------------------------------------------------------------------
    RenderOperation InstanceBatch::build( const SubMesh* baseSubMesh )
    {
        if( checkSubMeshCompatibility( baseSubMesh ) )
        {
            //Only triangle list at the moment
            mRenderOperation.operationType  = OT_TRIANGLE_LIST;
#if OGRE_DEBUG_MODE
            mRenderOperation.srcRenderable  = this;
#endif
            mRenderOperation.useIndexes = true;
            setupVertices( baseSubMesh );
            setupIndices( baseSubMesh );

            mRenderOperation.meshIndex = ++RenderOperation::MeshIndexId;

            createAllInstancedEntities();
        }

        return mRenderOperation;
    }
    //-----------------------------------------------------------------------
    void InstanceBatch::buildFrom( const SubMesh *baseSubMesh, const RenderOperation &renderOperation )
    {
        mRenderOperation = renderOperation;
        createAllInstancedEntities();
    }
    //-----------------------------------------------------------------------
    InstancedEntity* InstanceBatch::createInstancedEntity()
    {
        InstancedEntity *retVal = 0;

        if( !mUnusedEntities.empty() )
        {
            if( mUnusedEntities.size() == mInstancedEntities.size() && !isStatic() && mCreator )
                mCreator->_addToDynamicBatchList( this );

            retVal = mUnusedEntities.back();
            mUnusedEntities.pop_back();

            retVal->setInUse(true);
        }

        return retVal;
    }
    //-----------------------------------------------------------------------
    void InstanceBatch::removeInstancedEntity( InstancedEntity *instancedEntity )
    {
        if( instancedEntity->mBatchOwner != this )
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Trying to remove an InstancedEntity from scene created"
                        " with a different InstanceBatch",
                        "InstanceBatch::removeInstancedEntity()");
        }
        if( !instancedEntity->isInUse() )
        {
            OGRE_EXCEPT(Exception::ERR_INVALID_STATE,
                        "Trying to remove an InstancedEntity that is already removed!",
                        "InstanceBatch::removeInstancedEntity()");
        }

        if( instancedEntity->getParentSceneNode() )
            instancedEntity->getParentSceneNode()->detachObject( instancedEntity );

        instancedEntity->setInUse(false);
        instancedEntity->stopSharingTransform();

        //Put it back into the queue
        mUnusedEntities.push_back( instancedEntity );

        if( mUnusedEntities.size() == mInstancedEntities.size() && !isStatic() && mCreator )
            mCreator->_removeFromDynamicBatchList( this );
    }
    //-----------------------------------------------------------------------
#ifdef OGRE_LEGACY_ANIMATIONS
    void InstanceBatch::_addAnimatedInstance( InstancedEntity *instancedEntity )
    {
        assert( std::find( mAnimatedEntities.begin(), mAnimatedEntities.end(), instancedEntity ) ==
                mAnimatedEntities.end() && "Calling _addAnimatedInstance twice" );
        assert( instancedEntity->mBatchOwner == this && "Instanced Entity should belong to us" );

        mAnimatedEntities.push_back( instancedEntity );
    }
    //-----------------------------------------------------------------------
    void InstanceBatch::_removeAnimatedInstance( const InstancedEntity *instancedEntity )
    {
        InstanceBatch::InstancedEntityArray::iterator itor = std::find( mAnimatedEntities.begin(),
                                                                        mAnimatedEntities.end(),
                                                                        instancedEntity );
        if( itor != mAnimatedEntities.end() )
            efficientVectorRemove( mAnimatedEntities, itor );
    }
#endif
    //-----------------------------------------------------------------------
    void InstanceBatch::getInstancedEntitiesInUse( InstancedEntityVec &outEntities,
                                                    CustomParamsVec &outParams )
    {
        InstancedEntityVec::const_iterator itor = mInstancedEntities.begin();
        InstancedEntityVec::const_iterator end  = mInstancedEntities.end();

        while( itor != end )
        {
            if( (*itor)->isInUse() )
            {
                outEntities.push_back( *itor );

                for( unsigned char i=0; i<mCreator->getNumCustomParams(); ++i )
                    outParams.push_back( _getCustomParam( *itor, i ) );
            }

            ++itor;
        }
    }
    //-----------------------------------------------------------------------
    void InstanceBatch::defragmentBatchNoCull( InstancedEntityVec &usedEntities,
                                                CustomParamsVec &usedParams )
    {
        const size_t maxInstancesToCopy = std::min( mInstancesPerBatch, usedEntities.size() );
        InstancedEntityVec::iterator first = usedEntities.end() - maxInstancesToCopy;
        CustomParamsVec::iterator firstParams = usedParams.end() - maxInstancesToCopy *
                                                                    mCreator->getNumCustomParams();

        //Copy from the back to front, into mInstancedEntities
        mInstancedEntities.insert( mInstancedEntities.begin(), first, usedEntities.end() );
        //Remove them from the array
        usedEntities.resize( usedEntities.size() - maxInstancesToCopy );    

        mCustomParams.insert( mCustomParams.begin(), firstParams, usedParams.end() );
    }
    //-----------------------------------------------------------------------
    void InstanceBatch::defragmentBatchDoCull( InstancedEntityVec &usedEntities,
                                                CustomParamsVec &usedParams )
    {
        //Get the the entity closest to the minimum bbox edge and put into "first"
        InstancedEntityVec::const_iterator itor   = usedEntities.begin();
        InstancedEntityVec::const_iterator end   = usedEntities.end();

        Vector3 vMinPos = Vector3::ZERO, firstPos = Vector3::ZERO;
        InstancedEntity *first = 0;

        if( !usedEntities.empty() )
        {
            first      = *usedEntities.begin();
            firstPos   = first->getParentNode()->_getDerivedPosition();
            vMinPos    = first->getParentNode()->_getDerivedPosition();
        }

        while( itor != end )
        {
            const Vector3 &vPos      = (*itor)->getParentNode()->_getDerivedPosition();

            vMinPos.x = Ogre::min( vMinPos.x, vPos.x );
            vMinPos.y = Ogre::min( vMinPos.y, vPos.y );
            vMinPos.z = Ogre::min( vMinPos.z, vPos.z );

            if( vMinPos.squaredDistance( vPos ) < vMinPos.squaredDistance( firstPos ) )
            {
                firstPos   = vPos;
            }

            ++itor;
        }

        //Now collect entities closest to 'first'
        while( !usedEntities.empty() && mInstancedEntities.size() < mInstancesPerBatch )
        {
            InstancedEntityVec::iterator closest   = usedEntities.begin();
            InstancedEntityVec::iterator it        = usedEntities.begin();
            InstancedEntityVec::iterator e         = usedEntities.end();

            Vector3 closestPos;
            closestPos = (*closest)->getParentNode()->_getDerivedPosition();

            while( it != e )
            {
                const Vector3 &vPos   = (*it)->getParentNode()->_getDerivedPosition();

                if( firstPos.squaredDistance( vPos ) < firstPos.squaredDistance( closestPos ) )
                {
                    closest      = it;
                    closestPos   = vPos;
                }

                ++it;
            }

            mInstancedEntities.push_back( *closest );
            //Now the custom params
            const size_t idx = closest - usedEntities.begin();  
            for( unsigned char i=0; i<mCreator->getNumCustomParams(); ++i )
            {
                mCustomParams.push_back( usedParams[idx + i] );
            }

            //Remove 'closest' from usedEntities & usedParams using swap and pop_back trick
            *closest = *(usedEntities.end() - 1);
            usedEntities.pop_back();

            for( unsigned char i=1; i<=mCreator->getNumCustomParams(); ++i )
            {
                usedParams[idx + mCreator->getNumCustomParams() - i] = *(usedParams.end() - 1);
                usedParams.pop_back();
            }
        }
    }
    //-----------------------------------------------------------------------
    void InstanceBatch::_defragmentBatch( bool optimizeCulling, InstancedEntityVec &usedEntities,
                                            CustomParamsVec &usedParams )
    {
        //Remove and clear what we don't need
        mInstancedEntities.clear();
        mCustomParams.clear();
        deleteUnusedInstancedEntities();

        if( !optimizeCulling )
            defragmentBatchNoCull( usedEntities, usedParams );
        else
            defragmentBatchDoCull( usedEntities, usedParams );

        //Reassign instance IDs and tell we're the new parent
        uint32 instanceId = 0;
        InstancedEntityVec::const_iterator itor = mInstancedEntities.begin();
        InstancedEntityVec::const_iterator end  = mInstancedEntities.end();

        while( itor != end )
        {
            (*itor)->mInstanceId = instanceId++;
            (*itor)->mBatchOwner = this;
            ++itor;
        }

        //Recreate unused entities, if there's left space in our container
        assert( (signed)(mInstancesPerBatch) - (signed)(mInstancedEntities.size()) >= 0 );
        mInstancedEntities.reserve( mInstancesPerBatch );
        mUnusedEntities.reserve( mInstancesPerBatch );
#ifdef OGRE_LEGACY_ANIMATIONS
        mAnimatedEntities.reserve( mInstancesPerBatch );
#endif
        mCustomParams.reserve( mCreator->getNumCustomParams() * mInstancesPerBatch );
        for( size_t i=mInstancedEntities.size(); i<mInstancesPerBatch; ++i )
        {
            InstancedEntity *instance = generateInstancedEntity(i);
            mInstancedEntities.push_back( instance );
            mUnusedEntities.push_back( instance );
            mCustomParams.push_back( Ogre::Vector4::ZERO );
        }

        //We've potentially changed our bounds
        if( !isBatchUnused() )
            _notifyStaticDirty();
    }
    //-----------------------------------------------------------------------
    void InstanceBatch::_defragmentBatchDiscard(void)
    {
        //Remove and clear what we don't need
        mInstancedEntities.clear();
        deleteUnusedInstancedEntities();
    }
    //-----------------------------------------------------------------------
    bool InstanceBatch::setStatic( bool bStatic )
    {
        bool retVal = MovableObject::setStatic( bStatic );
        if( retVal )
        {
            if( bStatic )
            {
                if( mCreator )
                {
                    mCreator->_removeFromDynamicBatchList( this );
                    mCreator->_addDirtyStaticBatch( this );
                    mStaticDirty = true;
                }
            }
            else
            {
                if( mCreator && mUnusedEntities.size() != mInstancedEntities.size() )
                    mCreator->_addToDynamicBatchList( this );
            }
        }

        return retVal;
    }
    //-----------------------------------------------------------------------
    void InstanceBatch::_notifyStaticDirty(void)
    {
        if( mCreator && isStatic() && !mStaticDirty )
        {
            mCreator->_addDirtyStaticBatch( this );
            mStaticDirty = true;
        }
    }
    //-----------------------------------------------------------------------
    void InstanceBatch::instanceBatchCullFrustumThreadedImpl( const Camera *frustum,
                                                              const Camera *lodCamera,
                                                                uint32 combinedVisibilityFlags )
    {
        mCulledInstances.clear();

        ObjectData objData;
        const size_t numObjs = mLocalObjectMemoryManager.getFirstObjectData( objData, 0 );

        MovableObject::cullFrustum( numObjs, objData, frustum, combinedVisibilityFlags,
                                    mCulledInstances, lodCamera );
    }
    //-----------------------------------------------------------------------
    const String& InstanceBatch::getMovableType(void) const
    {
        static String sType = "InstanceBatch";
        return sType;
    }
    //-----------------------------------------------------------------------
    Real InstanceBatch::getSquaredViewDepth( const Camera* cam ) const
    {
        if( mCachedCamera != cam )
        {
            mCachedCameraDist = std::numeric_limits<Real>::infinity();

            InstancedEntityVec::const_iterator itor = mInstancedEntities.begin();
            InstancedEntityVec::const_iterator end  = mInstancedEntities.end();

            while( itor != end )
            {
                if( (*itor)->isVisible() )
                    mCachedCameraDist = std::min( mCachedCameraDist, (*itor)->getSquaredViewDepth( cam ) );
                ++itor;
            }

            mCachedCamera = cam;
        }

        return mCachedCameraDist;
    }
    //-----------------------------------------------------------------------
    const LightList& InstanceBatch::getLights( void ) const
    {
        return queryLights();
    }
    //-----------------------------------------------------------------------
    void InstanceBatch::_setCustomParam( InstancedEntity *instancedEntity, unsigned char idx,
                                         const Vector4 &newParam )
    {
        mCustomParams[instancedEntity->mInstanceId * mCreator->getNumCustomParams() + idx] = newParam;
    }
    //-----------------------------------------------------------------------
    const Vector4& InstanceBatch::_getCustomParam( InstancedEntity *instancedEntity, unsigned char idx )
    {
        return mCustomParams[instancedEntity->mInstanceId * mCreator->getNumCustomParams() + idx];
    }
}
}
