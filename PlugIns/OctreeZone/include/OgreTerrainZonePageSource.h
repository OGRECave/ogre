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
OgreTerrainZonePageSource.h  -  based on OgreTerrainPageSource.h from Ogre3d

-----------------------------------------------------------------------------
begin                : Thu May 3 2007
author               : Eric Cha
email                : ericcATxenopiDOTcom

-----------------------------------------------------------------------------
*/

#ifndef __TerrainZonePageSource_H__
#define __TerrainZonePageSource_H__

#include "OgreTerrainZonePrerequisites.h"
#include "OgreSingleton.h"

namespace Ogre {

    typedef std::pair<String, String> TerrainZonePageSourceOption;
    typedef vector<TerrainZonePageSourceOption>::type TerrainZonePageSourceOptionList;

    /** Abstract class which classes can override to receive notifications
        when a page is ready to be added to the terrain manager.
    */
    class _OgreOctreeZonePluginExport TerrainZonePageSourceListener
    {
    public:
        /** Listener method called when a new page is about to be constructed. 
		@param manager The manager in question
        @param pagex, pagez The index of the page being constructed
        @param heightData Array of normalised height data (0..1). The size of
            this buffer will conform to the scene manager page size. The listener
            may modify the data if it wishes.
        */
        virtual void pageConstructed(TerrainZone* zone, size_t pagex, size_t pagez, Real* heightData) = 0;
    };

	/** Simple manager class to hold onto a list of page source listeners 
	    across all sources.
	*/
	class _OgreOctreeZonePluginExport TerrainZonePageSourceListenerManager :
		public Singleton<TerrainZonePageSourceListenerManager>, public GeneralAllocatedObject
	{
	protected:
        typedef vector<TerrainZonePageSourceListener*>::type PageSourceListenerList;
        PageSourceListenerList mPageSourceListeners;
	public:
        TerrainZonePageSourceListenerManager() {}
        ~TerrainZonePageSourceListenerManager() {}

        /** Register a class which will be called back whenever a new page is
            available.
        @remarks
            Since this method is static, it applies to any page source which 
            is in active use; there is no need to register one per source.
        */
        void addListener(TerrainZonePageSourceListener* pl);
        /** Unregister a class which will be called back whenever a new page is
        available.
        */
        void removeListener(TerrainZonePageSourceListener* pl);
		
        /// Fire pageContructed events
        void firePageConstructed(TerrainZone* manager, size_t pagex, size_t pagez, Real* heightData);

       /** Override standard Singleton retrieval.
        */
        static TerrainZonePageSourceListenerManager& getSingleton(void);
        /** Override standard Singleton retrieval.
        */
        static TerrainZonePageSourceListenerManager* getSingletonPtr(void);	
	
	};


    /** Abstract class which describes the interface which a source of terrain 
        pages must implement.
    @remarks
        The TerrainZone can accept external classes as providers of
        terrain data, to allow terrain height data to come from anywhere the
        user application may choose, and additionally to support on-demand
        loading an unloading of terrain data. Providers must suclass this class, 
        and implement the abstract methods (details are described within each method)
    @par
        The overall sequence of events is this:
        <ol>
        <li>TerrainZone is created as usual, and options such as tile
        size etc are set.</li>
        <li>CustomTerrainZonePageSource is registered with TerrainZone by 
        calling registerPageSource(), registering a particular named type of source 
        data with this tile source. <li>
        <li>TerrainZone::setZoneGeometry is called. Depending on the
        configuration, this will call one of the page source classes 
        initialise methods, when the scene manager will communicate it's 
        preferred options. It does not have to load anything immediately on this 
        call (especially if the terrain options include paging). It will 
        also set this tile source as the primary.<li>
        <li>As and when TerrainZone requires more tiles (and this will
        either be done all up-front, or progressively depending on paging settings)
        it will call the primary tile source's requestPage() method, with the 
        page it requires. </li>
        <li>It is then the responsibility of the tile source to prepare
        TerrainRenderable instances for the page(s) requested, and to attach them 
        to the TerrainZone. Note that preparing the tiles does not
        involve modifying any shared data so may be done in an alternate thread,
        if required. Attaching them must be done synchronously though.
        <li>When paging, the TerrainZone will request tiles in advance,
        within it's 'buffer zone' so some delay in loading is acceptable. It
        will also indicate when tiles are no longer required (and will detach
        them); it is up to the tile source whether that memory is actually freed
        or held for a while longer.
        </ol>
    @note The comments on paging above are in principle, the implementation of
    paging in this manager is not present yet but the system is designed to 
    extend to it. For now, all tiles are requested up-front.
    */
	class _OgreOctreeZonePluginExport TerrainZonePageSource : public GeometryAllocatedObject
    {
    protected:
        /// Link back to parent manager
        TerrainZone* mTerrainZone;
        /// Has asynchronous loading been requested?
        bool mAsyncLoading;
        /// The expected size of the page in number of vertices
        unsigned short mPageSize;
        /// The expected size of a tile in number of vertices
        unsigned short mTileSize;

