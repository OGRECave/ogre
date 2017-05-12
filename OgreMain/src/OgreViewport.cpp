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

#include "OgreLogManager.h"
#include "OgreCamera.h"
#include "OgreRoot.h"
#include "OgreMaterialManager.h"
#include "OgreRenderSystem.h"
#include "OgreRenderTarget.h"
#include <iomanip>

namespace Ogre {
    OrientationMode Viewport::mDefaultOrientationMode = OR_DEGREE_0;
    //---------------------------------------------------------------------
    Viewport::Viewport(RenderTarget* target, Real left, Real top, Real width, Real height)
        : mGlobalIndex( -1 )
        , mTarget(target)
        , mRelLeft(left)
        , mRelTop(top)
        , mRelWidth(width)
        , mRelHeight(height)
        , mScissorRelLeft(left)
        , mScissorRelTop(top)
        , mScissorRelWidth(width)
        , mScissorRelHeight(height)
        , mCoversEntireTarget(true)
        , mScissorsMatchViewport(true)
        , mViewportRenderTargetFlags( VP_RTT_COLOUR_WRITE )
        // Actual dimensions will update later
        , mUpdated(false)
        , mShowOverlays(true)
        , mShowSkies(true)
        , mVisibilityMask(0)
        , mMaterialSchemeName(MaterialManager::DEFAULT_SCHEME_NAME)
        , mColourBuffer(CBT_BACK)
    {           
        // Set the default orientation mode
        mOrientationMode = mDefaultOrientationMode;
            
        // Set the default material scheme
        RenderSystem* rs = Root::getSingleton().getRenderSystem();
        mMaterialSchemeName = rs->_getDefaultViewportMaterialScheme();
        
        // Calculate actual dimensions
        _updateDimensions();
    }
    //---------------------------------------------------------------------
    Viewport::~Viewport()
    {
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

        assert( mScissorRelLeft >= mRelLeft     &&
                mScissorRelTop >= mRelTop       &&
                mScissorRelWidth <= mRelWidth   &&
                mScissorRelHeight <= mRelHeight &&
                "Scissor rectangle must be inside Viewport's!" );

        mActLeft = (int) (mRelLeft * width);
        mActTop = (int) (mRelTop * height);
        mActWidth = (int) (mRelWidth * width);
        mActHeight = (int) (mRelHeight * height);

        mScissorActLeft     = (int)( mScissorRelLeft * width );
        mScissorActTop      = (int)( mScissorRelTop * height );
        mScissorActWidth    = (int)( mScissorRelWidth * width );
        mScissorActHeight   = (int)( mScissorRelHeight * height );

        mScissorsMatchViewport =
                mActLeft == mScissorActLeft && mActTop == mScissorActTop &&
                mActWidth == mScissorActWidth && mActHeight == mScissorActHeight;
        mCoversEntireTarget =
                mActLeft == 0 && mActTop == 0 &&
                mActWidth == width && mActHeight == height &&
                mScissorsMatchViewport;

        mUpdated = true;
    }
    //---------------------------------------------------------------------
    RenderTarget* Viewport::getTarget(void) const
    {
        return mTarget;
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
    bool Viewport::coversEntireTarget(void) const
    {
        return mCoversEntireTarget;
    }
    //---------------------------------------------------------------------
    bool Viewport::scissorsMatchViewport(void) const
    {
        return mScissorsMatchViewport;
    }
    //---------------------------------------------------------------------
    void Viewport::setDimensions(Real left, Real top, Real width, Real height, bool overrideScissors)
    {
        mRelLeft = left;
        mRelTop = top;
        mRelWidth = width;
        mRelHeight = height;

        if( overrideScissors )
        {
            mScissorRelLeft     = left;
            mScissorRelTop      = top;
            mScissorRelWidth    = width;
            mScissorRelHeight   = height;
        }

        _updateDimensions();
    }
    //---------------------------------------------------------------------
    void Viewport::setScissors( Real left, Real top, Real width, Real height )
    {
        mScissorRelLeft     = left;
        mScissorRelTop      = top;
        mScissorRelWidth    = width;
        mScissorRelHeight   = height;
        _updateDimensions();
    }
    //---------------------------------------------------------------------
    void Viewport::_updateCullPhase01( Camera* camera, const Camera *lodCamera,
                                       uint8 firstRq, uint8 lastRq )
    {
        // Automatic AR cameras are useful for cameras that draw into multiple viewports
        const Real aspectRatio = (Real) mActWidth / (Real) std::max( 1, mActHeight );
        if( camera->getAutoAspectRatio() && camera->getAspectRatio() != aspectRatio )
        {
            camera->setAspectRatio( aspectRatio );
#if OGRE_NO_VIEWPORT_ORIENTATIONMODE == 0
            camera->setOrientationMode(mOrientationMode);
#endif
        }
        // Tell Camera to render into me
        camera->_notifyViewport(this);

        camera->_cullScenePhase01( lodCamera, this, firstRq, lastRq );
    }
    //---------------------------------------------------------------------
    void Viewport::_updateRenderPhase02( Camera* camera, const Camera *lodCamera,
                                         uint8 firstRq, uint8 lastRq )
    {
        camera->_renderScenePhase02( lodCamera, this, firstRq, lastRq, mShowOverlays );
    }
    //---------------------------------------------------------------------
    void Viewport::setOrientationMode(OrientationMode orientationMode, bool setDefault)
    {
#if OGRE_NO_VIEWPORT_ORIENTATIONMODE != 0
        OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
                    "Setting Viewport orientation mode is not supported",
                    __FUNCTION__);
#endif
        mOrientationMode = orientationMode;

        if (setDefault)
        {
            setDefaultOrientationMode(orientationMode);
        }

    // Update the render system config
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
        RenderSystem* rs = Root::getSingleton().getRenderSystem();
        if(mOrientationMode == OR_LANDSCAPELEFT)
            rs->setConfigOption("Orientation", "Landscape Left");
        else if(mOrientationMode == OR_LANDSCAPERIGHT)
            rs->setConfigOption("Orientation", "Landscape Right");
        else if(mOrientationMode == OR_PORTRAIT)
            rs->setConfigOption("Orientation", "Portrait");
#endif
    }
    //---------------------------------------------------------------------
    OrientationMode Viewport::getOrientationMode() const
    {
#if OGRE_NO_VIEWPORT_ORIENTATIONMODE != 0
        OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
                    "Getting Viewport orientation mode is not supported",
                    __FUNCTION__);
#endif
        return mOrientationMode;
    }
    //---------------------------------------------------------------------
    void Viewport::setDefaultOrientationMode(OrientationMode orientationMode)
    {
#if OGRE_NO_VIEWPORT_ORIENTATIONMODE != 0
        OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
                    "Setting default Viewport orientation mode is not supported",
                    __FUNCTION__);
