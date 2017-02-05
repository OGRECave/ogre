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
#include "OgreSceneManager.h"
#include "OgreRoot.h"

#include "Math/Array/OgreMathlib.h"
#include "Math/Array/OgreArraySphere.h"
#include "Math/Array/OgreBooleanMask.h"

namespace Ogre {
    //---------------------------------------------------------------------
    DefaultIntersectionSceneQuery::DefaultIntersectionSceneQuery(SceneManager* creator)
    : IntersectionSceneQuery(creator)
    {
        // No world geometry results supported
        mSupportedWorldFragments.insert(SceneQuery::WFT_NONE);
    }
    //---------------------------------------------------------------------
    DefaultIntersectionSceneQuery::~DefaultIntersectionSceneQuery()
    {
    }
    //---------------------------------------------------------------------
    void DefaultIntersectionSceneQuery::execute(IntersectionSceneQueryListener* listener)
    {
        OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED,
                     "IntersectionSceneQuery not yet ported to Ogre 2.x!",
                     "DefaultIntersectionSceneQuery::execute");
#ifdef ENABLE_INCOMPATIBLE_OGRE_2_0
        // Iterate over all movable types
        Root::MovableObjectFactoryIterator factIt = 
            Root::getSingleton().getMovableObjectFactoryIterator();
        while(factIt.hasMoreElements())
        {
            SceneManager::MovableObjectIterator objItA = 
                mParentSceneMgr->getMovableObjectIterator(
                    factIt.getNext()->getType());
            while (objItA.hasMoreElements())
            {
                MovableObject* a = objItA.getNext();
                // skip entire section if type doesn't match
                if (!(a->getTypeFlags() & mQueryTypeMask))
                    break;

                // Skip if a does not pass the mask
                if (!(a->getQueryFlags() & mQueryMask) ||
                    !a->isInScene())
                    continue;

                // Check against later objects in the same group
                SceneManager::MovableObjectIterator objItB = objItA;
                while (objItB.hasMoreElements())
                {
                    MovableObject* b = objItB.getNext();

                    // Apply mask to b (both must pass)
                    if ((b->getQueryFlags() & mQueryMask) && 
                        b->isInScene())
                    {
                        const AxisAlignedBox& box1 = a->getWorldBoundingBox();
                        const AxisAlignedBox& box2 = b->getWorldBoundingBox();

                        if (box1.intersects(box2))
                        {
                            if (!listener->queryResult(a, b)) return;
                        }
                    }
                }
                // Check  against later groups
                Root::MovableObjectFactoryIterator factItLater = factIt;
                while (factItLater.hasMoreElements())
                {
                    SceneManager::MovableObjectIterator objItC = 
                        mParentSceneMgr->getMovableObjectIterator(
                            factItLater.getNext()->getType());
                    while (objI7tC.hasMoreElements())
                    {
                        MovableObject* c = objItC.getNext();
                        // skip entire section if type doesn't match
                        if (!(c->getTypeFlags() & mQueryTypeMask))
                            break;

                        // Apply mask to c (both must pass)
                        if ((c->getQueryFlags() & mQueryMask) &&
                            c->isInScene())
                        {
                            const AxisAlignedBox& box1 = a->getWorldBoundingBox();
                            const AxisAlignedBox& box2 = c->getWorldBoundingBox();

                            if (box1.intersects(box2))
                            {
                                if (!listener->queryResult(a, c)) return;
                            }
                        }
                    }

                }

            }


        }
#endif

    }
    //---------------------------------------------------------------------
    DefaultAxisAlignedBoxSceneQuery::
    DefaultAxisAlignedBoxSceneQuery(SceneManager* creator)
    : AxisAlignedBoxSceneQuery(creator)
    {
        // No world geometry results supported
        mSupportedWorldFragments.insert(SceneQuery::WFT_NONE);
    }
    //---------------------------------------------------------------------
    DefaultAxisAlignedBoxSceneQuery::~DefaultAxisAlignedBoxSceneQuery()
    {
    }
    //---------------------------------------------------------------------
    void DefaultAxisAlignedBoxSceneQuery::execute(SceneQueryListener* listener)
    {
        assert( mFirstRq < mLastRq && "This query will never hit any result!" );

        for( size_t i=0; i<NUM_SCENE_MEMORY_MANAGER_TYPES; ++i )
        {
            ObjectMemoryManager &memoryManager = mParentSceneMgr->_getEntityMemoryManager(
                                                        static_cast<SceneMemoryMgrTypes>(i) );

            const size_t numRenderQueues = memoryManager.getNumRenderQueues();

            bool keepIterating = true;
            size_t firstRq = std::min<size_t>( mFirstRq, numRenderQueues );
            size_t lastRq  = std::min<size_t>( mLastRq,  numRenderQueues );

            for( size_t j=firstRq; j<lastRq && keepIterating; ++j )
            {
                ObjectData objData;
                const size_t totalObjs = memoryManager.getFirstObjectData( objData, j );
                keepIterating = execute( objData, totalObjs, listener );
            }
        }
    }
    //---------------------------------------------------------------------
    bool DefaultAxisAlignedBoxSceneQuery::execute( ObjectData objData, size_t numNodes,
                                                   SceneQueryListener* listener )
    {
        ArrayAabb aabb( ArrayVector3::ZERO, ArrayVector3::ZERO );
        aabb.setAll( Aabb::newFromExtents( mAABB.getMinimum(), mAABB.getMaximum() ) );

        ArrayInt ourQueryMask = Mathlib::SetAll( mQueryMask );

        for( size_t i=0; i<numNodes; i += ARRAY_PACKED_REALS )
        {
            ArrayInt * RESTRICT_ALIAS visibilityFlags = reinterpret_cast<ArrayInt*RESTRICT_ALIAS>
                                                                        (objData.mVisibilityFlags);
            ArrayInt * RESTRICT_ALIAS queryFlags = reinterpret_cast<ArrayInt*RESTRICT_ALIAS>
                                                                        (objData.mQueryFlags);

            //hitMask = hitMask && ( (*queryFlags & ourQueryMask) != 0 ) && isVisble;
            ArrayMaskI hitMask = CastRealToInt( aabb.intersects( *objData.mWorldAabb ) );
            hitMask = Mathlib::And( hitMask, Mathlib::TestFlags4( *queryFlags, ourQueryMask ) );
            hitMask = Mathlib::And( hitMask,
                                    Mathlib::TestFlags4( *visibilityFlags,
                                        Mathlib::SetAll( VisibilityFlags::LAYER_VISIBILITY ) ) );

            const uint32 scalarMask = BooleanMask4::getScalarMask( hitMask );

            for( size_t j=0; j<ARRAY_PACKED_REALS; ++j )
            {
                //Decompose the result for analyzing each MovableObject's
                //There's no need to check objData.mOwner[j] is null because
                //we set mVisibilityFlags to 0 on slot removals
                if( IS_BIT_SET( j, scalarMask ) )
                {
                    if( !listener->queryResult( objData.mOwner[j] ) )
                        return false;
                }

#if OGRE_DEBUG_MODE
                //Queries must be performed after all bounds have been updated
                //(i.e. SceneManager::updateSceneGraph does this for you), and don't
                //move the objects between that call and this query.
                //Ignore out of date Aabbs from objects that have been
                //explicitly disabled or fail the query mask.
                assert((!(objData.mVisibilityFlags[objData.mIndex] & VisibilityFlags::LAYER_VISIBILITY) ||
                        !(objData.mQueryFlags[objData.mIndex] & mQueryMask) ||
                        !objData.mOwner[j]->isCachedAabbOutOfDate()) &&
                        "Perform the queries after MovableObject::updateAllBounds has been called!");
#endif
            }

            objData.advancePack();
        }

        return true;
    }
    //---------------------------------------------------------------------
    DefaultRaySceneQuery::
    DefaultRaySceneQuery(SceneManager* creator) : RaySceneQuery(creator)
    {
        // No world geometry results supported
        mSupportedWorldFragments.insert(SceneQuery::WFT_NONE);
    }
    //---------------------------------------------------------------------
    DefaultRaySceneQuery::~DefaultRaySceneQuery()
    {
    }
    //---------------------------------------------------------------------
    void DefaultRaySceneQuery::execute(RaySceneQueryListener* listener)
    {
        assert( mFirstRq < mLastRq && "This query will never hit any result!" );

        for( size_t i=0; i<NUM_SCENE_MEMORY_MANAGER_TYPES; ++i )
        {
            ObjectMemoryManager &memoryManager = mParentSceneMgr->_getEntityMemoryManager(
                                                        static_cast<SceneMemoryMgrTypes>(i) );

            const size_t numRenderQueues = memoryManager.getNumRenderQueues();

            bool keepIterating = true;
            size_t firstRq = std::min<size_t>( mFirstRq, numRenderQueues );
            size_t lastRq  = std::min<size_t>( mLastRq,  numRenderQueues );

            for( size_t j=firstRq; j<lastRq && keepIterating; ++j )
            {
                ObjectData objData;
                const size_t totalObjs = memoryManager.getFirstObjectData( objData, j );
                keepIterating = execute( objData, totalObjs, listener );
            }
        }
    }
    //---------------------------------------------------------------------
    bool DefaultRaySceneQuery::execute( ObjectData objData, size_t numNodes,
                                        RaySceneQueryListener* listener )
    {
        ArrayVector3 rayOrigin;
        ArrayVector3 rayDir;

        ArrayInt ourQueryMask = Mathlib::SetAll( mQueryMask );

        rayOrigin.setAll( mRay.getOrigin() );
        rayDir.setAll( mRay.getDirection() );

        for( size_t i=0; i<numNodes; i += ARRAY_PACKED_REALS )
        {
            // Check origin inside first
            ArrayMaskR hitMaskR = objData.mWorldAabb->contains( rayOrigin );

            ArrayReal distance = Mathlib::CmovRobust( ARRAY_REAL_ZERO, Mathlib::INFINITEA, hitMaskR );

            ArrayVector3 vMin = objData.mWorldAabb->getMinimum();
            ArrayVector3 vMax = objData.mWorldAabb->getMaximum();

            ArrayInt * RESTRICT_ALIAS visibilityFlags = reinterpret_cast<ArrayInt*RESTRICT_ALIAS>
                                                                        (objData.mVisibilityFlags);
            ArrayInt * RESTRICT_ALIAS queryFlags = reinterpret_cast<ArrayInt*RESTRICT_ALIAS>
                                                                        (objData.mQueryFlags);

            // Check each face in turn
            // Min x, y & z
            for( size_t j=0; j<3; ++j )
            {
                ArrayReal t = (vMin.mChunkBase[j] - rayOrigin.mChunkBase[j]) / rayDir.mChunkBase[j];

                //mask = t >= 0; works even if t is nan (t = 0 / 0)
                ArrayMaskR mask = Mathlib::CompareGreaterEqual( t, ARRAY_REAL_ZERO );
                ArrayVector3 hitPoint = rayOrigin + rayDir * t;

                //Fix accuracy issues (this value will lay exactly in the extend,
				//but often is slightly beyond it, causing the test to fail)
                hitPoint.mChunkBase[j] = objData.mWorldAabb->mCenter.mChunkBase[j];

                //hitMaskR |= t >= 0 && mWorldAabb->contains( hitPoint );
                //distance = t >= 0 ? min( distance, t ) : t;
                hitMaskR = Mathlib::Or( hitMaskR, Mathlib::And( mask,
                                                    objData.mWorldAabb->contains( hitPoint ) ) );
                distance = Mathlib::CmovRobust( Mathlib::Min( distance, t ), distance, mask );
            }

            // Max x, y & z
            for( size_t j=0; j<3; ++j )
            {
                ArrayReal t = (vMax.mChunkBase[j] - rayOrigin.mChunkBase[j]) / rayDir.mChunkBase[j];

                //mask = t >= 0; works even if t is nan (t = 0 / 0)
                ArrayMaskR mask = Mathlib::CompareGreaterEqual( t, ARRAY_REAL_ZERO );
                ArrayVector3 hitPoint = rayOrigin + rayDir * t;

                //Fix accuracy issues (this value will lay exactly in the extend,
				//but often is slightly beyond it, causing the test to fail)
                hitPoint.mChunkBase[j] = objData.mWorldAabb->mCenter.mChunkBase[j];

                //hitMaskR |= t >= 0 && mWorldAabb->contains( hitPoint );
                //distance = t >= 0 ? min( distance, t ) : t;
                hitMaskR = Mathlib::Or( hitMaskR, Mathlib::And( mask,
                                                    objData.mWorldAabb->contains( hitPoint ) ) );
                distance = Mathlib::CmovRobust( Mathlib::Min( distance, t ), distance, mask );
            }

            //hitMask = hitMask && ( (*queryFlags & ourQueryMask) != 0 ) && isVisble;
            ArrayMaskI hitMask = CastRealToInt( hitMaskR );
            hitMask = Mathlib::And( hitMask, Mathlib::TestFlags4( *queryFlags, ourQueryMask ) );
            hitMask = Mathlib::And( hitMask,
                                    Mathlib::TestFlags4( *visibilityFlags,
                                        Mathlib::SetAll( VisibilityFlags::LAYER_VISIBILITY ) ) );

            const uint32 scalarMask = BooleanMask4::getScalarMask( hitMask );
            OGRE_ALIGNED_DECL( Real, scalarDistance[ARRAY_PACKED_REALS], OGRE_SIMD_ALIGNMENT );
            CastArrayToReal( scalarDistance, distance );

            for( size_t j=0; j<ARRAY_PACKED_REALS; ++j )
            {
                //Decompose the result for analyzing each MovableObject's
                //There's no need to check objData.mOwner[j] is null because
                //we set mVisibilityFlags to 0 on slot removals
                if( IS_BIT_SET( j, scalarMask ) )
                {
                    if( !listener->queryResult( objData.mOwner[j], scalarDistance[j] ) )
                        return false;
                }

#if OGRE_DEBUG_MODE
                //Queries must be performed after all bounds have been updated
                //(i.e. SceneManager::updateSceneGraph does this for you), and don't
                //move the objects between that call and this query.
                //Ignore out of date Aabbs from objects that have been
                //explicitly disabled or fail the query mask.
                assert((!(objData.mVisibilityFlags[objData.mIndex] & VisibilityFlags::LAYER_VISIBILITY) ||
                        !(objData.mQueryFlags[objData.mIndex] & mQueryMask) ||
                        !objData.mOwner[j]->isCachedAabbOutOfDate()) &&
                        "Perform the queries after MovableObject::updateAllBounds has been called!");
#endif
            }

            objData.advancePack();
        }

        return true;
    }
    //---------------------------------------------------------------------
    DefaultSphereSceneQuery::
    DefaultSphereSceneQuery(SceneManager* creator) : SphereSceneQuery(creator)
    {
        // No world geometry results supported
        mSupportedWorldFragments.insert(SceneQuery::WFT_NONE);
    }
    //---------------------------------------------------------------------
    DefaultSphereSceneQuery::~DefaultSphereSceneQuery()
    {
    }
    //---------------------------------------------------------------------
    void DefaultSphereSceneQuery::execute(SceneQueryListener* listener)
    {
        assert( mFirstRq < mLastRq && "This query will never hit any result!" );

        for( size_t i=0; i<NUM_SCENE_MEMORY_MANAGER_TYPES; ++i )
        {
            ObjectMemoryManager &memoryManager = mParentSceneMgr->_getEntityMemoryManager(
                                                        static_cast<SceneMemoryMgrTypes>(i) );

            const size_t numRenderQueues = memoryManager.getNumRenderQueues();

            bool keepIterating = true;
            size_t firstRq = std::min<size_t>( mFirstRq, numRenderQueues );
            size_t lastRq  = std::min<size_t>( mLastRq,  numRenderQueues );

            for( size_t j=firstRq; j<lastRq && keepIterating; ++j )
            {
                ObjectData objData;
                const size_t totalObjs = memoryManager.getFirstObjectData( objData, j );
                keepIterating = execute( objData, totalObjs, listener );
            }
        }
    }
    //---------------------------------------------------------------------
    bool DefaultSphereSceneQuery::execute( ObjectData objData, size_t numNodes,
                                           SceneQueryListener* listener )
    {
        ArraySphere ourSphere;
        ourSphere.setAll( mSphere );

        ArrayInt ourQueryMask = Mathlib::SetAll( mQueryMask );

        for( size_t i=0; i<numNodes; i += ARRAY_PACKED_REALS )
        {
            ArrayInt * RESTRICT_ALIAS visibilityFlags = reinterpret_cast<ArrayInt*RESTRICT_ALIAS>
                                                                        (objData.mVisibilityFlags);
            ArrayInt * RESTRICT_ALIAS queryFlags = reinterpret_cast<ArrayInt*RESTRICT_ALIAS>
                                                                        (objData.mQueryFlags);
            ArrayReal * RESTRICT_ALIAS worldRadius = reinterpret_cast<ArrayReal*RESTRICT_ALIAS>
                                                                        (objData.mWorldRadius);

            ArraySphere testSphere( *worldRadius, objData.mWorldAabb->mCenter );

            //hitMask = hitMask && ( (*queryFlags & ourQueryMask) != 0 ) && isVisble;
            ArrayMaskI hitMask = CastRealToInt( ourSphere.intersects( testSphere ) );
            hitMask = Mathlib::And( hitMask, Mathlib::TestFlags4( *queryFlags, ourQueryMask ) );
            hitMask = Mathlib::And( hitMask,
                                    Mathlib::TestFlags4( *visibilityFlags,
                                        Mathlib::SetAll( VisibilityFlags::LAYER_VISIBILITY ) ) );

            const uint32 scalarMask = BooleanMask4::getScalarMask( hitMask );

            for( size_t j=0; j<ARRAY_PACKED_REALS; ++j )
            {
                //Decompose the result for analyzing each MovableObject's
                //There's no need to check objData.mOwner[j] is null because
                //we set mVisibilityFlags to 0 on slot removals
                if( IS_BIT_SET( j, scalarMask ) )
                {
                    if( !listener->queryResult( objData.mOwner[j] ) )
                        return false;
                }

#if OGRE_DEBUG_MODE
                //Queries must be performed after all bounds have been updated
                //(i.e. SceneManager::updateSceneGraph does this for you), and don't
                //move the objects between that call and this query.
                //Ignore out of date Aabbs from objects that have been
                //explicitly disabled or fail the query mask.
                assert((!(objData.mVisibilityFlags[objData.mIndex] & VisibilityFlags::LAYER_VISIBILITY) ||
                        !(objData.mQueryFlags[objData.mIndex] & mQueryMask) ||
                        !objData.mOwner[j]->isCachedAabbOutOfDate()) &&
                        "Perform the queries after MovableObject::updateAllBounds has been called!");
#endif
            }

            objData.advancePack();
        }

        return true;
    }
    //---------------------------------------------------------------------
    DefaultPlaneBoundedVolumeListSceneQuery::
    DefaultPlaneBoundedVolumeListSceneQuery(SceneManager* creator) 
    : PlaneBoundedVolumeListSceneQuery(creator)
    {
        // No world geometry results supported
        mSupportedWorldFragments.insert(SceneQuery::WFT_NONE);
    }
    //---------------------------------------------------------------------
    DefaultPlaneBoundedVolumeListSceneQuery::~DefaultPlaneBoundedVolumeListSceneQuery()
    {
    }
    //---------------------------------------------------------------------
    void DefaultPlaneBoundedVolumeListSceneQuery::execute(SceneQueryListener* listener)
    {
        assert(mFirstRq < mLastRq && "This query will never hit any result!");

        //Create a SIMD friendly version of all our planes
        size_t totalPlanes = 0;
        for( size_t i=0; i<mVolumes.size(); ++i )
        {
            totalPlanes += mVolumes[i].planes.size();
        }

        mSimdPlaneList = RawSimdUniquePtr<ArrayPlane, MEMCATEGORY_GENERAL>(totalPlanes);
        {
            ArrayPlane * RESTRICT_ALIAS planes = mSimdPlaneList.get();
            totalPlanes = 0;
            for( size_t i=0; i<mVolumes.size(); ++i )
            {
                for( size_t j=0; j<mVolumes[i].planes.size(); ++j )
                {
                    Plane plane = mVolumes[i].planes[j];
                    ArrayPlane arrayPlane;
                    arrayPlane.planeNormal.setAll(plane.normal);
                    arrayPlane.signFlip.setAll(plane.normal);
                    arrayPlane.signFlip.setToSign();
                    arrayPlane.planeNegD = Mathlib::SetAll(-plane.d);
                    planes[totalPlanes++] = arrayPlane;
                }
            }
        }

        for( size_t i=0; i<NUM_SCENE_MEMORY_MANAGER_TYPES; ++i )
        {
            ObjectMemoryManager &memoryManager = mParentSceneMgr->_getEntityMemoryManager(
                static_cast<SceneMemoryMgrTypes>(i) );

            const size_t numRenderQueues = memoryManager.getNumRenderQueues();

            bool keepIterating = true;
            size_t firstRq = std::min<size_t>( mFirstRq, numRenderQueues );
            size_t lastRq  = std::min<size_t>( mLastRq,  numRenderQueues );

            for( size_t j=firstRq; j<lastRq && keepIterating; ++j )
            {
                ObjectData objData;
                const size_t totalObjs = memoryManager.getFirstObjectData( objData, j );
                keepIterating = execute( objData, totalObjs, listener );
            }
        }
    }
    //---------------------------------------------------------------------
    bool DefaultPlaneBoundedVolumeListSceneQuery::execute(ObjectData objData, size_t numNodes, SceneQueryListener* listener)
    {
        ArrayInt ourQueryMask = Mathlib::SetAll(mQueryMask);
        const ArrayPlane * RESTRICT_ALIAS planes = mSimdPlaneList.get();

        for( size_t n=0; n<numNodes; n += ARRAY_PACKED_REALS )
        {
            ArrayInt * RESTRICT_ALIAS visibilityFlags = reinterpret_cast<ArrayInt*RESTRICT_ALIAS>
                (objData.mVisibilityFlags);
            ArrayInt * RESTRICT_ALIAS queryFlags = reinterpret_cast<ArrayInt*RESTRICT_ALIAS>
                (objData.mQueryFlags);

            ArrayMaskR allVolumesMask = ARRAY_MASK_ZERO;
            size_t planeCounter = 0;
            for( size_t v=0; v<mVolumes.size(); ++v )
            {
                //For each volume test all planes and AND the dot product.
                //If one is false, then we dont intersect with this volume
                ArrayMaskR singleVolumeMask = CastIntToReal( Mathlib::SetAll( 0xffffffff ) );
                ArrayReal dotResult;
                ArrayVector3 centerPlusFlippedHS;
    
                for( size_t p=0; p<mVolumes[v].planes.size(); ++p )
                {
                    centerPlusFlippedHS = objData.mWorldAabb->mCenter +
                            objData.mWorldAabb->mHalfSize * planes[planeCounter].signFlip;
                    dotResult = planes[planeCounter].planeNormal.dotProduct(centerPlusFlippedHS);
                    singleVolumeMask =
                            Mathlib::And( singleVolumeMask,
                                          Mathlib::CompareGreater( dotResult,
                                                                   planes[planeCounter].planeNegD) );
                    ++planeCounter;
                }

                //Always pass the test if any of the components were
                //Infinity (dot product above could've caused nans)
                ArrayMaskR tmpMask = Mathlib::Or(
                    Mathlib::isInfinity(objData.mWorldAabb->mHalfSize.mChunkBase[0]),
                    Mathlib::isInfinity(objData.mWorldAabb->mHalfSize.mChunkBase[1]) );
                tmpMask = Mathlib::Or( Mathlib::isInfinity(objData.mWorldAabb->mHalfSize.mChunkBase[2]),
                                       tmpMask );
                singleVolumeMask = Mathlib::Or( tmpMask, singleVolumeMask );

                //Our query passes if just one of the volumes intersects with the object
                allVolumesMask = Mathlib::Or( allVolumesMask, singleVolumeMask );

            }

            ArrayMaskI hitMask = CastRealToInt(allVolumesMask);
            hitMask = Mathlib::And( hitMask, Mathlib::TestFlags4(*queryFlags, ourQueryMask) );
            hitMask = Mathlib::And( hitMask,
                                    Mathlib::TestFlags4(
                                        *visibilityFlags,
                                        Mathlib::SetAll(VisibilityFlags::LAYER_VISIBILITY) ) );

            const uint32 scalarMask = BooleanMask4::getScalarMask( hitMask );
            for( size_t j=0; j<ARRAY_PACKED_REALS; ++j )
            {
                //Decompose the result for analyzing each MovableObject's
                //There's no need to check objData.mOwner[j] is null because
                //we set mVisibilityFlags to 0 on slot removals
                if( IS_BIT_SET( j, scalarMask ) )
                {
                    if( !listener->queryResult(objData.mOwner[j]) )
                        return false;
                }

#if OGRE_DEBUG_MODE
                //Queries must be performed after all bounds have been updated
                //(i.e. SceneManager::updateSceneGraph does this for you), and don't
                //move the objects between that call and this query.
                //Ignore out of date Aabbs from objects that have been
                //explicitly disabled or fail the query mask.
                assert((!(objData.mVisibilityFlags[objData.mIndex] & VisibilityFlags::LAYER_VISIBILITY) ||
                    !(objData.mQueryFlags[objData.mIndex] & mQueryMask) ||
                    !objData.mOwner[j]->isCachedAabbOutOfDate()) &&
                    "Perform the queries after MovableObject::updateAllBounds has been called!");
#endif
            }
            objData.advancePack();
        }
        return true;
    }
}
