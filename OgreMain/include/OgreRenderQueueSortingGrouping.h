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
#ifndef __RenderQueueSortingGrouping_H__
#define __RenderQueueSortingGrouping_H__

// Precompiler options
#include "OgrePrerequisites.h"
#include "OgrePass.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup RenderSystem
    *  @{
    */

    typedef std::vector<Renderable*> RenderableList;

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
            If this is called, the other visit method
            will not be called.
        */
        virtual void visit(RenderablePass* rp) = 0;

        /** When visiting a collection grouped by pass, this is
            called.
        @remarks
            If this method is called, the RenderablePass visit 
            method will not be called for this collection.
        */
        virtual void visit(const Pass* p, RenderableList& rs) = 0;
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

    private:
        /// Comparator to order pass groups
        struct PassGroupLess
        {
            bool operator()(const Pass* a, const Pass* b) const
            {
                // Sort by passHash, which is pass, then texture unit changes
                uint32 hasha = a->getHash();
                uint32 hashb = b->getHash();
                if (hasha == hashb)
                {
                    // Must differentTransparentQueueItemLessiate by pointer in case 2 passes end up with the same hash
                    return a < b;
                }
                else
                {
                    return hasha < hashb;
                }
            }
        };
        /** Vector of RenderablePass objects, this is built on the assumption that
         vectors only ever increase in size, so even if we do clear() the memory stays
         allocated, ie fast */
        typedef std::vector<RenderablePass> RenderablePassList;
        /** Map of pass to renderable lists, this is a grouping by pass. */
        typedef std::map<Pass*, RenderableList, PassGroupLess> PassGroupRenderableMap;

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
            mOrganisationMode |= uint8(om);
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
    private:

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
        typedef std::map<ushort, RenderPriorityGroup*, std::less<ushort> > PriorityMap;
        typedef MapIterator<PriorityMap> PriorityMapIterator;
        typedef ConstMapIterator<PriorityMap> ConstPriorityMapIterator;
    private:
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
        RenderQueueGroup(bool splitPassesByLightingType,
            bool splitNoShadowPasses,
            bool shadowCastersNotReceivers) 
            : mSplitPassesByLightingType(splitPassesByLightingType)
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

        const PriorityMap& getPriorityGroups() const { return mPriorityGroups; }

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

                mPriorityGroups.emplace(priority, pPriorityGrp);
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
            mOrganisationMode |= uint8(om);

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
            for ( const auto& pg : rhs->getPriorityGroups() )
            {
                ushort priority = pg.first;
                RenderPriorityGroup* pSrcPriorityGrp = pg.second;
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

                    mPriorityGroups.emplace(priority, pDstPriorityGrp);
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

#include "OgreHeaderSuffix.h"

#endif