#endif
        mDefaultOrientationMode = orientationMode;
    }
    //---------------------------------------------------------------------
    OrientationMode Viewport::getDefaultOrientationMode()
    {
#if OGRE_NO_VIEWPORT_ORIENTATIONMODE != 0
        OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
                    "Getting default Viewport orientation mode is not supported",
                    __FUNCTION__);
#endif
        return mDefaultOrientationMode;
    }
    //---------------------------------------------------------------------
    void Viewport::clear(unsigned int buffers, const ColourValue& col,  
                         Real depth, unsigned short stencil)
    {
        RenderSystem* rs = Root::getSingleton().getRenderSystem();
        if (rs)
        {
            Viewport* currentvp = rs->_getViewport();
            if (currentvp == this)
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
    void Viewport::discard( unsigned int buffers )
    {
        RenderSystem* rs = Root::getSingleton().getRenderSystem();
        if (rs)
        {
            Viewport* currentvp = rs->_getViewport();
            if (currentvp == this)
                rs->discardFrameBuffer( buffers );
            else
            {
                rs->_setViewport(this);
                rs->discardFrameBuffer( buffers );
                rs->_setViewport(currentvp);
            }
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
    //-----------------------------------------------------------------------
    void Viewport::_setVisibilityMask( uint32 mask, uint32 lightMask )
    {
        mVisibilityMask = mask;
        mLightVisibilityMask = lightMask;
    }
    //-----------------------------------------------------------------------
    void Viewport::setColourWrite( bool colourWrite )
    {
        if( !colourWrite )
            mViewportRenderTargetFlags &= ~VP_RTT_COLOUR_WRITE;
        else
            mViewportRenderTargetFlags |= VP_RTT_COLOUR_WRITE;
        mUpdated = true;
    }
    //-----------------------------------------------------------------------
    bool Viewport::getColourWrite(void) const
    {
        return (mViewportRenderTargetFlags & VP_RTT_COLOUR_WRITE) != 0;
    }
    //-----------------------------------------------------------------------
    void Viewport::setReadOnly( bool readOnlyDepth, bool readOnlyStencil )
    {
        if( !readOnlyDepth )
            mViewportRenderTargetFlags &= ~VP_RTT_READ_ONLY_DEPTH;
        else
            mViewportRenderTargetFlags |= VP_RTT_READ_ONLY_DEPTH;
        if( !readOnlyStencil )
            mViewportRenderTargetFlags &= ~VP_RTT_READ_ONLY_STENCIL;
        else
            mViewportRenderTargetFlags |= VP_RTT_READ_ONLY_STENCIL;
        mUpdated = true;
    }
    //-----------------------------------------------------------------------
    bool Viewport::getReadOnlyDepth(void) const
    {
        return (mViewportRenderTargetFlags & VP_RTT_READ_ONLY_DEPTH) != 0;
    }
    //-----------------------------------------------------------------------
    bool Viewport::getReadOnlStencil(void) const
    {
        return (mViewportRenderTargetFlags & VP_RTT_READ_ONLY_STENCIL) != 0;
    }
    //-----------------------------------------------------------------------
    void Viewport::pointOrientedToScreen(const Vector2 &v, int orientationMode, Vector2 &outv)
    {
        pointOrientedToScreen(v.x, v.y, orientationMode, outv.x, outv.y);
    }
    //-----------------------------------------------------------------------
    void Viewport::pointOrientedToScreen(Real orientedX, Real orientedY, int orientationMode,
                                         Real &screenX, Real &screenY)
    {
        Real orX = orientedX;
        Real orY = orientedY;
        switch (orientationMode)
        {
        case 1:
            screenX = orY;
            screenY = Real(1.0) - orX;
            break;
        case 2:
            screenX = Real(1.0) - orX;
            screenY = Real(1.0) - orY;
            break;
        case 3:
            screenX = Real(1.0) - orY;
            screenY = orX;
            break;
        default:
            screenX = orX;
            screenY = orY;
            break;
        }
    }
    //-----------------------------------------------------------------------
    void Viewport::setDrawBuffer(ColourBufferType colourBuffer) 
    {
        mColourBuffer = colourBuffer;
    }
    //-----------------------------------------------------------------------
    ColourBufferType Viewport::getDrawBuffer() const
    {
        return mColourBuffer;
    }
    //-----------------------------------------------------------------------
}
