/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

 Copyright (c) 2000-2013 Torus Knot Software Ltd

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
