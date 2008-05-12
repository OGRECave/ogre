/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
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
#ifndef __AlignedAllocator_H__
#define __AlignedAllocator_H__

#include "OgrePrerequisites.h"

namespace Ogre {

    /** Class to provide aligned memory allocate functionality.
    @remarks
        All SIMD processing are friendly with aligned memory, and some SIMD routines
        are designed for working with aligned memory only. If the data are intended to
        use SIMD processing, it's need to be aligned for better performance boost.
        In additional, most time cache boundary aligned data also lead to better
        performance even if didn't used SIMD processing. So this class provides a couple
        of functions for allocate aligned memory.
    @par
        Anyways, in general, you don't need to use this class directly, Ogre internally
        will take care with most SIMD and cache friendly optimisation if possible.
    @par
        This isn't a "one-step" optimisation, there are a lot of underlying work to
        achieve performance boost. If you didn't know what are you doing or what there
        are going, just ignore this class.
    @note
        This class intended to use by advanced user only.
    */
	class _OgreExport AlignedMemory
	{
	public:
        /** Allocate memory with given alignment.
            @param
                size The size of memory need to allocate.
            @param
                alignment The alignment of result pointer, must be power of two
                and in range [1, 128].
            @returns
                The allocated memory pointer.
            @par
                On failure, exception will be throw.
        */
        static void* allocate(size_t size, size_t alignment);

        /** Allocate memory with default platform dependent alignment.
            @remarks
                The default alignment depend on target machine, this function
                guarantee aligned memory according with SIMD processing and
                cache boundary friendly.
            @param
                size The size of memory need to allocate.
            @returns
                The allocated memory pointer.
            @par
                On failure, exception will be throw.
        */
        static void* allocate(size_t size);

        /** Deallocate memory that allocated by this class.
            @param
                p Pointer to the memory allocated by this class or <b>NULL</b> pointer.
            @par
                On <b>NULL</b> pointer, nothing happen.
        */
        static void deallocate(void* p);
	};

    /** STL compatible allocator with aligned memory allocate, used for tweak
        STL containers work with aligned memory.
    @remarks
        This class designed for work with STL containers, by use this
        class instead of std::allocator, the STL containers can work
        with aligned memory allocate seamless.
    @note
        template parameter Alignment equal to zero means use default
        platform dependent alignment.
    */
    template <typename T, unsigned Alignment = 0>
    class AlignedAllocator
    {
        // compile-time check alignment is available.
        typedef int IsValidAlignment
            [Alignment <= 128 && ((Alignment & (Alignment-1)) == 0) ? +1 : -1];

    public:
        //--- typedefs for STL compatible

        typedef T value_type;

        typedef value_type * pointer;
        typedef const value_type * const_pointer;
        typedef value_type & reference;
        typedef const value_type & const_reference;
        typedef std::size_t size_type;
        typedef std::ptrdiff_t difference_type;

        template <typename U>
        struct rebind
        {
            typedef AlignedAllocator<U, Alignment> other;
        };

    public:
        AlignedAllocator() { /* nothing to do */ }

        // default copy constructor

        // default assignment operator

        // not explicit, mimicking std::allocator [20.4.1]
        template <typename U, unsigned A>
        AlignedAllocator(const AlignedAllocator<U, A> &) { /* nothing to do */ }

        // default destructor

        //--- functions for STL compatible

        static pointer address(reference r)
        { return &r; }
        static const_pointer address(const_reference s)
        { return &s; }
        static size_type max_size()
        { return (std::numeric_limits<size_type>::max)(); }
        static void construct(const pointer ptr, const value_type & t)
        { new (ptr) T(t); }
        static void destroy(const pointer ptr)
        {
            ptr->~T();
            (void) ptr; // avoid unused variable warning
        }

        bool operator==(const AlignedAllocator &) const
        { return true; }
        bool operator!=(const AlignedAllocator &) const
        { return false; }

        static pointer allocate(const size_type n)
        {
            // use default platform dependent alignment if 'Alignment' equal to zero.
            const pointer ret = static_cast<pointer>(Alignment ?
                AlignedMemory::allocate(sizeof(T) * n, Alignment) :
                AlignedMemory::allocate(sizeof(T) * n));
            return ret;
        }
        static pointer allocate(const size_type n, const void * const)
        {
            return allocate(n);
        }
        static void deallocate(const pointer ptr, const size_type)
        {
            AlignedMemory::deallocate(ptr);
        }
    };
}

#endif  // __AlignedAllocator_H__
