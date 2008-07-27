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
#include "OgreMemoryAllocatorConfig.h"

namespace Ogre
{
	/* 

	Ugh, I wish I didn't have to do this.

	The problem is that operator new/delete are *implicitly* static. We have to 
	instantiate them for each combination exactly once throughout all the compilation
	units that are linked together, and this appears to be the only way to do it. 

	At least I can do it via templates.

	*/

	/// operator new, with debug line info
	template <class Alloc>
	void* AllocatedObject<Alloc >::operator new(size_t sz, const char* file, int line, const char* func)
	{
		return Alloc::allocateBytes(sz, file, line, func);
	}

	template <class Alloc>
	void* AllocatedObject<Alloc >::operator new(size_t sz)
	{
		return Alloc::allocateBytes(sz);
	}

	/// placement operator new
	template <class Alloc>
	void* AllocatedObject<Alloc >::operator new(size_t sz, void* ptr)
	{
		return ptr;
	}

	/// array operator new, with debug line info
	template <class Alloc>
	void* AllocatedObject<Alloc >::operator new[] ( size_t sz, const char* file, int line, const char* func )
	{
		return Alloc::allocateBytes(sz, file, line, func);
	}

	template <class Alloc>
	void* AllocatedObject<Alloc >::operator new[] ( size_t sz )
	{
		return Alloc::allocateBytes(sz);
	}

	template <class Alloc>
	void AllocatedObject<Alloc >::operator delete( void* ptr )
	{
		Alloc::deallocateBytes(ptr);
	}

	// only called if there is an exception in corresponding 'new'
	template <class Alloc>
	void AllocatedObject<Alloc >::operator delete( void* ptr, const char* , int , const char*  )
	{
		Alloc::deallocateBytes(ptr);
	}

	template <class Alloc>
	void AllocatedObject<Alloc >::operator delete[] ( void* ptr )
	{
		Alloc::deallocateBytes(ptr);
	}

	template <class Alloc>
	void AllocatedObject<Alloc >::operator delete[] ( void* ptr, const char* , int , const char*  )
	{
		Alloc::deallocateBytes(ptr);
	}

	/*
	Now, we have to ensure that the new/delete methods are actually instantiated, 
	which means we have to call them. Do this via some static code which will
	never actually be called but it forces the code generation.
	*/
	void AllocatorInst::init()
	{
		// single-instance
		GeneralAllocatedObject* a1 = OGRE_NEW GeneralAllocatedObject();
		OGRE_DELETE a1;
		GeometryAllocatedObject* a2 = OGRE_NEW GeometryAllocatedObject();
		OGRE_DELETE a2;
		AnimationAllocatedObject* a3 = OGRE_NEW AnimationAllocatedObject();
		OGRE_DELETE a3;
		SceneCtlAllocatedObject* a4 = OGRE_NEW SceneCtlAllocatedObject();
		OGRE_DELETE a4;
		SceneObjAllocatedObject* a5 = OGRE_NEW SceneObjAllocatedObject();
		OGRE_DELETE a5;
		ResourceAllocatedObject* a6 = OGRE_NEW ResourceAllocatedObject();
		OGRE_DELETE a6;
		ScriptingAllocatedObject* a7 = OGRE_NEW ScriptingAllocatedObject();
		OGRE_DELETE a7;
		RenderSysAllocatedObject* a8 = OGRE_NEW RenderSysAllocatedObject();
		OGRE_DELETE a8;

		// array
		a1 = OGRE_NEW GeneralAllocatedObject[1];
		OGRE_DELETE [] a1;
		a2 = OGRE_NEW GeometryAllocatedObject[1];
		OGRE_DELETE [] a2;
		a3 = OGRE_NEW AnimationAllocatedObject[1];
		OGRE_DELETE [] a3;
		a4 = OGRE_NEW SceneCtlAllocatedObject[1];
		OGRE_DELETE [] a4;
		a5 = OGRE_NEW SceneObjAllocatedObject[1];
		OGRE_DELETE [] a5;
		a6 = OGRE_NEW ResourceAllocatedObject[1];
		OGRE_DELETE [] a6;
		a7 = OGRE_NEW ScriptingAllocatedObject[1];
		OGRE_DELETE [] a7;
		a8 = OGRE_NEW RenderSysAllocatedObject[1];
		OGRE_DELETE [] a8;

		// just incase anyone uses new instead of OGRE_NEW, don't fail, be nice
		a1 = new GeneralAllocatedObject();
		delete a1;
		a2 = new GeometryAllocatedObject();
		delete a2;
		a3 = new AnimationAllocatedObject();
		delete a3;
		a4 = new SceneCtlAllocatedObject();
		delete a4;
		a5 = new SceneObjAllocatedObject();
		delete a5;
		a6 = new ResourceAllocatedObject();
		delete a6;
		a7 = new ScriptingAllocatedObject();
		delete a7;
		a8 = new RenderSysAllocatedObject();
		delete a8;
	}


}
