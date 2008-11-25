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
#include "OgreRenderQueueSortingGrouping.h"
#include "OgreException.h"

namespace Ogre {
    // Init statics
    RadixSort<QueuedRenderableCollection::RenderablePassList,
        RenderablePass, uint32> QueuedRenderableCollection::msRadixSorter1;
    RadixSort<QueuedRenderableCollection::RenderablePassList,
        RenderablePass, float> QueuedRenderableCollection::msRadixSorter2;


	//-----------------------------------------------------------------------
	RenderPriorityGroup::RenderPriorityGroup(RenderQueueGroup* parent, 
            bool splitPassesByLightingType,
            bool splitNoShadowPasses, 
			bool shadowCastersNotReceivers)
	 	: mParent(parent)
        , mSplitPassesByLightingType(splitPassesByLightingType)
        , mSplitNoShadowPasses(splitNoShadowPasses)
        , mShadowCastersNotReceivers(shadowCastersNotReceivers)
	{
		// Initialise collection sorting options
		// this can become dynamic according to invocation later
		defaultOrganisationMode();

		// Transparents will always be sorted this way
		mTransparents.addOrganisationMode(QueuedRenderableCollection::OM_SORT_DESCENDING);

		
	}
	//-----------------------------------------------------------------------
	void RenderPriorityGroup::resetOrganisationModes(void)
	{
		mSolidsBasic.resetOrganisationModes();
		mSolidsDiffuseSpecular.resetOrganisationModes();
		mSolidsDecal.resetOrganisationModes();
		mSolidsNoShadowReceive.resetOrganisationModes();
		mTransparentsUnsorted.resetOrganisationModes();
	}
	//-----------------------------------------------------------------------
	void RenderPriorityGroup::addOrganisationMode(QueuedRenderableCollection::OrganisationMode om)
	{
		mSolidsBasic.addOrganisationMode(om);
		mSolidsDiffuseSpecular.addOrganisationMode(om);
		mSolidsDecal.addOrganisationMode(om);
		mSolidsNoShadowReceive.addOrganisationMode(om);
		mTransparentsUnsorted.addOrganisationMode(om);
	}
	//-----------------------------------------------------------------------
	void RenderPriorityGroup::defaultOrganisationMode(void)
	{
		resetOrganisationModes();
		addOrganisationMode(QueuedRenderableCollection::OM_PASS_GROUP);
	}
	//-----------------------------------------------------------------------
    void RenderPriorityGroup::addRenderable(Renderable* rend, Technique* pTech)
    {
        // Transparent and depth/colour settings mean depth sorting is required?
        // Note: colour write disabled with depth check/write enabled means
        //       setup depth buffer for other passes use.
        if (pTech->isTransparent() && 
            (!pTech->isDepthWriteEnabled() ||
             !pTech->isDepthCheckEnabled() ||
             pTech->hasColourWriteDisabled()))
        {
			if (pTech->isTransparentSortingEnabled())
				addTransparentRenderable(pTech, rend);
			else
				addUnsortedTransparentRenderable(pTech, rend);
        }
        else
        {
            if (mSplitNoShadowPasses &&
                mParent->getShadowsEnabled() &&
				(!pTech->getParent()->getReceiveShadows() ||
				rend->getCastsShadows() && mShadowCastersNotReceivers))
            {
                // Add solid renderable and add passes to no-shadow group
                addSolidRenderable(pTech, rend, true);
            }
            else
            {
                if (mSplitPassesByLightingType && mParent->getShadowsEnabled())
                {
                    addSolidRenderableSplitByLightType(pTech, rend);
                }
                else
                {
                    addSolidRenderable(pTech, rend, false);
                }
            }
        }

    }
    //-----------------------------------------------------------------------
    void RenderPriorityGroup::addSolidRenderable(Technique* pTech, 
        Renderable* rend, bool addToNoShadow)
    {
        Technique::PassIterator pi = pTech->getPassIterator();

		QueuedRenderableCollection* collection;
        if (addToNoShadow)
        {
            collection = &mSolidsNoShadowReceive;
        }
        else
        {
            collection = &mSolidsBasic;
        }


        while (pi.hasMoreElements())
        {
            // Insert into solid list
            Pass* p = pi.getNext();
			collection->addRenderable(p, rend);
        }
    }
    //-----------------------------------------------------------------------
    void RenderPriorityGroup::addSolidRenderableSplitByLightType(Technique* pTech,
        Renderable* rend)
    {
        // Divide the passes into the 3 categories
        Technique::IlluminationPassIterator pi = 
            pTech->getIlluminationPassIterator();

        while (pi.hasMoreElements())
        {
            // Insert into solid list
            IlluminationPass* p = pi.getNext();
            QueuedRenderableCollection* collection;
            switch(p->stage)
            {
            case IS_AMBIENT:
                collection = &mSolidsBasic;
                break;
            case IS_PER_LIGHT:
                collection = &mSolidsDiffuseSpecular;
                break;
            case IS_DECAL:
                collection = &mSolidsDecal;
                break;
            default:
                assert(false); // should never happen
            };

			collection->addRenderable(p->pass, rend);
        }
    }
    //-----------------------------------------------------------------------
    void RenderPriorityGroup::addUnsortedTransparentRenderable(Technique* pTech, Renderable* rend)
    {
        Technique::PassIterator pi = pTech->getPassIterator();

        while (pi.hasMoreElements())
        {
            // Insert into transparent list
            mTransparentsUnsorted.addRenderable(pi.getNext(), rend);
        }
    }
    //-----------------------------------------------------------------------
    void RenderPriorityGroup::addTransparentRenderable(Technique* pTech, Renderable* rend)
    {
        Technique::PassIterator pi = pTech->getPassIterator();

        while (pi.hasMoreElements())
        {
            // Insert into transparent list
            mTransparents.addRenderable(pi.getNext(), rend);
        }
    }
    //-----------------------------------------------------------------------
	void RenderPriorityGroup::removePassEntry(Pass* p)
	{
		mSolidsBasic.removePassGroup(p);
		mSolidsDiffuseSpecular.removePassGroup(p);
		mSolidsNoShadowReceive.removePassGroup(p);
		mSolidsDecal.removePassGroup(p);
		mTransparentsUnsorted.removePassGroup(p);
		mTransparents.removePassGroup(p); // shouldn't be any, but for completeness
	}	
    //-----------------------------------------------------------------------
    void RenderPriorityGroup::clear(void)
    {
        // Delete queue groups which are using passes which are to be
        // deleted, we won't need these any more and they clutter up 
        // the list and can cause problems with future clones
		{
			// Hmm, a bit hacky but least obtrusive for now
			OGRE_LOCK_MUTEX(Pass::msPassGraveyardMutex)
			const Pass::PassSet& graveyardList = Pass::getPassGraveyard();
			Pass::PassSet::const_iterator gi, giend;
			giend = graveyardList.end();
			for (gi = graveyardList.begin(); gi != giend; ++gi)
			{
				removePassEntry(*gi);
			}
		}

        // Now remove any dirty passes, these will have their hashes recalculated
        // by the parent queue after all groups have been processed
        // If we don't do this, the std::map will become inconsistent for new insterts
		{
			// Hmm, a bit hacky but least obtrusive for now
			OGRE_LOCK_MUTEX(Pass::msDirtyHashListMutex)
			const Pass::PassSet& dirtyList = Pass::getDirtyHashList();
			Pass::PassSet::const_iterator di, diend;
			diend = dirtyList.end();
			for (di = dirtyList.begin(); di != diend; ++di)
			{
				removePassEntry(*di);
			}
		}
        // NB we do NOT clear the graveyard or the dirty list here, because 
        // it needs to be acted on for all groups, the parent queue takes 
        // care of this afterwards

		// Now empty the remaining collections
		// Note that groups don't get deleted, just emptied hence the difference
		// between the pass groups which are removed above, and clearing done
		// here
		mSolidsBasic.clear();
        mSolidsDecal.clear();
        mSolidsDiffuseSpecular.clear();
        mSolidsNoShadowReceive.clear();
		mTransparentsUnsorted.clear();
        mTransparents.clear();

    }
	//-----------------------------------------------------------------------
	void RenderPriorityGroup::sort(const Camera* cam)
	{
		mSolidsBasic.sort(cam);
		mSolidsDecal.sort(cam);
		mSolidsDiffuseSpecular.sort(cam);
		mSolidsNoShadowReceive.sort(cam);
		mTransparentsUnsorted.sort(cam);
		mTransparents.sort(cam);
	}
    //-----------------------------------------------------------------------
	QueuedRenderableCollection::QueuedRenderableCollection(void)
		:mOrganisationMode(0)
	{
	}
    //-----------------------------------------------------------------------
	QueuedRenderableCollection::~QueuedRenderableCollection(void)
	{
        // destroy all the pass map entries (rather than clearing)
        PassGroupRenderableMap::iterator i, iend;
        iend = mGrouped.end();
        for (i = mGrouped.begin(); i != iend; ++i)
        {
            // Free the list associated with this pass
            OGRE_DELETE_T(i->second, RenderableList, MEMCATEGORY_SCENE_CONTROL);
        }
		
	}
    //-----------------------------------------------------------------------
	void QueuedRenderableCollection::clear(void)
	{
        PassGroupRenderableMap::iterator i, iend;
        iend = mGrouped.end();
        for (i = mGrouped.begin(); i != iend; ++i)
        {
            // Clear the list associated with this pass, but leave the pass entry
            i->second->clear();
        }

		// Clear sorted list
		mSortedDescending.clear();
	}
    //-----------------------------------------------------------------------
	void QueuedRenderableCollection::removePassGroup(Pass* p)
	{
        PassGroupRenderableMap::iterator i;

        i = mGrouped.find(p);
        if (i != mGrouped.end())
        {
            // free memory
            OGRE_DELETE_T(i->second, RenderableList, MEMCATEGORY_SCENE_CONTROL);
            // erase from map
            mGrouped.erase(i);
        }
	}
    //-----------------------------------------------------------------------
	void QueuedRenderableCollection::sort(const Camera* cam)
    {
		// ascending and descending sort both set bit 1
		// We always sort descending, becuase the only difference is in the
		// acceptVisitor method, where we iterate in reverse in ascending mode
		if (mOrganisationMode & OM_SORT_DESCENDING)
		{
			
			// We can either use a stable_sort and the 'less' implementation,
			// or a 2-pass radix sort (once by pass, then by distance, since
			// radix sorting is inherently stable this will work)
			// We use stable_sort if the number of items is 512 or less, since
			// the complexity of the radix sort is approximately O(10N), since 
			// each sort is O(5N) (1 pass histograms, 4 passes sort)
			// Since stable_sort has a worst-case performance of O(N(logN)^2)
			// the performance tipping point is from about 1500 items, but in
			// stable_sorts best-case scenario O(NlogN) it would be much higher.
			// Take a stab at 2000 items.
			
			if (mSortedDescending.size() > 2000)
			{
				// sort by pass
				msRadixSorter1.sort(mSortedDescending, RadixSortFunctorPass());
				// sort by depth
				msRadixSorter2.sort(mSortedDescending, RadixSortFunctorDistance(cam));
			}
			else
			{
				std::stable_sort(
					mSortedDescending.begin(), mSortedDescending.end(), 
					DepthSortDescendingLess(cam));
			}
		}

		// Nothing needs to be done for pass groups, they auto-organise

    }
    //-----------------------------------------------------------------------
    void QueuedRenderableCollection::addRenderable(Pass* pass, Renderable* rend)
	{
		// ascending and descending sort both set bit 1
		if (mOrganisationMode & OM_SORT_DESCENDING)
		{
			mSortedDescending.push_back(RenderablePass(rend, pass));
		}

		if (mOrganisationMode & OM_PASS_GROUP)
		{
            PassGroupRenderableMap::iterator i = mGrouped.find(pass);
            if (i == mGrouped.end())
            {
                std::pair<PassGroupRenderableMap::iterator, bool> retPair;
                // Create new pass entry, build a new list
                // Note that this pass and list are never destroyed until the 
				// engine shuts down, or a pass is destroyed or has it's hash
				// recalculated, although the lists will be cleared
                retPair = mGrouped.insert(
                    PassGroupRenderableMap::value_type(
						pass, OGRE_NEW_T(RenderableList, MEMCATEGORY_SCENE_CONTROL)() ));
                assert(retPair.second && 
					"Error inserting new pass entry into PassGroupRenderableMap");
                i = retPair.first;
            }
            // Insert renderable
            i->second->push_back(rend);
			
		}
		
	}
    //-----------------------------------------------------------------------
	void QueuedRenderableCollection::acceptVisitor(
		QueuedRenderableVisitor* visitor, OrganisationMode om) const
	{
		if ((om & mOrganisationMode) == 0)
		{
			// try to fall back
			if (OM_PASS_GROUP & mOrganisationMode)
				om = OM_PASS_GROUP;
			else if (OM_SORT_ASCENDING & mOrganisationMode)
				om = OM_SORT_ASCENDING;
			else if (OM_SORT_DESCENDING & mOrganisationMode)
				om = OM_SORT_DESCENDING;
			else
				OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
					"Organisation mode requested in acceptVistor was not notified "
					"to this class ahead of time, therefore may not be supported.", 
					"QueuedRenderableCollection::acceptVisitor");
		}

