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
#include "OgreStableHeaders.h"

#include "OgrePrerequisites.h"
#include "OgreAlignedAllocator.h"

#include "OgrePlatformInformation.h"
#include "OgreBitwise.h"

/**
*
* |___2___|3|_________5__________|__6__|
* ^         ^
* 1         4
*
* 1 -> Pointer to start of the block allocated by new.
* 2 -> Gap used to get 4 aligned on given alignment
* 3 -> Byte offset between 1 and 4
* 4 -> Pointer to the start of data block.
* 5 -> Data block.
* 6 -> Wasted memory at rear of data block.
*/

namespace Ogre {

    //---------------------------------------------------------------------
    void* AlignedMemory::allocate(size_t size, size_t alignment)
    {
        assert(0 < alignment && alignment <= 128 && Bitwise::isPO2(alignment));

        unsigned char* p = new unsigned char[size + alignment];
        size_t offset = alignment - (size_t(p) & (alignment-1));

        unsigned char* result = p + offset;
        result[-1] = (unsigned char)offset;

        return result;
    }
    //---------------------------------------------------------------------
    void* AlignedMemory::allocate(size_t size)
    {
        return allocate(size, OGRE_SIMD_ALIGNMENT);
    }
    //---------------------------------------------------------------------
    void AlignedMemory::deallocate(void* p)
    {
        if (p)
        {
            unsigned char* mem = (unsigned char*)p;
            mem = mem - mem[-1];
            delete [] mem;
        }
    }

}
