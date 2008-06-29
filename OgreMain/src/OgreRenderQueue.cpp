/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
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

#include "OgreRenderQueue.h"

#include "OgreRenderable.h"
#include "OgreMaterial.h"
#include "OgreRenderQueueSortingGrouping.h"
#include "OgrePass.h"
#include "OgreMaterialManager.h"


namespace Ogre {

    //---------------------------------------------------------------------
    RenderQueue::RenderQueue()
        : mSplitPassesByLightingType(false)
		, mSplitNoShadowPasses(false)
        , mShadowCastersCannotBeReceivers(false)
		, mRenderableListener(0)
    {
        // Create the 'main' queue up-front since we'll always need that
        mGroups.insert(
            RenderQueueGroupMap::value_type(
                RENDER_QUEUE_MAIN, 
                OGRE_NEW RenderQueueGroup(this,
                    mSplitPassesByLightingType,
                    mSplitNoShadowPasses,
                    mShadowCastersCannotBeReceivers)
                )
            );

        // set default queue
        mDefaultQueueGroup = RENDER_QUEUE_MAIN;
		mDefaultRenderablePriority = OGRE_RENDERABLE_DEFAULT_PRIORITY;

    }
    //---------------------------------------------------------------------
    RenderQueue::~RenderQueue()
    {
        
        // trigger the pending pass updates, otherwise we could leak
        Pass::processPendingPassUpdates();
        
        // Destroy the queues for good
        RenderQueueGroupMap::iterator i, iend;
        i = mGroups.begin();
        iend = mGroups.end();
        for (; i != iend; ++i)
        {
            OGRE_DELETE i->second;
        }
        mGroups.clear();




    }
    //-----------------------------------------------------------------------
    void RenderQueue::addRenderable(Renderable* pRend, uint8 groupID, ushort priority)
    {
        // Find group
        RenderQueueGroup* pGroup = getQueueGroup(groupID);


		Technique* pTech;

		// tell material it's been used
		if (!pRend->getMaterial().isNull())
			pRend->getMaterial()->touch();

		// Check material & technique supplied (the former since the default implementation
        // of getTechnique is based on it for backwards compatibility
        if(pRend->getMaterial().isNull() || !(pTech = pRend->getTechnique()))
        {
            // Use default base white
			MaterialPtr baseWhite = MaterialManager::getSingleton().getByName("BaseWhite");
            pTech = baseWhite->getTechnique(0);
        }

		if (mRenderableListener)
		{
			// Allow listener to override technique and to abort
			if (!mRenderableListener->renderableQueued(pRend, groupID, priority, 
				&pTech, this))
				return; // rejected

			// tell material it's been used (incase changed)
			pTech->getParent()->touch();
		}
		
        pGroup->addRenderable(pRend, pTech, priority);

    }
    //-----------------------------------------------------------------------
    void RenderQueue::clear(bool destroyPassMaps)
    {
        // Clear the queues
        RenderQueueGroupMap::iterator i, iend;
        i = mGroups.begin();
        iend = mGroups.end();
        for (; i != iend; ++i)
        {
            i->second->clear(destroyPassMaps);
        }

        // Now trigger the pending pass updates
        Pass::processPendingPassUpdates();

        // NB this leaves the items present (but empty)
        // We're assuming that frame-by-frame, the same groups are likely to 
        //  be used, so no point destroying the vectors and incurring the overhead
        //  that would cause, let them be destroyed in the destructor.
    }
    //-----------------------------------------------------------------------
    RenderQueue::QueueGroupIterator RenderQueue::_getQueueGroupIterator(void)
    {
        return QueueGroupIterator(mGroups.begin(), mGroups.end());
    }
    //-----------------------------------------------------------------------
    void RenderQueue::addRenderable(Renderable* pRend, uint8 groupID)
    {
        addRenderable(pRend, groupID, mDefaultRenderablePriority);
    }
	//-----------------------------------------------------------------------
    void RenderQueue::addRenderable(Renderable* pRend)
    {
        addRenderable(pRend, mDefaultQueueGroup, mDefaultRenderablePriority);
    }
    //-----------------------------------------------------------------------
    uint8 RenderQueue::getDefaultQueueGroup(void) const
    {
        return mDefaultQueueGroup;
    }
    //-----------------------------------------------------------------------
    void RenderQueue::setDefaultQueueGroup(uint8 grp)
    {
        mDefaultQueueGroup = grp;
    }
    //-----------------------------------------------------------------------
    ushort RenderQueue::getDefaultRenderablePriority(void) const
    {
        return mDefaultRenderablePriority;
    }
    //-----------------------------------------------------------------------
    void RenderQueue::setDefaultRenderablePriority(ushort priority)
    {
        mDefaultRenderablePriority = priority;
    }
	
	
	//-----------------------------------------------------------------------
	RenderQueueGroup* RenderQueue::getQueueGroup(uint8 groupID)
	{
		// Find group
		RenderQueueGroupMap::iterator groupIt;
		RenderQueueGroup* pGroup;

		groupIt = mGroups.find(groupID);
		if (groupIt == mGroups.end())
		{
			// Insert new
			pGroup = OGRE_NEW RenderQueueGroup(this,
                mSplitPassesByLightingType,
                mSplitNoShadowPasses,
                mShadowCastersCannotBeReceivers);
			mGroups.insert(RenderQueueGroupMap::value_type(groupID, pGroup));
		}
		else
		{
			pGroup = groupIt->second;
		}

		return pGroup;

	}
    //-----------------------------------------------------------------------
    void RenderQueue::setSplitPassesByLightingType(bool split)
    {
        mSplitPassesByLightingType = split;

        RenderQueueGroupMap::iterator i, iend;
        i = mGroups.begin();
        iend = mGroups.end();
        for (; i != iend; ++i)
        {
            i->second->setSplitPassesByLightingType(split);
        }
    }
    //-----------------------------------------------------------------------
    void RenderQueue::setSplitNoShadowPasses(bool split)
    {
        mSplitNoShadowPasses = split;

        RenderQueueGroupMap::iterator i, iend;
        i = mGroups.begin();
        iend = mGroups.end();
        for (; i != iend; ++i)
        {
            i->second->setSplitNoShadowPasses(split);
        }
    }
	//-----------------------------------------------------------------------
	void RenderQueue::setShadowCastersCannotBeReceivers(bool ind)
	{
		mShadowCastersCannotBeReceivers = ind;

		RenderQueueGroupMap::iterator i, iend;
		i = mGroups.begin();
		iend = mGroups.end();
		for (; i != iend; ++i)
		{
			i->second->setShadowCastersCannotBeReceivers(ind);
		}
	}

}

