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
#include "OgrePageManager.h"
#include "OgrePagedWorld.h"
#include "OgrePageStrategy.h"
#include "OgrePageContentCollectionFactory.h"
#include "OgrePagedWorldSection.h"
#include "OgrePageContentFactory.h"
#include "OgreStringConverter.h"
#include "OgreException.h"
#include "OgrePagedWorldSection.h"
#include "OgrePagedWorld.h"
#include "OgreGrid2DPageStrategy.h"
#include "OgreSimplePageContentCollection.h"
#include "OgreStreamSerialiser.h"
#include "OgreRoot.h"
#include "OgrePageContent.h"

namespace Ogre
{
	//---------------------------------------------------------------------
	PageManager::PageManager()
		: mWorldNameGenerator("World")
		, mPageProvider(0)
		, mPageResourceGroup(ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME)
		, mDebugDisplayLvl(0)
		, mGrid2DPageStrategy(0)
		, mSimpleCollectionFactory(0)
	{

		mEventRouter.pManager = this;
		mEventRouter.pWorldMap = &mWorlds;

		Root::getSingleton().addFrameListener(&mEventRouter);

		createStandardStrategies();
		createStandardContentFactories();

	}
	//---------------------------------------------------------------------
	PageManager::~PageManager()
	{
		Root::getSingleton().removeFrameListener(&mEventRouter);

		OGRE_DELETE mGrid2DPageStrategy;
		OGRE_DELETE mSimpleCollectionFactory;
	}
	//---------------------------------------------------------------------
	void PageManager::createStandardStrategies()
	{
		mGrid2DPageStrategy = OGRE_NEW Grid2DPageStrategy(this);
		addStrategy(mGrid2DPageStrategy);

	}
	//---------------------------------------------------------------------
	void PageManager::createStandardContentFactories()
	{
		// collections
		mSimpleCollectionFactory = OGRE_NEW SimplePageContentCollectionFactory();
		addContentCollectionFactory(mSimpleCollectionFactory);

	}
	//---------------------------------------------------------------------
	PagedWorld* PageManager::createWorld(const String& name)
	{
		String theName = name;
		if (theName.empty())
		{
			do 
			{
				theName = mWorldNameGenerator.generate();
			} while (mWorlds.find(theName) != mWorlds.end());
		}
		else if(mWorlds.find(theName) != mWorlds.end())
		{
			OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM, 
				"World named '" + theName + "' already exists!",
				"PageManager::createWorld");
		}

		PagedWorld* ret = OGRE_NEW PagedWorld(theName, this);
		mWorlds[theName] = ret;

