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
#include "OgreViewport.h"

#include "OgreLogManager.h"
#include "OgreRenderTarget.h"
#include "OgreCamera.h"
#include "OgreMath.h"
#include "OgreRoot.h"
#include "OgreMaterialManager.h"
#include "OgreRenderSystem.h"
#include "OgreRenderWindow.h"

namespace Ogre {
    //---------------------------------------------------------------------
    Viewport::Viewport(Camera* cam, RenderTarget* target, Real left, Real top, Real width, Real height, int ZOrder)
        : mCamera(cam)
        , mTarget(target)
        , mRelLeft(left)
        , mRelTop(top)
        , mRelWidth(width)
        , mRelHeight(height)
        // Actual dimensions will update later
        , mZOrder(ZOrder)
        , mOrientation(OR_PORTRAIT)
        , mBackColour(ColourValue::Black)
        , mClearEveryFrame(true)
		, mClearBuffers(FBT_COLOUR | FBT_DEPTH)
        , mUpdated(false)
        , mShowOverlays(true)
        , mShowSkies(true)
		, mShowShadows(true)
		, mVisibilityMask(0xFFFFFFFF)
		, mRQSequence(0)
		, mMaterialSchemeName(MaterialManager::DEFAULT_SCHEME_NAME)
    {
#if OGRE_COMPILER != OGRE_COMPILER_GCCE
		LogManager::getSingleton().stream(LML_TRIVIAL)
			<< "Creating viewport on target '" << target->getName() << "'"
			<< ", rendering from camera '" << (cam != 0 ? cam->getName() : "NULL") << "'"
			<< ", relative dimensions "	<< std::ios::fixed << std::setprecision(2) 
			<< "L: " << left << " T: " << top << " W: " << width << " H: " << height
			<< " ZOrder: " << ZOrder;
#endif

        // Calculate actual dimensions
        _updateDimensions();

        // notify camera
        if(cam) cam->_notifyViewport(this);
    }
    //---------------------------------------------------------------------
    Viewport::~Viewport()
    {

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

        mActLeft = (int) (mRelLeft * width);
        mActTop = (int) (mRelTop * height);
        mActWidth = (int) (mRelWidth * width);
        mActHeight = (int) (mRelHeight * height);

        // This will check if the cameras getAutoAspectRatio() property is set.
        // If it's true its aspect ratio is fit to the current viewport
        // If it's false the camera remains unchanged.
        // This allows cameras to be used to render to many viewports,
        // which can have their own dimensions and aspect ratios.

        if (mCamera && mCamera->getAutoAspectRatio()) 
        {
            mCamera->setAspectRatio((Real) mActWidth / (Real) mActHeight);
        }

#if OGRE_COMPILER != OGRE_COMPILER_GCCE
		LogManager::getSingleton().stream(LML_TRIVIAL)
			<< "Viewport for camera '" << (mCamera != 0 ? mCamera->getName() : "NULL") << "'"
			<< ", actual dimensions "	<< std::ios::fixed << std::setprecision(2) 
			<< "L: " << mActLeft << " T: " << mActTop << " W: " << mActWidth << " H: " << mActHeight;
#endif

        mUpdated = true;
    }
	//---------------------------------------------------------------------
	int Viewport::getZOrder(void) const
	{
		return mZOrder;
	}
	//---------------------------------------------------------------------
    RenderTarget* Viewport::getTarget(void) const
    {
        return mTarget;
    }
    //---------------------------------------------------------------------
    Camera* Viewport::getCamera(void) const
    {
        return mCamera;
    }
    //---------------------------------------------------------------------
    Real Viewport::getLeft(void) const
    {
        return mRelLeft;
    }
    //---------------------------------------------------------------------
    Real Viewport::getTop(void) const
    {
        return mRelTop;
    }
    //---------------------------------------------------------------------
    Real Viewport::getWidth(void) const
    {
        return mRelWidth;
    }
    //---------------------------------------------------------------------
    Real Viewport::getHeight(void) const
    {
        return mRelHeight;
    }
    //---------------------------------------------------------------------
    int Viewport::getActualLeft(void) const
    {
        return mActLeft;
    }
    //---------------------------------------------------------------------
    int Viewport::getActualTop(void) const
    {
        return mActTop;
    }
    //---------------------------------------------------------------------
    int Viewport::getActualWidth(void) const
    {
        return mActWidth;
    }
    //---------------------------------------------------------------------
    int Viewport::getActualHeight(void) const
    {
        return mActHeight;
    }
    //---------------------------------------------------------------------
    int Viewport::getOrientation(void)
    {
        return mOrientation;
    }
    //---------------------------------------------------------------------
    void Viewport::setOrientation(Orientation orient)
    {
#if OGRE_PLATFORM != OGRE_PLATFORM_IPHONE
        OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
                    "Setting Viewport orientation is only supported on iPhone",
                    __FUNCTION__);
#else
        mOrientation = orient;
        
