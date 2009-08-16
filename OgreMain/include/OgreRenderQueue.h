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
#ifndef __RenderQueue_H__
#define __RenderQueue_H__

#include "OgrePrerequisites.h"
#include "OgreIteratorWrappers.h"

namespace Ogre {

	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup RenderSystem
	*  @{
	*/
	/** Enumeration of queue groups, by which the application may group queued renderables
        so that they are rendered together with events in between
	@remarks
		When passed into methods these are actually passed as a uint8 to allow you
		to use values in between if you want to.
    */
    enum RenderQueueGroupID
    {
        /// Use this queue for objects which must be rendered first e.g. backgrounds
        RENDER_QUEUE_BACKGROUND = 0,
        /// First queue (after backgrounds), used for skyboxes if rendered first
        RENDER_QUEUE_SKIES_EARLY = 5,
        RENDER_QUEUE_1 = 10,
        RENDER_QUEUE_2 = 20,
		RENDER_QUEUE_WORLD_GEOMETRY_1 = 25,
        RENDER_QUEUE_3 = 30,
        RENDER_QUEUE_4 = 40,
		/// The default render queue
        RENDER_QUEUE_MAIN = 50,
        RENDER_QUEUE_6 = 60,
        RENDER_QUEUE_7 = 70,
		RENDER_QUEUE_WORLD_GEOMETRY_2 = 75,
        RENDER_QUEUE_8 = 80,
        RENDER_QUEUE_9 = 90,
        /// Penultimate queue(before overlays), used for skyboxes if rendered last
        RENDER_QUEUE_SKIES_LATE = 95,
        /// Use this queue for objects which must be rendered last e.g. overlays
        RENDER_QUEUE_OVERLAY = 100, 
		/// Final possible render queue, don't exceed this
		RENDER_QUEUE_MAX = 105
    };

    #define OGRE_RENDERABLE_DEFAULT_PRIORITY  100

    /** Class to manage the scene object rendering queue.
        @remarks
            Objects are grouped by material to minimise rendering state changes. The map from
            material to renderable object is wrapped in a class for ease of use.
        @par
            This class now includes the concept of 'queue groups' which allows the application
            adding the renderable to specifically schedule it so that it is included in 
            a discrete group. Good for separating renderables into the main scene,
            backgrounds and overlays, and also could be used in the future for more
            complex multipass routines like stenciling.
    */
    class _OgreExport RenderQueue : public RenderQueueAlloc
    {
    public:
        typedef map< uint8, RenderQueueGroup* >::type RenderQueueGroupMap;
        /// Iterator over queue groups
        typedef MapIterator<RenderQueueGroupMap> QueueGroupIterator;
        typedef ConstMapIterator<RenderQueueGroupMap> ConstQueueGroupIterator;
		/** Class to listen in on items being added to the render queue. 
		@remarks
			Use RenderQueue::setRenderableListener to get callbacks when an item
			is added to the render queue.
		*/
		class _OgreExport RenderableListener
		{
		public:
			RenderableListener() {}
			virtual ~RenderableListener() {}

			/** Method called when a Renderable is added to the queue.
			@remarks
				You can use this event hook to alter the Technique used to
				render a Renderable as the item is added to the queue. This is
				a low-level way to override the material settings for a given
				Renderable on the fly.
			@param rend The Renderable being added to the queue
			@param groupID The render queue group this Renderable is being added to
			@param priority The priority the Renderable has been given
			@param ppTech A pointer to the pointer to the Technique that is 
				intended to be used; you can alter this to an alternate Technique
				if you so wish (the Technique doesn't have to be from the same
				Material either).
			@param pQueue Pointer to the render queue that this object is being
				added to. You can for example call this back to duplicate the 
				object with a different technique
			@returns true to allow the Renderable to be added to the queue, 
				false if you want to prevent it being added
			*/
			virtual bool renderableQueued(Renderable* rend, uint8 groupID, 
				ushort priority, Technique** ppTech, RenderQueue* pQueue) = 0;
		};
    protected:
        RenderQueueGroupMap mGroups;
        /// The current default queue group
        uint8 mDefaultQueueGroup;
        /// The default priority
        ushort mDefaultRenderablePriority;

        bool mSplitPassesByLightingType;
        bool mSplitNoShadowPasses;
		bool mShadowCastersCannotBeReceivers;

		RenderableListener* mRenderableListener;
    public:
        RenderQueue();
        virtual ~RenderQueue();

        /** Empty the queue - should only be called by SceneManagers.
		@param destroyPassMaps Set to true to destroy all pass maps so that
			the queue is completely clean (useful when switching scene managers)
        */
        void clear(bool destroyPassMaps = false);

		/** Get a render queue group.
		@remarks
			OGRE registers new queue groups as they are requested, 
			therefore this method will always return a valid group.
		*/
		RenderQueueGroup* getQueueGroup(uint8 qid);

        /** Add a renderable object to the queue.
        @remarks
            This methods adds a Renderable to the queue, which will be rendered later by 
            the SceneManager. This is the advanced version of the call which allows the renderable
            to be added to any queue.
        @note
            Called by implementation of MovableObject::_updateRenderQueue.
        @param
            pRend Pointer to the Renderable to be added to the queue
        @param
            groupID The group the renderable is to be added to. This
            can be used to schedule renderable objects in separate groups such that the SceneManager
            respects the divisions between the groupings and does not reorder them outside these
            boundaries. This can be handy for overlays where no matter what you want the overlay to 
            be rendered last.
        @param
            priority Controls the priority of the renderable within the queue group. If this number
            is raised, the renderable will be rendered later in the group compared to it's peers.
            Don't use this unless you really need to, manually ordering renderables prevents OGRE
            from sorting them for best efficiency. However this could be useful for ordering 2D
            elements manually for example.
        */
        void addRenderable(Renderable* pRend, uint8 groupID, ushort priority);

        /** Add a renderable object to the queue.
        @remarks
            This methods adds a Renderable to the queue, which will be rendered later by 
            the SceneManager. This is the simplified version of the call which does not 
            require a priority to be specified. The queue priority is take from the
            current default (see setDefaultRenderablePriority).
        @note
            Called by implementation of MovableObject::_updateRenderQueue.
        @param
            pRend Pointer to the Renderable to be added to the queue
		@param
            groupID The group the renderable is to be added to. This
            can be used to schedule renderable objects in separate groups such that the SceneManager
            respects the divisions between the groupings and does not reorder them outside these
            boundaries. This can be handy for overlays where no matter what you want the overlay to 
            be rendered last.
        */
        void addRenderable(Renderable* pRend, uint8 groupId);

        /** Add a renderable object to the queue.
        @remarks
            This methods adds a Renderable to the queue, which will be rendered later by 
            the SceneManager. This is the simplified version of the call which does not 
            require a queue or priority to be specified. The queue group is taken from the
            current default (see setDefaultQueueGroup).  The queue priority is take from the
            current default (see setDefaultRenderablePriority).
        @note
            Called by implementation of MovableObject::_updateRenderQueue.
        @param
            pRend Pointer to the Renderable to be added to the queue
        */
        void addRenderable(Renderable* pRend);
        
        /** Gets the current default queue group, which will be used for all renderable which do not
            specify which group they wish to be on.
        */
        uint8 getDefaultQueueGroup(void) const;

        /** Sets the current default renderable priority, 
            which will be used for all renderables which do not
            specify which priority they wish to use.
        */
        void setDefaultRenderablePriority(ushort priority);

        /** Gets the current default renderable priority, which will be used for all renderables which do not
            specify which priority they wish to use.
        */
        ushort getDefaultRenderablePriority(void) const;

        /** Sets the current default queue group, which will be used for all renderable which do not
            specify which group they wish to be on.
        */
        void setDefaultQueueGroup(uint8 grp);
        
        /** Internal method, returns an iterator for the queue groups. */
        QueueGroupIterator _getQueueGroupIterator(void);
        ConstQueueGroupIterator _getQueueGroupIterator(void) const;

        /** Sets whether or not the queue will split passes by their lighting type,
            ie ambient, per-light and decal. 
        */
        void setSplitPassesByLightingType(bool split);

        /** Gets whether or not the queue will split passes by their lighting type,
            ie ambient, per-light and decal. 
        */
        bool getSplitPassesByLightingType(void) const;

        /** Sets whether or not the queue will split passes which have shadow receive
        turned off (in their parent material), which is needed when certain shadow
        techniques are used.
        */
        void setSplitNoShadowPasses(bool split);

        /** Gets whether or not the queue will split passes which have shadow receive
        turned off (in their parent material), which is needed when certain shadow
        techniques are used.
        */
        bool getSplitNoShadowPasses(void) const;

		/** Sets whether or not objects which cast shadows should be treated as
		never receiving shadows. 
		*/
		void setShadowCastersCannotBeReceivers(bool ind);

		/** Gets whether or not objects which cast shadows should be treated as
		never receiving shadows. 
		*/
		bool getShadowCastersCannotBeReceivers(void) const;

		/** Set a renderable listener on the queue.
		@remarks
			There can only be a single renderable listener on the queue, since
			that listener has complete control over the techniques in use.
		*/
		void setRenderableListener(RenderableListener* listener)
		{ mRenderableListener = listener; }

		RenderableListener* getRenderableListener(void) const
		{ return mRenderableListener; }

		/** Merge render queue.
		*/
		void merge( const RenderQueue* rhs );
    };

	/** @} */
	/** @} */

}


#endif
