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
#include "OgreViewport.h"
#include "OgreRenderTarget.h"

namespace Ogre {
    //---------------------------------------------------------------------
    Viewport::Viewport(Camera* cam, RenderTarget* target, float left, float top, float width, float height, int ZOrder)
        : mCamera(cam)
        , mTarget(target)
        , mRelRect(left, top, left + width, top + height)
        // Actual dimensions will update later
        , mZOrder(ZOrder)
        , mBackColour(ColourValue::Black)
        , mDepthClearValue(1)
        , mClearEveryFrame(true)
        , mClearBuffers(FBT_COLOUR | FBT_DEPTH)
        , mUpdated(false)
        , mShowOverlays(true)
        , mShowSkies(true)
        , mShowShadows(true)
        , mVisibilityMask(0xFFFFFFFF)
        , mMaterialSchemeName(MaterialManager::DEFAULT_SCHEME_NAME)
        , mIsAutoUpdated(true)
		, mColourBuffer(CBT_BACK)
    {           
#if OGRE_PLATFORM != OGRE_PLATFORM_ANDROID
        LogManager::getSingleton().stream(LML_TRIVIAL)
            << "Creating viewport on target '" << target->getName() << "'"
            << ", rendering from camera '" << (cam != 0 ? cam->getName() : "NULL") << "'"
            << ", relative dimensions " << mRelRect
            << " Z-order: " << ZOrder;
#endif
            
        // Set the default material scheme
        RenderSystem* rs = Root::getSingleton().getRenderSystem();
        mMaterialSchemeName = rs->_getDefaultViewportMaterialScheme();
        
        // Calculate actual dimensions
        _updateDimensions();

        // notify camera
        if(cam) cam->_notifyViewport(this);
    }
    //---------------------------------------------------------------------
    Viewport::~Viewport()
    {
        ListenerList listenersCopy;
        std::swap(mListeners, listenersCopy);
        for (auto *l : listenersCopy)
        {
            l->viewportDestroyed(this);
        }

        RenderSystem* rs = Root::getSingleton().getRenderSystem();
        if ((rs) && (rs->_getViewport() == this))
        {
            rs->_setViewport(NULL);
        }
    }
    //---------------------------------------------------------------------
    bool Viewport::_isUpdated(void) const
    {
        return mUpdated;
    }
    //---------------------------------------------------------------------
    void Viewport::_clearUpdatedFlag(void)
    {
        mUpdated = false;
    }
    //---------------------------------------------------------------------
    void Viewport::_updateDimensions(void)
    {
        Real height = (Real) mTarget->getHeight();
        Real width = (Real) mTarget->getWidth();

        mActRect = Rect(mRelRect.left * width, mRelRect.top * height, mRelRect.right * width, mRelRect.bottom * height);

        // This will check if the cameras getAutoAspectRatio() property is set.
        // If it's true its aspect ratio is fit to the current viewport
        // If it's false the camera remains unchanged.
        // This allows cameras to be used to render to many viewports,
        // which can have their own dimensions and aspect ratios.

        if (mCamera) 
        {
            if (mCamera->getAutoAspectRatio())
                mCamera->setAspectRatio((float)mActRect.width() / (float)mActRect.height());
        }

        LogManager::getSingleton().stream(LML_TRIVIAL)
            << "Viewport for camera '" << (mCamera ? mCamera->getName() : "NULL") << "'"
            << ", actual dimensions " << mActRect;

        mUpdated = true;

        for (auto *l : mListeners)
        {
            l->viewportDimensionsChanged(this);
        }
    }
    //---------------------------------------------------------------------
    void Viewport::setDimensions(float left, float top, float width, float height)
    {
        mRelRect = FloatRect(left, top, left + width, top + height);
        _updateDimensions();
    }
    //---------------------------------------------------------------------
    void Viewport::update(void)
    {
        if (mCamera)
        {
            if (mCamera->getViewport() != this)
                mCamera->_notifyViewport(this);

            // Tell Camera to render into me
            mCamera->_renderScene(this);
        }
    }
    //---------------------------------------------------------------------
    void Viewport::setClearEveryFrame(bool inClear, unsigned int inBuffers)
    {
        mClearEveryFrame = inClear;
        mClearBuffers = inClear ? inBuffers : 0;
    }
    //---------------------------------------------------------------------
    void Viewport::clear(uint32 buffers, const ColourValue& col, float depth, uint16 stencil)
    {
        RenderSystem* rs = Root::getSingleton().getRenderSystem();
        if (rs)
        {
            Viewport* currentvp = rs->_getViewport();
            if (currentvp && currentvp == this)
                rs->clearFrameBuffer(buffers, col, depth, stencil);
            else
            {
                rs->_setViewport(this);
                rs->clearFrameBuffer(buffers, col, depth, stencil);
                rs->_setViewport(currentvp);
            }
        }
    }
    //---------------------------------------------------------------------
    void Viewport::getActualDimensions(int &left, int&top, int &width, int &height) const
    {
        left = mActRect.left;
        top = mActRect.top;
        width = mActRect.width();
        height = mActRect.height();
    }
    //---------------------------------------------------------------------
    unsigned int Viewport::_getNumRenderedFaces(void) const
    {
        return mCamera ? mCamera->_getNumRenderedFaces() : 0;
    }
    //---------------------------------------------------------------------
    unsigned int Viewport::_getNumRenderedBatches(void) const
    {
        return mCamera ? mCamera->_getNumRenderedBatches() : 0;
    }
    //---------------------------------------------------------------------
    void Viewport::setCamera(Camera* cam)
    {
        if (cam != NULL && mCamera != NULL && mCamera->getViewport() == this)
        {
                mCamera->_notifyViewport(NULL);
        }

        mCamera = cam;
        if (cam)
        {
            // update aspect ratio of new camera if needed.
            if (cam->getAutoAspectRatio())
            {
                cam->setAspectRatio((float)mActRect.width() / (float)mActRect.height());
            }
            cam->_notifyViewport(this);
        }

        for (auto *l : mListeners)
        {
            l->viewportCameraChanged(this);
        }
    }
    //-----------------------------------------------------------------------
    void Viewport::addListener(Listener* l)
    {
        if (std::find(mListeners.begin(), mListeners.end(), l) == mListeners.end())
            mListeners.push_back(l);
    }
    //-----------------------------------------------------------------------
    void Viewport::removeListener(Listener* l)
    {
        ListenerList::iterator i = std::find(mListeners.begin(), mListeners.end(), l);
        if (i != mListeners.end())
            mListeners.erase(i);
    }
}
