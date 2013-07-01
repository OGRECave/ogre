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
	//-----------------------------------------------------------------------
	//-----------------------------------------------------------------------
	uint32 MovableObject::msDefaultQueryFlags = 0xFFFFFFFF;
	uint32 MovableObject::msDefaultVisibilityFlags = 0xFFFFFFFF;
    //-----------------------------------------------------------------------
    MovableObject::MovableObject( IdType id )
        : IdObject( id )
		, mCreator(0)
        , mManager(0)
        , mParentNode(0)
		, mDebugDisplay(false)
		, mUpperDistance( std::numeric_limits<float>::max() )
		, mMinPixelSize(0)
        , mRenderQueueID(RENDER_QUEUE_MAIN)
		, mRenderQueuePriority(100)
        , mCastShadows(true)
        , mListener(0)
    {
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
    }
    //-----------------------------------------------------------------------
    void MovableObject::_notifyAttached( Node* parent )
    {
        assert(!mParentNode || !parent);

        bool different = (parent != mParentNode);

        mParentNode = parent;
		mObjectData.mParents[mObjectData.mIndex] = parent;

        // Call listener (note, only called if there's something to do)
        if (mListener && different)
        {
            if (mParentNode)
                mListener->objectAttached(this);
            else
                mListener->objectDetached(this);
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

            // Notify lod event listeners
            cam->getSceneManager()->_notifyMovableObjectLodChanged(evt);

		}

        mRenderingDisabled = mListener && !mListener->objectRendering(this, cam);
	}*/
    //-----------------------------------------------------------------------
    void MovableObject::setRenderQueueGroup(uint8 queueID)
    {
		assert(queueID <= RENDER_QUEUE_MAX && "Render queue out of range!");
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
	Matrix4 MovableObject::_getParentNodeFullTransform(void) const
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
	struct LightListInfo
	{
		LightVec						lights;
		///Copy from lights[i]->getVisibilityFlags(), this copy avoids one level of indirection
		uint32	* RESTRICT_ALIAS		visibilityMask;
		Sphere	* RESTRICT_ALIAS 		boundingSphere;
	};
	void MovableObject::buildLightList( const size_t numNodes, ObjectData objData )
	{
		size_t numGlobalLights=0;
		LightListInfo globalLightList;
		//mLightList
		ArraySphere lightSphere;
		OGRE_ALIGNED_DECL( Real, distance[ARRAY_PACKED_REALS], 16 );
		for( size_t i=0; i<numNodes; i += ARRAY_PACKED_REALS )
		{
			//Retrieve from parents. Unfortunately we need to do SoA -> AoS -> SoA conversion
			/*ArrayVector3 parentPos, parentScale;
			ArrayQuaternion parentRot;

			for( size_t j=0; j<ARRAY_PACKED_REALS; ++j )
			{
				Vector3 pos, scale;
				Quaternion qRot;
				const Transform &parentTransform = objData.mParents[j]->_getTransform();
				parentTransform.mDerivedPosition->getAsVector3( pos, parentTransform.mIndex );
				parentTransform.mDerivedOrientation->getAsQuaternion( qRot, parentTransform.mIndex );
				parentTransform.mDerivedScale->getAsVector3( scale, parentTransform.mIndex );

				parentPos.setFromVector3( pos, j );
				parentRot.setFromQuaternion( qRot, j );
				parentScale.setFromVector3( scale, j );
			}*/

			ArrayReal * RESTRICT_ALIAS arrayRadius = reinterpret_cast<ArrayReal*RESTRICT_ALIAS>
																		(objData.mWorldRadius);
			ArraySphere objSphere( *arrayRadius, objData.mWorldAabb->m_center );

			const ArrayInt * RESTRICT_ALIAS objLightMask = reinterpret_cast<ArrayInt*RESTRICT_ALIAS>
																				(objData.mLightMask);

			for( size_t j=0; j<ARRAY_PACKED_REALS; ++j )
				objData.mOwner[j]->mLightList.clear();

			//Now iterate through all lights to find the influence on these 4 Objects at once
			LightVec::const_iterator lightsIt				= globalLightList.lights.begin();
			const uint32 * RESTRICT_ALIAS	visibilityMask	= globalLightList.visibilityMask;
			const Sphere * RESTRICT_ALIAS 	boundingSphere	= globalLightList.boundingSphere;
			for( size_t j=0; j<numGlobalLights; ++j )
			{
				//We check 1 light against 4 MovableObjects at a time.
				lightSphere.setAll( *boundingSphere );

				//Check if it intersects
				ArrayInt rMask = CastRealToInt( lightSphere.intersects( objSphere ) );
				ArrayReal distSimd = objSphere.m_center.squaredDistance( lightSphere.m_center );
				CastArrayToReal( distance, distSimd );

				//rMask = ( intersects() && lightMask & visibilityMask )
				rMask = MathlibSSE2::TestFlags32( rMask, MathlibSSE2::And( *objLightMask,
																			*visibilityMask ) );

				//Convert rMask into something smaller we can work with.
				uint32 r = BooleanMask4::getScalarMask( rMask );

				for( size_t k=0; k<ARRAY_PACKED_REALS; ++k )
				{
					//Decompose the result for analyzing each MovableObject's
					//There's no need to check objData.mOwner[k] is null because
					//we set lightMask to 0 on slot removals
					if( IS_BIT_SET( k, r ) && objData.mOwner[k] )
					{
						objData.mOwner[k]->mLightList.push_back(
													LightClosest( *lightsIt, distance[k] ) );
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
    const LightList& MovableObject::queryLights(void)
    {
        if (mParentNode)
        {
            SceneNode* sn = static_cast<SceneNode*>(mParentNode);

			const Vector3& scl = mParentNode->_getDerivedScale();
			Real factor = std::max(std::max(scl.x, scl.y), scl.z);

            sn->findLights(mLightList, this->getBoundingRadius() * factor, this->getLightMask());
        }
        else
        {
            mLightList.clear();
        }

        return mLightList;
    }
    //-----------------------------------------------------------------------
    MovableObject::ShadowRenderableListIterator MovableObject::getShadowVolumeRenderableIterator(
        ShadowTechnique shadowTechnique, const Light* light, 
        HardwareIndexBufferSharedPtr* indexBuffer, 
        bool inExtrudeVertices, Real extrusionDist, unsigned long flags )
    {
        static ShadowRenderableList dummyList;
        return ShadowRenderableListIterator(dummyList.begin(), dummyList.end());
    }
    //-----------------------------------------------------------------------
    const Aabb MovableObject::getLightCapBounds(void) const
    {
		//TODO: (dark_sylinc) Avoid using this function completely (use SIMD)
        // Same as original bounds
		return getWorldAabb();
    }
	//-----------------------------------------------------------------------
	const Aabb MovableObject::getLightCapBoundsUpdated(void)
    {
		//TODO: (dark_sylinc) Avoid using this function completely (use SIMD)
        // Same as original bounds
		return getWorldAabbUpdated();
    }
    //-----------------------------------------------------------------------
    /*const AxisAlignedBox& MovableObject::getDarkCapBounds(const Light& light, Real extrusionDist) const
    {
        // Extrude own light cap bounds
        mWorldDarkCapBounds = getLightCapBounds();
        this->extrudeBounds(mWorldDarkCapBounds, light.getAs4DVector(), 
            extrusionDist);
        return mWorldDarkCapBounds;
    }
    //-----------------------------------------------------------------------
    Real MovableObject::getPointExtrusionDistance(const Light* l) const
    {
        if (mParentNode)
        {
            return getExtrusionDistance(mParentNode->_getDerivedPosition(), l);
        }
        else
        {
            return 0;
        }
    }*/
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
	MovableObject* MovableObjectFactory::createInstance(
		const String& name, SceneManager* manager, 
		const NameValuePairList* params)
	{
		MovableObject* m = createInstanceImpl(name, params);
		m->_notifyCreator(this);
		m->_notifyManager(manager);
		return m;
	}


	inline bool MovableObject::LightClosest::operator < (const MovableObject::LightClosest &right) const
	{
		if( light->getType() == Light::LT_DIRECTIONAL &&
			right.light->getType() != Light::LT_DIRECTIONAL )
		{
			return true;
		}
		else if( light->getType() != Light::LT_DIRECTIONAL &&
				 right.light->getType() == Light::LT_DIRECTIONAL )
		{
			return false;
		}

		return sqDistance < right.sqDistance;
	}
}

