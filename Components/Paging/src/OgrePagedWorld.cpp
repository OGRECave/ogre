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

namespace Ogre
{
	//---------------------------------------------------------------------
	PagedWorld::PagedWorld(const String& name, PageManager* manager)
		:mName(name), mManager(manager), mSectionNameGenerator("Section")
	{

	}
	//---------------------------------------------------------------------
	PagedWorld::~PagedWorld()
	{

	}
	//---------------------------------------------------------------------
	void PagedWorld::load(const String& filename)
	{
		// Load from any resource location
		DataStreamPtr stream = ResourceGroupManager::getSingleton().openResource(filename);

		if (stream.isNull())
		{
			// try manual load (absolute files)
			struct stat tagStat;
			if (stat(filename.c_str(), &tagStat) == 0)
			{
				std::ifstream* fstr = OGRE_NEW_T(std::ifstream, MEMCATEGORY_GENERAL)();
				fstr->open(filename.c_str(), std::ios::in | std::ios::binary);
				if (fstr->fail())
				{
					OGRE_DELETE_T(fstr, basic_ifstream, MEMCATEGORY_GENERAL);
				}
				else
				{
					stream = DataStreamPtr(OGRE_NEW FileStreamDataStream(filename,
						fstr, tagStat.st_size, true));
				}
			}
		}

		if (stream.isNull())
		{
			OGRE_EXCEPT(Exception::ERR_FILE_NOT_FOUND,
				"Cannot open world file: " + filename,
				"PagedWorld::load");
		}

		load(stream);

	}
	//---------------------------------------------------------------------
	void PagedWorld::load(const DataStreamPtr& stream)
	{
		// TODO
	}
	//---------------------------------------------------------------------
	void PagedWorld::save(const String& filename, Archive* arch)
	{
		DataStreamPtr stream;
		if (arch)
		{
			stream = arch->create(filename);
		}
		else
		{
			std::fstream* fstr = OGRE_NEW_T(std::fstream, MEMCATEGORY_GENERAL)();
			fstr->open(filename.c_str(), std::ios::out | std::ios::binary);
			if (fstr->fail())
			{
				OGRE_DELETE_T(fstr, basic_fstream, MEMCATEGORY_GENERAL);
				OGRE_EXCEPT(Exception::ERR_FILE_NOT_FOUND,
					"Cannot open world file for writing: " + filename,
					"PageManager::saveWorld");
			}
			stream = DataStreamPtr(OGRE_NEW FileStreamDataStream(filename,
				fstr, 0, true));

		}

		save(stream);
		
	}
	//---------------------------------------------------------------------
	void PagedWorld::save(const DataStreamPtr& stream)
	{
		// TODO
	}
	//---------------------------------------------------------------------
	PagedWorldSection* PagedWorld::createSection(const String& strategyName, 
		const String& sectionName)
	{
		// get the strategy
		PageStrategy* strategy = mManager->getStrategy(strategyName);

		return createSection(strategy, sectionName);

	}
	//---------------------------------------------------------------------
	PagedWorldSection* PagedWorld::createSection(PageStrategy* strategy, 
		const String& sectionName)
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

		PagedWorldSection* ret = OGRE_NEW PagedWorldSection(theName, this, strategy);
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




}

