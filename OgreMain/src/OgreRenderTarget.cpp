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
#include "OgreRenderTarget.h"

#include "OgreViewport.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreRenderTargetListener.h"
#include "OgrePixelBox.h"
#include "OgreRoot.h"
#include "OgreDepthBuffer.h"
#include "OgreProfiler.h"
#include "OgreTimer.h"
#include "OgreCamera.h"
#include <iomanip>

namespace Ogre {

    RenderTarget::RenderTarget()
        : mPriority(OGRE_DEFAULT_RT_GROUP)
        , mFormat( PF_UNKNOWN )
        , mDepthBufferPoolId( DepthBuffer::POOL_DEFAULT )
        , mPreferDepthTexture( false )
        , mDesiredDepthBufferFormat( DepthBuffer::DefaultDepthBufferFormat )
        , mDepthBuffer(0)
        , mActive(true)
        , mHwGamma(false)
        , mFSAA(0)
        , mFsaaResolveDirty(false)
        , mMipmapsDirty(false)
#if OGRE_NO_QUAD_BUFFER_STEREO == 0
		, mStereoEnabled(true)
#else
		, mStereoEnabled(false)
#endif
    {
        resetStatistics();
    }

    RenderTarget::~RenderTarget()
    {
        // Delete viewports
        for (ViewportList::iterator i = mViewportList.begin();
            i != mViewportList.end(); ++i)
        {
            fireViewportRemoved( *i );
            OGRE_DELETE *i;
        }

        //DepthBuffer keeps track of us, avoid a dangling pointer
        detachDepthBuffer();

        // Write closing message
        LogManager::getSingleton().stream(LML_TRIVIAL) << "Render Target '" << mName << "' ";
    }

    const String& RenderTarget::getName(void) const
    {
        return mName;
    }


    void RenderTarget::getMetrics(unsigned int& width, unsigned int& height, unsigned int& colourDepth)
    {
        width = mWidth;
        height = mHeight;
        colourDepth = PixelUtil::getNumElemBits( mFormat );
    }