		return ret;
	}
	//---------------------------------------------------------------------
	void PageManager::destroyWorld(const String& name)
	{
		WorldMap::iterator i = mWorlds.find(name);
		if (i != mWorlds.end())
		{
			OGRE_DELETE i->second;
			mWorlds.erase(i);
		}

	}
	//---------------------------------------------------------------------
	void PageManager::destroyWorld(PagedWorld* world)
	{
		destroyWorld(world->getName());
	}
	//---------------------------------------------------------------------
	PagedWorld* PageManager::loadWorld(const String& filename, const String& name)
	{
		PagedWorld* ret = createWorld(name);

		StreamSerialiser* ser = _readWorldStream(filename);
		ret->load(*ser);
		OGRE_DELETE ser;

		return ret;


	}
	//---------------------------------------------------------------------
	PagedWorld* PageManager::loadWorld(const DataStreamPtr& stream, const String& name)
	{
		PagedWorld* ret = createWorld(name);

		ret->load(stream);

		return ret;
	}
	//---------------------------------------------------------------------
	void PageManager::saveWorld(PagedWorld* world, const String& filename)
	{
		world->save(filename);
	}
	//---------------------------------------------------------------------
	void PageManager::saveWorld(PagedWorld* world, const DataStreamPtr& stream)
	{
		world->save(stream);
	}
	//---------------------------------------------------------------------
	PagedWorld* PageManager::getWorld(const String& name)
	{
		WorldMap::iterator i = mWorlds.find(name);
		if (i != mWorlds.end())
			return i->second;
		else
			return 0;
	}
	//---------------------------------------------------------------------
	void PageManager::addStrategy(PageStrategy* strategy)
	{
		// note - deliberately allowing overriding
		mStrategies[strategy->getName()] = strategy;
	}
	//---------------------------------------------------------------------
	void PageManager::removeStrategy(PageStrategy* strategy)
	{
		StrategyMap::iterator i = mStrategies.find(strategy->getName());
		if (i != mStrategies.end() && i->second == strategy)
		{
			mStrategies.erase(i);
		}
	}
	//---------------------------------------------------------------------
	PageStrategy* PageManager::getStrategy(const String& name)
	{
		StrategyMap::iterator i = mStrategies.find(name);
		if (i != mStrategies.end())
			return i->second;
		else
			return 0;

	}
	//---------------------------------------------------------------------
	const PageManager::StrategyMap& PageManager::getStrategies() const
	{
		return mStrategies;
	}
	//---------------------------------------------------------------------
	void PageManager::addContentCollectionFactory(PageContentCollectionFactory* f)
	{
		// note - deliberately allowing overriding
		mContentCollectionFactories[f->getName()] = f;
	}
	//---------------------------------------------------------------------
	void PageManager::removeContentCollectionFactory(PageContentCollectionFactory* f)
	{
		ContentCollectionFactoryMap::iterator i = mContentCollectionFactories.find(f->getName());
		if (i != mContentCollectionFactories.end() && i->second == f)
		{
			mContentCollectionFactories.erase(i);
		}
	}
	//---------------------------------------------------------------------
	PageContentCollectionFactory* PageManager::getContentCollectionFactory(const String& name)
	{
		ContentCollectionFactoryMap::iterator i = mContentCollectionFactories.find(name);
		if (i != mContentCollectionFactories.end())
			return i->second;
		else
			return 0;

	}
	//---------------------------------------------------------------------
	const PageManager::ContentCollectionFactoryMap& PageManager::getContentCollectionFactories() const
	{
		return mContentCollectionFactories;
	}
	//---------------------------------------------------------------------
	PageContentCollection* PageManager::createContentCollection(const String& typeName)
	{
		PageContentCollectionFactory* fact = getContentCollectionFactory(typeName);
		if (!fact)
			OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
				typeName + " is not the name of a valid PageContentCollectionFactory", 
				"PageManager::createContentCollection");

		return fact->createInstance();
	}
	//---------------------------------------------------------------------
	void PageManager::destroyContentCollection(PageContentCollection* coll)
	{
		PageContentCollectionFactory* fact = getContentCollectionFactory(coll->getType());
		if (fact)
			fact->destroyInstance(coll);
		else
			OGRE_DELETE coll; // normally a safe fallback
	}
	//---------------------------------------------------------------------
	void PageManager::addContentFactory(PageContentFactory* f)
	{
		// note - deliberately allowing overriding
		mContentFactories[f->getName()] = f;
	}
	//---------------------------------------------------------------------
	void PageManager::removeContentFactory(PageContentFactory* f)
	{
		ContentFactoryMap::iterator i = mContentFactories.find(f->getName());
		if (i != mContentFactories.end() && i->second == f)
		{
			mContentFactories.erase(i);
		}
	}
	//---------------------------------------------------------------------
	PageContentFactory* PageManager::getContentFactory(const String& name)
	{
		ContentFactoryMap::iterator i = mContentFactories.find(name);
		if (i != mContentFactories.end())
			return i->second;
		else
			return 0;

	}
	//---------------------------------------------------------------------
	const PageManager::ContentFactoryMap& PageManager::getContentFactories() const
	{
		return mContentFactories;
	}
	//---------------------------------------------------------------------
	PageContent* PageManager::createContent(const String& typeName)
	{
		PageContentFactory* fact = getContentFactory(typeName);
		if (!fact)
			OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
			typeName + " is not the name of a valid PageContentFactory", 
			"PageManager::createContent");

		return fact->createInstance();
	}
	//---------------------------------------------------------------------
	void PageManager::destroyContent(PageContent* c)
	{
		PageContentFactory* fact = getContentFactory(c->getType());
		if (fact)
			fact->destroyInstance(c);
		else
			OGRE_DELETE c; // normally a safe fallback
	}
	//---------------------------------------------------------------------
	void PageManager::addWorldSectionFactory(PagedWorldSectionFactory* f)
	{
		// note - deliberately allowing overriding
		mWorldSectionFactories[f->getName()] = f;
	}
	//---------------------------------------------------------------------
	void PageManager::removeWorldSectionFactory(PagedWorldSectionFactory* f)
	{
		WorldSectionFactoryMap::iterator i = mWorldSectionFactories.find(f->getName());
		if (i != mWorldSectionFactories.end() && i->second == f)
		{
			mWorldSectionFactories.erase(i);
		}
	}
	//---------------------------------------------------------------------
	PagedWorldSectionFactory* PageManager::getWorldSectionFactory(const String& name)
	{
		WorldSectionFactoryMap::iterator i = mWorldSectionFactories.find(name);
		if (i != mWorldSectionFactories.end())
			return i->second;
		else
			return 0;

	}
	//---------------------------------------------------------------------
	const PageManager::WorldSectionFactoryMap& PageManager::getWorldSectionFactories() const
	{
		return mWorldSectionFactories;
	}
	//---------------------------------------------------------------------
	PagedWorldSection* PageManager::createWorldSection(const String& typeName)
	{
		PagedWorldSectionFactory* fact = getWorldSectionFactory(typeName);
		if (!fact)
			OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
			typeName + " is not the name of a valid PagedWorldSectionFactory", 
			"PageManager::createWorldSection");

		return fact->createInstance();
	}
	//---------------------------------------------------------------------
	void PageManager::destroyWorldSection(PagedWorldSection* coll)
	{
		PagedWorldSectionFactory* fact = getWorldSectionFactory(coll->getType());
		if (fact)
			fact->destroyInstance(coll);
		else
			OGRE_DELETE coll; // normally a safe fallback
	}
	//---------------------------------------------------------------------
	StreamSerialiser* PageManager::_readPageStream(PageID pageID, PagedWorldSection* section)
	{
		StreamSerialiser* ser = 0;
		if (mPageProvider)
			ser = mPageProvider->readPageStream(pageID, section);
		if (!ser)
		{
			// use default implementation
			StringUtil::StrStreamType nameStr;
			nameStr << section->getWorld()->getName() << "_" << section->getName() 
				<< "_" << pageID << ".page";
			DataStreamPtr stream = ResourceGroupManager::getSingleton().openResource(nameStr.str());

			ser = OGRE_NEW StreamSerialiser(stream);

		}

		return ser;

	}
	//---------------------------------------------------------------------
	StreamSerialiser* PageManager::_writePageStream(PageID pageID, PagedWorldSection* section)
	{
		StreamSerialiser* ser = 0;
		if (mPageProvider)
			ser = mPageProvider->writePageStream(pageID, section);
		if (!ser)
		{
			// use default implementation
			StringUtil::StrStreamType nameStr;
			nameStr << section->getWorld()->getName() << "_" << section->getName() 
				<< "_" << pageID << ".page";
			
			// create file, overwrite if necessary
			DataStreamPtr stream = ResourceGroupManager::getSingleton().createResource(
				nameStr.str(), mPageResourceGroup, true);

			ser = OGRE_NEW StreamSerialiser(stream);

		}

		return ser;

	}
	//---------------------------------------------------------------------
	StreamSerialiser* PageManager::_readWorldStream(const String& filename)
	{
		StreamSerialiser* ser = 0;
		if (mPageProvider)
			ser = mPageProvider->readWorldStream(filename);
		if (!ser)
		{
			// use default implementation
			DataStreamPtr stream = ResourceGroupManager::getSingleton().openResource(
				filename);

			ser = OGRE_NEW StreamSerialiser(stream);

		}

		return ser;

	}
	//---------------------------------------------------------------------
	StreamSerialiser* PageManager::_writeWorldStream(const String& filename)
	{
		StreamSerialiser* ser = 0;
		if (mPageProvider)
			ser = mPageProvider->writeWorldStream(filename);
		if (!ser)
		{
			// use default implementation
			// create file, overwrite if necessary
			DataStreamPtr stream = ResourceGroupManager::getSingleton().createResource(
				filename, mPageResourceGroup, true);

			ser = OGRE_NEW StreamSerialiser(stream);

		}

		return ser;

	}
	//---------------------------------------------------------------------
	bool PageManager::_prepareProceduralPage(Page* page, PagedWorldSection* section)
	{
		bool generated = false;
		if (mPageProvider)
			generated = mPageProvider->prepareProceduralPage(page, section);

		return generated;
	}
	//---------------------------------------------------------------------
	bool PageManager::_loadProceduralPage(Page* page, PagedWorldSection* section)
	{
		bool generated = false;
		if (mPageProvider)
			generated = mPageProvider->loadProceduralPage(page, section);

		return generated;
	}
	//---------------------------------------------------------------------
	bool PageManager::_unprepareProceduralPage(Page* page, PagedWorldSection* section)
	{
		bool generated = false;
		if (mPageProvider)
			generated = mPageProvider->unprepareProceduralPage(page, section);

		return generated;
	}
	//---------------------------------------------------------------------
	bool PageManager::_unloadProceduralPage(Page* page, PagedWorldSection* section)
	{
		bool generated = false;
		if (mPageProvider)
			generated = mPageProvider->unloadProceduralPage(page, section);

		return generated;
	}
	//---------------------------------------------------------------------
	void PageManager::addCamera(Camera* c)
	{
		if (std::find(mCameraList.begin(), mCameraList.end(), c) == mCameraList.end())
		{
			mCameraList.push_back(c);
			c->addListener(&mEventRouter);
		}
	}
	//---------------------------------------------------------------------
	void PageManager::removeCamera(Camera* c)
	{
		CameraList::iterator i = std::find(mCameraList.begin(), mCameraList.end(), c);
		if (i != mCameraList.end())
		{
			c->removeListener(&mEventRouter);
			mCameraList.erase(i);
		}
	}
	//---------------------------------------------------------------------
	bool PageManager::hasCamera(Camera* c) const
	{
		return std::find(mCameraList.begin(), mCameraList.end(), c) != mCameraList.end();
	}
	//---------------------------------------------------------------------
	const PageManager::CameraList& PageManager::getCameraList() const
	{
		return mCameraList;
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	void PageManager::EventRouter::cameraPreRenderScene(Camera* cam)
	{
		for(WorldMap::iterator i = pWorldMap->begin(); i != pWorldMap->end(); ++i)
			i->second->notifyCamera(cam);
	}
	//---------------------------------------------------------------------
	void PageManager::EventRouter::cameraDestroyed(Camera* cam)
	{
		pManager->removeCamera(cam);
	}
	//---------------------------------------------------------------------
	bool PageManager::EventRouter::frameStarted(const FrameEvent& evt)
	{

		for(WorldMap::iterator i = pWorldMap->begin(); i != pWorldMap->end(); ++i)
			i->second->frameStart(evt.timeSinceLastFrame);

		return true;
	}
	//---------------------------------------------------------------------
	bool PageManager::EventRouter::frameEnded(const FrameEvent& evt)
	{
		for(WorldMap::iterator i = pWorldMap->begin(); i != pWorldMap->end(); ++i)
			i->second->frameEnd(evt.timeSinceLastFrame);

		return true;
	}




}