        // Tell the render window to resize
		RenderWindow* rw = Root::getSingleton().getAutoCreatedWindow();
        rw->changeOrientation(orient);

		// Update the render system config
        RenderSystem* rs = Root::getSingleton().getRenderSystem();
        if(orient == OR_LANDSCAPELEFT)
            rs->setConfigOption("Orientation", "Landscape Left");
        else if(orient == OR_LANDSCAPERIGHT)
            rs->setConfigOption("Orientation", "Landscape Right");
        else if(orient == OR_PORTRAIT)
            rs->setConfigOption("Orientation", "Portrait");
#endif
    }
    //---------------------------------------------------------------------
    void Viewport::setDimensions(Real left, Real top, Real width, Real height)
    {
        mRelLeft = left;
        mRelTop = top;
        mRelWidth = width;
        mRelHeight = height;
        _updateDimensions();
    }
    //---------------------------------------------------------------------
    void Viewport::update(void)
    {
        if (mCamera)
        {
            // Tell Camera to render into me
            mCamera->_renderScene(this, mShowOverlays);
        }
    }
    //---------------------------------------------------------------------
    void Viewport::setBackgroundColour(const ColourValue& colour)
    {
        mBackColour = colour;
    }
    //---------------------------------------------------------------------
    const ColourValue& Viewport::getBackgroundColour(void) const
    {
        return mBackColour;
    }
    //---------------------------------------------------------------------
    void Viewport::setClearEveryFrame(bool clear, unsigned int buffers)
    {
        mClearEveryFrame = clear;
		mClearBuffers = buffers;
    }
    //---------------------------------------------------------------------
    bool Viewport::getClearEveryFrame(void) const
    {
        return mClearEveryFrame;
    }
    //---------------------------------------------------------------------
    unsigned int Viewport::getClearBuffers(void) const
    {
        return mClearBuffers;
    }
    //---------------------------------------------------------------------
	void Viewport::clear(unsigned int buffers, const ColourValue& col,  
						 Real depth, unsigned short stencil)
	{
		RenderSystem* rs = Root::getSingleton().getRenderSystem();
		if (rs)
		{
			Viewport* currentvp = rs->_getViewport();
			rs->_setViewport(this);
			rs->clearFrameBuffer(buffers, col, depth, stencil);
			if (currentvp && currentvp != this)
				rs->_setViewport(currentvp);
		}
	}
    //---------------------------------------------------------------------
    void Viewport::getActualDimensions(int &left, int&top, int &width, int &height) const
    {
        left = mActLeft;
        top = mActTop;
        width = mActWidth;
        height = mActHeight;

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
        mCamera = cam;
		_updateDimensions();
		if(cam) mCamera->_notifyViewport(this);
    }
    //---------------------------------------------------------------------
    void Viewport::setOverlaysEnabled(bool enabled)
    {
        mShowOverlays = enabled;
    }
    //---------------------------------------------------------------------
    bool Viewport::getOverlaysEnabled(void) const
    {
        return mShowOverlays;
    }
    //---------------------------------------------------------------------
    void Viewport::setSkiesEnabled(bool enabled)
    {
        mShowSkies = enabled;
    }
    //---------------------------------------------------------------------
    bool Viewport::getSkiesEnabled(void) const
    {
        return mShowSkies;
    }
    //---------------------------------------------------------------------
    void Viewport::setShadowsEnabled(bool enabled)
    {
        mShowShadows = enabled;
    }
    //---------------------------------------------------------------------
    bool Viewport::getShadowsEnabled(void) const
    {
        return mShowShadows;
    }
	//-----------------------------------------------------------------------
	void Viewport::setRenderQueueInvocationSequenceName(const String& sequenceName)
	{
		mRQSequenceName = sequenceName;
		if (mRQSequenceName.empty())
		{
			mRQSequence = 0;
		}
		else
		{
			mRQSequence =
				Root::getSingleton().getRenderQueueInvocationSequence(mRQSequenceName);
		}
	}
	//-----------------------------------------------------------------------
	const String& Viewport::getRenderQueueInvocationSequenceName(void) const
	{
		return mRQSequenceName;
	}
	//-----------------------------------------------------------------------
	RenderQueueInvocationSequence* Viewport::_getRenderQueueInvocationSequence(void)
	{
		return mRQSequence;
	}
	//-----------------------------------------------------------------------

}