    unsigned int RenderTarget::getWidth(void) const
    {
        return mWidth;
    }
    unsigned int RenderTarget::getHeight(void) const
    {
        return mHeight;
    }
    PixelFormat RenderTarget::getFormat(void) const
    {
        return mFormat;
    }
    //-----------------------------------------------------------------------
    void RenderTarget::getFormatsForPso( PixelFormat outFormats[OGRE_MAX_MULTIPLE_RENDER_TARGETS],
                                         bool outHwGamma[OGRE_MAX_MULTIPLE_RENDER_TARGETS] ) const
    {
        outFormats[0] = mFormat;
        outHwGamma[0] = mHwGamma;
        for( size_t i=1; i<OGRE_MAX_MULTIPLE_RENDER_TARGETS; ++i )
        {
            outFormats[i] = PF_NULL;
            outHwGamma[i] = false;
        }
    }
    //-----------------------------------------------------------------------
    void RenderTarget::setDepthBufferPool( uint16 poolId )
    {
        if( mDepthBufferPoolId != poolId )
        {
            mDepthBufferPoolId = poolId;
            detachDepthBuffer();
        }
    }
    //-----------------------------------------------------------------------
    uint16 RenderTarget::getDepthBufferPool() const
    {
        return mDepthBufferPoolId;
    }
    //-----------------------------------------------------------------------
    void RenderTarget::setPreferDepthTexture( bool preferDepthTexture )
    {
        if( mPreferDepthTexture != preferDepthTexture )
        {
            mPreferDepthTexture = preferDepthTexture;
            detachDepthBuffer();
        }
    }
    //-----------------------------------------------------------------------
    bool RenderTarget::prefersDepthTexture() const
    {
        return mPreferDepthTexture;
    }
    //-----------------------------------------------------------------------
    void RenderTarget::setDesiredDepthBufferFormat( PixelFormat desiredDepthBufferFormat )
    {
        assert( desiredDepthBufferFormat == PF_D24_UNORM_S8_UINT ||
                desiredDepthBufferFormat == PF_D24_UNORM_X8 ||
                desiredDepthBufferFormat == PF_D16_UNORM ||
                desiredDepthBufferFormat == PF_D32_FLOAT ||
                desiredDepthBufferFormat == PF_D32_FLOAT_X24_S8_UINT ||
                desiredDepthBufferFormat == PF_D32_FLOAT_X24_X8 );

        if( mDesiredDepthBufferFormat != desiredDepthBufferFormat )
        {
            mDesiredDepthBufferFormat = desiredDepthBufferFormat;
            detachDepthBuffer();
        }
    }
    //-----------------------------------------------------------------------
    PixelFormat RenderTarget::getDesiredDepthBufferFormat() const
    {
        return mDesiredDepthBufferFormat;
    }
    //-----------------------------------------------------------------------
    DepthBuffer* RenderTarget::getDepthBuffer() const
    {
        return mDepthBuffer;
    }
    //-----------------------------------------------------------------------
    bool RenderTarget::attachDepthBuffer( DepthBuffer *depthBuffer, bool exactFormatMatch )
    {
        bool retVal = false;

        if( (retVal = depthBuffer->isCompatible( this, exactFormatMatch )) )
        {
            detachDepthBuffer();
            mDepthBuffer = depthBuffer;
            mDepthBuffer->_notifyRenderTargetAttached( this );
        }

        return retVal;
    }
    //-----------------------------------------------------------------------
    void RenderTarget::detachDepthBuffer()
    {
        if( mDepthBuffer )
        {
            mDepthBuffer->_notifyRenderTargetDetached( this );
            mDepthBuffer = 0;
        }
    }
    //-----------------------------------------------------------------------
    void RenderTarget::_detachDepthBuffer()
    {
        mDepthBuffer = 0;
    }

    void RenderTarget::_beginUpdate()
    {
        // notify listeners (pre)
        firePreUpdate();

        mStats.triangleCount = 0;
        mStats.batchCount = 0;

        OgreProfileBeginGPUEvent("RenderTarget: " + getName());
    }

    void RenderTarget::_endUpdate()
    {
         // notify listeners (post)
        firePostUpdate();

        OgreProfileEndGPUEvent("RenderTarget: " + getName());
    }

    void RenderTarget::_updateViewportCullPhase01( Viewport* viewport, Camera *camera,
                                                   const Camera *lodCamera, uint8 firstRq, uint8 lastRq )
    {
        assert( viewport->getTarget() == this &&
                "RenderTarget::_updateViewportCullPhase the requested viewport is "
                "not bound to the rendertarget!" );

        fireViewportPreUpdate(viewport);
        viewport->_updateCullPhase01( camera, lodCamera, firstRq, lastRq );
    }
    //-----------------------------------------------------------------------
    void RenderTarget::_updateViewportRenderPhase02( Viewport* viewport, Camera *camera,
                                                     const Camera *lodCamera, uint8 firstRq,
                                                     uint8 lastRq, bool updateStatistics )
    {
        assert( viewport->getTarget() == this &&
                "RenderTarget::_updateViewport the requested viewport is "
                "not bound to the rendertarget!" );

        viewport->_updateRenderPhase02( camera, lodCamera, firstRq, lastRq );
        if(updateStatistics)
        {
            mStats.triangleCount += camera->_getNumRenderedFaces();
            mStats.batchCount += camera->_getNumRenderedBatches();
        }
        fireViewportPostUpdate(viewport);
    }
    //-----------------------------------------------------------------------
    Viewport* RenderTarget::addViewport( float left, float top, float width, float height )
    {       
        // Add viewport to list
        Viewport* vp = OGRE_NEW Viewport( this, left, top, width, height );
        mViewportList.push_back( vp );

        vp->mGlobalIndex = mViewportList.size() - 1;

        fireViewportAdded(vp);

        return vp;
    }
    //-----------------------------------------------------------------------
    void RenderTarget::removeViewport( Viewport *vp )
    {
        if( vp->mGlobalIndex >= mViewportList.size() ||
            vp != *(mViewportList.begin() + vp->mGlobalIndex) )
        {
            OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, "Viewport had it's mGlobalIndex out of "
                "date!!! (or Viewport wasn't created by this RenderTarget",
                "RenderTarget::removeViewport" );
        }

