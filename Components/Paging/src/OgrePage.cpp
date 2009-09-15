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
#include "OgrePage.h"
#include "OgreRoot.h"
#include "OgrePagedWorldSection.h"
#include "OgrePagedWorld.h"
#include "OgrePageStrategy.h"
#include "OgrePageManager.h"
#include "OgreSceneNode.h"
#include "OgreSceneManager.h"
#include "OgreStreamSerialiser.h"
#include "OgrePageContentCollectionFactory.h"
#include "OgrePageContentCollection.h"

namespace Ogre
{
	//---------------------------------------------------------------------
	const uint32 Page::CHUNK_ID = StreamSerialiser::makeIdentifier("PAGE");
	const uint16 Page::CHUNK_VERSION = 1;
	//---------------------------------------------------------------------
	Page::Page(PageID pageID)
		: mID(pageID)
		, mParent(0)
		, mDebugNode(0)
	{
		touch();
	}
	//---------------------------------------------------------------------
	Page::~Page()
	{
		destroy();

		for (ContentCollectionList::iterator i = mContentCollections.begin(); 
			i != mContentCollections.end(); ++i)
		{
			delete *i;
		}
		mContentCollections.clear();
	}
	//---------------------------------------------------------------------
	PageManager* Page::getManager() const
	{
		return mParent->getManager();
	}
	//---------------------------------------------------------------------
	void Page::_notifyAttached(PagedWorldSection* parent)
	{
		if (!parent && mParent && mDebugNode)
		{
			// destroy while we have the chance
			SceneNode::ObjectIterator it = mDebugNode->getAttachedObjectIterator();
			while(it.hasMoreElements())
				mParent->getSceneManager()->destroyMovableObject(it.getNext());
			mDebugNode->removeAndDestroyAllChildren();
			mParent->getSceneManager()->destroySceneNode(mDebugNode);

			mDebugNode = 0;
		}
		mParent = parent;
	}
	//---------------------------------------------------------------------
	void Page::touch()
	{
		mFrameLastHeld = Root::getSingleton().getNextFrameNumber() + 1;
	}
	//---------------------------------------------------------------------
	bool Page::isHeld() const
	{
		unsigned long nextFrame = Root::getSingleton().getNextFrameNumber();
		// 1-frame tolerance, since the next frame number varies depending which
		// side of frameRenderingQueued you are
		return (mFrameLastHeld == nextFrame ||
			mFrameLastHeld + 1 == nextFrame);
	}
	//---------------------------------------------------------------------
	bool Page::prepareImpl(StreamSerialiser& stream)
	{

		// Now do the real loading
		if (!stream.readChunkBegin(CHUNK_ID, CHUNK_VERSION, "Page"))
			return false;

		// pageID check (we should know the ID we're expecting)
		uint32 storedID;
		stream.read(&storedID);
		if (mID != storedID)
		{
			LogManager::getSingleton().stream() << "Error: Tried to populate Page ID " << mID
				<< " with data corresponding to page ID " << storedID;
			stream.undoReadChunk(CHUNK_ID);
			return false;
		}

		PageManager* mgr = getManager();
		
		while(stream.peekNextChunkID() == PageContentCollection::CHUNK_ID)
		{
			const StreamSerialiser::Chunk* collChunk = stream.readChunkBegin();
			String factoryName;
			stream.read(&factoryName);
			// Supported type?
			PageContentCollectionFactory* collFact = mgr->getContentCollectionFactory(factoryName);
			if (collFact)
			{
				PageContentCollection* collInst = collFact->createInstance();
				if (collInst->prepare(stream)) // read type-specific data
				{
					attachContentCollection(collInst);
				}
				else
				{
					LogManager::getSingleton().stream() << "Error preparing PageContentCollection type: " 
						<< factoryName << " in " << *this;
					collFact->destroyInstance(collInst);
				}
			}
			else
			{
				LogManager::getSingleton().stream() << "Unsupported PageContentCollection type: " 
					<< factoryName << " in " << *this;
				// skip
				stream.readChunkEnd(collChunk->id);
			}

		}


		return true;
	}
	//---------------------------------------------------------------------
	void Page::loadImpl()
	{

		// TODO
		
	}
	//---------------------------------------------------------------------
	void Page::unprepareImpl()
	{

		// TODO

	}
	//---------------------------------------------------------------------
	void Page::unloadImpl()
	{

		// TODO


	}
	//---------------------------------------------------------------------
	void Page::save(StreamSerialiser& stream)
	{
		stream.writeChunkBegin(CHUNK_ID, CHUNK_VERSION);

		// page id
		stream.write(&mID);

		// content collections
		for (ContentCollectionList::iterator i = mContentCollections.begin();
			i != mContentCollections.end(); ++i)
		{
			(*i)->save(stream);
		}

		stream.writeChunkEnd(CHUNK_ID);
	}
	//---------------------------------------------------------------------
	void Page::frameStart(Real timeSinceLastFrame)
	{
		updateDebugDisplay();

		// content collections
		for (ContentCollectionList::iterator i = mContentCollections.begin();
			i != mContentCollections.end(); ++i)
		{
			(*i)->frameStart(timeSinceLastFrame);
		}


	}
	//---------------------------------------------------------------------
	void Page::frameEnd(Real timeElapsed)
	{
		// content collections
		for (ContentCollectionList::iterator i = mContentCollections.begin();
			i != mContentCollections.end(); ++i)
		{
			(*i)->frameEnd(timeElapsed);
		}

	}
	//---------------------------------------------------------------------
	void Page::notifyCamera(Camera* cam)
	{
		// content collections
		for (ContentCollectionList::iterator i = mContentCollections.begin();
			i != mContentCollections.end(); ++i)
		{
			(*i)->notifyCamera(cam);
		}

	}
	//---------------------------------------------------------------------
	void Page::updateDebugDisplay()
	{
		uint8 dbglvl = getManager()->getDebugDisplayLevel();
		if (dbglvl > 0)
		{
			// update debug display
			if (!mDebugNode)
			{
				mDebugNode = mParent->getSceneManager()->getRootSceneNode()->createChildSceneNode();
			}
			mParent->getStrategy()->updateDebugDisplay(this, mDebugNode);

			mDebugNode->setVisible(true);
		}
		else if (mDebugNode)
		{
			mDebugNode->setVisible(false);
		}

	}
	//---------------------------------------------------------------------
	PageContentCollection* Page::createContentCollection(const String& typeName)
	{
		PageContentCollection* coll = getManager()->createContentCollection(typeName);
		attachContentCollection(coll);
		return coll;
	}
	//---------------------------------------------------------------------
	void Page::destroyContentCollection(PageContentCollection* coll)
	{
		detachContentCollection(coll);
		getManager()->destroyContentCollection(coll);
	}
	//---------------------------------------------------------------------
	void Page::attachContentCollection(PageContentCollection* coll)
	{
		coll->_notifyAttached(this);
		mContentCollections.push_back(coll);
	}
	//---------------------------------------------------------------------
	void Page::detachContentCollection(PageContentCollection* coll)
	{
		ContentCollectionList::iterator i = std::find(
			mContentCollections.begin(), mContentCollections.end(), coll);
		if (i != mContentCollections.end())
		{
			coll->_notifyAttached(0);
			mContentCollections.erase(i);
		}
	}
	//---------------------------------------------------------------------
	size_t Page::getContentCollectionCount() const
	{
		return mContentCollections.size();
	}
	//---------------------------------------------------------------------
	PageContentCollection* Page::getContentCollection(size_t index)
	{
		assert(index < mContentCollections.size());

		return mContentCollections[index];
	}
	//---------------------------------------------------------------------
	const Page::ContentCollectionList& Page::getContentCollectionList() const
	{
		return mContentCollections;
	}
	//---------------------------------------------------------------------
	SceneManager* Page::getSceneManager() const
	{
		return mParent->getSceneManager();
	}
	//---------------------------------------------------------------------
	std::ostream& operator <<( std::ostream& o, const Page& p )
	{
		o << "Page(ID:" << p.getID() << ", section:" << p.getParentSection()->getName()
			<< ", world:" << p.getParentSection()->getWorld()->getName() << ")";
		return o;
	}



}

