/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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

namespace Ogre {
	//-----------------------------------------------------------------------
	//-----------------------------------------------------------------------
	uint32 MovableObject::msDefaultQueryFlags = 0xFFFFFFFF;
	uint32 MovableObject::msDefaultVisibilityFlags = 0xFFFFFFFF;
    //-----------------------------------------------------------------------
    MovableObject::MovableObject()
        : mCreator(0)
        , mManager(0)
        , mParentNode(0)
        , mParentIsTagPoint(false)
        , mVisible(true)
		, mDebugDisplay(false)
        , mUpperDistance(0)
        , mSquaredUpperDistance(0)
        , mBeyondFarDistance(false)
        , mRenderQueueID(RENDER_QUEUE_MAIN)
        , mRenderQueueIDSet(false)
        , mQueryFlags(msDefaultQueryFlags)
        , mVisibilityFlags(msDefaultVisibilityFlags)
        , mCastShadows(true)
        , mRenderingDisabled(false)
        , mListener(0)
        , mLightListUpdated(0)
		, mLightMask(0xFFFFFFFF)
    {
    }
    //-----------------------------------------------------------------------
    MovableObject::MovableObject(const String& name)
        : mName(name)
        , mCreator(0)
        , mManager(0)
        , mParentNode(0)
        , mParentIsTagPoint(false)
        , mVisible(true)
		, mDebugDisplay(false)
        , mUpperDistance(0)
        , mSquaredUpperDistance(0)
        , mBeyondFarDistance(false)
        , mRenderQueueID(RENDER_QUEUE_MAIN)
        , mRenderQueueIDSet(false)
        , mQueryFlags(msDefaultQueryFlags)
        , mVisibilityFlags(msDefaultVisibilityFlags)
        , mCastShadows(true)
        , mRenderingDisabled(false)
        , mListener(0)
        , mLightListUpdated(0)
		, mLightMask(0xFFFFFFFF)
    {
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
            // detach from parent
            if (mParentIsTagPoint)
            {
                // May be we are a lod entity which not in the parent entity child object list,
                // call this method could safely ignore this case.
                static_cast<TagPoint*>(mParentNode)->getParentEntity()->detachObjectFromBone(this);
            }
            else
            {
                // May be we are a lod entity which not in the parent node child object list,
                // call this method could safely ignore this case.
                static_cast<SceneNode*>(mParentNode)->detachObject(this);
            }
        }
    }
    //-----------------------------------------------------------------------
    void MovableObject::_notifyAttached(Node* parent, bool isTagPoint)
    {
        assert(!mParentNode || !parent);

        bool different = (parent != mParentNode);

        mParentNode = parent;
        mParentIsTagPoint = isTagPoint;

        // Mark light list being dirty, simply decrease
        // counter by one for minimise overhead
        --mLightListUpdated;

        // Call listener (note, only called if there's something to do)
        if (mListener && different)
        {
            if (mParentNode)
                mListener->objectAttached(this);
            else
                mListener->objectDetached(this);
        }
    }
    //-----------------------------------------------------------------------
    Node* MovableObject::getParentNode(void) const
    {
        return mParentNode;
    }
    //-----------------------------------------------------------------------
    SceneNode* MovableObject::getParentSceneNode(void) const
    {
        if (mParentIsTagPoint)
        {
            TagPoint* tp = static_cast<TagPoint*>(mParentNode);
            return tp->getParentEntity()->getParentSceneNode();
        }
        else
        {
            return static_cast<SceneNode*>(mParentNode);
        }
    }
    //-----------------------------------------------------------------------
    bool MovableObject::isAttached(void) const
    {
        return (mParentNode != 0);

    }
	//---------------------------------------------------------------------
	void MovableObject::detachFromParent(void)
	{
		if (isAttached())
		{
			if (mParentIsTagPoint)
			{
				TagPoint* tp = static_cast<TagPoint*>(mParentNode);
				tp->getParentEntity()->detachObjectFromBone(this);
			}
			else
			{
				SceneNode* sn = static_cast<SceneNode*>(mParentNode);
				sn->detachObject(this);
			}
		}
	}
    //-----------------------------------------------------------------------
	bool MovableObject::isInScene(void) const
	{
		if (mParentNode != 0)
		{
			if (mParentIsTagPoint)
			{
				TagPoint* tp = static_cast<TagPoint*>(mParentNode);
				return tp->getParentEntity()->isInScene();
			}
			else
			{
				SceneNode* sn = static_cast<SceneNode*>(mParentNode);
				return sn->isInSceneGraph();
			}
		}
		else
		{
			return false;
		}
	}
    //-----------------------------------------------------------------------
    void MovableObject::_notifyMoved(void)
    {
        // Mark light list being dirty, simply decrease
        // counter by one for minimise overhead
        --mLightListUpdated;

        // Notify listener if exists
        if (mListener)
        {
            mListener->objectMoved(this);
        }
    }
    //-----------------------------------------------------------------------
    void MovableObject::setVisible(bool visible)
    {
        mVisible = visible;
    }
    //-----------------------------------------------------------------------
    bool MovableObject::getVisible(void) const
    {
        return mVisible;
    }
    //-----------------------------------------------------------------------
    bool MovableObject::isVisible(void) const
    {
        if (!mVisible || mBeyondFarDistance || mRenderingDisabled)
            return false;

        SceneManager* sm = Root::getSingleton()._getCurrentSceneManager();
        if (sm && !(getVisibilityFlags() & sm->_getCombinedVisibilityMask()))
            return false;

        return true;
    }
	//-----------------------------------------------------------------------
	void MovableObject::_notifyCurrentCamera(Camera* cam)
	{
		if (mParentNode)
		{
			if (cam->getUseRenderingDistance() && mUpperDistance > 0)
			{
				Real rad = getBoundingRadius();
				Real squaredDepth = mParentNode->getSquaredViewDepth(cam->getLodCamera());
				// Max distance to still render
				Real maxDist = mUpperDistance + rad;
				if (squaredDepth > Math::Sqr(maxDist))
				{
					mBeyondFarDistance = true;
				}
				else
				{
					mBeyondFarDistance = false;
				}
			}
			else
			{
				mBeyondFarDistance = false;
			}

            // Construct event object
            MovableObjectLodChangedEvent evt;
            evt.movableObject = this;
            evt.camera = cam;

            // Notify lod event listeners
            cam->getSceneManager()->_notifyMovableObjectLodChanged(evt);

		}

        mRenderingDisabled = mListener && !mListener->objectRendering(this, cam);
	}
    //-----------------------------------------------------------------------
    void MovableObject::setRenderQueueGroup(uint8 queueID)
    {
		assert(queueID <= RENDER_QUEUE_MAX && "Render queue out of range!");
        mRenderQueueID = queueID;
        mRenderQueueIDSet = true;
    }
    //-----------------------------------------------------------------------
    uint8 MovableObject::getRenderQueueGroup(void) const
    {
        return mRenderQueueID;
    }
    //-----------------------------------------------------------------------
	const Matrix4& MovableObject::_getParentNodeFullTransform(void) const
	{
		
		if(mParentNode)
		{
			// object attached to a sceneNode
			return mParentNode->_getFullTransform();
		}
        // fallback
        return Matrix4::IDENTITY;
	}
    //-----------------------------------------------------------------------
    const AxisAlignedBox& MovableObject::getWorldBoundingBox(bool derive) const
    {
        if (derive)
        {
            mWorldAABB = this->getBoundingBox();
            mWorldAABB.transformAffine(_getParentNodeFullTransform());
        }

        return mWorldAABB;

    }
    //-----------------------------------------------------------------------
	const Sphere& MovableObject::getWorldBoundingSphere(bool derive) const
	{
		if (derive)
		{
			const Vector3& scl = mParentNode->_getDerivedScale();
			Real factor = std::max(std::max(scl.x, scl.y), scl.z);
			mWorldBoundingSphere.setRadius(getBoundingRadius() * factor);
			mWorldBoundingSphere.setCenter(mParentNode->_getDerivedPosition());
		}
		return mWorldBoundingSphere;
	}
    //-----------------------------------------------------------------------
    const LightList& MovableObject::queryLights(void) const
    {
        // Try listener first
        if (mListener)
        {
            const LightList* lightList =
                mListener->objectQueryLights(this);
            if (lightList)
            {
                return *lightList;
            }
        }

        // Query from parent entity if exists
        if (mParentIsTagPoint)
        {
            TagPoint* tp = static_cast<TagPoint*>(mParentNode);
            return tp->getParentEntity()->queryLights();
        }

        if (mParentNode)
        {
            SceneNode* sn = static_cast<SceneNode*>(mParentNode);

            // Make sure we only update this only if need.
            ulong frame = sn->getCreator()->_getLightsDirtyCounter();
            if (mLightListUpdated != frame)
            {
                mLightListUpdated = frame;

                sn->findLights(mLightList, this->getBoundingRadius(), this->getLightMask());
            }
        }
        else
        {
            mLightList.clear();
        }

        return mLightList;
    }
    //-----------------------------------------------------------------------
    ShadowCaster::ShadowRenderableListIterator MovableObject::getShadowVolumeRenderableIterator(
        ShadowTechnique shadowTechnique, const Light* light, 
        HardwareIndexBufferSharedPtr* indexBuffer, 
        bool extrudeVertices, Real extrusionDist, unsigned long flags )
    {
        static ShadowRenderableList dummyList;
        return ShadowRenderableListIterator(dummyList.begin(), dummyList.end());
    }
    //-----------------------------------------------------------------------
    const AxisAlignedBox& MovableObject::getLightCapBounds(void) const
    {
        // Same as original bounds
        return getWorldBoundingBox();
    }
    //-----------------------------------------------------------------------
    const AxisAlignedBox& MovableObject::getDarkCapBounds(const Light& light, Real extrusionDist) const
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
	void MovableObject::setLightMask(uint32 lightMask)
	{
		this->mLightMask = lightMask;
		//make sure to request a new light list from the scene manager if mask changed
		mLightListUpdated = 0;
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


}