		switch(om)
		{
		case OM_PASS_GROUP:
			acceptVisitorGrouped(visitor);
			break;
		case OM_SORT_DESCENDING:
			acceptVisitorDescending(visitor);
			break;
		case OM_SORT_ASCENDING:
			acceptVisitorAscending(visitor);
			break;
		}
		
	}
    //-----------------------------------------------------------------------
	void QueuedRenderableCollection::acceptVisitorGrouped(
		QueuedRenderableVisitor* visitor) const
	{
		PassGroupRenderableMap::const_iterator ipass, ipassend;
		ipassend = mGrouped.end();
		for (ipass = mGrouped.begin(); ipass != ipassend; ++ipass)
		{
			// Fast bypass if this group is now empty
			if (ipass->second->empty()) continue;

			// Visit Pass - allow skip
			if (!visitor->visit(ipass->first))
				continue;

			RenderableList* rendList = ipass->second;
			RenderableList::const_iterator irend, irendend;
			irendend = rendList->end();
			for (irend = rendList->begin(); irend != irendend; ++irend)
			{
				// Visit Renderable
				visitor->visit(const_cast<Renderable*>(*irend));
			}
		} 

	}
    //-----------------------------------------------------------------------
	void QueuedRenderableCollection::acceptVisitorDescending(
		QueuedRenderableVisitor* visitor) const
	{
		// List is already in descending order, so iterate forward
		RenderablePassList::const_iterator i, iend;

		iend = mSortedDescending.end();
		for (i = mSortedDescending.begin(); i != iend; ++i)
		{
			visitor->visit(const_cast<RenderablePass*>(&(*i)));
		}
	}
    //-----------------------------------------------------------------------
	void QueuedRenderableCollection::acceptVisitorAscending(
		QueuedRenderableVisitor* visitor) const
	{
		// List is in descending order, so iterate in reverse
		RenderablePassList::const_reverse_iterator i, iend;

		iend = mSortedDescending.rend();
		for (i = mSortedDescending.rbegin(); i != iend; ++i)
		{
			visitor->visit(const_cast<RenderablePass*>(&(*i)));
		}

	}


}

