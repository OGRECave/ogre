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
#include "OgrePagedWorld.h"
#include "OgreResourceGroupManager.h"

#include "OgrePageManager.h"
#include "OgrePagedWorldSection.h"
#include "OgreStreamSerialiser.h"

namespace Ogre
{
	//---------------------------------------------------------------------
	const uint32 PagedWorld::CHUNK_ID = StreamSerialiser::makeIdentifier("PWLD");
	const uint32 PagedWorld::CHUNK_SECTIONDECLARATION_ID = StreamSerialiser::makeIdentifier("PWLS");
	const uint16 PagedWorld::CHUNK_VERSION = 1;
	//---------------------------------------------------------------------
	PagedWorld::PagedWorld(const String& name, PageManager* manager)
		:mName(name), mManager(manager), mPageProvider(0), mSectionNameGenerator("Section")
	{

	}
	//---------------------------------------------------------------------
	PagedWorld::~PagedWorld()
	{
		destroyAllSections();
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
		while(ser.peekNextChunkID() == CHUNK_SECTIONDECLARATION_ID)
		{
			ser.readChunkBegin();
			String sectionType, sectionName;
			ser.read(&sectionType);
			ser.read(&sectionName);
			ser.readChunkEnd(CHUNK_SECTIONDECLARATION_ID);
			// Scene manager will be loaded
			PagedWorldSection* sec = createSection(0, sectionType, sectionName);
			bool sectionsOk = sec->load(ser);
			if (!sectionsOk)
				destroySection(sec);
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
		{
			PagedWorldSection* sec = i->second;
			// declaration
			ser.writeChunkBegin(CHUNK_SECTIONDECLARATION_ID);
			ser.write(&sec->getType());
			ser.write(&sec->getName());
			ser.writeChunkEnd(CHUNK_SECTIONDECLARATION_ID);
			// data
			i->second->save(ser);
		}

		ser.writeChunkEnd(CHUNK_ID);
	}
	//---------------------------------------------------------------------
	PagedWorldSection* PagedWorld::createSection(SceneManager* sceneMgr,
		const String& typeName,
		const String& sectionName /*= StringUtil::BLANK*/)
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

		PagedWorldSection* ret = 0;
		if (typeName == "General")
			ret = OGRE_NEW PagedWorldSection(theName, this, sceneMgr);
		else
		{
			PagedWorldSectionFactory* fact = getManager()->getWorldSectionFactory(typeName);
			if (!fact)
			{
				OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
					"World section type '" + typeName + "' does not exist!",
					"PagedWorld::createSection");
			}

			ret = fact->createInstance(theName, this, sceneMgr);

		}
		mSections[theName] = ret;

		return ret;


	}
	//---------------------------------------------------------------------
	PagedWorldSection* PagedWorld::createSection(const String& strategyName, SceneManager* sceneMgr,
		const String& sectionName)
	{
		// get the strategy
		PageStrategy* strategy = mManager->getStrategy(strategyName);

		return createSection(strategy, sceneMgr, sectionName);
		
	}
	//---------------------------------------------------------------------
	PagedWorldSection* PagedWorld::createSection(PageStrategy* strategy, SceneManager* sceneMgr, 
		const String& sectionName)
	{
		PagedWorldSection* ret = createSection(sceneMgr, "General", sectionName);
		ret->setStrategy(strategy);

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
	void PagedWorld::destroyAllSections()
	{
		for (SectionMap::iterator i = mSections.begin(); i != mSections.end(); ++i)
			OGRE_DELETE i->second;
		mSections.clear();
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

