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

#ifndef __Ogre_PagedWorldSection_H__
#define __Ogre_PagedWorldSection_H__

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

	/** Represents a section of the PagedWorld which uses a given PageStrategy, and
		which is made up of a generally localised set of Page instances.
	@remarks
		The reason for PagedWorldSection is that you may wish to cater for multiple
		sections of your world which use a different approach to paging (ie a
		different PageStrategy), or which are significantly far apart or separate 
		that the parameters you want to pass to the PageStrategy are different.
	@par
		PagedWorldSection instances are fully contained within the PagedWorld and
		their definitions are loaded in their entirety when the PagedWorld is
		loaded. However, no Page instances are initially loaded - those are the
		responsibility of the PageStrategy.
	*/
	class PagedWorldSection : public PageAlloc
	{
	protected:
		String mName;
		PagedWorld* mParent;
		PageStrategy* mStrategy;
	public:
		/** Construct a new instance, specifying the parent and assigned strategy. */
		PagedWorldSection(const String& name, PagedWorld* parent, PageStrategy* strategy);
		virtual ~PagedWorldSection();

		/// Get the name of this section
		const String& getName() const { return mName; }
		/// Get the page strategy which this section is using
		PageStrategy* getStrategy() const { return mStrategy; }
		/// Get the parent world
		PagedWorld* getWorld() const { return mParent; }

	};

	/** @} */
	/** @} */
}




#endif 