        ViewportList::iterator itor = mViewportList.begin() + vp->mGlobalIndex;

        fireViewportRemoved( vp );
        OGRE_DELETE vp;

        itor = efficientVectorRemove( mViewportList, itor );

        //The Viewport that was at the end got swapped and has now a different index
        if( itor != mViewportList.end() )
            (*itor)->mGlobalIndex = itor - mViewportList.begin();
    }

    void RenderTarget::removeAllViewports(void)
    {
        for (ViewportList::iterator it = mViewportList.begin(); it != mViewportList.end(); ++it)
        {
            fireViewportRemoved( *it );
            OGRE_DELETE *it;
        }

        mViewportList.clear();
    }

    const RenderTarget::FrameStats& RenderTarget::getStatistics(void) const
    {
        return mStats;
    }

    size_t RenderTarget::getTriangleCount(void) const
    {
        return mStats.triangleCount;
    }

    size_t RenderTarget::getBatchCount(void) const
    {
        return mStats.batchCount;
    }

    void RenderTarget::resetStatistics(void)
    {
        mStats.triangleCount = 0;
        mStats.batchCount = 0;
    }

    void RenderTarget::getCustomAttribute(const String& name, void* pData)
    {
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Attribute not found. " + name, " RenderTarget::getCustomAttribute");
    }
    //-----------------------------------------------------------------------
    void RenderTarget::addListener(RenderTargetListener* listener)
    {
        mListeners.push_back(listener);
    }
    //-----------------------------------------------------------------------
    void RenderTarget::removeListener(RenderTargetListener* listener)
    {
        RenderTargetListenerList::iterator i;
        for (i = mListeners.begin(); i != mListeners.end(); ++i)
        {
            if (*i == listener)
            {
                mListeners.erase(i);
                break;
            }
        }

    }
    //-----------------------------------------------------------------------
    void RenderTarget::removeAllListeners(void)
    {
        mListeners.clear();
    }
    //-----------------------------------------------------------------------
    void RenderTarget::firePreUpdate(void)
    {
        RenderTargetEvent evt;
        evt.source = this;

        RenderTargetListenerList::iterator i, iend;
        i = mListeners.begin();
        iend = mListeners.end();
        for(; i != iend; ++i)
        {
            (*i)->preRenderTargetUpdate(evt);
        }


    }
    //-----------------------------------------------------------------------
    void RenderTarget::firePostUpdate(void)
    {
        RenderTargetEvent evt;
        evt.source = this;

        RenderTargetListenerList::iterator i, iend;
        i = mListeners.begin();
        iend = mListeners.end();
        for(; i != iend; ++i)
        {
            (*i)->postRenderTargetUpdate(evt);
        }
    }
    //-----------------------------------------------------------------------
    unsigned short RenderTarget::getNumViewports(void) const
    {
        return (unsigned short)mViewportList.size();

    }
    //-----------------------------------------------------------------------
    Viewport* RenderTarget::getViewport(unsigned short index)
    {
        return mViewportList[index];
    }
    //-----------------------------------------------------------------------
    bool RenderTarget::isActive() const
    {
        return mActive;
    }
    //-----------------------------------------------------------------------
    void RenderTarget::setActive( bool state )
    {
        mActive = state;
    }
    //-----------------------------------------------------------------------
    void RenderTarget::fireViewportPreUpdate(Viewport* vp)
    {
        RenderTargetViewportEvent evt;
        evt.source = vp;

        RenderTargetListenerList::iterator i, iend;
        i = mListeners.begin();
        iend = mListeners.end();
        for(; i != iend; ++i)
        {
            (*i)->preViewportUpdate(evt);
        }
    }
    //-----------------------------------------------------------------------
    void RenderTarget::fireViewportPostUpdate(Viewport* vp)
    {
        RenderTargetViewportEvent evt;
        evt.source = vp;

        RenderTargetListenerList::iterator i, iend;
        i = mListeners.begin();
        iend = mListeners.end();
        for(; i != iend; ++i)
        {
            (*i)->postViewportUpdate(evt);
        }
    }
    //-----------------------------------------------------------------------
    void RenderTarget::fireViewportAdded(Viewport* vp)
    {
        RenderTargetViewportEvent evt;
        evt.source = vp;

        RenderTargetListenerList::iterator i, iend;
        i = mListeners.begin();
        iend = mListeners.end();
        for(; i != iend; ++i)
        {
            (*i)->viewportAdded(evt);
        }
    }
    //-----------------------------------------------------------------------
    void RenderTarget::fireViewportRemoved(Viewport* vp)
    {
        RenderTargetViewportEvent evt;
        evt.source = vp;

        // Make a temp copy of the listeners
        // some will want to remove themselves as listeners when they get this
        RenderTargetListenerList tempList = mListeners;

        RenderTargetListenerList::iterator i, iend;
        i = tempList.begin();
        iend = tempList.end();
        for(; i != iend; ++i)
        {
            (*i)->viewportRemoved(evt);
        }
    }
    //-----------------------------------------------------------------------
    String RenderTarget::writeContentsToTimestampedFile( const String& filenamePrefix, const String& filenameSuffix,
                                                         PixelFormat format )
    {
        struct tm *pTime;
        time_t ctTime; time(&ctTime);
        pTime = localtime( &ctTime );
        Ogre::StringStream oss;
        oss << std::setw(2) << std::setfill('0') << (pTime->tm_mon + 1)
            << std::setw(2) << std::setfill('0') << pTime->tm_mday
            << std::setw(2) << std::setfill('0') << (pTime->tm_year + 1900)
            << "_" << std::setw(2) << std::setfill('0') << pTime->tm_hour
            << std::setw(2) << std::setfill('0') << pTime->tm_min
            << std::setw(2) << std::setfill('0') << pTime->tm_sec
            << std::setw(3) << std::setfill('0') <<
                        (Root::getSingleton().getTimer()->getMilliseconds() % 1000);
        String filename = filenamePrefix + oss.str() + filenameSuffix;
        writeContentsToFile( filename, format );
        return filename;

    }
    //-----------------------------------------------------------------------
    void RenderTarget::writeContentsToFile( const String& filename, PixelFormat format )
    {
        PixelFormat pf = format == PF_UNKNOWN ? suggestPixelFormat() : format;

        uchar *data = OGRE_ALLOC_T(uchar, mWidth * mHeight * PixelUtil::getNumElemBytes(pf), MEMCATEGORY_RENDERSYS);
        PixelBox pb(mWidth, mHeight, 1, pf, data);

        copyContentsToMemory(pb);

        Image().loadDynamicImage(data, mWidth, mHeight, 1, pf, false, 1, 0).save(filename);

        OGRE_FREE(data, MEMCATEGORY_RENDERSYS);
    }
    //-----------------------------------------------------------------------
    bool RenderTarget::isPrimary(void) const
    {
        // RenderWindow will override and return true for the primary window
        return false;
    }  
	//-----------------------------------------------------------------------
    bool RenderTarget::isStereoEnabled(void) const
    {
        return mStereoEnabled;
    }
    //-----------------------------------------------------------------------
    RenderTarget::Impl *RenderTarget::_getImpl()
    {
        return 0;
    }
}
