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

namespace Ogre
{
	//---------------------------------------------------------------------
	Page::Page(PageID pageID)
		: mID(pageID)
		, mParent(0)
		, mStatus(STATUS_UNLOADED)
		, mDebugNode(0)
	{
		touch();
	}
	//---------------------------------------------------------------------
	Page::~Page()
	{
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
	bool Page::prepare(StreamSerialiser& stream)
	{
		// Fast pre-check
		if (mStatus.get() != STATUS_UNLOADED) 
			return true;

		// Set to loading
		if (!mStatus.cas(STATUS_UNLOADED, STATUS_PREPARING))
			return true;

		// Now do the real loading
		// TODO


		mStatus.set(STATUS_PREPARED);
		return true;
	}
	//---------------------------------------------------------------------
	void Page::load()
	{
		// Fast pre-check
		if (mStatus.get() != STATUS_PREPARED) 
			return;

		// Set to loading
		if (!mStatus.cas(STATUS_PREPARED, STATUS_LOADING))
			return;

		// Now do the real loading
		// TODO


		mStatus.set(STATUS_LOADED);
	}
	//---------------------------------------------------------------------
	void Page::unprepare()
	{
		// Fast pre-check
		if (mStatus.get() != STATUS_PREPARED) 
			return;

		if (!mStatus.cas(STATUS_PREPARED, STATUS_UNPREPARING))
			return;

		// TODO


		mStatus.set(STATUS_UNLOADED);
	}
	//---------------------------------------------------------------------
	void Page::unload()
	{
		// Fast pre-check
		if (mStatus.get() != STATUS_LOADED) 
			return;

		if (!mStatus.cas(STATUS_LOADED, STATUS_UNLOADING))
			return;

		// TODO


		mStatus.set(STATUS_PREPARED);

	}
	//---------------------------------------------------------------------
	void Page::_markPreparing()
	{
		Status s = mStatus.get();
		mStatus.cas(s, STATUS_PREPARING);
	}
	//---------------------------------------------------------------------
	void Page::_markPrepared()
	{
		Status s = mStatus.get();
		mStatus.cas(s, STATUS_PREPARED);
	}
	//---------------------------------------------------------------------
	void Page::_markLoading()
	{
		Status s = mStatus.get();
		mStatus.cas(s, STATUS_LOADING);
	}
	//---------------------------------------------------------------------
	void Page::_markLoaded()
	{
		Status s = mStatus.get();
		mStatus.cas(s, STATUS_LOADED);
	}
	//---------------------------------------------------------------------
	void Page::_markUnloading()
	{
		Status s = mStatus.get();
		mStatus.cas(s, STATUS_UNLOADING);
	}
	//---------------------------------------------------------------------
	void Page::_markUnpreparing()
	{
		Status s = mStatus.get();
		mStatus.cas(s, STATUS_UNPREPARING);
	}
	//---------------------------------------------------------------------
	void Page::_markUnloaded()
	{
		Status s = mStatus.get();
		mStatus.cas(s, STATUS_UNLOADED);
	}
	//---------------------------------------------------------------------
	void Page::save(StreamSerialiser& stream)
	{
		// TODO
	}
	//---------------------------------------------------------------------
	void Page::frameStart(Real timeSinceLastFrame)
	{
		updateDebugDisplay();


	}
	//---------------------------------------------------------------------
	void Page::frameEnd(Real timeElapsed)
	{

	}
	//---------------------------------------------------------------------
	void Page::notifyCamera(Camera* cam)
	{

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


}

