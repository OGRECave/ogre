/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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
	//---------------------------------------------------------------------
	void PageLoadableUnit::setLoaded()
	{
		_changeStatus(mStatus.get(), STATUS_LOADED);
	}
}

