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
#include "OgreTerrainPageSource.h"
#include "OgreTerrainPage.h"
#include "OgreTerrainRenderable.h"
#include "OgreSceneNode.h"
#include "OgreTerrainSceneManager.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    template<> TerrainPageSourceListenerManager* Singleton<TerrainPageSourceListenerManager>::ms_Singleton = 0;
    TerrainPageSourceListenerManager* TerrainPageSourceListenerManager::getSingletonPtr(void)
    {
        return ms_Singleton;
    }
    TerrainPageSourceListenerManager& TerrainPageSourceListenerManager::getSingleton(void)
    {  
        assert( ms_Singleton );  return ( *ms_Singleton );  
    }
    //-------------------------------------------------------------------------
	void TerrainPageSourceListenerManager::addListener(TerrainPageSourceListener* pl)
	{
        mPageSourceListeners.push_back(pl);
	}
    //-------------------------------------------------------------------------
	void TerrainPageSourceListenerManager::removeListener(TerrainPageSourceListener* pl)
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
	void TerrainPageSourceListenerManager::firePageConstructed(
		TerrainSceneManager* sm, size_t pagex, size_t pagez, Real* heightData)
	{
        PageSourceListenerList::iterator i, iend;
        iend = mPageSourceListeners.end();
        for(i = mPageSourceListeners.begin(); i != iend; ++i)
        {
            (*i)->pageConstructed(sm, pagex, pagez, heightData);
        }
	}
	//-------------------------------------------------------------------------
	TerrainPageSource::TerrainPageSource() : mSceneManager(0), mAsyncLoading(false) {
	}
	//-------------------------------------------------------------------------
	TerrainPage* TerrainPageSource::buildPage(Real* heightData, const MaterialPtr& pMaterial)
    {
        String name;

        // Create a Terrain Page
        TerrainPage* page = OGRE_NEW TerrainPage((mPageSize-1) / (mTileSize-1));
        // Create a node for all tiles to be attached to
        // Note we sequentially name since page can be attached at different points
        // so page x/z is not appropriate
		StringUtil::StrStreamType page_str;
		size_t pageIndex = mSceneManager->_getPageCount();
		page_str << pageIndex;
        name = "page[";
        name += page_str.str() + "]";
		if (mSceneManager->hasSceneNode(name))
		{
			page->pageSceneNode = mSceneManager->getSceneNode(name);
		}
		else
		{
			page->pageSceneNode = mSceneManager->createSceneNode(name);
		}
        
        size_t q = 0;
        for ( ushort j = 0; j < mPageSize - 1; j += ( mTileSize - 1 ) )
        {
            size_t p = 0;

            for ( ushort i = 0; i < mPageSize - 1; i += ( mTileSize - 1 ) )
            {
				StringUtil::StrStreamType new_name_str;
				
                // Create scene node for the tile and the TerrainRenderable
                new_name_str << "tile[" << pageIndex << "][" << (int)p << "," << (int)q << "]";
				name = new_name_str.str();

                SceneNode *c;
				if (mSceneManager->hasSceneNode(name))
				{
					c = mSceneManager->getSceneNode( name );
					if (c->getParentSceneNode() != page->pageSceneNode)
						page->pageSceneNode->addChild(c);
				}
				else
				{
					c = page->pageSceneNode->createChildSceneNode( name );
				}

				TerrainRenderable *tile = OGRE_NEW TerrainRenderable(name, mSceneManager);
				// set queue
				tile->setRenderQueueGroup(mSceneManager->getWorldGeometryRenderQueue());
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

		if(mSceneManager->getOptions().lit)
		{
			q = 0;
			for ( ushort j = 0; j < mPageSize - 1; j += ( mTileSize - 1 ) )
			{
				size_t p = 0;

				for ( ushort i = 0; i < mPageSize - 1; i += ( mTileSize - 1 ) )
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
    void TerrainPageSource::firePageConstructed(size_t pagex, size_t pagez, Real* heightData)
    {
		TerrainPageSourceListenerManager::getSingleton().firePageConstructed(
			mSceneManager, pagex, pagez, heightData);
    }
    //-------------------------------------------------------------------------
    void TerrainPageSource::addListener(TerrainPageSourceListener* pl)
    {
		TerrainPageSourceListenerManager::getSingleton().addListener(pl);
    }
    //-------------------------------------------------------------------------
    void TerrainPageSource::removeListener(TerrainPageSourceListener* pl)
    {
		TerrainPageSourceListenerManager::getSingleton().removeListener(pl);

    }
    //-------------------------------------------------------------------------

}
