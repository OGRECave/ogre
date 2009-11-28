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
#include "OgreRenderTarget.h"
#include "OgreStringConverter.h"

#include "OgreViewport.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreRenderTargetListener.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"

namespace Ogre {

    RenderTarget::RenderTarget()
		:mPriority(OGRE_DEFAULT_RT_GROUP),
		mActive(true),
		mAutoUpdate(true),
		mHwGamma(false), 
		mFSAA(0)
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
            fireViewportRemoved(i->second);
            OGRE_DELETE (*i).second;
        }


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

    void RenderTarget::updateImpl(void)
    {
		_beginUpdate();
		_updateAutoUpdatedViewports(true);
		_endUpdate();
    }

	void RenderTarget::_beginUpdate()
	{
		// notify listeners (pre)
        firePreUpdate();

        mStats.triangleCount = 0;
        mStats.batchCount = 0;
	}

	void RenderTarget::_updateAutoUpdatedViewports(bool updateStatistics)
	{
		// Go through viewports in Z-order
        // Tell each to refresh
		ViewportList::iterator it = mViewportList.begin();
        while (it != mViewportList.end())
        {
			Viewport* viewport = (*it).second;
			if(viewport->isAutoUpdated())
			{
				_updateViewport(viewport,updateStatistics);
			}
			++it;
		}
	}

	void RenderTarget::_endUpdate()
	{
		 // notify listeners (post)
        firePostUpdate();

        // Update statistics (always on top)
        updateStats();
	}

	void RenderTarget::_updateViewport(Viewport* viewport, bool updateStatistics)
	{
		assert(viewport->getTarget() == this &&
				"RenderTarget::_updateViewport the requested viewport is "
				"not bound to the rendertarget!");

		fireViewportPreUpdate(viewport);
		viewport->update();
		if(updateStatistics)
		{
			mStats.triangleCount += viewport->_getNumRenderedFaces();
			mStats.batchCount += viewport->_getNumRenderedBatches();
		}
		fireViewportPostUpdate(viewport);
	}

	void RenderTarget::_updateViewport(int zorder, bool updateStatistics)
	{
		ViewportList::iterator it = mViewportList.find(zorder);
        if (it != mViewportList.end())
        {
			_updateViewport((*it).second,updateStatistics);
		}
		else
		{
			OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND,"No viewport with given zorder : "
				+ StringConverter::toString(zorder), "RenderTarget::_updateViewport");
		}
	}

    Viewport* RenderTarget::addViewport(Camera* cam, int ZOrder, float left, float top ,
        float width , float height)
    {
        // Check no existing viewport with this Z-order
        ViewportList::iterator it = mViewportList.find(ZOrder);

        if (it != mViewportList.end())
        {
			StringUtil::StrStreamType str;
			str << "Can't create another viewport for "
				<< mName << " with Z-Order " << ZOrder
				<< " because a viewport exists with this Z-Order already.";
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, str.str(), "RenderTarget::addViewport");
        }
        // Add viewport to list
        // Order based on Z-Order
        Viewport* vp = OGRE_NEW Viewport(cam, this, left, top, width, height, ZOrder);

        mViewportList.insert(ViewportList::value_type(ZOrder, vp));

		fireViewportAdded(vp);

        return vp;
    }
	//-----------------------------------------------------------------------
    void RenderTarget::removeViewport(int ZOrder)
    {
        ViewportList::iterator it = mViewportList.find(ZOrder);

        if (it != mViewportList.end())
        {
			fireViewportRemoved((*it).second);
            OGRE_DELETE (*it).second;
            mViewportList.erase(ZOrder);
        }
    }

    void RenderTarget::removeAllViewports(void)
    {


        for (ViewportList::iterator it = mViewportList.begin(); it != mViewportList.end(); ++it)
        {
            fireViewportRemoved(it->second);
            OGRE_DELETE (*it).second;
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
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Attribute not found.", "RenderTarget::getCustomAttribute");
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
        assert (index < mViewportList.size() && "Index out of bounds");

        ViewportList::iterator i = mViewportList.begin();
        while (index--)
            ++i;
        return i->second;
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
    void RenderTarget::_notifyCameraRemoved(const Camera* cam)
    {
        ViewportList::iterator i, iend;
        iend = mViewportList.end();
        for (i = mViewportList.begin(); i != iend; ++i)
        {
            Viewport* v = i->second;
            if (v->getCamera() == cam)
            {
                // disable camera link
                v->setCamera(0);
            }
        }
    }
    //-----------------------------------------------------------------------
    void RenderTarget::setAutoUpdated(bool autoup)
    {
        mAutoUpdate = autoup;
    }
    //-----------------------------------------------------------------------
    bool RenderTarget::isAutoUpdated(void) const
    {
        return mAutoUpdate;
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
    //-----------------------------------------------------------------------
    void RenderTarget::update(bool swap)
    {
        // call implementation
        updateImpl();


		if (swap)
		{
			// Swap buffers
    	    swapBuffers(Root::getSingleton().getRenderSystem()->getWaitForVerticalBlank());
		}
    }
	

}        
