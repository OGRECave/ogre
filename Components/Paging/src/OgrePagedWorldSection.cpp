/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd
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
#include "OgrePagedWorldSection.h"
#include "OgrePageStrategy.h"
#include "OgreStreamSerialiser.h"
#include "OgreException.h"
#include "OgrePagedWorld.h"
#include "OgrePageManager.h"
#include "OgrePage.h"
#include "OgrePageRequestQueue.h"
#include "OgreLogManager.h"

namespace Ogre
{
	//---------------------------------------------------------------------
	const uint32 PagedWorldSection::CHUNK_ID = StreamSerialiser::makeIdentifier("PWSC");
	const uint16 PagedWorldSection::CHUNK_VERSION = 1;
	//---------------------------------------------------------------------
	PagedWorldSection::PagedWorldSection(PagedWorld* parent)
		: mParent(parent), mStrategy(0), mPageStreamProvider(0)
	{

	}
	//---------------------------------------------------------------------
	PagedWorldSection::PagedWorldSection(const String& name, PagedWorld* parent, PageStrategy* strategy)
		: mName(name), mParent(parent), mStrategy(0), mPageStreamProvider(0)
	{
		setStrategy(strategy);
	}
	//---------------------------------------------------------------------
	PagedWorldSection::~PagedWorldSection()
	{
		if (mStrategy)
		{
			mStrategy->destroyData(mStrategyData);
			mStrategyData = 0;
		}
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
		}
	}
	//---------------------------------------------------------------------
	void PagedWorldSection::setStrategy(const String& stratName)
	{
		setStrategy(mParent->getManager()->getStrategy(stratName));
	}
	//---------------------------------------------------------------------
	bool PagedWorldSection::load(StreamSerialiser& ser)
	{
		const StreamSerialiser::Chunk* chunk = ser.readChunkBegin();
		if (chunk->id != CHUNK_ID)
		{
			ser.undoReadChunk(chunk->id);
			return false;
		}

		// Check version
		if (chunk->version > CHUNK_VERSION)
		{
			// skip the rest
			ser.readChunkEnd(chunk->id);
			OGRE_EXCEPT(Exception::ERR_INVALID_STATE, 
				"PagedWorldSection data version exceeds what this software can read!", 
				"PagedWorldSection::load");
		}

		// Name
		ser.read(&mName);
		// AABB
		ser.read(&mAABB);
		// Page Strategy Name
		String stratname;
		ser.read(&stratname);
		setStrategy(stratname);
		// Page Strategy Data
		bool strategyDataOk = mStrategyData->load(ser);
		if (!strategyDataOk)
			LogManager::getSingleton().stream() << "Error: PageStrategyData for section '"
			<< mName << "' was not loaded correctly, check file contents";

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
		// Page Strategy Name
		ser.write(&mStrategy->getName());
		// Page Strategy Data
		mStrategyData->save(ser);

		ser.writeChunkEnd(CHUNK_ID);

	}
	//---------------------------------------------------------------------
	void PagedWorldSection::loadPage(PageID pageID)
	{
		PageMap::iterator i = mPages.find(pageID);
		if (i == mPages.end())
			mParent->getManager()->getQueue()->loadPage(pageID, this);
		else
			i->second->touch();
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
	void PagedWorldSection::attachPage(Page* page)
	{
		// try to insert
		std::pair<PageMap::iterator, bool> ret = mPages.insert(
			PageMap::value_type(page->getID(), page));

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
		page->_notifyAttached(this);
			
	}
	//---------------------------------------------------------------------
	void PagedWorldSection::detachPage(Page* page)
	{
		PageMap::iterator i = mPages.find(page->getID());
		if (i != mPages.end() && i->second == page)
		{
			mPages.erase(i);
			page->_notifyAttached(0);
		}

	}
	//---------------------------------------------------------------------
	void PagedWorldSection::frameStart(Real timeSinceLastFrame)
	{
		mStrategy->frameStart(timeSinceLastFrame, this);
	}
	//---------------------------------------------------------------------
	void PagedWorldSection::frameEnd(Real timeElapsed)
	{
		mStrategy->frameEnd(timeElapsed, this);
	}
	//---------------------------------------------------------------------
	void PagedWorldSection::notifyCamera(Camera* cam)
	{
		mStrategy->notifyCamera(cam, this);
	}
	//---------------------------------------------------------------------
	StreamSerialiser* PagedWorldSection::_readPageStream(PageID pageID)
	{
		StreamSerialiser* ser = 0;
		if (mPageStreamProvider)
			ser = mPageStreamProvider->readPageStream(pageID, this);
		if (!ser)
			ser = mParent->_readPageStream(pageID, this);
		return ser;

	}
	//---------------------------------------------------------------------
	StreamSerialiser* PagedWorldSection::_writePageStream(PageID pageID)
	{
		StreamSerialiser* ser = 0;
		if (mPageStreamProvider)
			ser = mPageStreamProvider->writePageStream(pageID, this);
		if (!ser)
			ser = mParent->_writePageStream(pageID, this);
		return ser;

	}




}

