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

#ifndef __Ogre_PageStrategy_H__
#define __Ogre_PageStrategy_H__

#include "OgrePagingPrerequisites.h"


namespace Ogre
{
	/** \addtogroup Optional Components
	*  @{
	*/
	/** \addtogroup Paging
	*  Some details on paging component
	*/
	/*@{*/


	/** Abstract marker class representing the data held against the PagedWorldSection
	which is specifically used by the PageStrategy.
	*/
	class _OgrePagingExport PageStrategyData : public PageAlloc
	{
	public:
		PageStrategyData() {}
		virtual ~PageStrategyData() {}

		/// Load this data from a stream
		virtual void load(StreamSerialiser& stream) = 0;
		/// Save this data to a stream
		virtual void save(StreamSerialiser& stream) = 0;


	};


	/** Defines the interface to a strategy class which is responsible for deciding
		when Page instances are requested for addition and removal from the 
		paging system.
	@remarks
		The interface is deliberately light, with no specific mention of requesting
		new Page instances. It is entirely up to the PageStrategy to respond
		to the events raised on it and to call methods on other classes (such as
		requesting new pages). 
	*/
	class _OgrePagingExport PageStrategy : public PageAlloc
	{
	protected:
		String mName;
		PageManager* mManager;
	public:
		PageStrategy(const String& name, PageManager* manager)
			: mName(name), mManager(manager)
		{

		}

		virtual ~PageStrategy() {}

		const String& getName() const { return mName; }
		PageManager* getManager() const { return mManager; }

		/// Called when the frame starts
		virtual void frameStart(Real timeSinceLastFrame) {}
		/// Called when the frame ends
		virtual void frameEnd(Real timeElapsed) {}
		/** Called when a camera is used for any kind of rendering.
		@remarks
			This is probably the primary way in which the strategy will request
			new pages. 
		@param cam Camera which is being used for rendering. Class should not
			rely on this pointer remaining valid permanently because no notification 
			will be given when the camera is destroyed. 
		*/
		virtual void notifyCamera(Camera* cam) {}

		/** Create a PageStrategyData instance containing the data specific to this
			PageStrategy. 
		@par
			This data will be held by a given PagedWorldSection and the structure of
			the data will be specific to the PageStrategy subclass.
		*/
		virtual PageStrategyData* createData() = 0;

		/** Destroy a PageStrategyData instance containing the data specific to this
		PageStrategy. 
		@par
		This data will be held by a given PagedWorldSection and the structure of
		the data will be specific to the PageStrategy subclass.
		*/
		virtual void destroyData(PageStrategyData* d) = 0;
	};

	/*@}*/
	/*@}*/
}




#endif 