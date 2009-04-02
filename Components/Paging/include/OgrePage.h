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
	protected:
		PageID mID;
		PagedWorldSection* mParent;
		unsigned long mFrameLastHeld;
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


		/// Internal method to notify a page that it is attached
		virtual void _notifyAttached(PagedWorldSection* parent);

	};

	/** @} */
	/** @} */
}



#endif 