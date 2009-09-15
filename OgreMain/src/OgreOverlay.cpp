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

#include "OgreOverlay.h"
#include "OgreRoot.h"
#include "OgreSceneManager.h"
#include "OgreOverlayContainer.h"
#include "OgreCamera.h"
#include "OgreOverlayManager.h"
#include "OgreQuaternion.h"
#include "OgreVector3.h"


namespace Ogre {

    //---------------------------------------------------------------------
    Overlay::Overlay(const String& name) :
        mName(name),
        mRotate(0.0f), 
        mScrollX(0.0f), mScrollY(0.0f),
        mScaleX(1.0f), mScaleY(1.0f), 
        mTransformOutOfDate(true), mTransformUpdated(true), 
        mZOrder(100), mVisible(false), mInitialised(false)

    {
        mRootNode = OGRE_NEW SceneNode(NULL);

    }
    //---------------------------------------------------------------------
    Overlay::~Overlay()
    {
		// remove children

        OGRE_DELETE mRootNode;
		
		for (OverlayContainerList::iterator i = m2DElements.begin(); 
			 i != m2DElements.end(); ++i)
		{
			(*i)->_notifyParent(0, 0);
		}
    }
    //---------------------------------------------------------------------
    const String& Overlay::getName(void) const
    {
        return mName;
    }
    //---------------------------------------------------------------------
    void Overlay::assignZOrders()
	{
		ushort zorder = mZOrder * 100;

        // Notify attached 2D elements
        OverlayContainerList::iterator i, iend;
        iend = m2DElements.end();
        for (i = m2DElements.begin(); i != iend; ++i)
        {
            zorder = (*i)->_notifyZOrder(zorder);
        }
	}
    //---------------------------------------------------------------------
    void Overlay::setZOrder(ushort zorder)
    {
        // Limit to 650 since this is multiplied by 100 to pad out for containers
        assert (zorder <= 650 && "Overlay ZOrder cannot be greater than 650!");

        mZOrder = zorder;

		assignZOrders();
    }
    //---------------------------------------------------------------------
    ushort Overlay::getZOrder(void) const
    {
        return mZOrder;
    }
    //---------------------------------------------------------------------
    bool Overlay::isVisible(void) const
    {
        return mVisible;
    }
    //---------------------------------------------------------------------
    void Overlay::show(void)
    {
        mVisible = true;
		if (!mInitialised)
		{
			initialise();
		}
    }
    //---------------------------------------------------------------------
    void Overlay::hide(void)
    {
        mVisible = false;
    }
    //---------------------------------------------------------------------
	void Overlay::initialise(void)
	{
		OverlayContainerList::iterator i, iend;
		iend = m2DElements.end();
		for (i = m2DElements.begin(); i != m2DElements.end(); ++i)
		{
			(*i)->initialise();
		}
		mInitialised = true;
	}
	//---------------------------------------------------------------------
    void Overlay::add2D(OverlayContainer* cont)
    {
        m2DElements.push_back(cont);
        // Notify parent
        cont->_notifyParent(0, this);

		assignZOrders();

        Matrix4 xform;
        _getWorldTransforms(&xform);
        cont->_notifyWorldTransforms(xform);
        cont->_notifyViewport();
    }
    //---------------------------------------------------------------------
    void Overlay::remove2D(OverlayContainer* cont)
    {
        m2DElements.remove(cont);
		cont->_notifyParent(0, 0);
		assignZOrders();
    }
    //---------------------------------------------------------------------
    void Overlay::add3D(SceneNode* node)
    {
        mRootNode->addChild(node);
    }
    //---------------------------------------------------------------------
    void Overlay::remove3D(SceneNode* node)
    {
        mRootNode->removeChild(node->getName());
    }
    //---------------------------------------------------------------------
    void Overlay::clear(void)
    {
        mRootNode->removeAllChildren();
        m2DElements.clear();
        // Note no deallocation, memory handled by OverlayManager & SceneManager
    }
    //---------------------------------------------------------------------
    void Overlay::setScroll(Real x, Real y)
    {
        mScrollX = x;
        mScrollY = y;
        mTransformOutOfDate = true;
        mTransformUpdated = true;
    }
    //---------------------------------------------------------------------
    Real Overlay::getScrollX(void) const
    {
        return mScrollX;
    }
    //---------------------------------------------------------------------
    Real Overlay::getScrollY(void) const
    {
        return mScrollY;
    }
      //---------------------------------------------------------------------
    OverlayContainer* Overlay::getChild(const String& name)
    {

        OverlayContainerList::iterator i, iend;
        iend = m2DElements.end();
        for (i = m2DElements.begin(); i != iend; ++i)
        {
            if ((*i)->getName() == name)
			{
				return *i;

			}
        }
        return NULL;
    }
  //---------------------------------------------------------------------
    void Overlay::scroll(Real xoff, Real yoff)
    {
        mScrollX += xoff;
        mScrollY += yoff;
        mTransformOutOfDate = true;
        mTransformUpdated = true;
    }
    //---------------------------------------------------------------------
    void Overlay::setRotate(const Radian& angle)
    {
        mRotate = angle;
        mTransformOutOfDate = true;
        mTransformUpdated = true;
    }
    //---------------------------------------------------------------------
    void Overlay::rotate(const Radian& angle)
    {
        setRotate(mRotate + angle);
    }
    //---------------------------------------------------------------------
    void Overlay::setScale(Real x, Real y)
    {
        mScaleX = x;
        mScaleY = y;
        mTransformOutOfDate = true;
        mTransformUpdated = true;
    }
    //---------------------------------------------------------------------
    Real Overlay::getScaleX(void) const
    {
        return mScaleX;
    }
    //---------------------------------------------------------------------
    Real Overlay::getScaleY(void) const
    {
        return mScaleY;
    }
    //---------------------------------------------------------------------
    void Overlay::_getWorldTransforms(Matrix4* xform) const
    {
        if (mTransformOutOfDate)
        {
            updateTransform();
        }
        *xform = mTransform;

    }
    //---------------------------------------------------------------------
    void Overlay::_findVisibleObjects(Camera* cam, RenderQueue* queue)
    {
        OverlayContainerList::iterator i, iend;

        if (OverlayManager::getSingleton().hasViewportChanged())
        {
            iend = m2DElements.end();
            for (i = m2DElements.begin(); i != iend; ++i)
            {
                (*i)->_notifyViewport();
            }
        }

        // update elements
        if (mTransformUpdated)
        {
            OverlayContainerList::iterator i, iend;
            Matrix4 xform;

            _getWorldTransforms(&xform);
            iend = m2DElements.end();
            for (i = m2DElements.begin(); i != iend; ++i)
            {
                (*i)->_notifyWorldTransforms(xform);
            }

            mTransformUpdated = false;
        }

        if (mVisible)
        {
            // Add 3D elements
            mRootNode->setPosition(cam->getDerivedPosition());
            mRootNode->setOrientation(cam->getDerivedOrientation());
            mRootNode->_update(true, false);
            // Set up the default queue group for the objects about to be added
            uint8 oldgrp = queue->getDefaultQueueGroup();
            ushort oldPriority = queue-> getDefaultRenderablePriority();
            queue->setDefaultQueueGroup(RENDER_QUEUE_OVERLAY);
            queue->setDefaultRenderablePriority((mZOrder*100)-1);
            mRootNode->_findVisibleObjects(cam, queue, NULL, true, false);
            // Reset the group
            queue->setDefaultQueueGroup(oldgrp);
            queue->setDefaultRenderablePriority(oldPriority);
            // Add 2D elements
            iend = m2DElements.end();
            for (i = m2DElements.begin(); i != iend; ++i)
            {
                (*i)->_update();

                (*i)->_updateRenderQueue(queue);
            }
        }



       
    }
    //---------------------------------------------------------------------
    void Overlay::updateTransform(void) const
    {
        // Ordering:
        //    1. Scale
        //    2. Rotate
        //    3. Translate

        Matrix3 rot3x3, scale3x3;
        rot3x3.FromEulerAnglesXYZ(Radian(0),Radian(0),mRotate);
        scale3x3 = Matrix3::ZERO;
        scale3x3[0][0] = mScaleX;
        scale3x3[1][1] = mScaleY;
        scale3x3[2][2] = 1.0f;

        mTransform = Matrix4::IDENTITY;
        mTransform = rot3x3 * scale3x3;
        mTransform.setTrans(Vector3(mScrollX, mScrollY, 0));

        mTransformOutOfDate = false;
    }
    //---------------------------------------------------------------------
	OverlayElement* Overlay::findElementAt(Real x, Real y)
	{
		OverlayElement* ret = NULL;
		int currZ = -1;
        OverlayContainerList::iterator i, iend;
        iend = m2DElements.end();
        for (i = m2DElements.begin(); i != iend; ++i)
        {
			int z = (*i)->getZOrder();
			if (z > currZ)
			{
				OverlayElement* elementFound = (*i)->findElementAt(x,y);
				if(elementFound)
				{
					currZ = elementFound->getZOrder();
					ret = elementFound;
				}
			}
        }
		return ret;
	}

}

