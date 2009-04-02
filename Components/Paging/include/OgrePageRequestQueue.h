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

#ifndef __Ogre_PageRequestQueue_H__
#define __Ogre_PageRequestQueue_H__

#include "OgrePagingPrerequisites.h"
#include "OgreString.h"
#include "OgreCommon.h"

namespace Ogre
{
	/** \addtogroup Optional Components
	*  @{
	*/
	/** \addtogroup Paging
	*  Some details on paging component
	*  @{
	*/

	/** The PageRequestQueue is where pages are queued for loading and freeing.
	*/
	class _OgrePagingExport PageRequestQueue : public PageAlloc
	{
	public:
		PageRequestQueue(PageManager* manager);
		virtual ~PageRequestQueue();

		/** Load a Page with a given ID, for a given PagedWorldSection.
		@remarks
			This method is called from the main rendering thread in all cases. 
			If threading is enabled, the request is queued and processed by 
			a separate thread. At some point later on the loaded page will be attached
			to the section which requested it. 
		*/
		void loadPage(PageID pageID, PagedWorldSection* section);
		/** Dispose of a page */
		void unloadPage(Page* page);



	protected:
		PageManager* mManager;
	};

	/** @} */
	/** @} */
}





#endif 