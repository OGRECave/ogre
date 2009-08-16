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
#ifndef __RenderQueueSortingGrouping_H__
#define __RenderQueueSortingGrouping_H__

// Precompiler options
#include "OgrePrerequisites.h"
#include "OgreIteratorWrappers.h"
#include "OgreMaterial.h"
#include "OgreTechnique.h"
#include "OgrePass.h"
#include "OgreRadixSort.h"

namespace Ogre {

	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup RenderSystem
	*  @{
	*/
	/** Struct associating a single Pass with a single Renderable. 
		This is used to for objects sorted by depth and thus not
		grouped by pass.
	*/
	struct RenderablePass
	{
		/// Pointer to the Renderable details
		Renderable* renderable;
		/// Pointer to the Pass
		Pass* pass;

		RenderablePass(Renderable* rend, Pass* p) :renderable(rend), pass(p) {}
	};


	/** Visitor interface for items in a QueuedRenderableCollection.
	@remarks
		Those wishing to iterate over the items in a 
		QueuedRenderableCollection should implement this visitor pattern,
		since internal organisation of the collection depends on the 
		sorting method in use.
	*/
	class _OgreExport QueuedRenderableVisitor
	{
	public:
		QueuedRenderableVisitor() {}
		virtual ~QueuedRenderableVisitor() {}
		
		/** Called when visiting a RenderablePass, i.e. items in a
			sorted collection where items are not grouped by pass.
		@remarks
			If this is called, neither of the other 2 visit methods
			will be called.
		*/
		virtual void visit(RenderablePass* rp) = 0;

		/* When visiting a collection grouped by pass, this is
			called when the grouping pass changes.
		@remarks
			If this method is called, the RenderablePass visit 
			method will not be called for this collection. The 
			Renderable visit method will be called for each item
			underneath the pass grouping level.
		@returns True to continue, false to skip the Renderables underneath
		*/
		virtual bool visit(const Pass* p) = 0;
		/** Visit method called once per Renderable on a grouped 
			collection.
		@remarks
			If this method is called, the RenderablePass visit 
			method will not be called for this collection. 
		*/
		virtual void visit(Renderable* r) = 0;
		
		
	};

	/** Lowest level collection of renderables.
	@remarks
		To iterate over items in this collection, you must call
		the accept method and supply a QueuedRenderableVisitor.
		The order of the iteration, and whether that iteration is
		over a RenderablePass list or a 2-level grouped list which 
		causes a visit call at the Pass level, and a call for each
		Renderable underneath.
	*/
	class _OgreExport QueuedRenderableCollection : public RenderQueueAlloc
	{
	public:
		/** Organisation modes required for this collection.
		@remarks
			This affects the internal placement of the items added to this collection;
			if only one type of sorting / grouping is to be required, then renderables
			can be stored only once, whilst if multiple types are going to be needed
			then internally there will be multiple organisations. Changing the organisation
			needs to be done when the collection is empty.
		*/		
		enum OrganisationMode
		{
			/// Group by pass
			OM_PASS_GROUP = 1,
			/// Sort descending camera distance
			OM_SORT_DESCENDING = 2,
			/** Sort ascending camera distance 
				Note value overlaps with descending since both use same sort
			*/
			OM_SORT_ASCENDING = 6
		};

	protected:
        /// Comparator to order pass groups
        struct PassGroupLess
        {
            bool _OgreExport operator()(const Pass* a, const Pass* b) const
            {
                // Sort by passHash, which is pass, then texture unit changes
                uint32 hasha = a->getHash();
                uint32 hashb = b->getHash();
                if (hasha == hashb)
                {
                    // Must differentTransparentQueueItemLessiate by pointer incase 2 passes end up with the same hash
                    return a < b;
                }
                else
                {
                    return hasha < hashb;
                }
            }
        };
        /// Comparator to order objects by descending camera distance
		struct DepthSortDescendingLess
        {
            const Camera* camera;

            DepthSortDescendingLess(const Camera* cam)
                : camera(cam)
            {
            }

            bool _OgreExport operator()(const RenderablePass& a, const RenderablePass& b) const
            {
                if (a.renderable == b.renderable)
                {
                    // Same renderable, sort by pass hash
                    return a.pass->getHash() < b.pass->getHash();
                }
                else
                {
                    // Different renderables, sort by depth
                    Real adepth = a.renderable->getSquaredViewDepth(camera);
                    Real bdepth = b.renderable->getSquaredViewDepth(camera);
					if (Math::RealEqual(adepth, bdepth))
				    {
                        // Must return deterministic result, doesn't matter what
                        return a.pass < b.pass;
				    }
				    else
				    {
				        // Sort DESCENDING by depth (i.e. far objects first)
					    return (adepth > bdepth);
				    }
                }

            }
        };

        /** Vector of RenderablePass objects, this is built on the assumption that
         vectors only ever increase in size, so even if we do clear() the memory stays
         allocated, ie fast */
        typedef vector<RenderablePass>::type RenderablePassList;
        typedef vector<Renderable*>::type RenderableList;
        /** Map of pass to renderable lists, this is a grouping by pass. */
        typedef map<Pass*, RenderableList*, PassGroupLess>::type PassGroupRenderableMap;

		/// Functor for accessing sort value 1 for radix sort (Pass)
		struct RadixSortFunctorPass
		{
			uint32 operator()(const RenderablePass& p) const
            {
                return p.pass->getHash();
            }
		};

        /// Radix sorter for accessing sort value 1 (Pass)
		static RadixSort<RenderablePassList, RenderablePass, uint32> msRadixSorter1;

		/// Functor for descending sort value 2 for radix sort (distance)
		struct RadixSortFunctorDistance
		{
			const Camera* camera;

            RadixSortFunctorDistance(const Camera* cam)
                : camera(cam)
            {
            }

			float operator()(const RenderablePass& p) const
            {
                // Sort DESCENDING by depth (ie far objects first), use negative distance
                // here because radix sorter always dealing with accessing sort
                return static_cast<float>(- p.renderable->getSquaredViewDepth(camera));
            }
		};

        /// Radix sorter for sort value 2 (distance)
		static RadixSort<RenderablePassList, RenderablePass, float> msRadixSorter2;

		/// Bitmask of the organisation modes requested
		uint8 mOrganisationMode;

		/// Grouped 
		PassGroupRenderableMap mGrouped;
		/// Sorted descending (can iterate backwards to get ascending)
		RenderablePassList mSortedDescending;

		/// Internal visitor implementation
		void acceptVisitorGrouped(QueuedRenderableVisitor* visitor) const;
		/// Internal visitor implementation
		void acceptVisitorDescending(QueuedRenderableVisitor* visitor) const;
		/// Internal visitor implementation
		void acceptVisitorAscending(QueuedRenderableVisitor* visitor) const;

	public:
		QueuedRenderableCollection();
		~QueuedRenderableCollection();

		/// Empty the collection
		void clear(void);

		/** Remove the group entry (if any) for a given Pass.
		@remarks
			To be used when a pass is destroyed, such that any
			grouping level for it becomes useless.
		*/	
		void removePassGroup(Pass* p);
		
		/** Reset the organisation modes required for this collection. 
		@remarks
			You can only do this when the collection is empty.
		@see OrganisationMode
		*/
		void resetOrganisationModes(void) 
		{ 
			mOrganisationMode = 0; 
		}
		
		/** Add a required sorting / grouping mode to this collection when next used.
		@remarks
			You can only do this when the collection is empty.
		@see OrganisationMode
		*/
		void addOrganisationMode(OrganisationMode om) 
		{ 
			mOrganisationMode |= om; 
		}

        /// Add a renderable to the collection using a given pass
        void addRenderable(Pass* pass, Renderable* rend);
		
		/** Perform any sorting that is required on this collection.
		@param cam The camera
		*/
		void sort(const Camera* cam);

		/** Accept a visitor over the collection contents.
		@param visitor Visitor class which should be called back
		@param om The organisation mode which you want to iterate over.
			Note that this must have been included in an addOrganisationMode
			call before any renderables were added.
		*/
		void acceptVisitor(QueuedRenderableVisitor* visitor, OrganisationMode om) const;

		/** Merge renderable collection. 
		*/
		void merge( const QueuedRenderableCollection& rhs );
	};

	/** Collection of renderables by priority.
    @remarks
        This class simply groups renderables for rendering. All the 
        renderables contained in this class are destined for the same
        RenderQueueGroup (coarse groupings like those between the main
        scene and overlays) and have the same priority (fine groupings
        for detailed overlap control).
    @par
        This class can order solid renderables by a number of criteria; 
		it can optimise them into groups based on pass to reduce render 
		state changes, or can sort them by ascending or descending view 
		depth. Transparent objects are always ordered by descending depth.
	@par
		To iterate over items in the collections held by this object 
	   	you should retrieve the collection in use (e.g. solids, solids with
		no shadows, transparents) and use the accept() method, providing 
		a class implementing QueuedRenderableVisitor.
	
    */
    class _OgreExport RenderPriorityGroup : public RenderQueueAlloc
    {
	protected:

		/// Parent queue group
        RenderQueueGroup* mParent;
        bool mSplitPassesByLightingType;
        bool mSplitNoShadowPasses;
		bool mShadowCastersNotReceivers;
        /// Solid pass list, used when no shadows, modulative shadows, or ambient passes for additive
		QueuedRenderableCollection mSolidsBasic;
        /// Solid per-light pass list, used with additive shadows
        QueuedRenderableCollection mSolidsDiffuseSpecular;
        /// Solid decal (texture) pass list, used with additive shadows
        QueuedRenderableCollection mSolidsDecal;
        /// Solid pass list, used when shadows are enabled but shadow receive is turned off for these passes
        QueuedRenderableCollection mSolidsNoShadowReceive;
		/// Unsorted transparent list
		QueuedRenderableCollection mTransparentsUnsorted;
		/// Transparent list
		QueuedRenderableCollection mTransparents;

        /// remove a pass entry from all collections
        void removePassEntry(Pass* p);

        /// Internal method for adding a solid renderable
        void addSolidRenderable(Technique* pTech, Renderable* rend, bool toNoShadowMap);
        /// Internal method for adding a solid renderable
        void addSolidRenderableSplitByLightType(Technique* pTech, Renderable* rend);
        /// Internal method for adding an unsorted transparent renderable
        void addUnsortedTransparentRenderable(Technique* pTech, Renderable* rend);
        /// Internal method for adding a transparent renderable
        void addTransparentRenderable(Technique* pTech, Renderable* rend);

    public:
        RenderPriorityGroup(RenderQueueGroup* parent, 
            bool splitPassesByLightingType,
            bool splitNoShadowPasses, 
			bool shadowCastersNotReceivers); 
           
        ~RenderPriorityGroup() { }

        /** Get the collection of basic solids currently queued, this includes
			all solids when there are no shadows, or all solids which have shadow
			receiving enabled when using modulative shadows, or all ambient passes
			of solids which have shadow receive enabled for additive shadows. */
        const QueuedRenderableCollection& getSolidsBasic(void) const
        { return mSolidsBasic; }
        /** Get the collection of solids currently queued per light (only applicable in 
		 	additive shadow modes). */
        const QueuedRenderableCollection& getSolidsDiffuseSpecular(void) const
        { return mSolidsDiffuseSpecular; }
        /** Get the collection of solids currently queued for decal passes (only 
			applicable in additive shadow modes). */
        const QueuedRenderableCollection& getSolidsDecal(void) const
        { return mSolidsDecal; }
        /** Get the collection of solids for which shadow receipt is disabled (only
			applicable when shadows are enabled). */
        const QueuedRenderableCollection& getSolidsNoShadowReceive(void) const
        { return mSolidsNoShadowReceive; }
        /** Get the collection of transparent objects currently queued */
        const QueuedRenderableCollection& getTransparentsUnsorted(void) const
        { return mTransparentsUnsorted; }
        /** Get the collection of transparent objects currently queued */
        const QueuedRenderableCollection& getTransparents(void) const
        { return mTransparents; }


		/** Reset the organisation modes required for the solids in this group. 
		@remarks
			You can only do this when the group is empty, i.e. after clearing the 
			queue.
		@see QueuedRenderableCollection::OrganisationMode
		*/
		void resetOrganisationModes(void);
		
		/** Add a required sorting / grouping mode for the solids in this group.
		@remarks
			You can only do this when the group is empty, i.e. after clearing the 
			queue.
		@see QueuedRenderableCollection::OrganisationMode
		*/
		void addOrganisationMode(QueuedRenderableCollection::OrganisationMode om); 

		/** Set the sorting / grouping mode for the solids in this group to the default.
		@remarks
			You can only do this when the group is empty, i.e. after clearing the 
			queue.
		@see QueuedRenderableCollection::OrganisationMode
		*/
		void defaultOrganisationMode(void); 

		/** Add a renderable to this group. */
        void addRenderable(Renderable* pRend, Technique* pTech);

		/** Sorts the objects which have been added to the queue; transparent objects by their 
            depth in relation to the passed in Camera. */
		void sort(const Camera* cam);

        /** Clears this group of renderables. 
        */
        void clear(void);

        /** Sets whether or not the queue will split passes by their lighting type,
        ie ambient, per-light and decal. 
        */
        void setSplitPassesByLightingType(bool split)
        {
            mSplitPassesByLightingType = split;
        }

        /** Sets whether or not passes which have shadow receive disabled should
            be separated. 
        */
        void setSplitNoShadowPasses(bool split)
        {
            mSplitNoShadowPasses = split;
        }

		/** Sets whether or not objects which cast shadows should be treated as
			never receiving shadows. 
		*/
		void setShadowCastersCannotBeReceivers(bool ind)
		{
			mShadowCastersNotReceivers = ind;
		}

		/** Merge group of renderables. 
		*/
		void merge( const RenderPriorityGroup* rhs );


    };


    /** A grouping level underneath RenderQueue which groups renderables
    to be issued at coarsely the same time to the renderer.
    @remarks
        Each instance of this class itself hold RenderPriorityGroup instances, 
        which are the groupings of renderables by priority for fine control
        of ordering (not required for most instances).
    */
    class _OgreExport RenderQueueGroup : public RenderQueueAlloc
    {
    public:
        typedef map<ushort, RenderPriorityGroup*, std::less<ushort> >::type PriorityMap;
        typedef MapIterator<PriorityMap> PriorityMapIterator;
        typedef ConstMapIterator<PriorityMap> ConstPriorityMapIterator;
    protected:
        RenderQueue* mParent;
        bool mSplitPassesByLightingType;
        bool mSplitNoShadowPasses;
		bool mShadowCastersNotReceivers;
        /// Map of RenderPriorityGroup objects
        PriorityMap mPriorityGroups;
		/// Whether shadows are enabled for this queue
		bool mShadowsEnabled;
		/// Bitmask of the organisation modes requested (for new priority groups)
		uint8 mOrganisationMode;


    public:
		RenderQueueGroup(RenderQueue* parent,
            bool splitPassesByLightingType,
            bool splitNoShadowPasses,
            bool shadowCastersNotReceivers) 
            : mParent(parent)
            , mSplitPassesByLightingType(splitPassesByLightingType)
            , mSplitNoShadowPasses(splitNoShadowPasses)
            , mShadowCastersNotReceivers(shadowCastersNotReceivers)
            , mShadowsEnabled(true)
			, mOrganisationMode(0)
        {
        }

        ~RenderQueueGroup() {
            // destroy contents now
            PriorityMap::iterator i;
            for (i = mPriorityGroups.begin(); i != mPriorityGroups.end(); ++i)
            {
                OGRE_DELETE i->second;
            }
        }

        /** Get an iterator for browsing through child contents. */
        PriorityMapIterator getIterator(void)
        {
            return PriorityMapIterator(mPriorityGroups.begin(), mPriorityGroups.end());
        }

        /** Get a const iterator for browsing through child contents. */
        ConstPriorityMapIterator getIterator(void) const
        {
            return ConstPriorityMapIterator(mPriorityGroups.begin(), mPriorityGroups.end());
        }

        /** Add a renderable to this group, with the given priority. */
        void addRenderable(Renderable* pRend, Technique* pTech, ushort priority)
        {
            // Check if priority group is there
            PriorityMap::iterator i = mPriorityGroups.find(priority);
            RenderPriorityGroup* pPriorityGrp;
            if (i == mPriorityGroups.end())
            {
                // Missing, create
                pPriorityGrp = OGRE_NEW RenderPriorityGroup(this, 
                    mSplitPassesByLightingType,
                    mSplitNoShadowPasses, 
					mShadowCastersNotReceivers);
				if (mOrganisationMode)
				{
					pPriorityGrp->resetOrganisationModes();
					pPriorityGrp->addOrganisationMode((QueuedRenderableCollection::OrganisationMode)mOrganisationMode);
				}

                mPriorityGroups.insert(PriorityMap::value_type(priority, pPriorityGrp));
            }
            else
            {
                pPriorityGrp = i->second;
            }

            // Add
            pPriorityGrp->addRenderable(pRend, pTech);

        }

        /** Clears this group of renderables. 
        @param destroy
            If false, doesn't delete any priority groups, just empties them. Saves on 
            memory deallocations since the chances are roughly the same kinds of 
            renderables are going to be sent to the queue again next time. If
			true, completely destroys.
        */
        void clear(bool destroy = false)
        {
            PriorityMap::iterator i, iend;
            iend = mPriorityGroups.end();
            for (i = mPriorityGroups.begin(); i != iend; ++i)
            {
				if (destroy)
					OGRE_DELETE i->second;
				else
					i->second->clear();
            }

			if (destroy)
				mPriorityGroups.clear();

        }

		/** Indicate whether a given queue group will be doing any
		shadow setup.
		@remarks
		This method allows you to inform the queue about a queue group, and to 
		indicate whether this group will require shadow processing of any sort.
		In order to preserve rendering order, OGRE has to treat queue groups
		as very separate elements of the scene, and this can result in it
		having to duplicate shadow setup for each group. Therefore, if you
		know that a group which you are using will never need shadows, you
		should preregister the group using this method in order to improve
		the performance.
		*/
		void setShadowsEnabled(bool enabled) { mShadowsEnabled = enabled; }

		/** Are shadows enabled for this queue? */
		bool getShadowsEnabled(void) const { return mShadowsEnabled; }

        /** Sets whether or not the queue will split passes by their lighting type,
        ie ambient, per-light and decal. 
        */
        void setSplitPassesByLightingType(bool split)
        {
            mSplitPassesByLightingType = split;
            PriorityMap::iterator i, iend;
            iend = mPriorityGroups.end();
            for (i = mPriorityGroups.begin(); i != iend; ++i)
            {
                i->second->setSplitPassesByLightingType(split);
            }
        }
        /** Sets whether or not the queue will split passes which have shadow receive
        turned off (in their parent material), which is needed when certain shadow
        techniques are used.
        */
        void setSplitNoShadowPasses(bool split)
        {
            mSplitNoShadowPasses = split;
            PriorityMap::iterator i, iend;
            iend = mPriorityGroups.end();
            for (i = mPriorityGroups.begin(); i != iend; ++i)
            {
                i->second->setSplitNoShadowPasses(split);
            }
        }
		/** Sets whether or not objects which cast shadows should be treated as
		never receiving shadows. 
		*/
		void setShadowCastersCannotBeReceivers(bool ind)
		{
			mShadowCastersNotReceivers = ind;
			PriorityMap::iterator i, iend;
			iend = mPriorityGroups.end();
			for (i = mPriorityGroups.begin(); i != iend; ++i)
			{
				i->second->setShadowCastersCannotBeReceivers(ind);
			}
		}
		/** Reset the organisation modes required for the solids in this group. 
		@remarks
			You can only do this when the group is empty, ie after clearing the 
			queue.
		@see QueuedRenderableCollection::OrganisationMode
		*/
		void resetOrganisationModes(void)
		{
			mOrganisationMode = 0;

			PriorityMap::iterator i, iend;
			iend = mPriorityGroups.end();
			for (i = mPriorityGroups.begin(); i != iend; ++i)
			{
				i->second->resetOrganisationModes();
			}
		}
		
		/** Add a required sorting / grouping mode for the solids in this group.
		@remarks
			You can only do this when the group is empty, ie after clearing the 
			queue.
		@see QueuedRenderableCollection::OrganisationMode
		*/
		void addOrganisationMode(QueuedRenderableCollection::OrganisationMode om)
		{
			mOrganisationMode |= om;

			PriorityMap::iterator i, iend;
			iend = mPriorityGroups.end();
			for (i = mPriorityGroups.begin(); i != iend; ++i)
			{
				i->second->addOrganisationMode(om);
			}
		}

		/** Setthe  sorting / grouping mode for the solids in this group to the default.
		@remarks
			You can only do this when the group is empty, ie after clearing the 
			queue.
		@see QueuedRenderableCollection::OrganisationMode
		*/
		void defaultOrganisationMode(void)
		{
			mOrganisationMode = 0;

			PriorityMap::iterator i, iend;
			iend = mPriorityGroups.end();
			for (i = mPriorityGroups.begin(); i != iend; ++i)
			{
				i->second->defaultOrganisationMode();
			}
		}

		/** Merge group of renderables. 
		*/
		void merge( const RenderQueueGroup* rhs )
		{
			ConstPriorityMapIterator it = rhs->getIterator();

			while( it.hasMoreElements() )
			{
				ushort priority = it.peekNextKey();
				RenderPriorityGroup* pSrcPriorityGrp = it.getNext();
				RenderPriorityGroup* pDstPriorityGrp;

				// Check if priority group is there
				PriorityMap::iterator i = mPriorityGroups.find(priority);
				if (i == mPriorityGroups.end())
				{
					// Missing, create
					pDstPriorityGrp = OGRE_NEW RenderPriorityGroup(this, 
						mSplitPassesByLightingType,
						mSplitNoShadowPasses, 
						mShadowCastersNotReceivers);
					if (mOrganisationMode)
					{
						pDstPriorityGrp->resetOrganisationModes();
						pDstPriorityGrp->addOrganisationMode((QueuedRenderableCollection::OrganisationMode)mOrganisationMode);
					}

					mPriorityGroups.insert(PriorityMap::value_type(priority, pDstPriorityGrp));
				}
				else
				{
					pDstPriorityGrp = i->second;
				}

				// merge
				pDstPriorityGrp->merge( pSrcPriorityGrp );
			}
		}
    };

	/** @} */
	/** @} */


}

#endif