        /// Internal method for firing pageContructed events
        void firePageConstructed(size_t pagex, size_t pagez, Real* heightData);

        /** Utility method for building a page of tiles based on some source
        data, wherever that may have come from.
        @remarks
            It is expected that this height data is represented in the range
            [0..1], which will be duly scaled by the TerrainRenderables it
            creates.
        */
        virtual TerrainZonePage* buildPage(Real* heightData, const MaterialPtr& pMaterial);


    public:
        TerrainZonePageSource(); 
        virtual ~TerrainZonePageSource() { shutdown(); }

        /** Initialise this tile source based on a series of options as
            dictated by the scene manager. 
        @param tsm The TerrainZone doing the initialising. This should be 
            allowed NULL, for use by external tools if they want to read data 
            generically without necessarily having a real scene manager involved
        @param tileSize The number of horizontal (and hence also vertical) 
            vertices in a single tile (which is a TerrainRenderable). This will
            always be (2^n)+1.
        @param pageSize The number of horizontal (and hence also vertical) 
            vertices in a single page. This will always be (2^n)+1.
        @param asyncLoading
            True if the scene manager would like the tile source to load tiles
            asynchronously. It does not have to do this, although if it does not
            when requested, it will likely result in stalls in the terrain rendering.
        @param optionList 
            A list of name/value pairs describing custom options for this particular
            page source. The expected convention for option names is
            "TypeName.OptionName", where TypeName is the type under which this
            page source has been registered.
        */
        virtual void initialise(TerrainZone* tz, 
            ushort tileSize, ushort pageSize, bool asyncLoading, 
            TerrainZonePageSourceOptionList& optionList)
        {
            mTerrainZone = tz;
            mTileSize = tileSize;
            mPageSize = pageSize;
            mAsyncLoading = asyncLoading;
        }
        /** Shut down this tile source, freeing all it's memory ready for 
            decommissioning.
        @remarks
            This method will normally just be called on destruction; however
            it may also be called by the TerrainZone if another source
            is provided for the same type of tile source.
        */
        virtual void shutdown(void) {}

        /** Requests a new page of tiles from the source.
        @remarks
            The TerrainZone will call this method when it needs new tiles.
            In response, this class must prepare TerrainRenderable instances for
            the page requested and attach the entire page when ready using 
            TerrainZone::attachTerrainZonePage.
        @par
            Now, the tile source does not necessarily need to do all that before the
            return of this method. If it likes, and particularly if asynchronous 
            loading is enabled, it can merely queue this request, and process it
            either in another thread, or over a series of frames. The key thing
            is that attaching the new page has to be done synchronously with 
            the main rendering loop in order to avoid concurrency issues; 
            other than that, you are free to load and prepare new tiles in 
            a concurrent fashion if you like.
        @par
            Typically the scene manager will request at least one page up-front,
            with the possibility of requesting more if paging is enabled.
        @param x The x index of the page requested
        @param z The z index of the page requested
        */
        virtual void requestPage(ushort x, ushort z) = 0;
        /** This notifies the tile source that the specified page of tiles 
            has been automatically detached.
        @remarks
            When paging is enabled, tiles go out of scope and the TerrainZone
            detaches them automatically, notifying the TerrainZonePageSource that 
            this has happened. The tile source can choose to either keep these
            tiles in memory (incase they are requested again) or can delete them
            if it wishes to free memory. This freeing does not need to be done
            before the return of this method - like requesting tiles, the
            freeing of them can be done in another thread or across many frames
            if required, since the shared data in TerrainZone has already
            been updated synchronously when the page was detached.
        @param x The x index of the page expired
        @param z The z index of the page expired
        */
        virtual void expirePage(ushort x, ushort z) = 0;
        
        /** Register a class which will be called back whenever a new page is
            available.
        @remarks
            Since this method is static, it applies to any page source which 
            is in active use; there is no need to register one per source.
        */
        static void addListener(TerrainZonePageSourceListener* pl);
        /** Unregister a class which will be called back whenever a new page is
        available.
        */
        static void removeListener(TerrainZonePageSourceListener* pl);

    };

}

#endif
