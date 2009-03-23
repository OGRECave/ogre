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

#ifndef __Ogre_PagedWorld_H__
#define __Ogre_PagedWorld_H__

#include "OgrePagingPrerequisites.h"
#include "OgreString.h"
#include "OgreDataStream.h"

namespace Ogre
{
	class PageManager;

	/** \addtogroup Optional Components
	*  @{
	*/
	/** \addtogroup Paging
	*  Some details on paging component
	*  @{
	*/


	/** This class represents a collection of pages which make up a world. 
	@remarks
		It's important to bear in mind that the PagedWorld only delineates the
		world and knows how to find out about the contents of it. It does not, 
		by design, contain all elements of the world, in memory, at once. 
	*/
	class _OgrePagingExport PagedWorld : public PageAlloc
	{
	protected:
		String mName;
		PageManager* mManager;
	public:
		/** Constructor.
		@param name The name of the world, which must be enough to identify the 
			place where data for it can be loaded from (doesn't have to be a filename
			necessarily).
		@param manager The PageManager that is in charge of providing this world with
			services such as related object factories.
		*/
		PagedWorld(const String& name, PageManager* manager);
		virtual ~PagedWorld();

		const String& getName() const { return mName; }
		/// Get the manager of this world
		PageManager* getManager() const { return mManager; }

		/// Load world data from a file
		void load(const String& filename);
		/// Load world data from a stream
		void load(const DataStreamPtr& stream);
		/** Save world data to a file
		@param filename The name of the file to create
		@param Archive Optional archive which the filename is relative to
		*/
		void save(const String& filename, Archive* arch = 0);
		/// Save world data to a stream
		void save(const DataStreamPtr& stream);

		/** Create a new section of the world.
		@remarks
			World sections are areas of the world that use a particular
			PageStrategy, with a certain set of parameters specific to that
			strategy. So you would have more than one section in a world only 
			if you needed different simultaneous paging strategies, or you 
			wanted the same strategy but parameterised differently.
		@param strategyName The name of the strategy to use (must be registered	
			with PageManager)
		@param sectionName An optional name to give the section (if none is
			provided, one will be generated)
		*/
		PagedWorldSection* createSection(const String& strategyName, 
			const String& sectionName = StringUtil::BLANK);


		/** Create a new section of the world.
		@remarks
			World sections are areas of the world that use a particular
			PageStrategy, with a certain set of parameters specific to that
			strategy. So you would have more than one section in a world only 
			if you needed different simultaneous paging strategies, or you 
			wanted the same strategy but parameterised differently.
		@param strategy The strategy to use 
		@param sectionName An optional name to give the section (if none is
			provided, one will be generated)
		*/
		PagedWorldSection* createSection(PageStrategy* strategy, 
			const String& sectionName = StringUtil::BLANK);

		/** Destroy a section of world. */
		void destroySection(PagedWorldSection* sec);

		/** Retrieve a section of the world. */
		PagedWorldSection* getSection(const String& name);

		typedef map<String, PagedWorldSection*>::type SectionMap;
		/// Retrieve a const reference to all the sections in this world
		const SectionMap& getSections() const { return mSections; }

	protected:
		SectionMap mSections;




	};
	
	/** @} */
	/** @} */

}




#endif 