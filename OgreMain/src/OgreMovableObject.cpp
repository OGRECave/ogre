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

#include "OgreTagPoint.h"
#include "OgreEntity.h"
#include "OgreLodListener.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    uint32 MovableObject::msDefaultQueryFlags = 0xFFFFFFFF;
    uint32 MovableObject::msDefaultVisibilityFlags = 0xFFFFFFFF;
    //-----------------------------------------------------------------------
    MovableObject::MovableObject() : MovableObject(BLANKSTRING) {}
    //-----------------------------------------------------------------------
    MovableObject::MovableObject(const String& name)
        : mName(name)
        , mCreator(0)
        , mManager(0)
        , mParentNode(0)
        , mListener(0)
        , mParentIsTagPoint(false)
        , mVisible(true)
        , mDebugDisplay(false)
        , mBeyondFarDistance(false)
        , mCastShadows(true)
        , mRenderQueueIDSet(false)
        , mRenderQueuePrioritySet(false)
        , mRenderingDisabled(false)
        , mRenderQueueID(RENDER_QUEUE_MAIN)
        , mRenderQueuePriority(100)
        , mUpperDistance(0)
        , mSquaredUpperDistance(0)
        , mMinPixelSize(0)
        , mQueryFlags(msDefaultQueryFlags)
        , mVisibilityFlags(msDefaultVisibilityFlags)
        , mLightListUpdated(0)
        , mLightMask(0xFFFFFFFF)
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

        detachFromParent(); // this should never throw, if it does terminating is the thing to do
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
            mBeyondFarDistance = false;

            if (cam->getUseRenderingDistance() && mUpperDistance > 0)
            {
                Real rad = getBoundingRadiusScaled();
                Real squaredDist = mParentNode->getSquaredViewDepth(cam->getLodCamera());

                // Max distance to still render
                Real maxDist = mUpperDistance + rad;
                if (squaredDist > Math::Sqr(maxDist))
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
                float sqrObjMedianSize = std::max(std::max(
                                    std::min(objBound.x,objBound.y),
                                    std::min(objBound.x,objBound.z)),
                                    std::min(objBound.y,objBound.z));

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
    }
    //-----------------------------------------------------------------------
    void MovableObject::setRenderQueueGroup(uint8 queueID)
    {
        assert(queueID <= RENDER_QUEUE_MAX && "Render queue out of range!");
        mRenderQueueID = queueID;
        mRenderQueueIDSet = true;
    }

    //-----------------------------------------------------------------------
    void MovableObject::setRenderQueueGroupAndPriority(uint8 queueID, ushort priority)
    {
        setRenderQueueGroup(queueID);
        mRenderQueuePriority = priority;
        mRenderQueuePrioritySet = true;

    }

    //-----------------------------------------------------------------------
    const Affine3& MovableObject::_getParentNodeFullTransform(void) const
    {
        
        if(mParentNode)
        {
            // object attached to a sceneNode
            return mParentNode->_getFullTransform();
        }
        // fallback
        return Affine3::IDENTITY;
    }

    Real MovableObject::getBoundingRadiusScaled() const
    {
        const Vector3& scl = mParentNode->_getDerivedScale();
        Real factor = std::max(std::max(std::abs(scl.x), std::abs(scl.y)), std::abs(scl.z));
        return getBoundingRadius() * factor;
    }

    //-----------------------------------------------------------------------
    const AxisAlignedBox& MovableObject::getWorldBoundingBox(bool derive) const
    {
        if (derive)
        {
            mWorldAABB = this->getBoundingBox();
            mWorldAABB.transform(_getParentNodeFullTransform());
        }

        return mWorldAABB;

    }
    //-----------------------------------------------------------------------
    const Sphere& MovableObject::getWorldBoundingSphere(bool derive) const
    {
        if (derive)
        {
            mWorldBoundingSphere.setRadius(getBoundingRadiusScaled());
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

                sn->findLights(mLightList, getBoundingRadiusScaled(), this->getLightMask());
            }
        }
        else
        {
            mLightList.clear();
        }

        return mLightList;
    }
    //-----------------------------------------------------------------------
    const ShadowRenderableList& MovableObject::getShadowVolumeRenderableList(
        const Light* light, const HardwareIndexBufferPtr& indexBuffer, size_t& indexBufferUsedSize,
        float extrusionDist, int flags)
    {
        static ShadowRenderableList dummyList;
        return dummyList;
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
            // exclude distance from the light to the shadow caster from the extrusion
            Real extrusionDistance = l->getAttenuationRange() - getWorldBoundingBox().distance(l->getDerivedPosition());
            if(extrusionDistance < 0)
                extrusionDistance = 0;
            
            // Take into account that extrusion would be done in object-space,
            // and non-uniformly scaled objects would cast non-uniformly scaled shadows.
            Matrix3 m3 = _getParentNodeFullTransform().linear();
            Real c0 = m3.GetColumn(0).squaredLength(), c1 = m3.GetColumn(1).squaredLength(), c2 = m3.GetColumn(2).squaredLength();
            Real minScale = Math::Sqrt(std::min(std::min(c0, c1), c2));
            Real maxScale = Math::Sqrt(std::max(std::max(c0, c1), c2));
            if(minScale > 0.0)
                extrusionDistance *= (maxScale / minScale);
            
            return extrusionDistance;
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
            Any* pAny = 0) override
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

