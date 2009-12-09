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
		@param typeName The name of the type of content  (see PageManager::getContentFactories)
		*/
		virtual PageContent* createContent(const String& typeName);

		/** Destroy a PageContent within this page.
		This is equivalent to calling detachContent and 
		PageManager::destroyContent.
		*/
		virtual void destroyContent(PageContent* coll);

		/// Get const access to the list of content
		virtual const ContentList& getContentList() const { return mContentList; }

		// Overridden from PageContentCollection
		virtual void save(StreamSerialiser& stream);
		virtual void frameStart(Real timeSinceLastFrame);
		virtual void frameEnd(Real timeElapsed);
		virtual void notifyCamera(Camera* cam);
		bool prepare(StreamSerialiser& stream);
		void load();
		void unload();
		void unprepare();

	protected:




	};


	/// Factory class for SimplePageContentCollection
	class _OgrePagingExport SimplePageContentCollectionFactory : public PageContentCollectionFactory
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
