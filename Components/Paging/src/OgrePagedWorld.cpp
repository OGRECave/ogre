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
#include "OgrePagedWorld.h"
#include "OgreResourceGroupManager.h"

#include "OgrePageManager.h"
#include "OgrePagedWorldSection.h"
#include "OgreStreamSerialiser.h"

namespace Ogre
{
	//---------------------------------------------------------------------
	const uint32 PagedWorld::CHUNK_ID = StreamSerialiser::makeIdentifier("PWLD");
	const uint16 PagedWorld::CHUNK_VERSION = 1;
	//---------------------------------------------------------------------
	PagedWorld::PagedWorld(const String& name, PageManager* manager)
		:mName(name), mManager(manager), mPageProvider(0), mSectionNameGenerator("Section")
	{

	}
	//---------------------------------------------------------------------
	PagedWorld::~PagedWorld()
	{

	}
	//---------------------------------------------------------------------
	void PagedWorld::load(const String& filename)
	{
		StreamSerialiser* ser = mManager->_readWorldStream(filename);
		load(*ser);
		OGRE_DELETE ser;
	}
	//---------------------------------------------------------------------
	void PagedWorld::load(const DataStreamPtr& stream)
	{
		StreamSerialiser ser(stream);
		load(ser);
	}
	//---------------------------------------------------------------------
	bool PagedWorld::load(StreamSerialiser& ser)
	{
		if (!ser.readChunkBegin(CHUNK_ID, CHUNK_VERSION, "PagedWorld"))
			return false;

		// Name
		ser.read(&mName);
		// Sections
		while(ser.peekNextChunkID() == PagedWorldSection::CHUNK_ID)
		{
			PagedWorldSection* sec = OGRE_NEW PagedWorldSection(this);
			bool sectionsOk = sec->load(ser);
			if (sectionsOk)
				mSections[sec->getName()] = sec;
			else
			{
				OGRE_DELETE sec;
				break;
			}
		}

		ser.readChunkEnd(CHUNK_ID);

		return true;

	}
	//---------------------------------------------------------------------
	void PagedWorld::save(const String& filename)
	{
		StreamSerialiser* ser = mManager->_writeWorldStream(filename);
		save(*ser);
		OGRE_DELETE ser;
	}
	//---------------------------------------------------------------------
	void PagedWorld::save(const DataStreamPtr& stream)
	{
		StreamSerialiser ser(stream);
		save(ser);
	}
	//---------------------------------------------------------------------
	void PagedWorld::save(StreamSerialiser& ser)
	{
		ser.writeChunkBegin(CHUNK_ID, CHUNK_VERSION);

		// Name
		ser.write(&mName);
		// Sections
		for (SectionMap::iterator i = mSections.begin(); i != mSections.end(); ++i)
			i->second->save(ser);

		ser.writeChunkEnd(CHUNK_ID);
	}
	//---------------------------------------------------------------------
	PagedWorldSection* PagedWorld::createSection(const String& strategyName, 
		SceneManager* sceneMgr, const String& sectionName)
	{
		// get the strategy
		PageStrategy* strategy = mManager->getStrategy(strategyName);

		return createSection(strategy, sceneMgr, sectionName);

	}
	//---------------------------------------------------------------------
	PagedWorldSection* PagedWorld::createSection(PageStrategy* strategy, 
		SceneManager* sceneMgr, const String& sectionName)
	{
		String theName = sectionName;
		if (theName.empty())
		{
			do 
			{
				theName = mSectionNameGenerator.generate();
			} while (mSections.find(theName) != mSections.end());
		}
		else if(mSections.find(theName) != mSections.end())
		{
			OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM, 
				"World section named '" + theName + "' already exists!",
				"PagedWorld::createSection");
		}

		PagedWorldSection* ret = OGRE_NEW PagedWorldSection(theName, this, strategy, sceneMgr);
		mSections[theName] = ret;

		return ret;

	}
	//---------------------------------------------------------------------
	void PagedWorld::destroySection(const String& name)
	{
		SectionMap::iterator i = mSections.find(name);
		if (i != mSections.end())
		{
			OGRE_DELETE i->second;
			mSections.erase(i);
		}
	}
	//---------------------------------------------------------------------
	void PagedWorld::destroySection(PagedWorldSection* sec)
	{
		destroySection(sec->getName());
	}
	//---------------------------------------------------------------------
	PagedWorldSection* PagedWorld::getSection(const String& name)
	{
		SectionMap::iterator i = mSections.find(name);
		if (i != mSections.end())
			return i->second;
		else
			return 0;

	}
	//---------------------------------------------------------------------
	bool PagedWorld::_prepareProceduralPage(Page* page, PagedWorldSection* section)
	{
		bool generated = false;
		if (mPageProvider)
			generated = mPageProvider->prepareProceduralPage(page, section);
		if (!generated)
			generated = mManager->_prepareProceduralPage(page, section);
		return generated;

	}
	//---------------------------------------------------------------------
	bool PagedWorld::_loadProceduralPage(Page* page, PagedWorldSection* section)
	{
		bool generated = false;
		if (mPageProvider)
			generated = mPageProvider->loadProceduralPage(page, section);
		if (!generated)
			generated = mManager->_loadProceduralPage(page, section);
		return generated;

	}
	//---------------------------------------------------------------------
	bool PagedWorld::_unprepareProceduralPage(Page* page, PagedWorldSection* section)
	{
		bool generated = false;
		if (mPageProvider)
			generated = mPageProvider->unprepareProceduralPage(page, section);
		if (!generated)
			generated = mManager->_unprepareProceduralPage(page, section);
		return generated;

	}
	//---------------------------------------------------------------------
	bool PagedWorld::_unloadProceduralPage(Page* page, PagedWorldSection* section)
	{
		bool generated = false;
		if (mPageProvider)
			generated = mPageProvider->unloadProceduralPage(page, section);
		if (!generated)
			generated = mManager->_unloadProceduralPage(page, section);
		return generated;

	}
	//---------------------------------------------------------------------
	StreamSerialiser* PagedWorld::_readPageStream(PageID pageID, PagedWorldSection* section)
	{
		StreamSerialiser* ser = 0;
		if (mPageProvider)
			ser = mPageProvider->readPageStream(pageID, section);
		if (!ser)
			ser = mManager->_readPageStream(pageID, section);
		return ser;

	}
	//---------------------------------------------------------------------
	StreamSerialiser* PagedWorld::_writePageStream(PageID pageID, PagedWorldSection* section)
	{
		StreamSerialiser* ser = 0;
		if (mPageProvider)
			ser = mPageProvider->writePageStream(pageID, section);
		if (!ser)
			ser = mManager->_writePageStream(pageID, section);
		return ser;

	}
	//---------------------------------------------------------------------
	void PagedWorld::frameStart(Real t)
	{
		for (SectionMap::iterator i = mSections.begin(); i != mSections.end(); ++i)
		{
			i->second->frameStart(t);
		}
	}
	//---------------------------------------------------------------------
	void PagedWorld::frameEnd(Real t)
	{
		for (SectionMap::iterator i = mSections.begin(); i != mSections.end(); ++i)
		{
			i->second->frameEnd(t);
		}
	}
	//---------------------------------------------------------------------
	void PagedWorld::notifyCamera(Camera* cam)
	{
		for (SectionMap::iterator i = mSections.begin(); i != mSections.end(); ++i)
		{
			i->second->notifyCamera(cam);
		}
	}
	//---------------------------------------------------------------------
	std::ostream& operator <<( std::ostream& o, const PagedWorld& p )
	{
		o << "PagedWorld(" << p.getName() << ")";
		return o;
	}




}

