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

#ifndef __Ogre_PagedWorldSection_H__
#define __Ogre_PagedWorldSection_H__

#include "OgrePagingPrerequisites.h"
#include "OgreAxisAlignedBox.h"

namespace Ogre
{
    /** \addtogroup Optional Components
    *  @{
    */
    /** \addtogroup Paging
    *  Some details on paging component
    *  @{
    */

    /** Represents a section of the PagedWorld which uses a given PageStrategy, and
        which is made up of a generally localised set of Page instances.
    @remarks
        The reason for PagedWorldSection is that you may wish to cater for multiple
        sections of your world which use a different approach to paging (ie a
        different PageStrategy), or which are significantly far apart or separate 
        that the parameters you want to pass to the PageStrategy are different.
    @par
        PagedWorldSection instances are fully contained within the PagedWorld and
        their definitions are loaded in their entirety when the PagedWorld is
        loaded. However, no Page instances are initially loaded - those are the
        responsibility of the PageStrategy.
    @par
        PagedWorldSection can be subclassed and derived types provided by a
        PagedWorldSectionFactory. These subclasses might come preconfigured
        with a strategy for example, or with additional metadata used only for
        that particular type of section.
    @par
        A PagedWorldSection targets a specific SceneManager. When you create one
        in code via PagedWorld::createSection, you pass that SceneManager in manually.
        When loading from a saved world file however, the SceneManager type and
        instance name are saved and that SceneManager is looked up on loading, or
        created if it didn't exist. 
    */
    class _OgrePagingExport PagedWorldSection : public PageAlloc
    {
    public:
        typedef map<PageID, Page*>::type PageMap;
    protected:
        String mName;
        AxisAlignedBox mAABB;
        PagedWorld* mParent;
        PageStrategy* mStrategy;
        PageStrategyData* mStrategyData;
        PageMap mPages;
        PageProvider* mPageProvider;
        SceneManager* mSceneMgr;

        /// Load data specific to a subtype of this class (if any)
        virtual void loadSubtypeData(StreamSerialiser& ser) {}
        virtual void saveSubtypeData(StreamSerialiser& ser) {}


    public:
        static const uint32 CHUNK_ID;
        static const uint16 CHUNK_VERSION;

        /** Construct a new instance, specifying the parent and scene manager. */
        PagedWorldSection(const String& name, PagedWorld* parent, SceneManager* sm);
        virtual ~PagedWorldSection();

        PageManager* getManager() const;

        /// Get the name of this section
        virtual const String& getName() const { return mName; }
        /// Get the page strategy which this section is using
        virtual PageStrategy* getStrategy() const { return mStrategy; }
        /** Change the page strategy.
        @remarks
            Doing this will invalidate any pages attached to this world section, and
            require the PageStrategyData to be repopulated.
        */
        virtual void setStrategy(PageStrategy* strat);
        /** Change the page strategy.
        @remarks
            Doing this will invalidate any pages attached to this world section, and
            require the PageStrategyData to be repopulated.
        */
        virtual void setStrategy(const String& stratName);

        /** Change the SceneManager.
        @remarks
        Doing this will invalidate any pages attached to this world section, and
        require the pages to be reloaded.
        */
        virtual void setSceneManager(SceneManager* sm);
        
        /** Change the SceneManager.
        @remarks
        Doing this will invalidate any pages attached to this world section, and
        require the pages to be reloaded.
        @param smName The instance name of the SceneManager
        */
        virtual void setSceneManager(const String& smName);
        /// Get the current SceneManager
        virtual SceneManager* getSceneManager() const { return mSceneMgr; }

        /// Get the parent world
        virtual PagedWorld* getWorld() const { return mParent; }
        /// Get the data required by the PageStrategy which is specific to this world section
        virtual PageStrategyData* getStrategyData() const { return mStrategyData; }
        /// Set the bounds of this section
        virtual void setBoundingBox(const AxisAlignedBox& box);
        /// Get the bounds of this section
        virtual const AxisAlignedBox& getBoundingBox() const;


        /// Load this section from a stream (returns true if successful)
        virtual bool load(StreamSerialiser& stream);
        /// Save this section to a stream
        virtual void save(StreamSerialiser& stream);


        /// Called when the frame starts
        virtual void frameStart(Real timeSinceLastFrame);
        /// Called when the frame ends
        virtual void frameEnd(Real timeElapsed);
        /// Notify a section of the current camera
        virtual void notifyCamera(Camera* cam);

        /** Load or create a page against this section covering the given world 
            space position. 
        @remarks
            This method is designed mainly for editors - it will try to load
            an existing page if there is one, otherwise it will create a new one
            synchronously.
        */
        virtual Page* loadOrCreatePage(const Vector3& worldPos);

        /** Get the page ID for a given world position. */
        virtual PageID getPageID(const Vector3& worldPos);


        /** Ask for a page to be loaded with the given (section-relative) PageID
        @remarks
            You would not normally call this manually, the PageStrategy is in 
            charge of it usually.
            If this page is already loaded, this request will not load it again.
            If the page needs loading, then it may be an asynchronous process depending
            on whether threading is enabled.
        @param pageID The page ID to load
        @param forceSynchronous If true, the page will always be loaded synchronously
        */
        virtual void loadPage(PageID pageID, bool forceSynchronous = false);

