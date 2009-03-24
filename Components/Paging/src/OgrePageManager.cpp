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
#include "OgrePageManager.h"
#include "OgrePagedWorld.h"
#include "OgrePageStrategy.h"
#include "OgreStringConverter.h"
#include "OgreException.h"

namespace Ogre
{
	//---------------------------------------------------------------------
	PageManager::PageManager()
		: mWorldNameGenerator("World")
	{

	}
	//---------------------------------------------------------------------
	PageManager::~PageManager()
	{

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
		detachWorld(world);
		OGRE_DELETE world;
	}
	//---------------------------------------------------------------------
	void PageManager::attachWorld(PagedWorld* world)
	{
		if(mWorlds.find(world->getName()) != mWorlds.end())
		{
			OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM, 
				"World named '" + world->getName() + "' already exists!",
				"PageManager::attachWorld");
		}

		mWorlds[world->getName()] = world;
		
	}
	//---------------------------------------------------------------------
	void PageManager::detachWorld(PagedWorld* world)
	{
		WorldMap::iterator i = mWorlds.find(world->getName());
		if (i != mWorlds.end() && i->second == world)
		{
			mWorlds.erase(i);
		}
	}
	//---------------------------------------------------------------------
	PagedWorld* PageManager::loadWorld(const String& filename, const String& name)
	{
		PagedWorld* ret = createWorld(name);

		ret->load(filename);

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
	void PageManager::saveWorld(PagedWorld* world, const String& filename, Archive* arch)
	{
		world->save(filename, arch);
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


}

