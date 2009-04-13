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
#include "OgrePageLoadableUnit.h"
#include "OgreException.h"

namespace Ogre
{
	//---------------------------------------------------------------------
	PageLoadableUnit::PageLoadableUnit()
		: mStatus(STATUS_UNLOADED)
	{

	}
	//---------------------------------------------------------------------
	PageLoadableUnit::~PageLoadableUnit()
	{
		// call destroy() in subclasses to ensure fully complete
	}
	//---------------------------------------------------------------------
	void PageLoadableUnit::destroy()
	{
		// unload if needed (main thread)
		if (mStatus.get() == STATUS_LOADED)
			unload();
		if (mStatus.get() == STATUS_PREPARED)
			unprepare();

		assert(mStatus.get() == STATUS_UNLOADED);
	}
	//---------------------------------------------------------------------
	bool PageLoadableUnit::prepare(StreamSerialiser& stream)
	{
		if (!_changeStatus(STATUS_UNLOADED, STATUS_PREPARING))
			return false;

		bool ret = prepareImpl(stream);

		if (ret)
		{
			mStatus.set(STATUS_PREPARED);
		}
		else
		{
			mStatus.set(STATUS_UNLOADED);
		}
		return ret;
	}
	//---------------------------------------------------------------------
	void PageLoadableUnit::load()
	{
		if (mStatus.get() == STATUS_UNLOADED)
		{
			OGRE_EXCEPT(Exception::ERR_INVALID_STATE, 
				"Cannot load before prepare() is performed", 
				"PageLoadableUnit::load");
		}

		if (!_changeStatus(STATUS_PREPARED, STATUS_LOADING))
			return;

		loadImpl();

		mStatus.set(STATUS_LOADED);
	}
	//---------------------------------------------------------------------
	void PageLoadableUnit::unload()
	{
		if (!_changeStatus(STATUS_LOADED, STATUS_UNLOADING))
			return;

		unloadImpl();

		mStatus.set(STATUS_PREPARED);
	}
	//---------------------------------------------------------------------
	void PageLoadableUnit::unprepare()
	{
		if (!_changeStatus(STATUS_PREPARED, STATUS_UNPREPARING))
			return;

		unprepareImpl();

		mStatus.set(STATUS_UNLOADED);
	}
	//---------------------------------------------------------------------
	bool PageLoadableUnit::_changeStatus(UnitStatus oldStatus, UnitStatus newStatus)
	{
		// Fast pre-check (no lock)
		if (mStatus.get() != oldStatus) 
			return false;

		// Set, with check & lock
		return mStatus.cas(oldStatus, newStatus);

	}
}

