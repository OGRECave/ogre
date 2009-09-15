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
OgreTerrainZonePageSource.cpp  -  based on OgreTerrainPageSource.cpp from Ogre3d 

-----------------------------------------------------------------------------
begin                : Thu May 3 2007
author               : Eric Cha
email                : ericcATxenopiDOTcom

-----------------------------------------------------------------------------
*/

#include "OgreTerrainZonePageSource.h"
#include "OgreTerrainZonePage.h"
#include "OgreTerrainZoneRenderable.h"
#include "OgreSceneNode.h"
#include "OgreTerrainZone.h"
#include "OgrePCZSceneManager.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    template<> TerrainZonePageSourceListenerManager* Singleton<TerrainZonePageSourceListenerManager>::ms_Singleton = 0;
    TerrainZonePageSourceListenerManager* TerrainZonePageSourceListenerManager::getSingletonPtr(void)
    {
        return ms_Singleton;
    }
    TerrainZonePageSourceListenerManager& TerrainZonePageSourceListenerManager::getSingleton(void)
    {  
        assert( ms_Singleton );  return ( *ms_Singleton );  
    }
    //-------------------------------------------------------------------------
	void TerrainZonePageSourceListenerManager::addListener(TerrainZonePageSourceListener* pl)
	{
        mPageSourceListeners.push_back(pl);
	}
    //-------------------------------------------------------------------------
	void TerrainZonePageSourceListenerManager::removeListener(TerrainZonePageSourceListener* pl)
	{
        PageSourceListenerList::iterator i, iend;
        iend = mPageSourceListeners.end();
        for(i = mPageSourceListeners.begin(); i != iend; ++i)
        {
            if (*i == pl)
            {
                mPageSourceListeners.erase(i);
                break;
            }
        }
	}
    //-------------------------------------------------------------------------
	void TerrainZonePageSourceListenerManager::firePageConstructed(
		TerrainZone* sm, size_t pagex, size_t pagez, Real* heightData)
	{
        PageSourceListenerList::iterator i, iend;
        iend = mPageSourceListeners.end();
        for(i = mPageSourceListeners.begin(); i != iend; ++i)
        {
            (*i)->pageConstructed(sm, pagex, pagez, heightData);
        }
	}
	//-------------------------------------------------------------------------
	TerrainZonePageSource::TerrainZonePageSource() : mTerrainZone(0), mAsyncLoading(false) {
	}
	//-------------------------------------------------------------------------
	TerrainZonePage* TerrainZonePageSource::buildPage(Real* heightData, const MaterialPtr& pMaterial)
    {
        String name;

        // Create a TerrainZone Page
        TerrainZonePage* page = OGRE_NEW TerrainZonePage((mPageSize-1) / (mTileSize-1));
        // Create a node for all tiles to be attached to
        // Note we sequentially name since page can be attached at different points
        // so page x/z is not appropriate
		StringUtil::StrStreamType page_str;
		size_t pageIndex = mTerrainZone->_getPageCount();
		page_str << pageIndex;
        name = mTerrainZone->getName() + "_page[";
        name += page_str.str() + "]_Node";
		if (mTerrainZone->mPCZSM->hasSceneNode(name))
		{
			page->pageSceneNode = mTerrainZone->mPCZSM->getSceneNode(name);
			// set the home zone of the scene node to the terrainzone
			((PCZSceneNode*)(page->pageSceneNode))->anchorToHomeZone(mTerrainZone);
			// EXPERIMENTAL - prevent terrain zone pages from visiting other zones
			((PCZSceneNode*)(page->pageSceneNode))->allowToVisit(false);
		}
		else
		{
			page->pageSceneNode = mTerrainZone->getTerrainRootNode()->createChildSceneNode(name);
			// set the home zone of the scene node to the terrainzone
			((PCZSceneNode*)(page->pageSceneNode))->anchorToHomeZone(mTerrainZone);
			// EXPERIMENTAL - prevent terrain zone pages from visiting other zones
			((PCZSceneNode*)(page->pageSceneNode))->allowToVisit(false);

		}
        
        size_t q = 0;
        for ( unsigned short j = 0; j < mPageSize - 1; j += ( mTileSize - 1 ) )
        {
            size_t p = 0;

            for ( unsigned short i = 0; i < mPageSize - 1; i += ( mTileSize - 1 ) )
            {
				StringUtil::StrStreamType new_name_str;
				
                // Create scene node for the tile and the TerrainZoneRenderable
                new_name_str << mTerrainZone->getName() << "_tile[" << pageIndex << "][" << (int)p << "," << (int)q << "]_Node";
				name = new_name_str.str();

                SceneNode *c;
				if (mTerrainZone->mPCZSM->hasSceneNode(name))
				{
					c = mTerrainZone->mPCZSM->getSceneNode( name );
					if (c->getParentSceneNode() != page->pageSceneNode)
						page->pageSceneNode->addChild(c);
					// set the home zone of the scene node to the terrainzone
					((PCZSceneNode*)c)->anchorToHomeZone(mTerrainZone);
					// EXPERIMENTAL - prevent terrain zone pages from visiting other zones
					((PCZSceneNode*)c)->allowToVisit(false);
				}
				else
				{
					c = page->pageSceneNode->createChildSceneNode( name );
					// set the home zone of the scene node to the terrainzone
					((PCZSceneNode*)c)->anchorToHomeZone(mTerrainZone);
					// EXPERIMENTAL - prevent terrain zone pages from visiting other zones
					((PCZSceneNode*)c)->allowToVisit(false);
				}

				TerrainZoneRenderable *tile = OGRE_NEW TerrainZoneRenderable(name, mTerrainZone);
				// set queue
				tile->setRenderQueueGroup(mTerrainZone->mPCZSM->getWorldGeometryRenderQueue());
                // Initialise the tile
                tile->setMaterial(pMaterial);
                tile->initialise(i, j, heightData);
                // Attach it to the page
                page->tiles[ p ][ q ] = tile;
                // Attach it to the node
                c ->attachObject( tile );
                p++;
            }

            q++;

        }

        pageIndex++;

        // calculate neighbours for page
        page->linkNeighbours();

		if(mTerrainZone->getOptions().lit)
		{
			q = 0;
			for ( unsigned short j = 0; j < mPageSize - 1; j += ( mTileSize - 1 ) )
			{
				size_t p = 0;

				for ( unsigned short i = 0; i < mPageSize - 1; i += ( mTileSize - 1 ) )
				{
					page->tiles[ p ][ q ]->_calculateNormals();
					p++;
				}
				q++;
			}
		}

        return page;
    }
    //-------------------------------------------------------------------------
    void TerrainZonePageSource::firePageConstructed(size_t pagex, size_t pagez, Real* heightData)
    {
		TerrainZonePageSourceListenerManager::getSingleton().firePageConstructed(
			mTerrainZone, pagex, pagez, heightData);
    }
    //-------------------------------------------------------------------------
    void TerrainZonePageSource::addListener(TerrainZonePageSourceListener* pl)
    {
		TerrainZonePageSourceListenerManager::getSingleton().addListener(pl);
    }
    //-------------------------------------------------------------------------
    void TerrainZonePageSource::removeListener(TerrainZonePageSourceListener* pl)
    {
		TerrainZonePageSourceListenerManager::getSingleton().removeListener(pl);

    }
    //-------------------------------------------------------------------------

}
