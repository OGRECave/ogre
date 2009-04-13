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
		const StreamSerialiser::Chunk* chunk = stream.readChunkBegin();
		if (chunk->id != CHUNK_ID)
		{
			LogManager::getSingleton().stream() << "Error: Tried to populate Page ID " << mID
				<< " with non-Page data; chunk ID: " << chunk->id;
			stream.undoReadChunk(chunk->id);
			return false;
		}
		else if (chunk->version > CHUNK_VERSION)
		{
			// skip the rest
			stream.readChunkEnd(chunk->id);
			OGRE_EXCEPT(Exception::ERR_INVALID_STATE, 
				"Page data version exceeds what this software can read!", 
				"Page::prepare");

		}

		// pageID check (we should know the ID we're expecting)
		uint32 storedID;
		stream.read(&storedID);
		if (mID != storedID)
		{
			LogManager::getSingleton().stream() << "Error: Tried to populate Page ID " << mID
				<< " with data corresponding to page ID " << storedID;
			stream.undoReadChunk(chunk->id);
			return false;
		}

		PageManager* mgr = mParent->getWorld()->getManager();
		
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
		uint8 dbglvl = mParent->getWorld()->getManager()->getDebugDisplayLevel();
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
	std::ostream& operator <<( std::ostream& o, const Page& p )
	{
		o << "Page(ID:" << p.getID() << ", section:" << p.getParentSection()->getName()
			<< ", world:" << p.getParentSection()->getWorld()->getName() << ")";
		return o;
	}



}

