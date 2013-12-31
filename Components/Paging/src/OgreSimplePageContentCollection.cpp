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
		for (ContentList::iterator i = mContentList.begin(); i != mContentList.end(); ++i)
			delete *i;
		mContentList.clear();
	}
	//---------------------------------------------------------------------
	PageContent* SimplePageContentCollection::createContent(const String& typeName)
	{
		PageContent* c = getManager()->createContent(typeName);
		mContentList.push_back(c);
		return c;
	}
	//---------------------------------------------------------------------
	void SimplePageContentCollection::destroyContent(PageContent* c)
	{
		ContentList::iterator i = std::find(mContentList.begin(), mContentList.end(), c);
		if (i != mContentList.end())
			mContentList.erase(i);
		getManager()->destroyContent(c);
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
	bool SimplePageContentCollection::prepare(StreamSerialiser& stream)
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
	void SimplePageContentCollection::load()
	{
		for (ContentList::iterator i = mContentList.begin(); i != mContentList.end(); ++i)
			(*i)->load();

	}
	//---------------------------------------------------------------------
	void SimplePageContentCollection::unload()
	{
		for (ContentList::iterator i = mContentList.begin(); i != mContentList.end(); ++i)
			(*i)->unload();

	}
	//---------------------------------------------------------------------
	void SimplePageContentCollection::unprepare()
	{
		for (ContentList::iterator i = mContentList.begin(); i != mContentList.end(); ++i)
			(*i)->unprepare();
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	String SimplePageContentCollectionFactory::FACTORY_NAME = "Simple";
	//---------------------------------------------------------------------



}

