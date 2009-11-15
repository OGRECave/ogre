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
Torus Knot Software Ltd
---------------------------------------------------------------------------
*/

#ifndef __AllocatedObject_H__
#define __AllocatedObject_H__

#include "OgrePrerequisites.h"

// Anything that has done a #define new <blah> will screw operator new definitions up
// so undefine
#ifdef new
#  undef new
#endif
#ifdef delete
#  undef delete
#endif

namespace Ogre
{
	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Memory
	*  @{
	*/
	/** Superclass for all objects that wish to use custom memory allocators
		when their new / delete operators are called.
		Requires a template parameter identifying the memory allocator policy 
		to use (e.g. see StdAllocPolicy). 
	*/
	template <class Alloc>
	class _OgreExport AllocatedObject
	{
	public:
		explicit AllocatedObject()
		{ }

		~AllocatedObject()
		{ }

		/// operator new, with debug line info
		void* operator new(size_t sz, const char* file, int line, const char* func)
		{
			return Alloc::allocateBytes(sz, file, line, func);
		}

		void* operator new(size_t sz)
		{
			return Alloc::allocateBytes(sz);
		}

		/// placement operator new
		void* operator new(size_t sz, void* ptr)
		{
			(void) sz;
			return ptr;
		}

		/// array operator new, with debug line info
		void* operator new[] ( size_t sz, const char* file, int line, const char* func )
		{
			return Alloc::allocateBytes(sz, file, line, func);
		}

		void* operator new[] ( size_t sz )
		{
			return Alloc::allocateBytes(sz);
		}

		void operator delete( void* ptr )
		{
			Alloc::deallocateBytes(ptr);
		}

		// Corresponding operator for placement delete (second param same as the first)
		void operator delete( void* ptr, void* )
		{
			Alloc::deallocateBytes(ptr);
		}

		// only called if there is an exception in corresponding 'new'
		void operator delete( void* ptr, const char* , int , const char*  )
		{
			Alloc::deallocateBytes(ptr);
		}

		void operator delete[] ( void* ptr )
		{
			Alloc::deallocateBytes(ptr);
		}


		void operator delete[] ( void* ptr, const char* , int , const char*  )
		{
			Alloc::deallocateBytes(ptr);
		}
	};


	/** @} */
	/** @} */

}
#endif
