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
#include "OgreRenderTarget.h"
#include "OgreStringConverter.h"

#include "OgreViewport.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreRenderTargetListener.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreDepthBuffer.h"
#include "OgreProfiler.h"

namespace Ogre {

    RenderTarget::RenderTarget()
		: mPriority(OGRE_DEFAULT_RT_GROUP)
		, mDepthBufferPoolId(DepthBuffer::POOL_DEFAULT)
		, mDepthBuffer(0)
		, mActive(true)
		, mHwGamma(false)
		, mFSAA(0)
    {
        mTimer = Root::getSingleton().getTimer();
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
		LogManager::getSingleton().stream(LML_TRIVIAL)
			<< "Render Target '" << mName << "' "
			<< "Average FPS: " << mStats.avgFPS << " "
			<< "Best FPS: " << mStats.bestFPS << " "
			<< "Worst FPS: " << mStats.worstFPS; 

    }

    const String& RenderTarget::getName(void) const
    {
        return mName;
    }


    void RenderTarget::getMetrics(unsigned int& width, unsigned int& height, unsigned int& colourDepth)
    {
        width = mWidth;
        height = mHeight;
        colourDepth = mColourDepth;
    }

    unsigned int RenderTarget::getWidth(void) const
    {
        return mWidth;
    }
    unsigned int RenderTarget::getHeight(void) const
    {
        return mHeight;
    }
    unsigned int RenderTarget::getColourDepth(void) const
    {
        return mColourDepth;
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
	DepthBuffer* RenderTarget::getDepthBuffer() const
	{
		return mDepthBuffer;
	}
	//-----------------------------------------------------------------------
	bool RenderTarget::attachDepthBuffer( DepthBuffer *depthBuffer )
	{
		bool retVal = false;

		if( (retVal = depthBuffer->isCompatible( this )) )
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

        // Update statistics (always on top)
        updateStats();
	}

	void RenderTarget::_updateViewportCullPhase01( Viewport* viewport, Camera *camera,
													uint8 firstRq, uint8 lastRq )
	{
		assert( viewport->getTarget() == this &&
				"RenderTarget::_updateViewportCullPhase the requested viewport is "
				"not bound to the rendertarget!" );

		fireViewportPreUpdate(viewport);
		viewport->_updateCullPhase01( camera, firstRq, lastRq );
	}
	//-----------------------------------------------------------------------
	void RenderTarget::_updateViewportRenderPhase02( Viewport* viewport, Camera *camera, uint8 firstRq,
													 uint8 lastRq, bool updateStatistics )
	{
		assert( viewport->getTarget() == this &&
				"RenderTarget::_updateViewport the requested viewport is "
				"not bound to the rendertarget!" );

		viewport->_updateRenderPhase02( camera, firstRq, lastRq );
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

    void RenderTarget::getStatistics(float& lastFPS, float& avgFPS,
        float& bestFPS, float& worstFPS) const
    {

        // Note - the will have been updated by the last render
        lastFPS = mStats.lastFPS;
        avgFPS = mStats.avgFPS;
        bestFPS = mStats.bestFPS;
        worstFPS = mStats.worstFPS;


    }

    const RenderTarget::FrameStats& RenderTarget::getStatistics(void) const
    {
        return mStats;
    }

    float RenderTarget::getLastFPS() const
    {
        return mStats.lastFPS;
    }
    float RenderTarget::getAverageFPS() const
    {
        return mStats.avgFPS;
    }
    float RenderTarget::getBestFPS() const
    {
        return mStats.bestFPS;
    }
    float RenderTarget::getWorstFPS() const
    {
        return mStats.worstFPS;
    }

    size_t RenderTarget::getTriangleCount(void) const
    {
        return mStats.triangleCount;
    }

    size_t RenderTarget::getBatchCount(void) const
    {
        return mStats.batchCount;
    }

    float RenderTarget::getBestFrameTime() const
    {
        return (float)mStats.bestFrameTime;
    }

    float RenderTarget::getWorstFrameTime() const
    {
        return (float)mStats.worstFrameTime;
    }

    void RenderTarget::resetStatistics(void)
    {
        mStats.avgFPS = 0.0;
        mStats.bestFPS = 0.0;
        mStats.lastFPS = 0.0;
        mStats.worstFPS = 999.0;
        mStats.triangleCount = 0;
        mStats.batchCount = 0;
        mStats.bestFrameTime = 999999;
        mStats.worstFrameTime = 0;

        mLastTime = mTimer->getMilliseconds();
        mLastSecond = mLastTime;
        mFrameCount = 0;
    }

    void RenderTarget::updateStats(void)
    {
        ++mFrameCount;
        unsigned long thisTime = mTimer->getMilliseconds();

        // check frame time
        unsigned long frameTime = thisTime - mLastTime ;
        mLastTime = thisTime ;

        mStats.bestFrameTime = std::min(mStats.bestFrameTime, frameTime);
        mStats.worstFrameTime = std::max(mStats.worstFrameTime, frameTime);

        // check if new second (update only once per second)
        if (thisTime - mLastSecond > 1000) 
        { 
            // new second - not 100% precise
            mStats.lastFPS = (float)mFrameCount / (float)(thisTime - mLastSecond) * 1000.0f;

            if (mStats.avgFPS == 0)
                mStats.avgFPS = mStats.lastFPS;
            else
                mStats.avgFPS = (mStats.avgFPS + mStats.lastFPS) / 2; // not strictly correct, but good enough

            mStats.bestFPS = std::max(mStats.bestFPS, mStats.lastFPS);
            mStats.worstFPS = std::min(mStats.worstFPS, mStats.lastFPS);

            mLastSecond = thisTime ;
            mFrameCount  = 0;

        }

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
    String RenderTarget::writeContentsToTimestampedFile(const String& filenamePrefix, const String& filenameSuffix)
    {
        struct tm *pTime;
        time_t ctTime; time(&ctTime);
        pTime = localtime( &ctTime );
        Ogre::StringStream oss;
        oss	<< std::setw(2) << std::setfill('0') << (pTime->tm_mon + 1)
            << std::setw(2) << std::setfill('0') << pTime->tm_mday
            << std::setw(2) << std::setfill('0') << (pTime->tm_year + 1900)
            << "_" << std::setw(2) << std::setfill('0') << pTime->tm_hour
            << std::setw(2) << std::setfill('0') << pTime->tm_min
            << std::setw(2) << std::setfill('0') << pTime->tm_sec
            << std::setw(3) << std::setfill('0') << (mTimer->getMilliseconds() % 1000);
        String filename = filenamePrefix + oss.str() + filenameSuffix;
        writeContentsToFile(filename);
        return filename;

    }
	//-----------------------------------------------------------------------
	void RenderTarget::writeContentsToFile(const String& filename)
	{
		PixelFormat pf = suggestPixelFormat();

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
    RenderTarget::Impl *RenderTarget::_getImpl()
    {
        return 0;
    }
}
