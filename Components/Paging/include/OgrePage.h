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

#ifndef __Ogre_Page_H__
#define __Ogre_Page_H__

#include "OgrePagingPrerequisites.h"
#include "OgreAtomicWrappers.h"


namespace Ogre
{
	/** \addtogroup Optional Components
	*  @{
	*/
	/** \addtogroup Paging
	*  Some details on paging component
	*  @{
	*/

	/** Page class
	*/
	class Page : public PageAlloc
	{
	public:
		/** Status of the Page. 
		*/
		enum Status
		{
			/// Just defined, not loaded
			STATUS_UNLOADED,
			/// In the process of getting / generating data for the page
			STATUS_PREPARING, 
			/** At this stage all data has been read, and all non-GPU tasks have been done. 
				This is the end of the background thread's involvement.
			*/
			STATUS_PREPARED, 
			/// Finalising the load in the main render thread
			STATUS_LOADING,
			/// Data loaded / generated 
			STATUS_LOADED, 
			/// Unloading in main render thread (goes back to STATUS_PREPARED)
			STATUS_UNLOADING,
			/// Unpreparing, potentially in a background thread (goes back to STATUS_UNLOADED)
			STATUS_UNPREPARING
		};
	protected:
		PageID mID;
		PagedWorldSection* mParent;
		unsigned long mFrameLastHeld;
		AtomicScalar<Status> mStatus;

		SceneNode* mDebugNode;
		void updateDebugDisplay();
	public:
		Page(PageID pageID);
		virtual ~Page();

		/// Get the ID of this page, unique withing the parent
		virtual PageID getID() const { return mID; }
		/// Get the PagedWorldSection this page belongs to, or zero if unattached
		virtual PagedWorldSection* getParentSection() const { return mParent; }
		/** Get the frame number in which this Page was last loaded or held.
		@remarks
			A Page that has not been requested to be loaded or held in the recent
			past will be a candidate for removal.
		*/
		virtual unsigned long getFrameLastHeld() { return mFrameLastHeld; }
		/// 'Touch' the page to let it know it's being used
		virtual void touch();
		/// Get whether or not this page is currently attached 
		virtual bool isAttached() const { return mParent != 0; }


		/** Read page data from a serialiser (returns true if successful) & prepare.
		@remarks
			'prepare' means to pull data in from a file, and to do as much processing
			on it as required to be ready to create GPU resources. Since this method can 
			be called from a non-render thread, this implementation must not access
			any GPU resources.
		*/
		bool prepare(StreamSerialiser& stream);
		/** Finalise the loading of the data.
		@remarks
			This implementation will finalise any work done in prepare() and create
			or access any GPU resources. This method will be called from the main
			render thread.
		*/
		void load();

		/** Deallocate any background resources.
		@remarks
			This method may be called in a background, non-render thread after 
			unload() therefore should only deallocate non-GPU resources. 
			GPU resources should be freed in unload(). 

		*/
		void unprepare();

		/** Unload the Page, deallocating any GPU resources. 
		@remarks
			This method will be called in the main render thread just before the unprepare()
			call (which may be done in the background). Any GPU-dependent 
			instances must be cleaned up in this call, anything else can be done
			in the unprepare() call.
		*/
		void unload();

		/** Returns whether this page was 'held' in the last frame, that is
			was it either directly needed, or requested to stay in memory (held - as
			in a buffer region for example). If not, this page is eligible for 
			removal.
		*/
		bool isHeld() const;

		/// Mark a page as preparing (internal use)
		void _markPreparing();
		/// Mark a page as prepared (internal use)
		void _markPrepared();
		/// Mark a page as loading (internal use)
		void _markLoading();
		/// Mark a page as loaded (internal use)
		void _markLoaded();
		/// Mark a page as unloading (internal use)
		void _markUnloading();
		/// Mark a page as unpreparing (internal use)
		void _markUnpreparing();
		/// Mark a page as unloaded (internal use)
		void _markUnloaded();

		/// Save page data to a serialiser 
		void save(StreamSerialiser& stream);

		/// Internal method to notify a page that it is attached
		virtual void _notifyAttached(PagedWorldSection* parent);

		/** Returns true if the Page has been fully loaded, false otherwise.
		*/
		virtual bool isLoaded() const { return (mStatus.get() == STATUS_LOADED); }

		/** Returns whether the resource is currently in the process of
			loading.
		*/
		virtual bool isLoading() const
		{
			Status s = mStatus.get();
			return (s == STATUS_LOADING || s == STATUS_PREPARING);
		}

		/// Returns the current status
		virtual Status getStatus() const { return mStatus.get(); }

		/// Called when the frame starts
		virtual void frameStart(Real timeSinceLastFrame);
		/// Called when the frame ends
		virtual void frameEnd(Real timeElapsed);
		/// Notify a section of the current camera
		virtual void notifyCamera(Camera* cam);



	};

	/** @} */
	/** @} */
}



#endif 