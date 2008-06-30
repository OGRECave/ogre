/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2005 The OGRE Team
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
#include "OgreStableHeaders.h"

#include "OgreRenderQueueInvocation.h"
#include "OgreSceneManager.h"
#include "OgreException.h"

namespace Ogre
{
	String RenderQueueInvocation::RENDER_QUEUE_INVOCATION_SHADOWS = "SHADOWS";
	//-----------------------------------------------------------------------------
	RenderQueueInvocation::RenderQueueInvocation(uint8 renderQueueGroupID, 
		const String& invocationName)
		: mRenderQueueGroupID(renderQueueGroupID), mInvocationName(invocationName), 
		mSolidsOrganisation(QueuedRenderableCollection::OM_PASS_GROUP), 
		mSuppressShadows(false), mSuppressRenderStateChanges(false)
	{

	}
	//-----------------------------------------------------------------------------
	RenderQueueInvocation::~RenderQueueInvocation()
	{

	}
	//-----------------------------------------------------------------------------
	void RenderQueueInvocation::invoke(RenderQueueGroup* group, SceneManager* targetSceneManager)
	{
		bool oldShadows = targetSceneManager->_areShadowsSuppressed();
		bool oldRSChanges = targetSceneManager->_areRenderStateChangesSuppressed();

		targetSceneManager->_suppressShadows(mSuppressShadows);
		targetSceneManager->_suppressRenderStateChanges(mSuppressRenderStateChanges);

		targetSceneManager->_renderQueueGroupObjects(group, mSolidsOrganisation);

		targetSceneManager->_suppressShadows(oldShadows);
		targetSceneManager->_suppressRenderStateChanges(oldRSChanges);

	}
	//-----------------------------------------------------------------------------
	//-----------------------------------------------------------------------------
	RenderQueueInvocationSequence::RenderQueueInvocationSequence(const String& name)
		:mName(name)
	{
	}
	//-----------------------------------------------------------------------------
	RenderQueueInvocationSequence::~RenderQueueInvocationSequence()
	{
		clear();
	}
	//-----------------------------------------------------------------------------
	RenderQueueInvocation* RenderQueueInvocationSequence::add(uint8 renderQueueGroupID, 
		const String& invocationName)
	{
		RenderQueueInvocation* ret = 
			OGRE_NEW RenderQueueInvocation(renderQueueGroupID, invocationName);

		mInvocations.push_back(ret);

		return ret;

	}
	//-----------------------------------------------------------------------------
	void RenderQueueInvocationSequence::add(RenderQueueInvocation* i)
	{
		mInvocations.push_back(i);
	}
	//-----------------------------------------------------------------------------
	void RenderQueueInvocationSequence::clear(void)
	{
		for (RenderQueueInvocationList::iterator i = mInvocations.begin();
			i != mInvocations.end(); ++i)
		{
			OGRE_DELETE *i;
		}
		mInvocations.clear();
	}
	//-----------------------------------------------------------------------------
	RenderQueueInvocation* RenderQueueInvocationSequence::get(size_t index)
	{
		if (index >= size())
			OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
				"Index out of bounds", 
				"RenderQueueInvocationSequence::get");

		return mInvocations[index];
	}
	//-----------------------------------------------------------------------------
	void RenderQueueInvocationSequence::remove(size_t index)
	{
		if (index >= size())
			OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
			"Index out of bounds", 
			"RenderQueueInvocationSequence::remove");
		
		RenderQueueInvocationList::iterator i = mInvocations.begin();
		std::advance(i, index);
		OGRE_DELETE *i;
		mInvocations.erase(i);
		
	}
	//-----------------------------------------------------------------------------
	RenderQueueInvocationIterator RenderQueueInvocationSequence::iterator(void)
	{
		return RenderQueueInvocationIterator(mInvocations.begin(), mInvocations.end());
	}
	//-----------------------------------------------------------------------------


}