        /** Ask for a page to be unloaded with the given (section-relative) PageID
        @remarks
            You would not normally call this manually, the PageStrategy is in 
            charge of it usually.
        @param pageID The page ID to unload
        @param forceSynchronous If true, the page will always be unloaded synchronously
        */
        virtual void unloadPage(PageID pageID, bool forceSynchronous = false);
        /** Ask for a page to be unloaded with the given (section-relative) PageID
        @remarks
        You would not normally call this manually, the PageStrategy is in 
        charge of it usually.
        @param p The Page to unload
        @param forceSynchronous If true, the page will always be unloaded synchronously
        */
        virtual void unloadPage(Page* p, bool forceSynchronous = false);
        /** Give a section the opportunity to prepare page content procedurally. 
        @remarks
        You should not call this method directly. This call may well happen in 
        a separate thread so it should not access GPU resources, use _loadProceduralPage
        for that
        @return true if the page was populated, false otherwise
        */
        virtual bool _prepareProceduralPage(Page* page);
        /** Give a section the opportunity to prepare page content procedurally. 
        @remarks
        You should not call this method directly. This call will happen in 
        the main render thread so it can access GPU resources. Use _prepareProceduralPage
        for background preparation.
        @return true if the page was populated, false otherwise
        */
        virtual bool _loadProceduralPage(Page* page);
        /** Give a section  the opportunity to unload page content procedurally. 
        @remarks
        You should not call this method directly. This call will happen in 
        the main render thread so it can access GPU resources. Use _unprepareProceduralPage
        for background preparation.
        @return true if the page was populated, false otherwise
        */
        virtual bool _unloadProceduralPage(Page* page);
        /** Give a section  the opportunity to unprepare page content procedurally. 
        @remarks
        You should not call this method directly. This call may well happen in 
        a separate thread so it should not access GPU resources, use _unloadProceduralPage
        for that
        @return true if the page was unpopulated, false otherwise
        */
        virtual bool _unprepareProceduralPage(Page* page);

        /** Ask for a page to be kept in memory if it's loaded.
        @remarks
            This method indicates that a page should be retained if it's already
            in memory, but if it's not then it won't trigger a load. This is useful
            for retaining pages that have just gone out of range, but which you
            don't want to unload just yet because it's quite possible they may come
            back into the active set again very quickly / easily. But at the same
            time, if they've already been purged you don't want to force them to load. 
            This is the 'maybe' region of pages. 
        @par
            Any Page that is neither requested nor held in a frame will be
            deemed a candidate for unloading.
        */
        virtual void holdPage(PageID pageID);

        /** Retrieves a Page.
        @remarks
            This method will only return Page instances that are already loaded. It
            will return null if a page is not loaded. 
        */
        virtual Page* getPage(PageID pageID);

        /** Remove all pages immediately. 
        @remarks
            Effectively 'resets' this section by deleting all pages. 
        */
        virtual void removeAllPages();

        /** Set the PageProvider which can provide streams Pages in this section. 
        @remarks
            This is the top-level way that you can direct how Page data is loaded. 
            When data for a Page is requested for a PagedWorldSection, the following
            sequence of classes will be checked to see if they have a provider willing
            to supply the stream: PagedWorldSection, PagedWorld, PageManager.
            If none of these do, then the default behaviour is to look for a file
            called worldname_sectionname_pageID.page. 
        @note
            The caller remains responsible for the destruction of the provider.
        */
        virtual void setPageProvider(PageProvider* provider) { mPageProvider = provider; }
        
        /** Get the PageProvider which can provide streams for Pages in this section. */
        virtual PageProvider* getPageProvider() const { return mPageProvider; }

        /** Get a serialiser set up to read Page data for the given PageID. 
        @param pageID The ID of the page being requested
        @remarks
        The StreamSerialiser returned is the responsibility of the caller to
        delete. 
        */
        virtual StreamSerialiser* _readPageStream(PageID pageID);

        /** Get a serialiser set up to write Page data for the given PageID. 
        @param pageID The ID of the page being requested
        @remarks
        The StreamSerialiser returned is the responsibility of the caller to
        delete. 
        */
        virtual StreamSerialiser* _writePageStream(PageID pageID);

        /** Function for writing to a stream.
        */
        _OgrePagingExport friend std::ostream& operator <<( std::ostream& o, const PagedWorldSection& p );

        /** Get the type name of this section. */
        virtual const String& getType();

    };


    /** A factory class for creating types of world section.
    */
    class _OgrePagingExport PagedWorldSectionFactory : public PageAlloc
    {
    public:
        virtual ~PagedWorldSectionFactory() {}
        virtual const String& getName() const = 0;
        virtual PagedWorldSection* createInstance(const String& name, PagedWorld* parent, SceneManager* sm) = 0;
        virtual void destroyInstance(PagedWorldSection*) = 0;


    };

    /** @} */
    /** @} */
}

#endif
