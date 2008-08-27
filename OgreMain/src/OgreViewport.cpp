/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
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

		LogManager::getSingleton().stream(LML_TRIVIAL)
			<< "Creating viewport on target '" << target->getName() << "'"
			<< ", rendering from camera '" << (cam != 0 ? cam->getName() : "NULL") << "'"
			<< ", relative dimensions "	<< std::ios::fixed << std::setprecision(2) 
			<< "L: " << left << " T: " << top << " W: " << width << " H: " << height
			<< " ZOrder: " << ZOrder;

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

        // This will check if  the cameras getAutoAspectRation() property is set.
        // If it's true its aspect ratio is fit to the current viewport
        // If it's false the camera remains unchanged.
        // This allows cameras to be used to render to many viewports,
        // which can have their own dimensions and aspect ratios.

        if (mCamera && mCamera->getAutoAspectRatio()) 
        {
            mCamera->setAspectRatio((Real) mActWidth / (Real) mActHeight);
        }

		LogManager::getSingleton().stream(LML_TRIVIAL)
			<< "Viewport for camera '" << (mCamera != 0 ? mCamera->getName() : "NULL") << "'"
			<< ", actual dimensions "	<< std::ios::fixed << std::setprecision(2) 
			<< "L: " << mActLeft << " T: " << mActTop << " W: " << mActWidth << " H: " << mActHeight;


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
