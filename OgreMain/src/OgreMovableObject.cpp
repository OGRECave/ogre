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

#include "OgreMovableObject.h"
#include "OgreSceneNode.h"
#include "OgreTagPoint.h"
#include "OgreLight.h"
#include "OgreEntity.h"
#include "OgreRoot.h"
#include "OgreSceneManager.h"
#include "OgreCamera.h"
#include "OgreLodListener.h"
#include "OgreLight.h"
#include "Math/Array/OgreArraySphere.h"
#include "Math/Array/OgreBooleanMask.h"

namespace Ogre {
	using namespace VisibilityFlags;
	const FastArray<Real> c_DefaultLodMesh = FastArray<Real>( 1, std::numeric_limits<Real>::max() );
	//-----------------------------------------------------------------------
	//-----------------------------------------------------------------------
	const String NullEntity::msMovableType = "NullEntity";
	const uint32 VisibilityFlags::LAYER_SHADOW_RECEIVER		= 1 << 31;
	const uint32 VisibilityFlags::LAYER_SHADOW_CASTER		= 1 << 30;
	const uint32 VisibilityFlags::LAYER_VISIBILITY			= 1 << 29;
	const uint32 VisibilityFlags::RESERVED_VISIBILITY_FLAGS	= ~(LAYER_SHADOW_RECEIVER|LAYER_SHADOW_CASTER|
																LAYER_VISIBILITY);
	uint32 MovableObject::msDefaultQueryFlags = 0xFFFFFFFF;
	uint32 MovableObject::msDefaultVisibilityFlags = 0xFFFFFFFF & (~LAYER_VISIBILITY);
    //-----------------------------------------------------------------------
    MovableObject::MovableObject( IdType id, ObjectMemoryManager *objectMemoryManager,
									uint8 renderQueueId )
        : IdObject( id )
		, mCreator(0)
        , mManager(0)
		, mLodMesh( &c_DefaultLodMesh )
		, mLodMaterial( 0 )
		, mCurrentMeshLod( 0 )
        , mParentNode(0)
		, mMinPixelSize(0)
        , mRenderQueueID(renderQueueId)
		, mRenderQueuePriority(100)
        , mListener(0)
		, mDebugDisplay(false)
		, mObjectMemoryManager( objectMemoryManager )
		, mGlobalIndex( -1 )
		, mParentIndex( -1 )
    {
		mCurrentMaterialLod.resize( 1, 0 );
		if (Root::getSingletonPtr())
			mMinPixelSize = Root::getSingleton().getDefaultMinPixelSize();

		//Will initialize mObjectData
		mObjectMemoryManager->objectCreated( mObjectData, mRenderQueueID );
		mObjectData.mOwner[mObjectData.mIndex] = this;
    }
	//-----------------------------------------------------------------------
    MovableObject::MovableObject( ObjectData *objectDataPtrs )
        : IdObject( 0 )
		, mCreator(0)
        , mManager(0)
		, mLodMesh( 0 )
		, mLodMaterial( 0 )
		, mCurrentMeshLod( 0 )
        , mParentNode(0)
		, mMinPixelSize(0)
        , mRenderQueueID(RENDER_QUEUE_MAIN)
		, mRenderQueuePriority(100)
        , mListener(0)
		, mDebugDisplay(false)
		, mObjectMemoryManager( 0 )
		, mGlobalIndex( -1 )
		, mParentIndex( -1 )
    {
		mCurrentMaterialLod.resize( 0, 0 );
		if (Root::getSingletonPtr())
			mMinPixelSize = Root::getSingleton().getDefaultMinPixelSize();
    }
    //-----------------------------------------------------------------------
    MovableObject::~MovableObject()
    {
        // Call listener (note, only called if there's something to do)
        if (mListener)
        {
            mListener->objectDestroyed(this);
        }

        if (mParentNode)
        {
			// May be we are a lod entity which not in the parent node child object list,
			// call this method could safely ignore this case.
			static_cast<SceneNode*>(mParentNode)->detachObject( this );
        }

		if( mObjectMemoryManager )
			mObjectMemoryManager->objectDestroyed( mObjectData, mRenderQueueID );
    }
    //-----------------------------------------------------------------------
    void MovableObject::_notifyAttached( Node* parent )
    {
        assert(!mParentNode || !parent);

        bool different = (parent != mParentNode);

		if( different )
		{
			mParentNode = parent;
			if( parent )
				mObjectData.mParents[mObjectData.mIndex] = parent;
			else
				mObjectData.mParents[mObjectData.mIndex] = mObjectMemoryManager->_getDummyNode();

			setVisible( parent != 0 );

			// Call listener (note, only called if there's something to do)
			if (mListener)
			{
				if (mParentNode)
					mListener->objectAttached(this);
				else
					mListener->objectDetached(this);
			}

			if( mManager && isStatic() )
				mManager->notifyStaticDirty( this );
		}
    }
	//---------------------------------------------------------------------
	void MovableObject::detachFromParent(void)
	{
		if (isAttached())
		{
			SceneNode* sn = static_cast<SceneNode*>(mParentNode);
			sn->detachObject(this);
		}
	}
    //-----------------------------------------------------------------------
    void MovableObject::_notifyMoved(void)
    {
#ifndef NDEBUG
		mCachedAabbOutOfDate = true;
#endif

        // Notify listener if exists
        if (mListener)
        {
            mListener->objectMoved(this);
        }
    }
	//-----------------------------------------------------------------------
	bool MovableObject::isStatic() const
	{
		return mObjectMemoryManager->getMemoryManagerType() == SCENE_STATIC;
	}
	//-----------------------------------------------------------------------
	bool MovableObject::setStatic( bool bStatic )
	{
		bool retVal = false;
		if( mObjectMemoryManager->getTwin() &&
            ((mObjectMemoryManager->getMemoryManagerType() == SCENE_STATIC && !bStatic) ||
             (mObjectMemoryManager->getMemoryManagerType() == SCENE_DYNAMIC && bStatic)) )
		{
			mObjectMemoryManager->migrateTo( mObjectData, mRenderQueueID,
											 mObjectMemoryManager->getTwin() );
			mObjectMemoryManager = mObjectMemoryManager->getTwin();

			if( mParentNode && mParentNode->isStatic() != bStatic )
				mParentNode->setStatic( bStatic );

			if( mManager && bStatic )
				mManager->notifyStaticDirty( this );

			retVal = true;
		}

		return retVal;
	}
    //-----------------------------------------------------------------------
    bool MovableObject::isVisible(void) const
    {
        if( !getVisible() )
            return false;

        SceneManager* sm = Root::getSingleton()._getCurrentSceneManager();
        if (sm && !(getVisibilityFlags() & sm->_getCombinedVisibilityMask()))
            return false;

        return true;
    }
	/*//-----------------------------------------------------------------------
	void MovableObject::_notifyCurrentCamera(Camera* cam)
	{
		if (mParentNode)
		{
			mBeyondFarDistance = false;

			if (cam->getUseRenderingDistance() && mUpperDistance > 0)
			{
				Real rad = getBoundingRadius();
				Real squaredDepth = mParentNode->getSquaredViewDepth(cam->getLodCamera());

				const Vector3& scl = mParentNode->_getDerivedScale();
				Real factor = max(max(scl.x, scl.y), scl.z);

				// Max distance to still render
				Real maxDist = mUpperDistance + rad * factor;
				if (squaredDepth > Math::Sqr(maxDist))
				{
					mBeyondFarDistance = true;
				}
			}

			if (!mBeyondFarDistance && cam->getUseMinPixelSize() && mMinPixelSize > 0)
			{

				Real pixelRatio = cam->getPixelDisplayRatio();
				
				//if ratio is relative to distance than the distance at which the object should be displayed
				//is the size of the radius divided by the ratio
				//get the size of the entity in the world
				Ogre::Vector3 objBound = getBoundingBox().getSize() * 
							getParentNode()->_getDerivedScale();
				
				//We object are projected from 3 dimensions to 2. The shortest displayed dimension of 
				//as object will always be at most the second largest dimension of the 3 dimensional
				//bounding box.
				//The square calculation come both to get rid of minus sign and for improve speed
				//in the final calculation
				objBound.x = Math::Sqr(objBound.x);
				objBound.y = Math::Sqr(objBound.y);
				objBound.z = Math::Sqr(objBound.z);
				float sqrObjMedianSize = max(max(
									min(objBound.x,objBound.y),
									min(objBound.x,objBound.z)),
									min(objBound.y,objBound.z));

				//If we have a perspective camera calculations are done relative to distance
				Real sqrDistance = 1;
				if (cam->getProjectionType() == PT_PERSPECTIVE)
				{
					sqrDistance = mParentNode->getSquaredViewDepth(cam->getLodCamera());
				}

				//Final Calculation to tell whether the object is to small
				mBeyondFarDistance =  sqrObjMedianSize < 
							sqrDistance * Math::Sqr(pixelRatio * mMinPixelSize); 
			}
			
            // Construct event object
            MovableObjectLodChangedEvent evt;
            evt.movableObject = this;
            evt.camera = cam;

            // Notify LOD event listeners
            cam->getSceneManager()->_notifyMovableObjectLodChanged(evt);

		}

        mRenderingDisabled = mListener && !mListener->objectRendering(this, cam);
	}*/
    //-----------------------------------------------------------------------
    void MovableObject::setRenderQueueGroup(uint8 queueID)
    {
		if( mRenderQueueID != queueID )
			mObjectMemoryManager->objectMoved( mObjectData, mRenderQueueID, queueID );

        mRenderQueueID = queueID;
    }
	//-----------------------------------------------------------------------
	void MovableObject::setRenderQueueGroupAndPriority(uint8 queueID, ushort priority)
	{
		setRenderQueueGroup(queueID);
		mRenderQueuePriority = priority;
	}
    //-----------------------------------------------------------------------
    uint8 MovableObject::getRenderQueueGroup(void) const
    {
        return mRenderQueueID;
    }
    //-----------------------------------------------------------------------
	const Matrix4& MovableObject::_getParentNodeFullTransform(void) const
	{
		return mParentNode->_getFullTransform();
	}
	//-----------------------------------------------------------------------
	const Aabb MovableObject::getWorldAabb() const
	{
		assert( !mCachedAabbOutOfDate );
		return mObjectData.mWorldAabb->getAsAabb( mObjectData.mIndex );
	}
	//-----------------------------------------------------------------------
	const Aabb MovableObject::getWorldAabbUpdated()
	{
		return updateSingleWorldAabb();
	}
	//-----------------------------------------------------------------------
	float MovableObject::getWorldRadius() const
	{
		assert( !mCachedAabbOutOfDate );
		return mObjectData.mWorldRadius[mObjectData.mIndex];
	}
	//-----------------------------------------------------------------------
	float MovableObject::getWorldRadiusUpdated()
	{
		return updateSingleWorldRadius();
	}
    //-----------------------------------------------------------------------
	Aabb MovableObject::updateSingleWorldAabb()
	{
		Matrix4 derivedTransform = mParentNode->_getFullTransformUpdated();

		Aabb retVal;
		mObjectData.mLocalAabb->getAsAabb( retVal, mObjectData.mIndex );
		retVal.transformAffine( derivedTransform );

		mObjectData.mWorldAabb->setFromAabb( retVal, mObjectData.mIndex );

#ifndef NDEBUG
		mCachedAabbOutOfDate = false;
#endif

		return retVal;
	}
	//-----------------------------------------------------------------------
	float MovableObject::updateSingleWorldRadius()
	{
		const Vector3 derivedScale = mParentNode->_getDerivedScaleUpdated();

		float retVal = mObjectData.mLocalRadius[mObjectData.mIndex] *
						max( max( derivedScale.x, derivedScale.y ), derivedScale.z );
		mObjectData.mWorldRadius[mObjectData.mIndex] = retVal;

		return retVal;
	}
    //-----------------------------------------------------------------------
	/*const Sphere& MovableObject::getWorldBoundingSphere(bool derive) const
	{
		if (derive)
		{
			const Vector3& scl = mParentNode->_getDerivedScale();
			Real factor = std::max(std::max(scl.x, scl.y), scl.z);
			mWorldBoundingSphere.setRadius(getBoundingRadius() * factor);
			mWorldBoundingSphere.setCenter(mParentNode->_getDerivedPosition());
		}
		return mWorldBoundingSphere;
	}*/
    //-----------------------------------------------------------------------
	void MovableObject::updateAllBounds( const size_t numNodes, ObjectData objData )
	{
		SimpleMatrix4 mats[ARRAY_PACKED_REALS];
		for( size_t i=0; i<numNodes; i += ARRAY_PACKED_REALS )
		{
			//Retrieve from parents. Unfortunately we need to do SoA -> AoS -> SoA conversion
			ArrayMatrix4 parentMat;
			ArrayVector3 parentScale;

			for( size_t j=0; j<ARRAY_PACKED_REALS; ++j )
			{
				//Profiling shows these prefetches do make a difference. Perhaps playing with them could
				//achieve even greater speed ups. This function is terribly bounded by memory latency
				//Last tested on:
				//	* Intel Quad Core Extreme QX9650 3Ghz
				OGRE_PREFETCH_NTA( (const char*)(objData.mParents[i+OGRE_PREFETCH_SLOT_DISTANCE]) );

				Vector3 scale;
				const Transform &parentTransform = objData.mParents[j]->_getTransform();
				parentTransform.mDerivedScale->getAsVector3( scale, parentTransform.mIndex );
				mats[j].load( parentTransform.mDerivedTransform[parentTransform.mIndex] );
				parentScale.setFromVector3( scale, j );

				// j + OGRE_PREFETCH_SLOT_DISTANCE won't go out of bounds because
				// the memory manager allocates enough extra space
				OGRE_PREFETCH_NTA( (const char*)objData.mParents[j+(OGRE_PREFETCH_SLOT_DISTANCE>>1)]->
									_getTransform().mDerivedScale );
				OGRE_PREFETCH_NTA( (const char*)(objData.mParents[j+(OGRE_PREFETCH_SLOT_DISTANCE>>1)]->
									_getTransform().mDerivedTransform+parentTransform.mIndex) );
			}

			parentMat.loadFromAoS( mats );

			ArrayReal * RESTRICT_ALIAS worldRadius = reinterpret_cast<ArrayReal*RESTRICT_ALIAS>
																		(objData.mWorldRadius);
			ArrayReal * RESTRICT_ALIAS localRadius = reinterpret_cast<ArrayReal*RESTRICT_ALIAS>
																		(objData.mLocalRadius);

			*objData.mWorldAabb = *objData.mLocalAabb;
			objData.mWorldAabb->transformAffine( parentMat );
			*worldRadius = (*localRadius) * parentScale.getMaxComponent();

#ifndef NDEBUG
			for( size_t j=0; j<ARRAY_PACKED_REALS; ++j )
			{
				if( objData.mOwner[j] )
					objData.mOwner[j]->mCachedAabbOutOfDate = false;
			}
#endif

			objData.advanceBoundsPack();
		}
	}
	//-----------------------------------------------------------------------
	void MovableObject::cullFrustum( const size_t numNodes, ObjectData objData, const Frustum *frustum,
									 uint32 sceneVisibilityFlags, MovableObjectArray &outCulledObjects,
									 AxisAlignedBox *outReceiversBox, const Camera *lodCamera )
	{
		//On threaded environments, the internal variables from outCulledObjects cause
		//a false cache sharing because they're too close to each other. Perfoming
		//a swap places those internal vars in the local stack, increasing scalability
		MovableObjectArray culledObjects;
		culledObjects.swap( outCulledObjects );

		//Thanks to Fabian Giesen for summing up all known methods of frustum culling:
		//http://fgiesen.wordpress.com/2010/10/17/view-frustum-culling/
		// (we use method Method 5: "If you really don’t care whether a box is
		// partially or fully inside"):
		// vector4 signFlip = componentwise_and(plane, 0x80000000);
		// return dot3(center + xor(extent, signFlip), plane) > -plane.w;
		struct ArrayPlane
		{
			ArrayVector3	planeNormal;
			ArrayVector3	signFlip;
			ArrayReal		planeNegD;
		};

		ArrayVector3 lodCameraPos;
		lodCameraPos.setAll( lodCamera->getDerivedPosition() );

		// Flip the bit from shadow caster, and leave only that in "includeNonCasters"
		ArrayInt includeNonCasters = Mathlib::SetAll( ((sceneVisibilityFlags & LAYER_SHADOW_CASTER) ^ -1)
														& LAYER_SHADOW_CASTER );
		sceneVisibilityFlags &= RESERVED_VISIBILITY_FLAGS;

		ArrayInt sceneFlags = Mathlib::SetAll( sceneVisibilityFlags );
		ArrayPlane planes[6];
		const Plane *frustumPlanes = frustum->getFrustumPlanes();

		for( size_t i=0; i<6; ++i )
		{
			planes[i].planeNormal.setAll( frustumPlanes[i].normal );
			planes[i].signFlip.setAll( frustumPlanes[i].normal );
			planes[i].signFlip.setToSign();
			planes[i].planeNegD = Mathlib::SetAll( -frustumPlanes[i].d );
		}

		ArrayVector3 vMinBounds( Mathlib::MAX_POS, Mathlib::MAX_POS, Mathlib::MAX_POS );
		ArrayVector3 vMaxBounds( Mathlib::MAX_NEG, Mathlib::MAX_NEG, Mathlib::MAX_NEG );

		//TODO: Profile whether we should use XOR to flip the sign or simple multiplication.
		//In theory xor is faster, but some archs have a penalty for switching between integer
		//& floating point, even if it's simd sse
		for( size_t i=0; i<numNodes; i += ARRAY_PACKED_REALS )
		{
			ArrayInt * RESTRICT_ALIAS visibilityFlags = reinterpret_cast<ArrayInt*RESTRICT_ALIAS>
																		(objData.mVisibilityFlags);
			ArrayReal * RESTRICT_ALIAS worldRadius = reinterpret_cast<ArrayReal*RESTRICT_ALIAS>
																		(objData.mWorldRadius);
			ArrayReal * RESTRICT_ALIAS upperDistance = reinterpret_cast<ArrayReal*RESTRICT_ALIAS>
																		(objData.mUpperDistance);

			//Test all 6 planes and AND the dot product. If one is false, then we're not visible
			ArrayReal dotResult;
			ArrayMaskR mask;
			ArrayVector3 centerPlusFlippedHS;
			centerPlusFlippedHS = objData.mWorldAabb->mCenter + objData.mWorldAabb->mHalfSize *
																 planes[0].signFlip;
			dotResult = planes[0].planeNormal.dotProduct( centerPlusFlippedHS );
			mask = Mathlib::CompareGreater( dotResult, planes[0].planeNegD );

			centerPlusFlippedHS = objData.mWorldAabb->mCenter + objData.mWorldAabb->mHalfSize *
																 planes[1].signFlip;
			dotResult = planes[1].planeNormal.dotProduct( centerPlusFlippedHS );
			mask = Mathlib::And( mask, Mathlib::CompareGreater( dotResult, planes[1].planeNegD ) );

			centerPlusFlippedHS = objData.mWorldAabb->mCenter + objData.mWorldAabb->mHalfSize *
																 planes[2].signFlip;
			dotResult = planes[2].planeNormal.dotProduct( centerPlusFlippedHS );
			mask = Mathlib::And( mask, Mathlib::CompareGreater( dotResult, planes[2].planeNegD ) );

			centerPlusFlippedHS = objData.mWorldAabb->mCenter + objData.mWorldAabb->mHalfSize *
																 planes[3].signFlip;
			dotResult = planes[3].planeNormal.dotProduct( centerPlusFlippedHS );
			mask = Mathlib::And( mask, Mathlib::CompareGreater( dotResult, planes[3].planeNegD ) );

			centerPlusFlippedHS = objData.mWorldAabb->mCenter + objData.mWorldAabb->mHalfSize *
																 planes[4].signFlip;
			dotResult = planes[4].planeNormal.dotProduct( centerPlusFlippedHS );
			mask = Mathlib::And( mask, Mathlib::CompareGreater( dotResult, planes[4].planeNegD ) );

			centerPlusFlippedHS = objData.mWorldAabb->mCenter + objData.mWorldAabb->mHalfSize *
																 planes[5].signFlip;
			dotResult = planes[5].planeNormal.dotProduct( centerPlusFlippedHS );
			mask = Mathlib::And( mask, Mathlib::CompareGreater( dotResult, planes[5].planeNegD ) );

			//Always pass the test if any of the components were
			//Infinity (dot product above could've caused nans)
			ArrayMaskR tmpMask = Mathlib::Or(
							Mathlib::isInfinity( objData.mWorldAabb->mHalfSize.mChunkBase[0] ),
							Mathlib::isInfinity( objData.mWorldAabb->mHalfSize.mChunkBase[1] ) );
			mask = Mathlib::Or( Mathlib::isInfinity( objData.mWorldAabb->mHalfSize.mChunkBase[2] ),
								mask );

			ArrayReal distance = lodCameraPos.distance( objData.mWorldAabb->mCenter );
			mask = Mathlib::And( Mathlib::Or( mask, tmpMask ),
								 Mathlib::CompareLessEqual( distance, *worldRadius + *upperDistance ) );

			//isVisible = isVisible() && (isCaster || includeNonCasters)
			ArrayMaskI isVisible = Mathlib::And(
								Mathlib::TestFlags4( *visibilityFlags,
														Mathlib::SetAll( LAYER_VISIBILITY ) ),
								Mathlib::TestFlags4( Mathlib::Or( *visibilityFlags, includeNonCasters ),
														Mathlib::SetAll( LAYER_SHADOW_CASTER ) ) );
			ArrayMaskI isReceiver= Mathlib::TestFlags4( *visibilityFlags,
														Mathlib::SetAll( LAYER_SHADOW_RECEIVER ) );

			//Fuse result with visibility flag
			// finalMask = ((visible|infinite_aabb) & sceneFlags & visibilityFlags) != 0 ? 0xffffffff : 0
			ArrayMaskI finalMask = Mathlib::TestFlags4( CastRealToInt( mask ),
														Mathlib::And( sceneFlags, *visibilityFlags ) );
            finalMask				= Mathlib::And( finalMask, isVisible );
			ArrayMaskR receiverMask  = CastIntToReal( Mathlib::And( finalMask, isReceiver ) );

			//Merge with bounds only if they're visible & are receivers. We first merge,
			//then CMov its older value if the object isn't visible/receiver.
			ArrayVector3 newVal( vMinBounds );
			newVal.makeFloor( objData.mWorldAabb->mCenter - objData.mWorldAabb->mHalfSize );
			vMinBounds.CmovRobust( receiverMask, newVal );

			newVal = vMaxBounds;
			newVal.makeCeil( objData.mWorldAabb->mCenter + objData.mWorldAabb->mHalfSize );
			vMaxBounds.CmovRobust( receiverMask, newVal );

			const uint32 scalarMask = BooleanMask4::getScalarMask( finalMask );

			for( size_t j=0; j<ARRAY_PACKED_REALS; ++j )
			{
				//Decompose the result for analyzing each MovableObject's
				//There's no need to check objData.mOwner[j] is null because
				//we set mVisibilityFlags to 0 on slot removals
				if( IS_BIT_SET( j, scalarMask ) )
				{
					culledObjects.push_back( objData.mOwner[j] );
				}
			}

			objData.advanceFrustumPack();
		}

		//Merge the individual values and return the result.
		if( outReceiversBox )
		{
			Vector3 vMin = vMinBounds.collapseMin();
			Vector3 vMax = vMaxBounds.collapseMax();
			if( vMin > vMax )
				outReceiversBox->setNull();
			else
				outReceiversBox->setExtents( vMin, vMax );
		}

		culledObjects.swap( outCulledObjects );
	}
	//-----------------------------------------------------------------------
	void MovableObject::cullReceiversBox( const size_t numNodes, ObjectData objData,
											const Frustum *frustum, uint32 sceneVisibilityFlags,
											AxisAlignedBox *outReceiversBox, const Camera *lodCamera )
	{
		//Thanks to Fabian Giesen for summing up all known methods of frustum culling:
		//http://fgiesen.wordpress.com/2010/10/17/view-frustum-culling/
		// (we use method Method 5: "If you really don’t care whether a box is
		// partially or fully inside"):
		// vector4 signFlip = componentwise_and(plane, 0x80000000);
		// return dot3(center + xor(extent, signFlip), plane) > -plane.w;
		struct ArrayPlane
		{
			ArrayVector3	planeNormal;
			ArrayVector3	signFlip;
			ArrayReal		planeNegD;
		};

		ArrayVector3 lodCameraPos;
		lodCameraPos.setAll( lodCamera->getDerivedPosition() );

		sceneVisibilityFlags &= RESERVED_VISIBILITY_FLAGS;

		ArrayInt sceneFlags = Mathlib::SetAll( sceneVisibilityFlags );
		ArrayPlane planes[6];
		const Plane *frustumPlanes = frustum->getFrustumPlanes();

		for( size_t i=0; i<6; ++i )
		{
			planes[i].planeNormal.setAll( frustumPlanes[i].normal );
			planes[i].signFlip.setAll( frustumPlanes[i].normal );
			planes[i].signFlip.setToSign();
			planes[i].planeNegD = Mathlib::SetAll( -frustumPlanes[i].d );
		}

		ArrayVector3 vMinBounds( Mathlib::MAX_POS, Mathlib::MAX_POS, Mathlib::MAX_POS );
		ArrayVector3 vMaxBounds( Mathlib::MAX_NEG, Mathlib::MAX_NEG, Mathlib::MAX_NEG );

		//TODO: Profile whether we should use XOR to flip the sign or simple multiplication.
		//In theory xor is faster, but some archs have a penalty for switching between integer
		//& floating point, even if it's simd sse
		for( size_t i=0; i<numNodes; i += ARRAY_PACKED_REALS )
		{
			ArrayInt * RESTRICT_ALIAS visibilityFlags = reinterpret_cast<ArrayInt*RESTRICT_ALIAS>
																		(objData.mVisibilityFlags);
			ArrayReal * RESTRICT_ALIAS worldRadius = reinterpret_cast<ArrayReal*RESTRICT_ALIAS>
																		(objData.mWorldRadius);
			ArrayReal * RESTRICT_ALIAS upperDistance = reinterpret_cast<ArrayReal*RESTRICT_ALIAS>
																		(objData.mUpperDistance);

			//Test all 6 planes and AND the dot product. If one is false, then we're not visible
			ArrayReal dotResult;
			ArrayMaskR mask;
			ArrayVector3 centerPlusFlippedHS;
			centerPlusFlippedHS = objData.mWorldAabb->mCenter + objData.mWorldAabb->mHalfSize *
																 planes[0].signFlip;
			dotResult = planes[0].planeNormal.dotProduct( centerPlusFlippedHS );
			mask = Mathlib::CompareGreater( dotResult, planes[0].planeNegD );

			centerPlusFlippedHS = objData.mWorldAabb->mCenter + objData.mWorldAabb->mHalfSize *
																 planes[1].signFlip;
			dotResult = planes[1].planeNormal.dotProduct( centerPlusFlippedHS );
			mask = Mathlib::And( mask, Mathlib::CompareGreater( dotResult, planes[1].planeNegD ) );

			centerPlusFlippedHS = objData.mWorldAabb->mCenter + objData.mWorldAabb->mHalfSize *
																 planes[2].signFlip;
			dotResult = planes[2].planeNormal.dotProduct( centerPlusFlippedHS );
			mask = Mathlib::And( mask, Mathlib::CompareGreater( dotResult, planes[2].planeNegD ) );

			centerPlusFlippedHS = objData.mWorldAabb->mCenter + objData.mWorldAabb->mHalfSize *
																 planes[3].signFlip;
			dotResult = planes[3].planeNormal.dotProduct( centerPlusFlippedHS );
			mask = Mathlib::And( mask, Mathlib::CompareGreater( dotResult, planes[3].planeNegD ) );

			centerPlusFlippedHS = objData.mWorldAabb->mCenter + objData.mWorldAabb->mHalfSize *
																 planes[4].signFlip;
			dotResult = planes[4].planeNormal.dotProduct( centerPlusFlippedHS );
			mask = Mathlib::And( mask, Mathlib::CompareGreater( dotResult, planes[4].planeNegD ) );

			centerPlusFlippedHS = objData.mWorldAabb->mCenter + objData.mWorldAabb->mHalfSize *
																 planes[5].signFlip;
			dotResult = planes[5].planeNormal.dotProduct( centerPlusFlippedHS );
			mask = Mathlib::And( mask, Mathlib::CompareGreater( dotResult, planes[5].planeNegD ) );

			//Always pass the test if any of the components were
			//Infinity (dot product above could've caused nans)
			ArrayMaskR tmpMask = Mathlib::Or(
							Mathlib::isInfinity( objData.mWorldAabb->mHalfSize.mChunkBase[0] ),
							Mathlib::isInfinity( objData.mWorldAabb->mHalfSize.mChunkBase[1] ) );
			mask = Mathlib::Or( Mathlib::isInfinity( objData.mWorldAabb->mHalfSize.mChunkBase[2] ),
								mask );

			ArrayReal distance = lodCameraPos.distance( objData.mWorldAabb->mCenter );
			mask = Mathlib::And( Mathlib::Or( mask, tmpMask ),
								 Mathlib::CompareLessEqual( distance, *worldRadius + *upperDistance ) );

			//No need to check for casters as in cullFrustum. See function documentation
			ArrayMaskI isVisible = Mathlib::TestFlags4( *visibilityFlags,
														Mathlib::SetAll( LAYER_VISIBILITY ) );
			ArrayMaskI isReceiver= Mathlib::TestFlags4( *visibilityFlags,
														Mathlib::SetAll( LAYER_SHADOW_RECEIVER ) );

			//Fuse result with visibility flag
			// finalMask = ((visible|infinite_aabb) & sceneFlags & visibilityFlags) != 0 ? 0xffffffff : 0
			ArrayMaskI finalMask = Mathlib::TestFlags4( CastRealToInt( mask ),
														Mathlib::And( sceneFlags, *visibilityFlags ) );
            finalMask				= Mathlib::And( finalMask, isVisible );
			ArrayMaskR receiverMask  = CastIntToReal( Mathlib::And( finalMask, isReceiver ) );

			//Merge with bounds only if they're visible & are receivers. We first merge,
			//then CMov its older value if the object isn't visible/receiver.
			ArrayVector3 newVal( vMinBounds );
			newVal.makeFloor( objData.mWorldAabb->mCenter - objData.mWorldAabb->mHalfSize );
			vMinBounds.CmovRobust( receiverMask, newVal );

			newVal = vMaxBounds;
			newVal.makeCeil( objData.mWorldAabb->mCenter + objData.mWorldAabb->mHalfSize );
			vMaxBounds.CmovRobust( receiverMask, newVal );

			objData.advanceFrustumPack();
		}

		//Merge the individual values and return the result.
		Vector3 vMin = vMinBounds.collapseMin();
		Vector3 vMax = vMaxBounds.collapseMax();
		if( vMin > vMax )
			outReceiversBox->setNull();
		else
			outReceiversBox->setExtents( vMin, vMax );
	}
	//-----------------------------------------------------------------------
	void MovableObject::cullLights( const size_t numNodes, ObjectData objData,
									LightListInfo &outGlobalLightList, const FrustumVec &frustums )
	{
		struct ArrayPlane
		{
			ArrayVector3	planeNormal;
			ArrayVector3	signFlip;
			ArrayReal		planeNegD;
		};
		struct ArraySixPlanes
		{
			ArrayPlane planes[6];
		};
		const size_t numFrustums = frustums.size();
		ArraySixPlanes *planes = OGRE_ALLOC_T_SIMD( ArraySixPlanes, numFrustums,
													MEMCATEGORY_SCENE_CONTROL );

		FrustumVec::const_iterator itor = frustums.begin();
		FrustumVec::const_iterator end  = frustums.end();
		ArraySixPlanes *planesIt = planes;

		while( itor != end )
		{
			const Plane *frustumPlanes = (*itor)->getFrustumPlanes();

			for( size_t i=0; i<6; ++i )
			{
				planesIt->planes[i].planeNormal.setAll( frustumPlanes[i].normal );
				planesIt->planes[i].signFlip.setAll( frustumPlanes[i].normal );
				planesIt->planes[i].signFlip.setToSign();
				planesIt->planes[i].planeNegD = Mathlib::SetAll( -frustumPlanes[i].d );
			}

			++planesIt;
			++itor;
		}

		//Implementation detail: Ogre 1.9 treated spotlights as a point (Sphere vs Plane collision test)
		//for simplicity (and presumably performance). We use aabbs for all lights in Ogre 2.0, which
		//plays better with area lights when we implemented (and spotlights too) degrading performance
		//for point lights

		//TODO: Profile whether we should use XOR to flip signs
		//instead of multiplication (see cullFrustum)
		for( size_t i=0; i<numNodes; i += ARRAY_PACKED_REALS )
		{
			//Initialize mask to 0
			ArrayMaskI mask = ARRAY_INT_ZERO;

			for( size_t j=0; j<numFrustums; ++j )
			{
				ArrayMaskR tmpMask;

				//Test all 6 planes and AND the dot product. If one is false, then we're not visible
				ArrayReal dotResult;
				ArrayVector3 centerPlusFlippedHS;
				centerPlusFlippedHS = objData.mWorldAabb->mCenter + objData.mWorldAabb->mHalfSize *
																	planes[j].planes[0].signFlip;
				dotResult = planes[j].planes[0].planeNormal.dotProduct( centerPlusFlippedHS );
				tmpMask = Mathlib::CompareGreater( dotResult, planes[j].planes[0].planeNegD );

				centerPlusFlippedHS = objData.mWorldAabb->mCenter + objData.mWorldAabb->mHalfSize *
																	 planes[j].planes[1].signFlip;
				dotResult = planes[j].planes[1].planeNormal.dotProduct( centerPlusFlippedHS );
				tmpMask = Mathlib::And( tmpMask, Mathlib::CompareGreater( dotResult,
																planes[j].planes[1].planeNegD ) );

				centerPlusFlippedHS = objData.mWorldAabb->mCenter + objData.mWorldAabb->mHalfSize *
																	 planes[j].planes[2].signFlip;
				dotResult = planes[j].planes[2].planeNormal.dotProduct( centerPlusFlippedHS );
				tmpMask = Mathlib::And( tmpMask, Mathlib::CompareGreater( dotResult,
																planes[j].planes[2].planeNegD ) );

				centerPlusFlippedHS = objData.mWorldAabb->mCenter + objData.mWorldAabb->mHalfSize *
																	 planes[j].planes[3].signFlip;
				dotResult = planes[j].planes[3].planeNormal.dotProduct( centerPlusFlippedHS );
				tmpMask = Mathlib::And( tmpMask, Mathlib::CompareGreater( dotResult,
																planes[j].planes[3].planeNegD ) );

				centerPlusFlippedHS = objData.mWorldAabb->mCenter + objData.mWorldAabb->mHalfSize *
																	 planes[j].planes[4].signFlip;
				dotResult = planes[j].planes[4].planeNormal.dotProduct( centerPlusFlippedHS );
				tmpMask = Mathlib::And( tmpMask, Mathlib::CompareGreater( dotResult,
																planes[j].planes[4].planeNegD ) );

				centerPlusFlippedHS = objData.mWorldAabb->mCenter + objData.mWorldAabb->mHalfSize *
																	 planes[j].planes[5].signFlip;
				dotResult = planes[j].planes[5].planeNormal.dotProduct( centerPlusFlippedHS );
				tmpMask = Mathlib::And( tmpMask, Mathlib::CompareGreater( dotResult,
																planes[j].planes[5].planeNegD ) );

				//Accumulate into mask. If one Frustum can see, then we need to include it.
				mask = Mathlib::Or( mask, CastRealToInt( tmpMask ) );
			}

			//Always pass the test if any of the components were
			//Infinity (dot product above could've caused nans)
			ArrayMaskR tmpMask = Mathlib::Or( Mathlib::Or(
							Mathlib::isInfinity( objData.mWorldAabb->mHalfSize.mChunkBase[0] ),
							Mathlib::isInfinity( objData.mWorldAabb->mHalfSize.mChunkBase[1] ) ),
							Mathlib::isInfinity( objData.mWorldAabb->mHalfSize.mChunkBase[2] ) );
			mask = Mathlib::Or( mask, CastRealToInt( tmpMask ) );

			//Use the light mask to discard null mOwner ptrs
			mask = Mathlib::TestFlags4( mask, *reinterpret_cast<ArrayInt*RESTRICT_ALIAS>
												(objData.mLightMask) );

			const uint32 scalarMask = BooleanMask4::getScalarMask( mask );

			for( size_t j=0; j<ARRAY_PACKED_REALS; ++j )
			{
				//Decompose the result for analyzing each MovableObject's
				//There's no need to check objData.mOwner[j] is null because
				//we set mVisibilityFlags to 0 on slot removals
				if( IS_BIT_SET( j, scalarMask ) )
				{
					const size_t idx = outGlobalLightList.lights.size();
					outGlobalLightList.visibilityMask[idx] = objData.mVisibilityFlags[j];
					outGlobalLightList.boundingSphere[idx] = Sphere(
														objData.mWorldAabb->mCenter.getAsVector3( j ),
														objData.mWorldRadius[j] );
					assert( dynamic_cast<Light*>( objData.mOwner[j] ) );
					outGlobalLightList.lights.push_back( static_cast<Light*>( objData.mOwner[j] ) );
				}
			}

			objData.advanceCullLightPack();
		}

		OGRE_FREE_SIMD( planes, MEMCATEGORY_SCENE_CONTROL );
		planes = 0;
	}
	//-----------------------------------------------------------------------
	void MovableObject::buildLightList( const size_t numNodes, ObjectData objData,
										const LightListInfo &globalLightList )
	{
		const size_t numGlobalLights = globalLightList.lights.size();
		ArraySphere lightSphere;
		OGRE_ALIGNED_DECL( Real, distance[ARRAY_PACKED_REALS], OGRE_SIMD_ALIGNMENT );
		for( size_t i=0; i<numNodes; i += ARRAY_PACKED_REALS )
		{
			ArrayReal * RESTRICT_ALIAS arrayRadius = reinterpret_cast<ArrayReal*RESTRICT_ALIAS>
																		(objData.mWorldRadius);
			ArraySphere objSphere( *arrayRadius, objData.mWorldAabb->mCenter );

			const ArrayInt * RESTRICT_ALIAS objLightMask = reinterpret_cast<ArrayInt*RESTRICT_ALIAS>
																				(objData.mLightMask);

			for( size_t j=0; j<ARRAY_PACKED_REALS; ++j )
				objData.mOwner[j]->mLightList.clear();

			//Now iterate through all lights to find the influence on these 4 Objects at once
			LightArray::const_iterator lightsIt				= globalLightList.lights.begin();
			const uint32 * RESTRICT_ALIAS	visibilityMask	= globalLightList.visibilityMask;
			const Sphere * RESTRICT_ALIAS 	boundingSphere	= globalLightList.boundingSphere;
			for( size_t j=0; j<numGlobalLights; ++j )
			{
				//We check 1 light against 4 MovableObjects at a time.
				lightSphere.setAll( *boundingSphere );

				//Check if it intersects
				ArrayMaskI rMask = CastRealToInt( lightSphere.intersects( objSphere ) );
				ArrayReal distSimd = objSphere.mCenter.distance( lightSphere.mCenter ) -
										lightSphere.mRadius;
				CastArrayToReal( distance, distSimd );

				//Note visibilityMask is shuffled ARRAY_PACKED_REALS times (it's 1 light, not 4)
				//rMask = ( intersects() && lightMask & visibilityMask )
				rMask = Mathlib::TestFlags4( rMask, Mathlib::And( *objLightMask, *visibilityMask ) );

				//Convert rMask into something smaller we can work with.
				uint32 r = BooleanMask4::getScalarMask( rMask );

				for( size_t k=0; k<ARRAY_PACKED_REALS; ++k )
				{
					//Decompose the result for analyzing each MovableObject's
					//There's no need to check objData.mOwner[k] is null because
					//we set lightMask to 0 on slot removals
					if( IS_BIT_SET( k, r ) )
					{
						LightList &lightList = objData.mOwner[k]->mLightList;
						lightList.push_back( LightClosest( *lightsIt, j, distance[k],
															lightList.size() ) );
					}
				}

				++lightsIt;
				++visibilityMask;
				++boundingSphere;
			}

			for( size_t j=0; j<ARRAY_PACKED_REALS; ++j )
			{
				std::stable_sort( objData.mOwner[j]->mLightList.begin(),
									objData.mOwner[j]->mLightList.end() );
			}

			objData.advanceLightPack();
		}
	}
	//-----------------------------------------------------------------------
	void MovableObject::calculateCastersBox( const size_t numNodes, ObjectData objData,
											 uint32 sceneVisibilityFlags, AxisAlignedBox *outBox )
	{
		ArrayInt sceneFlags = Mathlib::SetAll( sceneVisibilityFlags );

		ArrayVector3 vMinBounds( Mathlib::MAX_POS, Mathlib::MAX_POS, Mathlib::MAX_POS );
		ArrayVector3 vMaxBounds( Mathlib::MAX_NEG, Mathlib::MAX_NEG, Mathlib::MAX_NEG );

		for( size_t i=0; i<numNodes; i += ARRAY_PACKED_REALS )
		{
			ArrayInt * RESTRICT_ALIAS visibilityFlags = reinterpret_cast<ArrayInt*RESTRICT_ALIAS>
																		(objData.mVisibilityFlags);

			objData.mWorldAabb->mCenter + objData.mWorldAabb->mHalfSize;

			//Ignore casters with infinite boxes
			ArrayMaskR infMask = Mathlib::Or( Mathlib::Or(
							Mathlib::isInfinity( objData.mWorldAabb->mHalfSize.mChunkBase[0] ),
							Mathlib::isInfinity( objData.mWorldAabb->mHalfSize.mChunkBase[1] ) ),
							Mathlib::isInfinity( objData.mWorldAabb->mHalfSize.mChunkBase[2] ) );

			ArrayMaskI isVisible = Mathlib::TestFlags4( *visibilityFlags,
														Mathlib::SetAll( LAYER_VISIBILITY ) );
			ArrayMaskI isCaster  = Mathlib::TestFlags4( *visibilityFlags,
														Mathlib::SetAll( LAYER_SHADOW_CASTER ) );

			//Fuse result with visibility flag
			// finalMask = ( visible&!infiniteAabb & sceneFlags & visibilityFlags) != 0 ? 0xffffffff : 0
			ArrayMaskI finalMask = Mathlib::TestFlags4( Mathlib::And( sceneFlags, *visibilityFlags ),
												Mathlib::AndNot( isVisible, CastRealToInt( infMask ) ) );
			finalMask				= Mathlib::And( finalMask, isCaster );
			ArrayMaskR casterMask	= CastIntToReal( finalMask );

			//Merge with bounds only if they're visible. We first merge, then CMov its older
			//value if the object isn't visible. Also do the same with the receiver-only aabb
			ArrayVector3 newVal( vMinBounds );
			newVal.makeFloor( objData.mWorldAabb->mCenter - objData.mWorldAabb->mHalfSize );
			vMinBounds.CmovRobust( casterMask, newVal );

			newVal = vMaxBounds;
			newVal.makeCeil( objData.mWorldAabb->mCenter + objData.mWorldAabb->mHalfSize );
			vMaxBounds.CmovRobust( casterMask, newVal );

			objData.advanceFrustumPack();
		}

		//Merge the individual values and return the result.
		Vector3 vMin = vMinBounds.collapseMin();
		Vector3 vMax = vMaxBounds.collapseMax();
		if( vMin > vMax )
			outBox->setNull();
		else
			outBox->setExtents( vMin, vMax );
	}
	//-----------------------------------------------------------------------
	inline void MovableObject::lodSet( ObjectData &objData, Real lodValues[ARRAY_PACKED_REALS] )
	{
		for( size_t j=0; j<ARRAY_PACKED_REALS; ++j )
		{
			MovableObject *owner = objData.mOwner[j];

			//This may look like a lot of ugly indirections, but mLodMerged is a pointer that allows
			//sharing with many MovableObjects (it should perfectly fit even in small caches).
			{
				FastArray<Real>::const_iterator it = std::lower_bound( owner->mLodMesh->begin(),
																		owner->mLodMesh->end(),
																		lodValues[j] );
				owner->mCurrentMeshLod = it - owner->mLodMesh->begin();
			}

			FastArray< FastArray<Real> >::const_iterator itor = owner->mLodMaterial->begin();
			FastArray< FastArray<Real> >::const_iterator end  = owner->mLodMaterial->end();

			FastArray<unsigned char>::iterator itMatLodLevel = owner->mCurrentMaterialLod.begin();

			while( itor != end )
			{
				FastArray<Real>::const_iterator it = std::lower_bound( itor->begin(), itor->end(),
																	   lodValues[j] );
				*itMatLodLevel++ = (unsigned char)std::min<size_t>( (it - itor->begin()), itor->size() );
				++itor;
			}
		}
	}
	//-----------------------------------------------------------------------
	void MovableObject::lodDistance( const size_t numNodes, ObjectData objData, const Camera *camera )
	{
		ArrayVector3 cameraPos;
		cameraPos.setAll( camera->getDerivedPosition() );

		ArrayReal lodInvBias( Mathlib::SetAll( camera->_getLodBiasInverse() ) );
		OGRE_ALIGNED_DECL( Real, lodValues[ARRAY_PACKED_REALS], OGRE_SIMD_ALIGNMENT );

		for( size_t i=0; i<numNodes; i += ARRAY_PACKED_REALS )
		{
			ArrayReal * RESTRICT_ALIAS worldRadius = reinterpret_cast<ArrayReal*RESTRICT_ALIAS>
																		(objData.mWorldRadius);
			ArrayReal arrayLodValue = objData.mWorldAabb->mCenter.distance( cameraPos ) - (*worldRadius);
			arrayLodValue = arrayLodValue * lodInvBias;
			CastArrayToReal( lodValues, arrayLodValue );

			lodSet( objData, lodValues );

			objData.advanceLodPack();
		}
	}
	//-----------------------------------------------------------------------
	void MovableObject::lodPixelCount( const size_t numNodes, ObjectData objData, const Camera *camera )
	{
		if( camera->getProjectionType() == PT_PERSPECTIVE )
		{
			lodPixelCountPerspective( numNodes, objData, camera );
		}
		else
		{
			lodPixelCountOrthographic( numNodes, objData, camera );
		}
	}
	//-----------------------------------------------------------------------
	void MovableObject::lodPixelCountPerspective( const size_t numNodes, ObjectData objData,
													const Camera *camera )
	{
		const Viewport *viewport = camera->getLastViewport();
        Real viewportArea = static_cast<Real>(viewport->getActualWidth() * viewport->getActualHeight());

		ArrayVector3 cameraPos;
		cameraPos.setAll( camera->getDerivedPosition() );

		//vpAreaDotProjMat00dot11 is negative so we can store Lod values in ascending
		//order and use lower_bound (which wouldn't be the same as using upper_bound)
		const Matrix4 &projMat = camera->getProjectionMatrix();
		ArrayReal PiDotVpAreaDotProjMat00dot11(
						Mathlib::SetAll( -Math::PI * viewportArea * projMat[0][0] * projMat[1][1] ) );

		OGRE_ALIGNED_DECL( Real, lodValues[ARRAY_PACKED_REALS], OGRE_SIMD_ALIGNMENT );

		for( size_t i=0; i<numNodes; i += ARRAY_PACKED_REALS )
		{
			ArrayReal * RESTRICT_ALIAS worldRadius = reinterpret_cast<ArrayReal*RESTRICT_ALIAS>
																		(objData.mWorldRadius);
			ArrayReal sqDistance = objData.mWorldAabb->mCenter.squaredDistance( cameraPos );

			//Avoid division by zero
			sqDistance = Mathlib::Max( sqDistance, Mathlib::fEpsilon );

			// Get area of unprojected circle with object bounding radius
			//ArrayReal boundingArea = Mathlib::PI * (*worldRadius * *worldRadius);
			//ArrayReal arrayLodValue = (boundingArea * vpAreaDotProjMat00dot11) / sqDistance;

			ArrayReal sqRadius = (*worldRadius * *worldRadius);
			ArrayReal arrayLodValue = (sqRadius * PiDotVpAreaDotProjMat00dot11) / sqDistance;

			CastArrayToReal( lodValues, arrayLodValue );

			lodSet( objData, lodValues );

			objData.advanceLodPack();
		}
	}
	//-----------------------------------------------------------------------
	void MovableObject::lodPixelCountOrthographic( const size_t numNodes, ObjectData objData,
													const Camera *camera )
	{
		const Viewport *viewport = camera->getLastViewport();
        Real viewportArea = static_cast<Real>(viewport->getActualWidth() * viewport->getActualHeight());
		Real orthoArea = camera->getOrthoWindowHeight() * camera->getOrthoWindowWidth();

		//Avoid division by zero
		orthoArea = Ogre::max( orthoArea, 1e-6f );

		ArrayVector3 cameraPos;
		cameraPos.setAll( camera->getDerivedPosition() );

		//vpAreaDotProjMat00dot11 is negative so we can store Lod values in ascending
		//order and use lower_bound (which wouldn't be the same as using upper_bound)
		const Matrix4 &projMat = camera->getProjectionMatrix();
		ArrayReal PiDotVpAreaDivOrhtoArea( Mathlib::SetAll( -Math::PI * viewportArea / orthoArea ) );

		OGRE_ALIGNED_DECL( Real, lodValues[ARRAY_PACKED_REALS], OGRE_SIMD_ALIGNMENT );

		for( size_t i=0; i<numNodes; i += ARRAY_PACKED_REALS )
		{
			ArrayReal * RESTRICT_ALIAS worldRadius = reinterpret_cast<ArrayReal*RESTRICT_ALIAS>
																		(objData.mWorldRadius);
			ArrayReal arrayLodValue = (*worldRadius * *worldRadius) * PiDotVpAreaDivOrhtoArea;
			CastArrayToReal( lodValues, arrayLodValue );

			lodSet( objData, lodValues );

			objData.advanceLodPack();
		}
	}
	//-----------------------------------------------------------------------
	uint32 MovableObject::getTypeFlags(void) const
	{
		if (mCreator)
		{
			return mCreator->getTypeFlags();
		}
		else
		{
			return 0xFFFFFFFF;
		}
	}
	//---------------------------------------------------------------------
	class MORecvShadVisitor : public Renderable::Visitor
	{
	public:
		bool anyReceiveShadows;
		MORecvShadVisitor() : anyReceiveShadows(false)
		{

		}
		void visit(Renderable* rend, ushort lodIndex, bool isDebug, 
			Any* pAny = 0)
		{
			Technique* tech = rend->getTechnique();
			bool techReceivesShadows = tech && tech->getParent()->getReceiveShadows();
			anyReceiveShadows = anyReceiveShadows || 
				techReceivesShadows || !tech;
		}
	};
	//---------------------------------------------------------------------
	bool MovableObject::getReceivesShadows()
	{
		MORecvShadVisitor visitor;
		visitRenderables(&visitor);
		return visitor.anyReceiveShadows;		

	}
	//-----------------------------------------------------------------------
	//-----------------------------------------------------------------------
	MovableObject* MovableObjectFactory::createInstance( IdType id,
								ObjectMemoryManager *objectMemoryManager, SceneManager* manager,
								const NameValuePairList* params )
	{
		MovableObject* m = createInstanceImpl( id, objectMemoryManager, params );
		m->_notifyCreator(this);
		m->_notifyManager(manager);
		return m;
	}


	inline bool LightClosest::operator < (const LightClosest &right) const
	{
		/*
		Shouldn't be necessary. distance is insanely low (big negative number)
		if( light->getType() == Light::LT_DIRECTIONAL &&
			right.light->getType() != Light::LT_DIRECTIONAL )
		{
			return true;
		}
		else if( light->getType() != Light::LT_DIRECTIONAL &&
				 right.light->getType() == Light::LT_DIRECTIONAL )
		{
			return false;
		}*/

		return distance < right.distance;
	}
}

