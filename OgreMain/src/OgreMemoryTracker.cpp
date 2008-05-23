/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2008 Torus Knot Software Ltd
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
Torus Knot Software Ltd
---------------------------------------------------------------------------
*/
#include "OgreStableHeaders.h"
#include "OgreMemoryTracker.h"

namespace Ogre
{
	
#if OGRE_DEBUG_MODE
	//--------------------------------------------------------------------------
	MemoryTracker& MemoryTracker::get()
	{
		static MemoryTracker tracker;
		return tracker;
	}
	//--------------------------------------------------------------------------
	void MemoryTracker::_recordAlloc(void* ptr, size_t sz, MemoryCategory cat, 
					  const String file, size_t ln, const String& func)
	{
		OGRE_LOCK_AUTO_MUTEX

		assert(mAllocations.find(ptr) == mAllocations.end() && "Double allocation with same address");
		mAllocations[ptr] = Alloc(sz, cat, file, ln, func);
		mAllocationsByCategory[cat] += sz;
		mTotalAllocations += sz;
	}
	//--------------------------------------------------------------------------
	void MemoryTracker::_recordDealloc(void* ptr)
	{
		OGRE_LOCK_AUTO_MUTEX

		AllocationMap::iterator i = mAllocations.find(ptr);
		assert(i != mAllocations.end() && "Unable to locate allocation unit");
		// update category stats
		mAllocationsByCategory[i->second.cat] -= i->second.bytes;
		// global stats
		mTotalAllocations -= i->second.bytes;
		mAllocations.erase(i);
	}	
	//--------------------------------------------------------------------------
	size_t MemoryTracker::getTotalMemoryAllocated() const
	{
		return mTotalAllocations;
	}
	//--------------------------------------------------------------------------
	size_t MemoryTracker::getMemoryAllocatedForCat(MemoryCategory cat) const
	{
		return mAllocationsByCategory[cat];
	}
	//--------------------------------------------------------------------------
	void MemoryTracker::reportLeaks()
	{
		StringUtil::StrStreamType os;
		
		if (mAllocations.empty())
			os << "No leaks!";
		else
		{
			size_t totalMem = 0;
			os << "Leaks detected:" << std::endl << std::endl;
			for (AllocationMap::const_iterator i = mAllocations.begin(); i != mAllocations.end(); ++i)
			{
				const Alloc& alloc = i->second;
				os << alloc.filename << "(" << alloc.line << ", " << alloc.function << "): "
					<< alloc.bytes << " bytes" << std::endl;
				totalMem += alloc.bytes;
			}
			
			os << std::endl;
			os << mAllocations.size() << " leaks detected, " << totalMem << " bytes total";
		}
		
		if (mDumpToStdOut)
			std::cout << os.str();
		
		std::ofstream of;
		of.open(mLeakFileName.c_str());
		of << os.str();
		of.close();
	}
#endif // OGRE_DEBUG_MODE	
	
}



