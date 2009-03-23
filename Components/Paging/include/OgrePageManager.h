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

#ifndef __Ogre_PageManager_H__
#define __Ogre_PageManager_H__

#include "OgrePagingPrerequisites.h"
#include "OgreString.h"
#include "OgreResourceGroupManager.h"

namespace Ogre
{
	/** \addtogroup Optional Components
	*  @{
	*/
	/** \addtogroup Paging
	*  Some details on paging component
	*  @{
	*/
	
	/** The PageManager is the entry point through which you load all PagedWorld instances, 
		and the place where PageStrategy instances and factory classes are
		registered to customise the paging behaviour.
	*/
	class _OgrePagingExport PageManager : public GeneralAllocatedObject
	{
	public:
		PageManager();
		virtual ~PageManager();

		/** Create a new PagedWorld instance. 
		@param name Optionally give a name to the world (if no name is given, one
			will be generated).
		*/
		PagedWorld* createWorld(const String& name = StringUtil::BLANK);

		/** Destroy a world. */
		void destroyWorld(const String& name);

		/** Destroy a world. */
		void destroyWorld(PagedWorld* world);

		/** Attach a pre-created PagedWorld instance to this manager. 
		*/
		void attachWorld(PagedWorld* world);
		/** Detach a PagedWorld instance from this manager (note: the caller then
			becomes responsible for the correct destruction of this instance)
		*/
		void detachWorld(PagedWorld* world);
		/** Load a new PagedWorld from a file. 
		@param filename The name of the file to load (standard is .world)
		@param name Optionally give a name to the world (if no name is given, one
		will be generated).
		*/
		PagedWorld* loadWorld(const String& filename, const String& name = StringUtil::BLANK);
		/** Load a new PagedWorld from a stream. 
		@param stream A stream from which to load the world data
		@param name Optionally give a name to the world (if no name is given, one
		will be generated).
		*/
		PagedWorld* loadWorld(const DataStreamPtr& stream, const String& name = StringUtil::BLANK);
		/** Save a PagedWorld instance to a file. 
		@param world The world to be saved
		@param filename The filename to save the data to
		@param arch The Archive that filename is relative to (optional)
		*/
		void saveWorld(PagedWorld* world, const String& filename, Archive* arch = 0);
		/** Save a PagedWorld instance to a file. 
		@param world The world to be saved
		@param stream The stream to save the data to
		*/
		void saveWorld(PagedWorld* world, const DataStreamPtr& stream);
		/** Get a named world.
		@params name The name of the world (not a filename, the identifying name)
		@returns The world, or null if the world doesn't exist.
		*/
		PagedWorld* getWorld(const String& name);
		typedef map<String, PagedWorld*>::type WorldMap;
		/** Get a reference to the worlds that are currently loaded. */
		const WorldMap& getWorlds() const { return mWorlds; }


		typedef map<String, PageStrategy*>::type StrategyMap;
		/** Add a new PageStrategy implementation. 
		@remarks
			The caller remains resonsible for destruction of this instance. 
		*/
		void addStrategy(PageStrategy* strategy);

		/** Remove a PageStrategy implementation. 
		*/
		void removeStrategy(PageStrategy* strategy);

		/** Get a PageStrategy.
		@param name The name of the strategy to retrieve
		@returns Pointer to a PageStrategy, or null if the strategy was not found.
		*/
		PageStrategy* getStrategy(const String& name);

		/** Get a reference to the registered strategies.
		*/
		const StrategyMap& getStrategies() const;
	protected:
		WorldMap mWorlds;
		StrategyMap mStrategies;
		size_t mWorldNameIndex;

	};

	/** @} */
	/** @} */
}





#endif 