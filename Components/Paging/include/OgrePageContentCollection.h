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

#ifndef __Ogre_PageContentCollection_H__
#define __Ogre_PageContentCollection_H__

#include "OgrePagingPrerequisites.h"
#include "OgrePageLoadableUnit.h"


namespace Ogre
{
	/** \addtogroup Optional Components
	*  @{
	*/
	/** \addtogroup Paging
	*  Some details on paging component
	*  @{
	*/


	/** Definition of the interface for a collection of PageContent instances. 
	@remarks
		This class acts as a grouping level for PageContent instances. Rather than 
		PageContent instances being held in a list directly under Page, which might 
		be the most obvious solution, this intermediate class is here to allow
		the collection of relevant PageContent instances to be modified at runtime
		if required. For example, potentially you might want to define Page-level LOD
		in which different collections of PageContent are loaded at different times.
	*/
	class _OgrePagingExport PageContentCollection : public PageLoadableUnit
	{
	protected:
		PageContentCollectionFactory* mCreator;
		Page* mParent;
	public:
		static const uint32 CHUNK_ID;
		static const uint16 CHUNK_VERSION;

		PageContentCollection(PageContentCollectionFactory* creator);
		virtual ~PageContentCollection();

		PageManager* getManager() const;
		Page* getParentPage() const { return mParent; }
		SceneManager* getSceneManager() const;

		/// Get the type of the collection, which will match it's factory
		virtual const String& getType() const;

		/// Internal method to notify a page that it is attached
		virtual void _notifyAttached(Page* parent);
		/// Save the collection to a stream
		virtual void save(StreamSerialiser& stream) = 0;
		/// Called when the frame starts
		virtual void frameStart(Real timeSinceLastFrame) = 0;
		/// Called when the frame ends
		virtual void frameEnd(Real timeElapsed) = 0;
		/// Notify a section of the current camera
		virtual void notifyCamera(Camera* cam) = 0;


	};


	/** @} */
	/** @} */
}




#endif 