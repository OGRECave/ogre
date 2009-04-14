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
#include "OgreSimplePageContentCollection.h"
#include "OgrePageContent.h"
#include "OgreStreamSerialiser.h"
#include "OgrePageManager.h"

namespace Ogre
{
	//---------------------------------------------------------------------
	const uint32 SimplePageContentCollection::SUBCLASS_CHUNK_ID = StreamSerialiser::makeIdentifier("SPCD");
	const uint16 SimplePageContentCollection::SUBCLASS_CHUNK_VERSION = 1;
	//---------------------------------------------------------------------
	SimplePageContentCollection::SimplePageContentCollection(PageContentCollectionFactory* creator)
		: PageContentCollection(creator)
	{

	}
	//---------------------------------------------------------------------
	SimplePageContentCollection::~SimplePageContentCollection()
	{
		destroy();

		for (ContentList::iterator i = mContentList.begin(); i != mContentList.end(); ++i)
			delete *i;
		mContentList.clear();
	}
	//---------------------------------------------------------------------
	PageContent* SimplePageContentCollection::createContent(const String& typeName)
	{
		PageContent* c = getManager()->createContent(typeName);
		attachContent(c);
		return c;
	}
	//---------------------------------------------------------------------
	void SimplePageContentCollection::destroyContent(PageContent* c)
	{
		detachContent(c);
		getManager()->destroyContent(c);
	}
	//---------------------------------------------------------------------
	void SimplePageContentCollection::attachContent(PageContent* content)
	{
		mContentList.push_back(content);
	}
	//---------------------------------------------------------------------
	void SimplePageContentCollection::detachContent(PageContent* content)
	{
		ContentList::iterator i = std::find(mContentList.begin(), mContentList.end(), content);
		if (i != mContentList.end())
			mContentList.erase(i);
	}
	//---------------------------------------------------------------------
	void SimplePageContentCollection::save(StreamSerialiser& stream)
	{
		stream.writeChunkBegin(SUBCLASS_CHUNK_ID, SUBCLASS_CHUNK_VERSION);

		for (ContentList::iterator i = mContentList.begin(); i != mContentList.end(); ++i)
			(*i)->save(stream);

		stream.writeChunkEnd(SUBCLASS_CHUNK_ID);
	}
	//---------------------------------------------------------------------
	void SimplePageContentCollection::frameStart(Real timeSinceLastFrame)
	{
		for (ContentList::iterator i = mContentList.begin(); i != mContentList.end(); ++i)
			(*i)->frameStart(timeSinceLastFrame);

	}
	//---------------------------------------------------------------------
	void SimplePageContentCollection::frameEnd(Real timeElapsed)
	{
		for (ContentList::iterator i = mContentList.begin(); i != mContentList.end(); ++i)
			(*i)->frameEnd(timeElapsed);
	}
	//---------------------------------------------------------------------
	void SimplePageContentCollection::notifyCamera(Camera* cam)
	{
		for (ContentList::iterator i = mContentList.begin(); i != mContentList.end(); ++i)
			(*i)->notifyCamera(cam);
	}
	//---------------------------------------------------------------------
	bool SimplePageContentCollection::prepareImpl(StreamSerialiser& stream)
	{
		if (!stream.readChunkBegin(SUBCLASS_CHUNK_ID, SUBCLASS_CHUNK_VERSION, "SimplePageContentCollection"))
			return false;

		bool ret = true;
		for (ContentList::iterator i = mContentList.begin(); i != mContentList.end(); ++i)
			ret = (*i)->prepare(stream) && ret;


		stream.readChunkEnd(SUBCLASS_CHUNK_ID);

		return ret;

	}
	//---------------------------------------------------------------------
	void SimplePageContentCollection::loadImpl()
	{
		for (ContentList::iterator i = mContentList.begin(); i != mContentList.end(); ++i)
			(*i)->load();

	}
	//---------------------------------------------------------------------
	void SimplePageContentCollection::unloadImpl()
	{
		for (ContentList::iterator i = mContentList.begin(); i != mContentList.end(); ++i)
			(*i)->unload();

	}
	//---------------------------------------------------------------------
	void SimplePageContentCollection::unprepareImpl()
	{
		for (ContentList::iterator i = mContentList.begin(); i != mContentList.end(); ++i)
			(*i)->unprepare();
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	String SimplePageContentCollectionFactory::FACTORY_NAME = "Simple";
	//---------------------------------------------------------------------



}

