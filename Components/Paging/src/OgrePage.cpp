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
	const uint32 Page::CHUNK_CONTENTCOLLECTION_DECLARATION_ID = StreamSerialiser::makeIdentifier("PCNT");
	const uint16 Page::WORKQUEUE_PREPARE_REQUEST = 1;
	const uint16 Page::WORKQUEUE_CHANGECOLLECTION_REQUEST = 3;

	//---------------------------------------------------------------------
	Page::Page(PageID pageID, PagedWorldSection* parent)
		: mID(pageID)
		, mParent(parent)
		, mDeferredProcessInProgress(false)
		, mModified(false)
		, mDebugNode(0)
	{
		WorkQueue* wq = Root::getSingleton().getWorkQueue();
		mWorkQueueChannel = wq->getChannel("Ogre/Page");
		wq->addRequestHandler(mWorkQueueChannel, this);
		wq->addResponseHandler(mWorkQueueChannel, this);
		touch();
	}
	//---------------------------------------------------------------------
	Page::~Page()
	{
		WorkQueue* wq = Root::getSingleton().getWorkQueue();
		wq->removeRequestHandler(mWorkQueueChannel, this);
		wq->removeResponseHandler(mWorkQueueChannel, this);

		destroyAllContentCollections();
		if (mDebugNode)
		{
			// destroy while we have the chance
			SceneNode::ObjectIterator it = mDebugNode->getAttachedObjectIterator();
			while(it.hasMoreElements())
				mParent->getSceneManager()->destroyMovableObject(it.getNext());
			mDebugNode->removeAndDestroyAllChildren();
			mParent->getSceneManager()->destroySceneNode(mDebugNode);

			mDebugNode = 0;
		}
	}
	//---------------------------------------------------------------------
	void Page::destroyAllContentCollections()
	{
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
	bool Page::prepareImpl(StreamSerialiser& stream, PageData* dataToPopulate)
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
		
		while(stream.peekNextChunkID() == CHUNK_CONTENTCOLLECTION_DECLARATION_ID)
		{
			const StreamSerialiser::Chunk* collChunk = stream.readChunkBegin();
			String factoryName;
			stream.read(&factoryName);
			stream.readChunkEnd(CHUNK_CONTENTCOLLECTION_DECLARATION_ID);
			// Supported type?
			PageContentCollectionFactory* collFact = mgr->getContentCollectionFactory(factoryName);
			if (collFact)
			{
				PageContentCollection* collInst = collFact->createInstance();
				if (collInst->prepare(stream)) // read type-specific data
				{
					dataToPopulate->collectionsToAdd.push_back(collInst);
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


		mModified = false;

		return true;
	}
	//---------------------------------------------------------------------
	void Page::load(bool synchronous)
	{
		if (!mDeferredProcessInProgress)
		{
			destroyAllContentCollections();
			PageRequest req(this);
			mDeferredProcessInProgress = true;
			Root::getSingleton().getWorkQueue()->addRequest(mWorkQueueChannel, WORKQUEUE_PREPARE_REQUEST, 
				Any(req), 0, synchronous);
		}

	}
	//---------------------------------------------------------------------
	void Page::unload()
	{
		destroyAllContentCollections();
	}
	//---------------------------------------------------------------------
	bool Page::canHandleRequest(const WorkQueue::Request* req, const WorkQueue* srcQ)
	{
		PageRequest preq = any_cast<PageRequest>(req->getData());
		// only deal with own requests
		// we do this because if we delete a page we want any pending tasks to be discarded
		if (preq.srcPage != this)
			return false;
		else
			return true;

	}
	//---------------------------------------------------------------------
	bool Page::canHandleResponse(const WorkQueue::Response* res, const WorkQueue* srcQ)
	{
		PageRequest preq = any_cast<PageRequest>(res->getRequest()->getData());
		// only deal with own requests
		// we do this because if we delete a page we want any pending tasks to be discarded
		if (preq.srcPage != this)
			return false;
		else
			return true;

	}
	//---------------------------------------------------------------------
	WorkQueue::Response* Page::handleRequest(const WorkQueue::Request* req, const WorkQueue* srcQ)
	{
		// Background thread (maybe)

		PageRequest preq = any_cast<PageRequest>(req->getData());
		// only deal with own requests; we shouldn't ever get here though
		if (preq.srcPage != this)
			return 0;

		PageResponse res;
		res.pageData = OGRE_NEW PageData();
		WorkQueue::Response* response = 0;
		try
		{
			prepareImpl(res.pageData);
			response = OGRE_NEW WorkQueue::Response(req, true, Any(res));
		}
		catch (Exception& e)
		{
			// oops
			response = OGRE_NEW WorkQueue::Response(req, false, Any(res), 
				e.getFullDescription());
		}

		return response;
	}
	//---------------------------------------------------------------------
	void Page::handleResponse(const WorkQueue::Response* res, const WorkQueue* srcQ)
	{
		// Main thread
		PageResponse pres = any_cast<PageResponse>(res->getData());
		PageRequest preq = any_cast<PageRequest>(res->getRequest()->getData());

		// only deal with own requests
		if (preq.srcPage!= this)
			return;

		// final loading behaviour
		if (res->succeeded())
		{
			std::swap(mContentCollections, pres.pageData->collectionsToAdd);
			loadImpl();
		}

		OGRE_DELETE pres.pageData;

		mDeferredProcessInProgress = false;

	}
	//---------------------------------------------------------------------
	bool Page::prepareImpl(PageData* dataToPopulate)
	{
		// Background loading
		String filename = generateFilename();

		DataStreamPtr stream;
		if (ResourceGroupManager::getSingleton().resourceExists(
			getManager()->getPageResourceGroup(), filename))
		{
			stream = ResourceGroupManager::getSingleton().openResource(
				filename, getManager()->getPageResourceGroup());
		}
		else
		{
			// try direct
			std::ifstream *ifs = OGRE_NEW_T(std::ifstream, MEMCATEGORY_GENERAL);
			ifs->open(filename.c_str(), std::ios::in | std::ios::binary);
			if(!*ifs)
			{
				OGRE_DELETE_T(ifs, basic_ifstream, MEMCATEGORY_GENERAL);
				OGRE_EXCEPT(
					Exception::ERR_FILE_NOT_FOUND, "'" + filename + "' file not found!", __FUNCTION__);
			}
			stream.bind(OGRE_NEW FileStreamDataStream(filename, ifs));
		}

		StreamSerialiser ser(stream);
		return prepareImpl(ser, dataToPopulate);


	}
	//---------------------------------------------------------------------
	void Page::loadImpl()
	{

	}
	//---------------------------------------------------------------------
	void Page::save()
	{
		String filename = generateFilename();
		save(filename);
	}
	//---------------------------------------------------------------------
	void Page::save(const String& filename)
	{
		// Does this file include path specifiers?
		String path, basename;
		StringUtil::splitFilename(filename, basename, path);

		// no path elements, try the resource system first
		DataStreamPtr stream;
		if (path.empty())
		{
			try
			{
				stream = ResourceGroupManager::getSingleton().createResource(
					filename, getManager()->getPageResourceGroup(), true);
			}
			catch (...) {}

		}

		if (stream.isNull())		
		{
			// save direct in filesystem
			std::fstream fs;
			fs.open(filename.c_str(), std::ios::out | std::ios::binary);
			if (!fs)
				OGRE_EXCEPT(Exception::ERR_CANNOT_WRITE_TO_FILE, 
				"Can't open " + filename + " for writing", __FUNCTION__);

			stream = DataStreamPtr(OGRE_NEW FileStreamDataStream(filename, &fs, false));
		}

		StreamSerialiser ser(stream);
		save(ser);
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
			// declaration
			stream.writeChunkBegin(CHUNK_CONTENTCOLLECTION_DECLARATION_ID);
			stream.write(&(*i)->getType());
			stream.writeChunkEnd(CHUNK_CONTENTCOLLECTION_DECLARATION_ID);
			// data
			(*i)->save(stream);
		}

		stream.writeChunkEnd(CHUNK_ID);

		mModified = false;
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
		coll->_notifyAttached(this);
		mContentCollections.push_back(coll);
		return coll;
	}
	//---------------------------------------------------------------------
	void Page::destroyContentCollection(PageContentCollection* coll)
	{
		ContentCollectionList::iterator i = std::find(
			mContentCollections.begin(), mContentCollections.end(), coll);
		if (i != mContentCollections.end())
		{
			mContentCollections.erase(i);
		}
		getManager()->destroyContentCollection(coll);
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
	//---------------------------------------------------------------------
	String Page::generateFilename() const
	{
		StringUtil::StrStreamType str;
		if (mParent)
			str << mParent->getWorld()->getName() << "_" <<	mParent->getName();

		str	<< std::setw(8) << std::setfill('0') << std::hex << mID << ".page";

		return str.str();

	}



}

