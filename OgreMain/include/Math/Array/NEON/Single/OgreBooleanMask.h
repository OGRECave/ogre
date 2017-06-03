/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

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

#ifndef __NEON_BooleanMask_H__
#define __NEON_BooleanMask_H__

#ifndef __BooleanMask_H__
    #error "Don't include this file directly. include Math/Array/OgreBooleanMask.h"
#endif

namespace Ogre
{
    class _OgreExport BooleanMask4
    {
    public:
        enum
        {
            MASK_NONE           = 0,
            MASK_X              = 1,

            MASK_Y              = 2,
            MASK_XY             = 3, //MASK_X|MASK_Y

            MASK_Z              = 4,
            MASK_XZ             = 5, //MASK_X|MASK_Z
            MASK_YZ             = 6, //MASK_Y|MASK_Z
            MASK_XYZ            = 7, //MASK_X|MASK_Y|MASK_Z

            MASK_W              = 8,
            MASK_XW             = 9, //MASK_X|MASK_W
            MASK_YW             =10, //MASK_Y|MASK_W
            MASK_XYW            =11, //MASK_X|MASK_Y|MASK_W
            MASK_ZW             =12, //MASK_Z|MASK_W
            MASK_XZW            =13, //MASK_X|MASK_Z|MASK_W
            MASK_YZW            =14, //MASK_Y|MASK_Z|MASK_W
            MASK_XYZW           =15, //MASK_X|MASK_Y|MASK_Z|MASK_W

            NUM_MASKS           =16
        };
    private:
        static const ArrayMaskR mMasks[NUM_MASKS];
    public:
        inline static ArrayMaskR getMask( bool x, bool y, bool z, bool w );
        inline static ArrayMaskR getMask( bool booleans[4] );

        inline static ArrayMaskR getAllSetMask(void);

        /// Returns true if alls bit in mask0[i] and mask1[i] are set.
        inline static bool allBitsSet( bool mask0[4], bool mask1[4] );

        /** Converts a SIMD mask into a mask that fits in 32-bit number
        @remarks
            @See IS_SET_MASK_X & co. to read the mask, since the result may need
            byteswapping in some architectures (i.e. SSE2)
        */
        inline static uint32 getScalarMask( ArrayMaskR mask );

        inline static uint32 getScalarMask( ArrayInt mask );
    };
}

#include "OgreBooleanMask.inl"

#endif
