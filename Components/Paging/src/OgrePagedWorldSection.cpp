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
#include "OgrePagedWorldSection.h"
#include "OgrePageStrategy.h"
#include "OgreStreamSerialiser.h"
#include "OgrePagedWorld.h"
#include "OgrePageManager.h"
#include "OgrePage.h"
#include "OgreLogManager.h"
#include "OgreRoot.h"

namespace Ogre
{
    //---------------------------------------------------------------------
    const uint32 PagedWorldSection::CHUNK_ID = StreamSerialiser::makeIdentifier("PWSC");
    const uint16 PagedWorldSection::CHUNK_VERSION = 1;
    //---------------------------------------------------------------------
    PagedWorldSection::PagedWorldSection(const String& name, PagedWorld* parent, SceneManager* sm)
        : mName(name), mParent(parent), mStrategy(0), mStrategyData(0), mPageProvider(0), mSceneMgr(sm)
    {
    }
    //---------------------------------------------------------------------
    PagedWorldSection::~PagedWorldSection()
    {
        if (mStrategy)
        {
            mStrategy->destroyData(mStrategyData);
            mStrategyData = 0;
        }

        removeAllPages();
    }
    //---------------------------------------------------------------------
    PageManager* PagedWorldSection::getManager() const
    {
        return mParent->getManager();
    }
    //---------------------------------------------------------------------
    void PagedWorldSection::setBoundingBox(const AxisAlignedBox& box)
    {
        mAABB = box;
    }
    //---------------------------------------------------------------------
    const AxisAlignedBox& PagedWorldSection::getBoundingBox() const
    {
        return mAABB;
    }
    //---------------------------------------------------------------------
    void PagedWorldSection::setStrategy(PageStrategy* strat)
    {
        if (strat != mStrategy)
        {
            if (mStrategy)
            {
                mStrategy->destroyData(mStrategyData);
                mStrategy = 0;
                mStrategyData = 0;
            }

            mStrategy = strat;
            if (mStrategy)
                mStrategyData = mStrategy->createData();

            removeAllPages();
        }

    }
    //---------------------------------------------------------------------
    void PagedWorldSection::setStrategy(const String& stratName)
    {
        setStrategy(getManager()->getStrategy(stratName));
    }
    //---------------------------------------------------------------------
    void PagedWorldSection::setSceneManager(SceneManager* sm)
    {
        if (sm != mSceneMgr)
        {
            mSceneMgr = sm;
            removeAllPages();
        }
        
        
    }
    //---------------------------------------------------------------------
    void PagedWorldSection::setSceneManager(const String& smName)
    {
        setSceneManager(Root::getSingleton().getSceneManager(smName));
    }
    //---------------------------------------------------------------------
    bool PagedWorldSection::load(StreamSerialiser& ser)
    {
        if (!ser.readChunkBegin(CHUNK_ID, CHUNK_VERSION, "PagedWorldSection"))
            return false;

        // Name
        ser.read(&mName);
        // AABB
        ser.read(&mAABB);
        // SceneManager type
        String smType, smInstanceName;
        SceneManager* sm = 0;
        ser.read(&smType);
        ser.read(&smInstanceName);
        Root& root = Root::getSingleton();
        if (root.hasSceneManager(smInstanceName))
            sm = root.getSceneManager(smInstanceName);
        else
            sm = root.createSceneManager(smType, smInstanceName);
        setSceneManager(sm);
        // Page Strategy Name
        String stratname;
        ser.read(&stratname);
        setStrategy(stratname);
        // Page Strategy Data
        bool strategyDataOk = mStrategyData->load(ser);
        if (!strategyDataOk)
            LogManager::getSingleton().stream(LML_CRITICAL) << "Error: PageStrategyData for section '"
            << mName << "' was not loaded correctly, check file contents";

        /// Load any data specific to a subtype of this class
        loadSubtypeData(ser);

        ser.readChunkEnd(CHUNK_ID);

        return true;

    }
    //---------------------------------------------------------------------
    void PagedWorldSection::save(StreamSerialiser& ser)
    {
        ser.writeChunkBegin(CHUNK_ID, CHUNK_VERSION);

        // Name
        ser.write(&mName);
        // AABB
        ser.write(&mAABB);
        // SceneManager type & name
        ser.write(&mSceneMgr->getTypeName());
        ser.write(&mSceneMgr->getName());
        // Page Strategy Name
        ser.write(&mStrategy->getName());
        // Page Strategy Data
        mStrategyData->save(ser);

        /// Save any data specific to a subtype of this class
        saveSubtypeData(ser);

        ser.writeChunkEnd(CHUNK_ID);

        // save all pages (in separate files)
        for (PageMap::iterator i = mPages.begin(); i != mPages.end(); ++i)
        {
            i->second->save();
        }


    }
    //---------------------------------------------------------------------
    PageID PagedWorldSection::getPageID(const Vector3& worldPos)
    {
        return mStrategy->getPageID(worldPos, this);
    }
    //---------------------------------------------------------------------
    Page* PagedWorldSection::loadOrCreatePage(const Vector3& worldPos)
    {
        PageID id = getPageID(worldPos);
        // this will create a Page instance no matter what, even if load fails
        // we force the load attempt to happen immediately (forceSynchronous)
        loadPage(id, true);
        return getPage(id);
    }
    //---------------------------------------------------------------------
    void PagedWorldSection::loadPage(PageID pageID, bool sync)
    {
        if (!mParent->getManager()->getPagingOperationsEnabled())
            return;

        PageMap::iterator i = mPages.find(pageID);
        if (i == mPages.end())
        {
            Page* page = OGRE_NEW Page(pageID, this);
            // try to insert
            std::pair<PageMap::iterator, bool> ret = mPages.emplace(page->getID(), page);

            if (!ret.second)
            {
                // page with this ID already in map
                if (ret.first->second != page)
                {
                    // replacing a page, delete the old one
                    OGRE_DELETE ret.first->second;
                    ret.first->second = page;
                }
            }
            page->load(sync);
        }
        else
            i->second->touch();
    }
    //---------------------------------------------------------------------
    void PagedWorldSection::unloadPage(PageID pageID, bool sync)
    {
        if (!mParent->getManager()->getPagingOperationsEnabled())
            return;

        PageMap::iterator i = mPages.find(pageID);
        if (i != mPages.end())
        {
            Page* page = i->second;
            mPages.erase(i);

            page->unload();

            OGRE_DELETE page;
            
        }
    }
    //---------------------------------------------------------------------
    void PagedWorldSection::unloadPage(Page* p, bool sync)
    {
        unloadPage(p->getID(), sync);
    }
    //---------------------------------------------------------------------
    bool PagedWorldSection::_prepareProceduralPage(Page* page)
    {
        bool generated = false;
        if (mPageProvider)
            generated = mPageProvider->prepareProceduralPage(page, this);
        if (!generated)
            generated = mParent->_prepareProceduralPage(page, this);
        return generated;

    }
    //---------------------------------------------------------------------
    bool PagedWorldSection::_loadProceduralPage(Page* page)
    {
        bool generated = false;
        if (mPageProvider)
            generated = mPageProvider->loadProceduralPage(page, this);
        if (!generated)
            generated = mParent->_loadProceduralPage(page, this);
        return generated;

    }
    //---------------------------------------------------------------------
    bool PagedWorldSection::_unloadProceduralPage(Page* page)
    {
        bool generated = false;
        if (mPageProvider)
            generated = mPageProvider->unloadProceduralPage(page, this);
        if (!generated)
            generated = mParent->_unloadProceduralPage(page, this);
        return generated;

    }
    //---------------------------------------------------------------------
    bool PagedWorldSection::_unprepareProceduralPage(Page* page)
    {
        bool generated = false;
        if (mPageProvider)
            generated = mPageProvider->unprepareProceduralPage(page, this);
        if (!generated)
            generated = mParent->_unprepareProceduralPage(page, this);
        return generated;

    }
    //---------------------------------------------------------------------
    void PagedWorldSection::holdPage(PageID pageID)
    {
        PageMap::iterator i = mPages.find(pageID);
        if (i != mPages.end())
            i->second->touch();
    }
    //---------------------------------------------------------------------
    Page* PagedWorldSection::getPage(PageID pageID)
    {
        PageMap::iterator i = mPages.find(pageID);
        if (i != mPages.end())
            return i->second;
        else
            return 0;
    }
    //---------------------------------------------------------------------
    void PagedWorldSection::removeAllPages()
    {
        if (!mParent->getManager()->getPagingOperationsEnabled())
            return;

        for (PageMap::iterator i= mPages.begin(); i != mPages.end(); ++i)
        {
            OGRE_DELETE i->second;
        }
        mPages.clear();

    }
    //---------------------------------------------------------------------
    void PagedWorldSection::frameStart(Real timeSinceLastFrame)
    {
        mStrategy->frameStart(timeSinceLastFrame, this);

        for (PageMap::iterator i = mPages.begin(); i != mPages.end(); ++i)
            i->second->frameStart(timeSinceLastFrame);
    }
    //---------------------------------------------------------------------
    void PagedWorldSection::frameEnd(Real timeElapsed)
    {
        mStrategy->frameEnd(timeElapsed, this);

        for (PageMap::iterator i = mPages.begin(); i != mPages.end(); )
        {
            // if this page wasn't used, unload
            Page* p = i->second;
            // pre-increment since unloading will remove it
            ++i;
            if (!p->isHeld())
                unloadPage(p);
            else
                p->frameEnd(timeElapsed);
        }

    }
    //---------------------------------------------------------------------
    void PagedWorldSection::notifyCamera(Camera* cam)
    {
        mStrategy->notifyCamera(cam, this);

        for (PageMap::iterator i = mPages.begin(); i != mPages.end(); ++i)
            i->second->notifyCamera(cam);
    }
    //---------------------------------------------------------------------
    StreamSerialiser* PagedWorldSection::_readPageStream(PageID pageID)
    {
        StreamSerialiser* ser = 0;
        if (mPageProvider)
            ser = mPageProvider->readPageStream(pageID, this);
        if (!ser)
            ser = mParent->_readPageStream(pageID, this);
        return ser;

    }
    //---------------------------------------------------------------------
    StreamSerialiser* PagedWorldSection::_writePageStream(PageID pageID)
    {
        StreamSerialiser* ser = 0;
        if (mPageProvider)
            ser = mPageProvider->writePageStream(pageID, this);
        if (!ser)
            ser = mParent->_writePageStream(pageID, this);
        return ser;

    }
    //---------------------------------------------------------------------
    const String& PagedWorldSection::getType()
    {
        static const String stype("General");
        return stype;
    }
    //---------------------------------------------------------------------
    std::ostream& operator <<( std::ostream& o, const PagedWorldSection& p )
    {
        o << "PagedWorldSection(" << p.getName() << ", world:" << p.getWorld()->getName() << ")";
        return o;
    }





}

