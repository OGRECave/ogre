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
			STATUS_LOADED
		};
	protected:
		PageID mID;
		PagedWorldSection* mParent;
		unsigned long mFrameLastHeld;
		AtomicScalar<Status> mStatus;
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


		/// Load page data from a serialiser (returns true if successful)
		bool load(StreamSerialiser& stream);
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


	};

	/** @} */
	/** @} */
}



#endif 