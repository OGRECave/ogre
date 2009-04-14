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

#ifndef __Ogre_SimplePageContentCollection_H__
#define __Ogre_SimplePageContentCollection_H__

#include "OgrePagingPrerequisites.h"
#include "OgrePageContentCollection.h"
#include "OgrePageContentCollectionFactory.h"

namespace Ogre
{
	/** \addtogroup Optional Components
	*  @{
	*/
	/** \addtogroup Paging
	*  Some details on paging component
	*/
	/*@{*/


	/** Specialisation of PageContentCollection which just provides a simple list
		of PageContent instances. 
	@par
	The data format for this in a file is:<br/>
	<b>SimplePageContentCollectionData (Identifier 'SPCD')</b>\n
	[Version 1]
	<table>
	<tr>
	<td><b>Name</b></td>
	<td><b>Type</b></td>
	<td><b>Description</b></td>
	</tr>
	<tr>
	<td>Nested contents</td>
	<td>Nested chunk list</td>
	<td>A sequence of nested PageContent chunks</td>
	</tr>
	</table>
	*/
	class _OgrePagingExport SimplePageContentCollection : public PageContentCollection
	{
	public:
		typedef vector<PageContent*>::type ContentList;
		static const uint32 SUBCLASS_CHUNK_ID;
		static const uint16 SUBCLASS_CHUNK_VERSION;
	protected:
		ContentList mContentList;
	public:
		SimplePageContentCollection(PageContentCollectionFactory* creator);
		~SimplePageContentCollection();

		/** Create a new PageContent within this collection.
		This is equivalent to calling PageManager::createContent and 
		then attachContent.
		@param typeName The name of the type of content  (see PageManager::getContentFactories)
		*/
		virtual PageContent* createContent(const String& typeName);

		/** Destroy a PageContent within this page.
		This is equivalent to calling detachContent and 
		PageManager::destroyContent.
		*/
		virtual void destroyContent(PageContent* coll);

		/** Add content to this collection.
		@remarks
			This collection will be responsible for deleting the content. 
		*/
		virtual void attachContent(PageContent* content);
		/** Remove content from this collection.
		@remarks
		This collection will no longer be responsible for deleting the content. 
		*/
		virtual void detachContent(PageContent* content);
		/// Get const access to the list of content
		virtual const ContentList& getContentList() const { return mContentList; }

		// Overridden from PageContentCollection
		virtual void save(StreamSerialiser& stream);
		virtual void frameStart(Real timeSinceLastFrame);
		virtual void frameEnd(Real timeElapsed);
		virtual void notifyCamera(Camera* cam);

	protected:
		// Overridden from PageLoadableUnit
		bool prepareImpl(StreamSerialiser& stream);
		void loadImpl();
		void unloadImpl();
		void unprepareImpl();




	};


	/// Factory class for SimplePageContentCollection
	class SimplePageContentCollectionFactory : public PageContentCollectionFactory
	{
	public:
		static String FACTORY_NAME;

		SimplePageContentCollectionFactory() {}
		~SimplePageContentCollectionFactory() {}

		const String& getName() const { return FACTORY_NAME; }

		PageContentCollection* createInstance() 
		{
			return OGRE_NEW SimplePageContentCollection(this); 
		}
		void destroyInstance(PageContentCollection* c)
		{
			OGRE_DELETE c;
		}


	};

	/*@}*/
	/*@}*/
}




#endif 